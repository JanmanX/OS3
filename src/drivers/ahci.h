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
{
	// DW0
	uint8_t  cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
	uint8_t  a:1;		// ATAPI
	uint8_t  w:1;		// Write, 1: H2D, 0: D2H
	uint8_t  p:1;		// Prefetchable
 
	uint8_t  r:1;		// Reset
	uint8_t  b:1;		// BIST
	uint8_t  c:1;		// Clear busy upon R_OK
	uint8_t  rsv0:1;		// Reserved
	uint8_t  pmp:4;		// Port multiplier port
 
	uint16_t prdtl;		// Physical region descriptor table length in entries
 
	// DW1
	volatile uint32_t prdbc;		// Physical region descriptor byte count transferred
 
	// DW2, 3
	uint64_t ctbau;
 
	// DW4 - 7
	uint32_t rsv1[4];	// Reserved

} __attribute__((packed)) ahci_cmd_hdr_t;


typedef volatile struct ahci_prd_entry {
	uint64_t dbau; // both dba and dbau
	uint32_t reserved;
	uint32_t flags;
} __attribute__((packed)) ahci_prd_entry_t;

typedef volatile struct ahci_command_table {
	uint8_t fis[0x40];	// Up to 64 bytes

	uint8_t cmd[0x10];	// up to 16 bytes

	uint8_t reserved[0x30]; // reserved, 48 bytes

#define AHCI_CMD_TBL_NUM_PRDS	(0x20)
	ahci_prd_entry_t prdt_entry[AHCI_CMD_TBL_NUM_PRDS];	

} __attribute__((packed)) ahci_cmd_tbl_t;

typedef volatile struct ahci_register_set {
	ahci_generic_registers_t generic;

	uint8_t reserved[52];
	uint8_t reserved_nvmhci[64];

	uint8_t vendor_specific[96];

	/* AHCI Port registers*/
	ahci_port_register_set_t port_register[];
} __attribute__((packed)) ahci_register_set_t;

typedef volatile struct ahci_h2d_register_fis {
#define AHCI_FIS_TYPE_H2D_REGISTER		(0x27)
	uint8_t type;
	uint8_t flags;
	uint8_t command;
	uint8_t features;

	uint8_t lba_0;
	uint8_t lba_1;
	uint8_t lba_2;
	uint8_t device;
	uint8_t lba_3;
	uint8_t lba_4;
	uint8_t lba_5;

	uint8_t features_exp;
	uint8_t sector_low;
	uint8_t secotr_high;

	uint8_t reserved0;
	uint8_t control;

	uint32_t reserved1;
}__attribute__((packed)) ahci_h2d_register_fis_t;


/* Implementation specific */
typedef struct sata_controller {
	ahci_register_set_t* ahci;
}__attribute__((packed)) sata_controller_t;

void sata_init(void);
void ahci_read(ahci_register_set_t* controller,
				uint32_t port,
				uint64_t lba,
				uint64_t count,
				uint8_t buffer);