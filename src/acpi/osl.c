#include <acpi.h>

#include "acpica.h"

#include <kprintf.h>
#include <libc.h>
#include <lib/semaphore.h>
#include <lib/spinlock.h>
#include <kernel/time.h>
#include <drivers/pci.h>
#include <kernel/interrupt.h>
#include <kernel/time.h>

/* 9.1 Environmental and ACPI Tables */
/* 9.1.1 */
ACPI_STATUS AcpiOsInitialize(void)
{
	LOG("Initializing");
	return AE_OK;
}

/* 9.1.2 */
ACPI_STATUS AcpiOsTerminate(void)
{
	LOG("Terminating");
	return AE_OK;
}

/* 9.1.3 */
ACPI_PHYSICAL_ADDRESS AcpiOsGetRootPointer(void)
{
	return acpica_get_rsdp();
}

/* 9.1.4 */
ACPI_STATUS AcpiOsPredefinedOverride(
			const ACPI_PREDEFINED_NAMES *PredefinedObject,
			ACPI_STRING *NewValue)
{
	*NewValue = NULL;
	return AE_OK;
}

/* 9.1.5 */
ACPI_STATUS AcpiOsTableOverride(
			ACPI_TABLE_HEADER *ExistingTable,
			ACPI_TABLE_HEADER **NewTable)
{
	*NewTable = NULL;
	return AE_OK;
}

/* 9.1.6 */
ACPI_STATUS AcpiOsPhysicalTableOverride(
			ACPI_TABLE_HEADER *ExistingTable,
			ACPI_PHYSICAL_ADDRESS *NewAddress,
			UINT32 *NewTableLength)
{
	*NewAddress = NULL;
	return AE_OK;
}

/* 9.2 Memory Management */
/* 9.2.1 */
// Disabled

/* 9.2.2 */
// Disabled

/* 9.2.3 */
// Disabled

/* 9.2.4 */
// Disabled

/* 9.2.5 */
// Disabled

/* 9.2.6 */
void *AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS Phys,
		      ACPI_SIZE	Len)
{
	/* We are identity mapped */
	return Phys;
}

/* 9.2.7 */
void AcpiOsUnmapMemory(void *LogicalAddress,
			ACPI_SIZE Len)
{
	/* Nothing to do */
	return;
}

/* 9.2.8 */
ACPI_STATUS AcpiOsGetPhysicalAddress(
				void *LogicalAddress,
				ACPI_PHYSICAL_ADDRESS *PhysicalAddress)
{
	*PhysicalAddress = LogicalAddress;
	return AE_OK;
}

/* 9.2.9 */
void* AcpiOsAllocate(ACPI_SIZE Size)
{
	void* ptr = malloc(Size);
	ASSERT(ptr != NULL, "ACPICA Could not allocate heap!");

	return ptr;
}


/* 9.2.10 */
void AcpiOsFree(void *Mem)
{
	free(Mem);
}

/* 9.2.11 */
BOOLEAN AcpiOsReadAble(void *Mem,
		       ACPI_SIZE Len)
{
	return TRUE;
}

/* 9.2.12 */
BOOLEAN AcpiOsWriteable(void *Mem,
		       ACPI_SIZE Len)
{
	return TRUE;
}


/* 9.3 Multithreading and Scheduling Services */
/* XXX */

/* 9.3.1 */
ACPI_THREAD_ID AcpiOsGetThreadId(void)
{
	/* XXX */
	return 1;
}

/* 9.3.2 */
ACPI_STATUS AcpiOsExecute(ACPI_EXECUTE_TYPE Type,
			  ACPI_OSD_EXEC_CALLBACK Func,
			  void *Context)
{
	/* I guess just run it now */
	Func(Context);

	return AE_OK;
}

/* 9.3.3 */
void AcpiOsSleep(UINT64 ms)
{
	sleep(ms);
}

/* 9.3.4 */
void AcpiOsStall(UINT32 ms)
{
	sleep(ms * 1000);
}

/* 9.3.5 */
void AcpiOsWaitEventsComplete(void)
{

}


/* 9.4 Mutual exclusion and Synchronization */
/* 9.4.1 */
// Disabled

/* 9.4.2 */
// Disabled

/* 9.4.3 */
// Disabled

