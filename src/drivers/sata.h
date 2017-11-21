#pragma once
#include <stdint.h>


typedef struct ahci_generic_registers {
	uint32_t cap;
	uint32_t ghc;
	uint32_t is;
	uint32_t pi;
	uint32_t vs;
	uint32_t ccc_ctl;
	uint32_t ccc_ports;

	/* Version 1.1 */
	uint32_t em_loc;
	uint32_t em_ctrl;

	/* Version 1.2 */
	uint32_t cap2;
	uint32_t bohc;
} __attribute__((packed)) ahci_generic_registers_t;

typedef struct ahci_port_register_set {
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

typedef struct ahci_register_set {
	ahci_generic_registers_t generic;

	uint8_t reserved[52];
	uint8_t reserved_nvmhci[64];

	uint8_t vendor_specific[96];

	/* AHCI Port registers*/
	ahci_port_register_set_t port_register[];
} __attribute__((packed)) ahci_register_set_t;


void sata_init(void);
