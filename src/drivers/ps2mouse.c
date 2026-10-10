#include "ps2mouse.h"
#include <drivers/ps2.h>
#include <drivers/screen.h>
#include <stdint.h>
#include <errno.h>
#include <libc.h>
#include <kernel/interrupt.h>
#include <cpu/apic.h>

/* How many times we retry a command the mouse asks us to resend */
#define PS2_MOUSE_MAX_RESEND	(3)

/* Most bytes one interrupt will take off the controller. A movement packet
 * is three, so this is generous - it is here only so that a controller that
 * never clears its output flag cannot spin us forever with interrupts off. */
#define PS2_MOUSE_MAX_DRAIN	(32)

/* Pointer state. Written by the IRQ handler, read by whoever is drawing.
 * "packets" doubles as a sequence counter: it is bumped last, so a reader
 * that sees the same value before and after its copy read a consistent
 * snapshot. */
static volatile int32_t mouse_x = 0;
static volatile int32_t mouse_y = 0;
static volatile uint8_t mouse_buttons = 0x00;
static volatile uint64_t mouse_packets = 0;

/* Area the pointer is confined to */
static int32_t bound_x = 0;
static int32_t bound_y = 0;

/* Packet assembly */
static uint8_t packet[3] = {0, 0, 0};
static uint8_t packet_index = 0;

static uint8_t mouse_present = 0x00;

/* Interrupts that hit the drain cap. Non-zero means the controller is
 * handing out bytes faster than one interrupt can take them, which is worth
 * knowing about but is not on its own an error. */
static volatile uint64_t drain_overruns = 0;


static void ps2mouse_clamp(void)
{
	if(mouse_x < 0) {
		mouse_x = 0;
	}
	if(mouse_y < 0) {
		mouse_y = 0;
	}
	if(mouse_x > bound_x) {
		mouse_x = bound_x;
	}
	if(mouse_y > bound_y) {
		mouse_y = bound_y;
	}
}


/* Fold a finished 3-byte packet into the pointer state */
static void ps2mouse_apply_packet(void)
{
	uint8_t flags = packet[0];

	/* A packet whose counters overflowed tells us nothing useful */
	if(flags & (PS2_MOUSE_PKT_X_OVF | PS2_MOUSE_PKT_Y_OVF)) {
		return;
	}

	/* The movement bytes are 9-bit two's complement, with the sign
	 * living in the flags byte */
	int32_t dx = (int32_t)packet[1];
	int32_t dy = (int32_t)packet[2];

	if(flags & PS2_MOUSE_PKT_X_SIGN) {
		dx -= 0x100;
	}
	if(flags & PS2_MOUSE_PKT_Y_SIGN) {
		dy -= 0x100;
	}

	mouse_x += dx;

	/* The mouse counts up when pushed away from the user, the screen
	 * counts down */
	mouse_y -= dy;

	ps2mouse_clamp();

	mouse_buttons = flags & (PS2_MOUSE_BUTTON_LEFT
				 | PS2_MOUSE_BUTTON_RIGHT
				 | PS2_MOUSE_BUTTON_MIDDLE);

	/* Publish. Everything above must be visible before the counter moves */
	asm volatile("" ::: "memory");
	mouse_packets++;
}


uint8_t ps2mouse_interrupt_handler(pt_regs_t *regs)
{
	(void)regs;

	/* Drain the bytes the controller has for us. One IRQ can cover more
	 * than one byte, and leaving bytes behind wedges the queue - but the
	 * loop is bounded, because everything in here runs with interrupts
	 * off and a device that never clears its output flag would otherwise
	 * take the machine with it. Anything left over arrives next time. */
	uint32_t drained;

	for(drained = 0; drained < PS2_MOUSE_MAX_DRAIN; drained++) {
		uint8_t status = inb(PS2_STATUS);

		if((status & PS2_STATUS_OUTPUT_FULL) == 0) {
			break;
		}

		/* Not ours - the keyboard handler will deal with it */
		if((status & PS2_STATUS_FROM_AUX) == 0) {
			break;
		}

		uint8_t byte = inb(PS2_DATA);

		/* Bit 3 of the first byte is always set. If it is not, we are
		 * out of step with the device: drop the byte and resync. */
		if(packet_index == 0 && (byte & PS2_MOUSE_PKT_ALWAYS_1) == 0) {
			continue;
		}

		packet[packet_index++] = byte;

		if(packet_index == 3) {
			packet_index = 0;
			ps2mouse_apply_packet();
		}
	}

	if(drained >= PS2_MOUSE_MAX_DRAIN) {
		drain_overruns++;
	}

	lapic_eoi();

	return EOK;
}


/* Send one byte to the device behind the aux port and collect its reply */
static uint8_t ps2mouse_command(uint8_t command, uint8_t *reply)
{
	uint8_t attempt;
	uint8_t response = 0x00;

	for(attempt = 0; attempt < PS2_MOUSE_MAX_RESEND; attempt++) {
		/* Tell the controller the next data byte is for the mouse */
		if(ps2_write_poll(PS2_COMMAND, PS2_CMD_WRITE_AUX) != EOK) {
			return EBUSY;
		}

		if(ps2_write_poll(PS2_DATA, command) != EOK) {
			return EBUSY;
		}

		if(ps2_read_poll(&response) != EOK) {
			return EBUSY;
		}

		if(response == PS2_RESEND) {
			continue;
		}

		if(reply != NULL) {
			*reply = response;
		}

		return (response == PS2_ACK) ? EOK : EIO;
	}

	return EIO;
}


