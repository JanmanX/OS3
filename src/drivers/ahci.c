#include "ahci.h"
#include <stdint.h>
#include <types.h>
#include <libc.h>
#include <drivers/pci.h>
#include <kernel/time.h>
#include <cpu/hpet.h>
#include <errno.h>

/* How long a port gets to stop, start or finish a command */
#define AHCI_TIMEOUT_MS		(5000)

/* Without an HPET there is no clock to time out by, so a poll gives up after
 * this many reads of the register instead */
#define AHCI_TIMEOUT_SPINS	(50000000ULL)

/* Every transfer goes through one bounce buffer, so this is the most a single
 * command moves: 64KiB, well inside what one PRD entry can describe */
#define AHCI_MAX_SECTORS	(128)

/*
 * What the HBA reads and writes by DMA for one port. The command list has to
 * be 1KiB aligned, the received-FIS area 256 bytes and a command table 128
 * bytes; laid out in this order from a 1KiB boundary, all three are. Memory
 * is identity mapped, so these addresses are the physical ones the HBA needs.
 */
typedef struct ahci_port_dma {
	ahci_cmd_hdr_t headers[32];
	uint8_t received_fis[256];
	ahci_cmd_tbl_t table;
} __attribute__((packed, aligned(1024))) ahci_port_dma_t;

typedef struct ahci_disk {
	ahci_register_set_t *hba;
	uint32_t port;
	uint64_t sectors;
	ahci_port_dma_t *dma;
} ahci_disk_t;

typedef struct ahci_timer {
	uint64_t start;
	uint64_t spins;
} ahci_timer_t;

static ahci_port_dma_t port_dma[AHCI_MAX_DISKS];

/* Callers hand us buffers from anywhere, including the heap, which does not
 * even guarantee the word alignment a PRD entry needs */
static uint8_t bounce[AHCI_MAX_SECTORS * AHCI_SECTOR_SIZE]
	__attribute__((aligned(4096)));

static ahci_disk_t disks[AHCI_MAX_DISKS];
static uint32_t disk_count = 0;


static void ahci_timer_start(ahci_timer_t *timer)
{
	timer->start = hpet_get_ms_elapsed();
	timer->spins = 0;
}

static uint8_t ahci_timer_expired(ahci_timer_t *timer)
{
	if(hpet_is_initialized()) {
		return (hpet_get_ms_elapsed() - timer->start) > AHCI_TIMEOUT_MS;
	}

	return ++timer->spins > AHCI_TIMEOUT_SPINS;
}

/* Wait for all of "bits" to clear in the port's PxCMD, or in its PxTFD when
 * "tfd" is set */
static uint8_t ahci_wait_clear(ahci_port_register_set_t *port, uint8_t tfd,
			       uint32_t bits)
{
	ahci_timer_t timer;

	ahci_timer_start(&timer);

	while((tfd ? port->tfd : port->cmd) & bits) {
		if(ahci_timer_expired(&timer)) {
			return EBUSY;
		}
	}

	return EOK;
}


bool ahci_gain_ownership(ahci_register_set_t* controller)
{
	/* Only 1.2.0 and above supported */
	if(controller->generic.vs >= 0x00010200) {
		/* Only supported if CAP2.BOH is supported */
		if(controller->generic.cap2 & (1 << 0)) {
			/* Set the OOS bit */
			controller->generic.bohc |= (1 << 1);

			sleep(25);

			/* If after 25 msec the BB is set, we need to wait 2
			 * seconds. */
			if((controller->generic.bohc &
			    ((1 << 4)	 // Bios Busy
			     | (1 << 1)  // OOS
			     | (1 << 0))) // BOS
			   != (1 << 1)) { // OOS
				// Failed to gain ownership
				return FALSE;
			}
		}
	}
	return TRUE;
}


static uint8_t ahci_port_stop(ahci_port_register_set_t *port)
{
	port->cmd &= ~AHCI_PxCMD_ST;

	if(ahci_wait_clear(port, 0, AHCI_PxCMD_CR) != EOK) {
		return EBUSY;
	}

	port->cmd &= ~AHCI_PxCMD_FRE;

	return ahci_wait_clear(port, 0, AHCI_PxCMD_FR);
}

static uint8_t ahci_port_start(ahci_port_register_set_t *port)
{
	if(ahci_wait_clear(port, 0, AHCI_PxCMD_CR) != EOK) {
		return EBUSY;
	}

	port->cmd |= AHCI_PxCMD_FRE;
	port->cmd |= AHCI_PxCMD_ST;

	return EOK;
}

/* Stopping the port drops whatever command was stuck on it, so the next one
 * starts from a clean slate */
static void ahci_port_recover(ahci_port_register_set_t *port)
{
	ahci_port_stop(port);

	port->serr = 0xFFFFFFFF;
	port->is = 0xFFFFFFFF;

	ahci_port_start(port);
}


/* Run one command in slot 0 and wait for it to finish. Whatever the drive
 * sends back lands in the bounce buffer. */