/* 9.4.4 */
// Disabled


/* 9.4.5 */
ACPI_STATUS AcpiOsCreateSemaphore(UINT32 MaxUnits,
				  UINT32 InitialUnits,
				  ACPI_SEMAPHORE *OutHandle)
{
	return AE_OK;
	semaphore_t* s = semaphore_create();
	ASSERT(s != NULL, "Could not create semaphore!");
	s->n = InitialUnits;

	*OutHandle = s;

	return AE_OK;
}

/* 9.4.6 */
ACPI_STATUS AcpiOsDeleteSemaphore(ACPI_SEMAPHORE Handle)
{
	/* TODO: Should I implement MaxUnits? */
	free(Handle);

	return AE_OK;
}

/* 9.4.7 */
ACPI_STATUS AcpiOsWaitSemaphore(ACPI_SEMAPHORE Handle,
				UINT32 Units,
				UINT16 Timeout)
{
	return AE_OK;
	if(!Handle) {
		/* XXX */
		return AE_BAD_PARAMETER;
	}

	/* XXX: Linux does not use Units, so neither will we ...*/
	semaphore_wait(&Handle);
	return AE_OK;
}

/* 9.4.8 */
ACPI_STATUS AcpiOsSignalSemaphore(ACPI_SEMAPHORE Handle,
				  UINT32 Units)
{
	return AE_OK;
	semaphore_signal(&Handle);
	return AE_OK;
}


/* 9.4.9 */
ACPI_STATUS AcpiOsCreateLock(ACPI_SPINLOCK *OutHandle)
{
	return AE_OK;
	spinlock_t *sl = malloc(sizeof(spinlock_t));

	LOGF("Creating spinlock@0x%x\n", sl);
	ASSERT(sl != NULL, "Could not allocate spinlock");
	*sl = 0;
	*OutHandle = sl;

	return AE_OK;
}

/* 9.4.10 */
void AcpiOsDeleteLock(ACPI_SPINLOCK Handle)
{
	return AE_OK;
	free(Handle);
}


/* 9.4.11 */
ACPI_CPU_FLAGS AcpiOsAcquireLock(ACPI_SPINLOCK Handle)
{
	return AE_OK;
	LOGF("Acquiring lock@0x%x\n", &Handle);
	spinlock_acquire(&Handle);

	return AE_OK;
}


/* 9.4.12 */
void AcpiOsReleaseLock(ACPI_SPINLOCK Handle,
			ACPI_CPU_FLAGS Flags)
{
	return AE_OK;
	kprintf("Releasing spinlock ");
	spinlock_release(Handle);
}


/* 9.5 Interrupt Handling */
/* 9.5.1 */
void* acpica_interrupt_context = NULL;
ACPI_OSD_HANDLER acpi_osd_handler;
void acpica_interrupt_handler(pt_regs_t* regs)
{
	acpi_osd_handler(acpica_interrupt_context);
}


ACPI_STATUS AcpiOsInstallInterruptHandler(UINT32 InterruptLevel,
					  ACPI_OSD_HANDLER Handler,
					  void *Context)
{
	if(interrupt_is_vector_free(InterruptLevel) == 0) {
		ERRORF("Could not register interrupt for ACPICA:\
		       InterruptLevel: 0x%x\n", InterruptLevel);
		while(1)
			HALT;
	}

	acpica_interrupt_context = Context;
	acpi_osd_handler = Handler;

	LOGF("Trying to install interrupt handler: 0x%x\n", InterruptLevel);
	interrupt_install(InterruptLevel, acpica_interrupt_handler );

	return AE_OK;
}


/* 9.5.2 */
ACPI_STATUS AcpiOsRemoveInterruptHandler(UINT32 InterruptNumber,
					 ACPI_OSD_HANDLER Handler)
{
	interrupt_uninstall(InterruptNumber);
}


/* 9.6 Memory Access and Memory Mapped I/O */
/* 9.6.1 */
ACPI_STATUS AcpiOsReadMemory(ACPI_PHYSICAL_ADDRESS Address,
			     UINT64 *Value,
			     UINT32 Width)
{

	/* We have identity mapping, so reading should be trivial */
	switch(Width) {
	case 8:
		*Value = GET_UINT8(Address, 0);
		break;
	case 16:
		*Value = GET_UINT16(Address, 0);
		break;

	case 32:
		*Value = GET_UINT32(Address, 0);
		break;

	case 64:
		*Value = GET_UINT64(Address, 0);
		break;

	default:
		return AE_BAD_PARAMETER;
	}


	return AE_OK;


}