void ps2mouse_set_bounds(uint32_t width, uint32_t height)
{
	bound_x = (width  > 0) ? (int32_t)width  - 1 : 0;
	bound_y = (height > 0) ? (int32_t)height - 1 : 0;

	ps2mouse_clamp();
}

void ps2mouse_set_position(int32_t x, int32_t y)
{
	mouse_x = x;
	mouse_y = y;
	ps2mouse_clamp();

	asm volatile("" ::: "memory");
	mouse_packets++;
}

void ps2mouse_get_state(ps2mouse_state_t *out)
{
	uint64_t seq;

	if(out == NULL) {
		return;
	}

	/* Retry until the IRQ handler leaves us alone long enough to copy
	 * the whole thing */
	do {
		seq = mouse_packets;
		asm volatile("" ::: "memory");

		out->x = mouse_x;
		out->y = mouse_y;
		out->buttons = mouse_buttons;
		out->packets = seq;

		asm volatile("" ::: "memory");
	} while(seq != mouse_packets);
}

uint8_t ps2mouse_is_present(void)
{
	return mouse_present;
}

uint64_t ps2mouse_get_drain_overruns(void)
{
	return drain_overruns;
}


uint8_t ps2mouse_init(void)
{
	uint8_t config = 0x00;
	uint8_t reply = 0x00;
	uint8_t status = EOK;

	LOG("Mouse init");

	CLI;

	/* Keep both devices quiet while we talk to the mouse, so that no
	 * stray scancode gets mistaken for a command reply */
	ps2_write_poll(PS2_COMMAND, PS2_CMD_DISABLE_KB);
	ps2_write_poll(PS2_COMMAND, PS2_CMD_DISABLE_AUX);
	ps2_flush();

	/* Give the second port a clock, but no interrupt yet - the replies
	 * below have to reach us and not the handler */
	if(ps2_read_config(&config) != EOK) {
		LOG("PS/2 controller did not hand over its config byte");
		STI;
		return EBUSY;
	}

	config &= ~PS2_CONFIG_AUX_IRQ;
	config &= ~PS2_CONFIG_AUX_CLOCK_OFF;
	ps2_write_config(config);

	/* Turn the port on */
	ps2_write_poll(PS2_COMMAND, PS2_CMD_ENABLE_AUX);

	/* Reset. The device answers ACK, then its self-test result, then
	 * its device id. */
	status = ps2mouse_command(PS2_MOUSE_RESET, &reply);
	LOGF("mouse reset  - 0x%x\n", reply);

	if(status != EOK) {
		LOGF("No PS/2 mouse behind the aux port (reply 0x%x)\n", reply);
		ps2_write_poll(PS2_COMMAND, PS2_CMD_ENABLE_KB);
		STI;
		return ENODEV;
	}

	if(ps2_read_poll(&reply) == EOK) {
		LOGF("mouse self-test - 0x%x\n", reply);

		if(reply != PS2_SELF_TEST_OK) {
			LOGF("mouse failed self-test: 0x%x\n", reply);
		}
	}

	if(ps2_read_poll(&reply) == EOK) {
		LOGF("mouse id     - 0x%x\n", reply);
	}

	/* Known-good sample rate, resolution and scaling */
	status = ps2mouse_command(PS2_MOUSE_SET_DEFAULTS, &reply);
	LOGF("mouse defaults - 0x%x\n", reply);

	/* Start streaming movement packets */
	status = ps2mouse_command(PS2_MOUSE_ENABLE, &reply);
	LOGF("mouse enable - 0x%x\n", reply);

	if(status != EOK) {
		LOGF("Mouse refused to start reporting (reply 0x%x)\n", reply);
		ps2_write_poll(PS2_COMMAND, PS2_CMD_ENABLE_KB);
		STI;
		return EIO;
	}

	/* The keyboard can talk again */
	ps2_write_poll(PS2_COMMAND, PS2_CMD_ENABLE_KB);

	/* Confine the pointer to the screen and park it in the middle */
	ps2mouse_set_bounds(screen_get_width(), screen_get_height());
	ps2mouse_set_position((int32_t)screen_get_width() / 2,
			      (int32_t)screen_get_height() / 2);

	/* Route IRQ 12 and install the handler before unmasking it */
	LOGF("IRQ routing for mouse: 0x%x\n", ioapic_get_iso(PS2_MOUSE_IRQ));

	interrupt_install(APIC_VECTOR_OFFSET + PS2_MOUSE_IRQ,
			  ps2mouse_interrupt_handler);

	ioapic_set_irq(ioapic_get_iso(PS2_MOUSE_IRQ),
		       0x00,
		       APIC_VECTOR_OFFSET + PS2_MOUSE_IRQ);

	/* Now let the packets arrive as interrupts */
	if(ps2_read_config(&config) == EOK) {
		config |= PS2_CONFIG_AUX_IRQ;
		ps2_write_config(config);
		LOGF("PS/2 config byte: 0x%x\n", config);
	}

	/* Anything that piled up during setup is stale */
	ps2_flush();
	packet_index = 0;

	mouse_present = 0x01;

	STI;

	LOG("Mouse installed");

	return EOK;
}