static uint8_t ahci_command(ahci_disk_t *disk, uint8_t command,
			    uint64_t lba, uint16_t count, uint32_t bytes)
{
	ahci_port_register_set_t *port = &disk->hba->port_register[disk->port];
	ahci_cmd_hdr_t *header = &disk->dma->headers[0];
	ahci_cmd_tbl_t *table = &disk->dma->table;
	ahci_fis_register_h2d_t *fis = (ahci_fis_register_h2d_t*)table->fis;
	ahci_timer_t timer;

	/* The drive has to have settled from whatever it did last */
	if(ahci_wait_clear(port, 1, AHCI_PxTFD_BSY | AHCI_PxTFD_DRQ) != EOK) {
		LOGF("AHCI: port %d is stuck busy (tfd 0x%x)\n",
		     (uint64_t)disk->port, (uint64_t)port->tfd);
		ahci_port_recover(port);
		return EBUSY;
	}

	memset((void*)table, 0x00, sizeof(ahci_cmd_tbl_t));

	fis->type = FIS_TYPE_REG_H2D;
	fis->flags = 0x80;		/* A command, not a control update */
	fis->command = command;
	fis->device = (1 << 6);		/* LBA addressing */

	fis->lba_0 = (uint8_t)(lba);
	fis->lba_1 = (uint8_t)(lba >> 8);
	fis->lba_2 = (uint8_t)(lba >> 16);
	fis->lba_3 = (uint8_t)(lba >> 24);
	fis->lba_4 = (uint8_t)(lba >> 32);
	fis->lba_5 = (uint8_t)(lba >> 40);

	fis->sector_low = (uint8_t)(count);
	fis->secotr_high = (uint8_t)(count >> 8);

	table->prdt_entry[0].dbau = (uint64_t)bounce;
	table->prdt_entry[0].flags = bytes - 1;	/* Byte count, less one */

	memset((void*)header, 0x00, sizeof(ahci_cmd_hdr_t));
	header->cfl = sizeof(ahci_fis_register_h2d_t) / sizeof(uint32_t);
	header->prdtl = 1;
	header->ctbau = (uint64_t)table;

	/* All of the above has to be in memory before the HBA goes to fetch
	 * it, and nothing may be read out of the bounce buffer before the
	 * command is seen to finish */
	asm volatile("" ::: "memory");

	port->is = 0xFFFFFFFF;		/* Write one to clear */
	port->ci = 1;

	ahci_timer_start(&timer);

	while(port->ci & 1) {
		if(port->is & AHCI_PxIS_TFES) {
			break;
		}

		if(ahci_timer_expired(&timer)) {
			LOGF("AHCI: command 0x%x timed out on port %d\n",
			     (uint64_t)command, (uint64_t)disk->port);
			ahci_port_recover(port);
			return EBUSY;
		}
	}

	asm volatile("" ::: "memory");

	if((port->is & AHCI_PxIS_TFES) || (port->tfd & AHCI_PxTFD_ERR)) {
		LOGF("AHCI: command 0x%x failed on port %d (tfd 0x%x)\n",
		     (uint64_t)command, (uint64_t)disk->port,
		     (uint64_t)port->tfd);
		ahci_port_recover(port);
		return EIO;
	}

	return EOK;
}

static uint8_t ahci_identify(ahci_disk_t *disk)
{
	uint16_t *id = (uint16_t*)bounce;
	char model[41];
	uint32_t i;

	if(ahci_command(disk, ATA_CMD_IDENTIFY, 0, 0, AHCI_SECTOR_SIZE)
	   != EOK) {
		return EIO;
	}

	/* 48-bit capacity when the drive has it, the 28-bit one otherwise */
	if(id[83] & (1 << 10)) {
		disk->sectors = (uint64_t)id[100]
			| ((uint64_t)id[101] << 16)
			| ((uint64_t)id[102] << 32)
			| ((uint64_t)id[103] << 48);
	} else {
		disk->sectors = (uint64_t)id[60] | ((uint64_t)id[61] << 16);
	}

	/* The model is ASCII packed two to a word, high byte first, and
	 * padded with spaces */
	for(i = 0; i < 20; i++) {
		model[i * 2] = (char)(id[27 + i] >> 8);
		model[i * 2 + 1] = (char)(id[27 + i] & 0xFF);
	}
	model[40] = '\0';

	for(i = 40; i > 0 && model[i - 1] == ' '; i--) {
		model[i - 1] = '\0';
	}

	LOGF("AHCI: disk %d is port %d: \"%s\", %d sectors (%d MiB)\n",
	     (uint64_t)disk_count, (uint64_t)disk->port, model,
	     disk->sectors, disk->sectors / (MiB / AHCI_SECTOR_SIZE));

	return EOK;
}


