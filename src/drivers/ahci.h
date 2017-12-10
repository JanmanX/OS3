#pragma once
#include <stdint.h>
#include <list.h>

/* Defines */
#define AHCI_PORT_SIG_NONE 			(0x00000000)
#define AHCI_PORT_SIG_SATA 			(0x00000101)
#define AHCI_PORT_SIG_SATAPI		(0xEB140101)
#define AHCI_PORT_SIG_ENCLOSURE		(0xC33C0101)
#define AHCI_PORT_SIG_MULTIPLIER	(0x96690101)
#define AHCI_PORT_SIG_DEFAULT 		(0xFFFFFFFF)


typedef struct ahci_generic_registers {
	uint32_t cap;
	uint32_t ghc;
	uint32_t is;
	uint32_t pi;
	uint32_t vs;

	/* Version 1.1 */
	uint32_t ccc_ctl;
	uint32_t ccc_ports;
	uint32_t em_loc;
	uint32_t em_ctrl;

	/* Version 1.2 */
	uint32_t cap2;
	uint32_t bohc;
} __attribute__((packed)) ahci_generic_registers_t;

typedef volatile struct ahci_port_register_set {
	uint64_t clbu;
	uint64_t fbu;

	uint32_t is;
	uint32_t ie;
	uint32_t cmd;
	uint32_t reserved0;
	uint32_t tfd;
	uint32_t sig;
	uint32_t ssts;
	uint32_t sctl;
	uint32_t serr;
	uint32_t sact;
	uint32_t ci;
	uint32_t sntf;
	uint32_t fbs;
	uint32_t devslp;

	uint8_t reserved1[40];

	uint8_t vendor_specific[16];
} __attribute__((packed)) ahci_port_register_set_t;

typedef volatile struct ahci_command_header {
	uint32_t flags;
	uint32_t prdbc;
	uint64_t ctbau; // both ctba and ctbau
	uint8_t reserved[16];
} __attribute__((packed)) ahci_cmd_hdr_t;

typedef struct ahci_prd_entry {
	uint64_t dbau; // both dba and dbau
	uint32_t reserved;
	uint32_t flags;
} __attribute__((packed)) ahci_prd_entry_t;

typedef volatile struct ahci_register_set {
	ahci_generic_registers_t generic;

	uint8_t reserved[52];
	uint8_t reserved_nvmhci[64];

	uint8_t vendor_specific[96];

	/* AHCI Port registers*/
	ahci_port_register_set_t port_register[];
} __attribute__((packed)) ahci_register_set_t;

typedef volatile struct ahci_h2d_register_fis {
	uint8_t type;
	uint8_t flags;
	uint8_t command;
	uint8_t features;

	uint8_t lba_0;
	uint8_t lba_1;
	uint8_t lba_2;
	uint8_t lba_6;
	uint8_t lba_3;
	uint8_t lba_4;
	uint8_t lba_5;

	uint8_t features_exp;
	uint8_t sector_low;
	uint8_t secotr_high;

	uint8_t reserved;
	uint8_t control;

	uint32_t reserved;
}__attribute__((packed)) ahci_h2d_register_fis_t;

typedef volatile struct ahci_receive_fis
{
	// 0x00
	FIS_DMA_SETUP	dsfis;		// DMA Setup FIS
	uint8_t         pad0[4];
 
	// 0x20
	FIS_PIO_SETUP	psfis;		// PIO Setup FIS
	uint8_t         pad1[12];
 
	// 0x40
	FIS_REG_D2H	rfis;		// Register – Device to Host FIS
	uint8_t         pad2[4];
 
	// 0x58
	FIS_DEV_BITS	sdbfis;		// Set Device Bit FIS
 
	// 0x60
	uint8_t         ufis[64];
 
	// 0xA0
	uint8_t   	rsv[0x100-0xA0];
} __attribute__((packed)) ahci_receive_fis_t;

/* Implementation specific */
typedef struct sata_controller {
	ahci_register_set_t* ahci;
}__attribute__((packed)) sata_controller_t;

void sata_init(void);
