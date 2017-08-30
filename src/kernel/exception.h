#pragma once

#include <kernel/interrupt.h>
#include <stdint.h>
#include <libc.h>

#define EXCEPTION_DE		(0x00)
#define EXCEPTION_DB		(0x01)
#define EXCEPTION_NMI		(0x02)
#define EXCEPTION_BP		(0x03)
#define EXCEPTION_OF		(0x04)
#define EXCEPTION_BR		(0x05)
#define EXCEPTION_UD		(0x06)
#define EXCEPTION_NM		(0x07)
#define EXCEPTION_DF		(0x08)
#define EXCEPTION_9		(0x09)
#define EXCEPTION_TS		(0x0A)
#define EXCEPTION_NP		(0x0B)
#define EXCEPTION_SS		(0x0C)
#define EXCEPTION_GP		(0x0D)
#define EXCEPTION_PF		(0x0E)
#define EXCEPTION_15		(0x0F)
#define EXCEPTION_MF		(0x10)
#define EXCEPTION_AC		(0x11)
#define EXCEPTION_MC		(0x12)
#define EXCEPTION_XM		(0x13)
#define EXCEPTION_VE		(0x14)


void exception_init();


uint8_t exception_handler_de(pt_regs_t *regs);
uint8_t exception_handler_db(pt_regs_t *regs);
uint8_t exception_handler_nmi(pt_regs_t *regs);
uint8_t exception_handler_bp(pt_regs_t *regs);
uint8_t exception_handler_of(pt_regs_t *regs);
uint8_t exception_handler_br(pt_regs_t *regs);
uint8_t exception_handler_ud(pt_regs_t *regs);
uint8_t exception_handler_nm(pt_regs_t *regs);
uint8_t exception_handler_df(pt_regs_t *regs);
uint8_t exception_handler_ts(pt_regs_t *regs);
uint8_t exception_handler_np(pt_regs_t *regs);
uint8_t exception_handler_ss(pt_regs_t *regs);
uint8_t exception_handler_gp(pt_regs_t *regs);
uint8_t exception_handler_pf(pt_regs_t *regs);
uint8_t exception_handler_mf(pt_regs_t *regs);
uint8_t exception_handler_ac(pt_regs_t *regs);
uint8_t exception_handler_mc(pt_regs_t *regs);
uint8_t exception_handler_xm(pt_regs_t *regs);
uint8_t exception_handler_ve(pt_regs_t *regs);
uint8_t exception_handler_reserved(pt_regs_t *regs);