static void ahci_port_init(ahci_register_set_t *hba, uint32_t index)
{
	ahci_port_register_set_t *port = &hba->port_register[index];
	ahci_disk_t *disk;

	/* Nothing plugged in, or the link never came up */
	if((port->ssts & AHCI_PxSSTS_DET_MASK) != AHCI_PxSSTS_DET_PRESENT) {
		return;
	}

	/* Optical drives and port multipliers are not disks we can read */
	if(port->sig != AHCI_PORT_SIG_SATA) {
		LOGF("AHCI: port %d has signature 0x%x, not a disk\n",
		     (uint64_t)index, (uint64_t)port->sig);
		return;
	}

	if(disk_count >= AHCI_MAX_DISKS) {
		LOGF("AHCI: ignoring the disk on port %d, too many disks\n",
		     (uint64_t)index);
		return;
	}

	disk = &disks[disk_count];
	disk->hba = hba;
	disk->port = index;
	disk->sectors = 0;
	disk->dma = &port_dma[disk_count];

	memset((void*)disk->dma, 0x00, sizeof(ahci_port_dma_t));

	/* The firmware may have left the port running on its own buffers */
	if(ahci_port_stop(port) != EOK) {
		LOGF("AHCI: port %d will not stop\n", (uint64_t)index);
		return;
	}

	port->clb = (uint32_t)(uint64_t)disk->dma->headers;
	port->clbu = (uint32_t)((uint64_t)disk->dma->headers >> 32);
	port->fb = (uint32_t)(uint64_t)disk->dma->received_fis;
	port->fbu = (uint32_t)((uint64_t)disk->dma->received_fis >> 32);

	/* Polled, so no interrupts, and nothing stale left to trip over */
	port->ie = 0;
	port->serr = 0xFFFFFFFF;
	port->is = 0xFFFFFFFF;

	if(ahci_port_start(port) != EOK) {
		LOGF("AHCI: port %d will not start\n", (uint64_t)index);
		return;
	}

	if(ahci_identify(disk) != EOK) {
		LOGF("AHCI: the disk on port %d did not identify itself\n",
		     (uint64_t)index);
		return;
	}

	disk_count++;
}

static void ahci_found(uint8_t bus, uint8_t dev, uint8_t func)
{
	ahci_register_set_t *hba;
	uint32_t command;
	uint32_t implemented;
	uint32_t i;

	/* The registers are memory mapped, and every transfer is the HBA
	 * mastering the bus */
	command = pci_read(bus, dev, func, PCI_REGISTER_COMMAND, 2);
	pci_write(bus, dev, func, PCI_REGISTER_COMMAND,
		  command | PCI_COMMAND_MEMORY | PCI_COMMAND_BUS_MASTER, 2);

	/* ABAR. The low bits describe the BAR rather than address it. */
	hba = (ahci_register_set_t*)(uint64_t)
		(pci_read(bus, dev, func, PCI_REGISTER_BAR5, 4) & 0xFFFFFFF0);

	LOGF("AHCI: controller %d:%d.%d, registers at 0x%x, version 0x%x\n",
	     (uint64_t)bus, (uint64_t)dev, (uint64_t)func,
	     (uint64_t)hba, (uint64_t)hba->generic.vs);

	if(ahci_gain_ownership(hba) == FALSE) {
		LOG("AHCI: the firmware would not hand the controller over");
		return;
	}

	hba->generic.ghc |= AHCI_GHC_AE;
	hba->generic.ghc &= ~AHCI_GHC_IE;

	implemented = hba->generic.pi;

	for(i = 0; i < 32; i++) {
		if(implemented & (1U << i)) {
			ahci_port_init(hba, i);
		}
	}
}


uint32_t ahci_disk_count(void)
{
	return disk_count;
}

uint64_t ahci_disk_sectors(uint32_t disk)
{
	if(disk >= disk_count) {
		return 0;
	}

	return disks[disk].sectors;
}

uint8_t ahci_read(uint32_t disk, uint64_t lba, uint32_t count, void *buffer)
{
	uint8_t *out = (uint8_t*)buffer;
	ahci_disk_t *d;

	if(disk >= disk_count) {
		return ENODEV;
	}

	d = &disks[disk];

	if(lba > d->sectors || count > d->sectors - lba) {
		return EINVAL;
	}

	while(count > 0) {
		uint32_t n = (count > AHCI_MAX_SECTORS) ? AHCI_MAX_SECTORS : count;
		uint8_t status = ahci_command(d, ATA_CMD_READ_DMA_EXT, lba,
					      (uint16_t)n,
					      n * AHCI_SECTOR_SIZE);

		if(status != EOK) {
			LOGF("AHCI: reading %d sectors at LBA %d failed\n",
			     (uint64_t)n, lba);
			return status;
		}

		memcpy(out, bounce, n * AHCI_SECTOR_SIZE);

		out += n * AHCI_SECTOR_SIZE;
		lba += n;
		count -= n;
	}

	return EOK;
}


void ahci_init(void)
{
	LOG("AHCI Init");

	pci_find(ahci_found, PCI_CLASS_ATA, PCI_SUBCLASS_SATA);

	LOGF("AHCI: %d disk(s) ready\n", (uint64_t)disk_count);
}
