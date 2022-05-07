#include <common.h>

static void read_dramconfig_registers(void)
{
	int i;
	volatile u32 *addr;
	volatile u32 dummy;

	for (i = 0; i < 0x290; i += 0x10) {
		addr = (u32 *)(0xb0000000 + i);
		dummy = *addr;
	}

	addr = (volatile u32 *)(0xb0000000 + 0x650);
	dummy = *addr;
	addr = (u32 *)(0xb0000000 + 0x660);
	dummy = *addr;
	addr = (u32 *)(0xb0000000 + 0x760);
	dummy = *addr;
	addr = (u32 *)(0xb0000000 + 0x770);
	dummy = *addr;
	addr = (u32 *)(0xb0000000 + 0x7d0);
	dummy = *addr;
	addr = (u32 *)(0xb0000000 + 0x7e0);
	dummy = *addr;
	addr = (u32 *)(0xb0000000 + 0xe10);
	dummy = *addr;
	addr = (u32 *)(0xb0000000 + 0xe80);
	dummy = *addr;
}


void reconfigure_dram(void)
{
	int i;
	volatile u32 *addr;
	volatile u32 dummy;

	/* This code does a lot of DDR register readbacks. They seem unnecessary, but
	 * Chumby's original u-boot fork did it...so it's replicated here. */

	*(volatile u32 *)0xb0000080 = 0x00000000;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000140 = 0x20004433;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000210 = 0x00300002;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000100 = 0x000B0001;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000050 = 0x4CD600C8;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000060 = 0x64660332;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000190 =  0x20C83752;
	read_dramconfig_registers();

	*(volatile u32 *)0xb00001c0 = 0x353AA0A0;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000650 = 0x00100121;
	read_dramconfig_registers();

	// Prioritize peripherals above memory and data accesses.
	*(volatile u32 *)0xb0000280 = 0x0101010e;
	read_dramconfig_registers();

	*(volatile u32 *)0xb00000f0 = 0xc0000000;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000020 = 0x00066330;
	read_dramconfig_registers();

	*(volatile u32 *)0xb00001a0 = 0x20818004;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000090 = 0x00080000;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000760 = 0x00000001;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000770 = 0x02000002;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000240 = 0x80000000;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000230 = 0x00000108;
	read_dramconfig_registers();
	*(volatile u32 *)0xb0000e10 = 0x00000100;
	read_dramconfig_registers();

	*(volatile u32 *)0xb00001F0 = 0x00000011;
	read_dramconfig_registers();

	*(volatile u32 *)0xb00001D0 = 0x177C2779;
	read_dramconfig_registers();
	*(volatile u32 *)0xb00001E0 = 0x03300330;
	read_dramconfig_registers();

	*(volatile u32 *)0xb0000240 = (*(volatile u32 *)0xb0000240 | 0x20000000);
	read_dramconfig_registers();
	addr = (volatile u32 *)0x0;
	for (i = 0; i < 5120; i++) {
		dummy = *(addr + i);
	}

	*(volatile u32 *)0xb0000240 = (*(volatile u32 *)0xb0000240 & 0xDFFFFFFF);
	read_dramconfig_registers();
	addr = (volatile u32 *)0x0;
	for (i = 0; i < 5120; i++) {
		dummy = *(addr + i);
	}

	*(volatile u32 *)0xb0000230 = (*(volatile u32 *)0xb0000230 & 0xFFFFFFFB);
	read_dramconfig_registers();
	addr = (volatile u32 *)0x0;
	for (i = 0; i < 5120; i++) {
		dummy = *(addr + i);
	}


	*(volatile u32 *)0xb0000240 = (*(volatile u32 *)0xb0000240 | 0x08000000);
	read_dramconfig_registers();
	addr = (volatile u32 *)0x0;
	for (i = 0; i < 5120; i++) {
		dummy = *(addr + i);
	}

	*(volatile u32 *)0xb0000240 = (*(volatile u32 *)0xb0000240 | 0x08000000);
	read_dramconfig_registers();
	for (i = 0; i < 5120; i++) {
		dummy = *(addr + i);
	}

	*(volatile u32 *)0xb0000120 = 0x00000001;
	read_dramconfig_registers();

	/* Read back the registers a bunch afterward to let everything stabilize.
	 * Original Chumby u-boot blamed this on a silicon bug */
	addr = (volatile u32 *)0x0;
	for (i = 0; i < 5220; i++) {
		read_dramconfig_registers();
		dummy = *(addr + i);
	}
}