/* 9.6.2 */
ACPI_STATUS AcpiOsWriteMemory(ACPI_PHYSICAL_ADDRESS Address,
			      UINT64 Value,
			      UINT32 Width)
{
	/* We have identity mapping, so reading should be trivial */
	switch(Width) {
	case 8:
		GET_UINT8(Address, 0) = Value;
		break;
	case 16:
		GET_UINT16(Address, 0) = Value;
		break;

	case 32:
		GET_UINT32(Address, 0) = Value;
		break;

	case 64:
		GET_UINT64(Address, 0) = Value;
		break;

	default:
		return AE_BAD_PARAMETER;
	}

	return AE_OK;

}

/* 9.7 Port I/O */
/* 9.7.1 */
ACPI_STATUS AcpiOsReadPort(ACPI_IO_ADDRESS Address,
			   UINT32 *Value,
			   UINT32 Width)
{
	switch(Width) {
	case 8:
		*Value = inb(Address);
		break;
	case 16:
		*Value = inw(Address);
		break;
	case 32:
		*Value = ind(Address);
		break;
	default:
		return AE_BAD_PARAMETER;

	}

	return AE_OK;
}

/* 9.7.2 */
ACPI_STATUS AcpiOsWritePort(ACPI_IO_ADDRESS Address,
			    UINT32 Value,
			    UINT32 Width)
{
	switch(Width) {
	case 8:
		outb(Address, Value);
		break;
	case 16:
		outw(Address, Value);
		break;
	case 32:
		outd(Address, Value);
		break;
	default:
		return AE_BAD_PARAMETER;

	}

	return AE_OK;
}


/* 9.8 PCI Configuration Space Access */
/* 9.8.1 */
ACPI_STATUS AcpiOsReadPciConfiguration(ACPI_PCI_ID* PciId,
				       UINT32 Register,
				       UINT64 *Value,
				       UINT32 Width)
{
	ERROR("Not implemented");
}
ACPI_STATUS AcpiOsWritePciConfiguration(ACPI_PCI_ID* PciId,
					UINT32 Register,
					UINT64 Value,
					UINT32 Width)
{
	ERROR("Not implemented");
}

/* 9.9 Formatted Output */
void AcpiOsPrintf(const char *format, ...) {
	va_list args;
	va_start(args, format);

	kprintf(format, args);

	va_end(args);
}

void AcpiOsVprintf(const char *format, va_list args )
{
	kprintf(format, args);
}

void AcpiOsRedirectOutput(void *destination) {

}


/* 9.10 System ACPI Table Access */
ACPI_STATUS AcpiOsGetTableByAddress(ACPI_PHYSICAL_ADDRESS address ,
				    ACPI_TABLE_HEADER **out ) {
	return AE_SUPPORT;
}

ACPI_STATUS AcpiOsGetTableByIndex(uint32_t index ,
				  ACPI_TABLE_HEADER **table ,
				  uint32_t *instance ,
				  ACPI_PHYSICAL_ADDRESS *address ) {
	return AE_SUPPORT;
}

ACPI_STATUS AcpiOsGetTableByName(char *signature ,
				 uint32_t instance ,
				 ACPI_TABLE_HEADER **table ,
				 ACPI_PHYSICAL_ADDRESS *address ) {
	return AE_SUPPORT;
}


/* 9.11 Miscellaneous */
UINT64 AcpiOsGetTimer (void)
{
	/* XXX:HACK
	 * TODO: Implement time */
	static uint64_t time = 0x00;
	return time++;
}

ACPI_STATUS  AcpiOsSignal (UINT32 Function,
			   void *Info)
{
	DEBUGF("AcpiOsSignal: 0x%x\n", Function);
	return AE_OK;
}



/* ??? Undocumented ??? */
ACPI_STATUS AcpiOsEnterSleep(uint8_t state , uint32_t a , uint32_t b )
{
	return AE_OK;
}
