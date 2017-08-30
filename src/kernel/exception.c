#include "exception.h"
#include <stdint.h>
#include <errno.h>
#include <libc.h>
#include <kernel/interrupt.h>

void exception_init()
{
	/* Install handlers */
	interrupt_install(EXCEPTION_DE, exception_handler_de);
	interrupt_install(EXCEPTION_DB, exception_handler_db);
	interrupt_install(EXCEPTION_NMI, exception_handler_nmi);
	interrupt_install(EXCEPTION_BP, exception_handler_bp);
	interrupt_install(EXCEPTION_OF, exception_handler_of);
	interrupt_install(EXCEPTION_BR, exception_handler_br);
	interrupt_install(EXCEPTION_UD, exception_handler_ud);
	interrupt_install(EXCEPTION_NM, exception_handler_nm);
	interrupt_install(EXCEPTION_DF, exception_handler_df);
	interrupt_install(EXCEPTION_TS, exception_handler_ts);
	interrupt_install(EXCEPTION_NP, exception_handler_np);
	interrupt_install(EXCEPTION_SS, exception_handler_ss);
	interrupt_install(EXCEPTION_GP, exception_handler_gp);
	interrupt_install(EXCEPTION_PF, exception_handler_pf);
	interrupt_install(EXCEPTION_MF, exception_handler_mf);
	interrupt_install(EXCEPTION_AC, exception_handler_ac);
	interrupt_install(EXCEPTION_MC, exception_handler_mc);
	interrupt_install(EXCEPTION_XM, exception_handler_xm);
	interrupt_install(EXCEPTION_VE, exception_handler_ve);

	/* Reserved exception have a common handler */
	interrupt_install(EXCEPTION_9, exception_handler_reserved);
	interrupt_install(EXCEPTION_15, exception_handler_reserved);
	uint8_t i;
	for(i = 21; i <= 31; i++) {
		interrupt_install(i, exception_handler_reserved);
	}

	LOG("EXCEPTION HANDLERS INSTALLED");
}

uint8_t exception_handler_de(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_db(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_nmi(pt_regs_t *regs){return EOK;}
uint8_t exception_handler_bp(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_of(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_br(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_ud(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_nm(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_df(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_ts(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_np(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_ss(pt_regs_t *regs) {return EOK;}

uint8_t exception_handler_gp(pt_regs_t *regs)
{
	/* DEBUG */
	LOG("GP caught!");
	interrupt_print_regs(regs);

	if(regs->error_code != 0x00) {
		kprintf("ERROR CODE\n\tE: 0x%x\n\tTbl: 0x%x\n\tIndex: 0x%x\n",
			regs->error_code & 0x01,
			(regs->error_code >> 1) & 0x03,
			(regs->error_code >> 3) & (0xFFF));
	}


	return EOK;
}


uint8_t exception_handler_pf(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_mf(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_ac(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_mc(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_xm(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_ve(pt_regs_t *regs) {return EOK;}
uint8_t exception_handler_reserved(pt_regs_t *regs) {LOG("Reserved exception caught");return EOK;}

