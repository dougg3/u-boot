#include <common.h>

static void reconfigure_pll_sram(void);

#define APMU_SD3_CLK_RES_CTRL	(*((volatile u32 *)0xd42828e0))
#define APMU_SD1_CLK_RES_CTRL	(*((volatile u32 *)0xd4282854))
#define APMU_SD2_CLK_RES_CTRL	(*((volatile u32 *)0xd4282858))
#define APMU_SD4_CLK_RES_CTRL	(*((volatile u32 *)0xd42828e4))
#define APMU_DMA_CLK_RES_CTRL	(*((volatile u32 *)0xd4282864))
#define APMU_BUS_CLK_RES_CTRL	(*((volatile u32 *)0xd428286C))
#define MPMU_AWUCRM				(*((volatile u32 *)0xd405104c))
#define MPMU_ACGR				(*((volatile u32 *)0xd4051024))
#define MPMU_POCR				(*((volatile u32 *)0xd405000c))
#define APMU_IMR				(*((volatile u32 *)0xd4282898))
#define APMU_ISR				(*((volatile u32 *)0xd42828A0))
#define APMU_CCR				(*((volatile u32 *)0xd4282804))
#define MPMU_PLL1_REG1			(*((volatile u32 *)0xd4050050))
#define MPMU_PLL1_REG2			(*((volatile u32 *)0xd4050054))
#define MPMU_PLL1_SSC			(*((volatile u32 *)0xd4050058))
#define MPMU_PLL2_REG1			(*((volatile u32 *)0xd4050060))
#define MPMU_PLL2_REG2			(*((volatile u32 *)0xd4050064))
#define MPMU_PLL2_SSC			(*((volatile u32 *)0xd4050068))
#define APMU_PLL_SEL_STATUS		(*((volatile u32 *)0xd42828c4))
#define MPMU_PLL2CR				(*((volatile u32 *)0xd4050034))
#define MPMU_FCCR				(*((volatile u32 *)0xd4050008))
#define MPMU_APCR				(*((volatile u32 *)0xd4051000))
#define APMU_MC_HW_SLP_TYPE		(*((volatile u32 *)0xd42828b0))
#define APMU_MC_SW_SLP_TYPE		(*((volatile u32 *)0xd42828c0))
#define APMU_MC_SLP_REQ			(*((volatile u32 *)0xd42828b4))
#define APMU_IDLE_CFG			(*((volatile u32 *)0xd4282818))
#define APMU_RES_FRM_SLP_CLR	(*((volatile u32 *)0xd4282894))
#define APBC_TIMERS_CLK_RST		(*((volatile u32 *)0xd4015034))
#define APBC_TIMER1_CLK_RST		(*((volatile u32 *)0xd4015044))
#define TMR_CCR					(*((volatile u32 *)0xd4014000))
#define TMR_CR1					(*((volatile u32 *)0xd4014028))
#define TMR_T1_M2				(*((volatile u32 *)0xd401400c))
#define TMR_IER1				(*((volatile u32 *)0xd4014040))
#define TMR_ICR1				(*((volatile u32 *)0xd4014074))
#define TMR_SR1					(*((volatile u32 *)0xd4014034))
#define ICU_INT_CONF13			(*((volatile u32 *)0xd4282034))
#define ICU_INT_CONF36			(*((volatile u32 *)0xd4282090))
#define ICU_GBL_IRQ_MSK			(*((volatile u32 *)0xd4282114))

#define SDRAM_CNTRL_1			(*((volatile u32 *)0xb0000080))
#define USER_INITIATED_COMMAND	(*((volatile u32 *)0xb0000120))
#define PHY_CNTRL_13			(*((volatile u32 *)0xb0000230))
#define PHY_CNTRL_14			(*((volatile u32 *)0xb0000240))

#define inline_wait(NAME,NUM)	asm("ldr r7, =8\r\n"	\
									#NAME"_"#NUM ":\r\n" \
									"subs r7, r7, #2\r\n" \
									"bne " #NAME"_"#NUM "\r\n");

#define SRAM_BASE				0xd1020000

void reconfigure_plls(void)
{
	// Get stub into SRAM
	// 1) Ensure SRAM is configured as such.
	// 2) Copy program to SRAM
	// 3) Jump to SRAM

	// Fixing PLL1
	// 1) Set MPMU_PLL1_REG1=0x91040664
	// 2) Set MPMU_PLL1_REG2=0x84000030
	// 3) Set MPMU_FCCR=0x2000C290

	// Fixing PLL2
	// 1) Disable PLL2 (PLL2CR[EN]=0)
	// 2) Set PLL2CR=0x00097200
	// 3) Program MPMU_PLL2_REG1=0x90020464
	// 4) Program MPMU_PLL2_REG2=0x84000070
	// 5) Enable PLL2

	// Applying settings
	// 1) Set up TIMER1 to wake us from sleep
	// 2) Place DRAM into self-refresh mode (APMU_MC_HW_SLP_TYPE / APMU_MC_SLP_REQ)
	// 3) Set IDLE bit in APMU_IDLE_CFG
	// 4) Set AXISDD bit in MPMU_APCR
	// 5) Set SLPEN in MPMU_APCR
	// 6) Bring DRAM out of self-refresh.

	void (*reconfigure_plls_in_sram)(void) = (void (*)(void))SRAM_BASE;

	// Copy the function to SRAM.
	memcpy(reconfigure_plls_in_sram, reconfigure_pll_sram, 8192);

	// Run the function from SRAM.
	reconfigure_plls_in_sram();

	while (TMR_CR1 < TMR_T1_M2);

	// Reset everything back to their previous state.
	APMU_IDLE_CFG = APMU_IDLE_CFG & ~2L;
	MPMU_APCR = MPMU_APCR & ~((1 << 31) | (1 << 29));
	ICU_INT_CONF13 = 0x40;
	TMR_IER1 = TMR_IER1 & ~0x4L;
	TMR_ICR1 = TMR_ICR1 | 0x4;
	ICU_GBL_IRQ_MSK = 1;
}


// This code runs out of the L2 cache, reconfigured as SRAM.
static void reconfigure_pll_sram(void) {
	u32 i, j, mode;

	// Original notes from Chumby about this routine:

	// note to curious readers:
	// all registers are defined as volatile to avoid optimizations
	// also, every register, when written, is read back to a dummy variable
	// this is due to the synchronizer architecture inside the PXA168
	// I believe it uses a system of shadow registers, where data is written in
	// one clock domain, and lazily committed to the other, which can lead to
	// race conditions and non-committed writes. In order to flush the data,
	// every write is followed up with a dummy read and a short pause.

	// also, all code in this segment must be position independent, and it must fit within
	// 4kbytes. You can tweak this by changing the amount of data copied in the memcpy above
	// and the maximum size is maybe 128k or 256k. Also, you can't use stack, because
	// DRAM goes away during this execution, and we don't reset the SP. It helps to
	// decompile the .o file to make sure you've done all legal operations when you're done.

	// This is truly frustrating to program. I hope you never have to tweak it.

	if (MPMU_FCCR & 0x40000000)
		mode = 3;
	else
		mode = 0;

	// Enable spread spectrum, 2.5ppm downspread: our token to the FCC.
	MPMU_PLL1_SSC = 0x0; inline_wait(wait64, 1); j = MPMU_PLL1_SSC; // temp until AXI issues resolved
	inline_wait(wait64, 2);
	MPMU_PLL2_SSC = 0x0C7E3415; inline_wait(wait64, 3); j = MPMU_PLL2_SSC;

	///////////////////////////////
	////////////////////// first set system to use PLL1; also, fix PLL1 settings
	///////////////////////////////
	// set global interrupt mask in the icu register to mask the sync irq to the core
	ICU_INT_CONF36 = 0; inline_wait(wait64, 13); j = ICU_INT_CONF36;
	inline_wait(wait64, 14);

	// Set the PLL registers to fix the clock bug (errata 5.24 & 5.25)
	MPMU_PLL1_REG1 = 0x91040664; inline_wait(wait64, 7); j = MPMU_PLL1_REG1;
	inline_wait(wait64, 8);
	MPMU_PLL1_REG2 = 0x84000030; inline_wait(wait64, 9); j = MPMU_PLL1_REG2;
	inline_wait(wait64, 10);
	MPMU_FCCR      = 0x2000C290; inline_wait(wait64, 11); j = MPMU_FCCR;
	inline_wait(wait64, 12);
	MPMU_FCCR      = 0x2000C290; inline_wait(wait64, 11a); j = MPMU_FCCR;
	inline_wait(wait64, 12a);

	j = APMU_PLL_SEL_STATUS;

	// disable AXI
	APMU_SD3_CLK_RES_CTRL &= ~0x1B;
	APMU_SD4_CLK_RES_CTRL &= ~0x13;
	APMU_SD1_CLK_RES_CTRL &= ~0x1B;
	APMU_SD2_CLK_RES_CTRL &= ~0x13;

	////////////////// Now, sleep and allow PLL1 to reconfigure.
	// set wakeup source
	//MPMU_AWUCRM = 0x03e600cd; j = MPMU_AWUCRM;  // this is default from linux kernel, but using this causes it to never wake up
	MPMU_AWUCRM = 0x110; j = MPMU_AWUCRM;
	MPMU_ACGR = 0x007effde; j = MPMU_ACGR;
	MPMU_POCR = 0x1003f; j = MPMU_POCR;

	if (mode == 3)
		APMU_CCR = 0x77c082c0; // request the frequency changes

	// Set self-refresh on DRAM
	APMU_MC_HW_SLP_TYPE = 0x0; j = APMU_MC_HW_SLP_TYPE;
	APMU_MC_SW_SLP_TYPE = 0x0; j = APMU_MC_SW_SLP_TYPE;
	APMU_MC_SLP_REQ = 0x1; j = APMU_MC_SLP_REQ;

	// Wait for dram controller to acknowledge self-refresh.
	j = 0;
	while ((!(APMU_MC_SLP_REQ & 0x2)) && j < 0x1000000)
		j++;

	// set idle bit in AP idle configuration register
	ICU_GBL_IRQ_MSK = 3; j = ICU_GBL_IRQ_MSK;
	APMU_IDLE_CFG = 0x300302; j = APMU_IDLE_CFG;

	// This will kick us out of Sleep.
	// Enable the timer interrupt, with a priority mask of 15.
	i = ICU_INT_CONF13;
	ICU_INT_CONF13 = 0x5f;
	// setup TMR clocks
	APBC_TIMERS_CLK_RST = 0x33;
	TMR_CCR = 0x0;
	// Enable the IRQ to fire when the timer hits.
	TMR_ICR1 = TMR_ICR1 | 0x4;
	TMR_IER1 = TMR_IER1 | 0x4;

	// Timer1 set to run at 3.25 MHz.  So set an alarm to fire in the near future.
	TMR_T1_M2 = TMR_CR1 + (3250000/1000); // about 1 millisecond

	// set AXISDD bit in AP power control register to 1
	// set DDRCORSD and APBSD bits in MPMU_APCR to 1
	// set SLPEN bit in the MPMU_APCR to 1
	MPMU_APCR = 0xbe004000; j = MPMU_APCR;

	// Place system into sleep.

	// Wait for interrupt
	asm("mcr p15, 0, r0, c7, c0, 4\r\n"
		"nop\r\n"
		"nop\r\n"
		"nop\r\n"
		"nop\r\n"
		"nop\r\n"
		"nop\r\n"
		"nop\r\n"
		"nop\r\n"
		"nop\r\n"
	);
	j = APMU_PLL_SEL_STATUS;

	// clear the timer interrupts so they don't cause trouble later.
	ICU_INT_CONF13 = i;
	TMR_IER1 = TMR_IER1 & ~0x4;
	TMR_ICR1 = TMR_ICR1 | 0x4;
	ICU_GBL_IRQ_MSK=1; j = ICU_GBL_IRQ_MSK;
	APMU_IDLE_CFG = 0x300;
	// clear all the other bits and pieces we mucked with
	MPMU_APCR = 0;
	MPMU_POCR = 0x70fff;

	// re-enable AXI
	// phase 1: clock on while in reset
	APMU_SD3_CLK_RES_CTRL |= 0x18;
	APMU_SD4_CLK_RES_CTRL |= 0x12;
	APMU_SD1_CLK_RES_CTRL |= 0x18;
	APMU_SD2_CLK_RES_CTRL |= 0x12;
	// phase 2: clock on and out of reset
	APMU_SD3_CLK_RES_CTRL |= 0x1B;
	APMU_SD4_CLK_RES_CTRL |= 0x13;
	APMU_SD1_CLK_RES_CTRL |= 0x1B;
	APMU_SD2_CLK_RES_CTRL |= 0x13;

	if (mode == 3) {
		/////////////////////////// Now that PLL1 is fixed, move from PLL2 to PLL1
		MPMU_FCCR = 0x2000C290; inline_wait(wait64, x11); j = MPMU_FCCR;
		inline_wait(wait64, x12);
		MPMU_FCCR = 0x2000C290; inline_wait(wait64, x11a); j = MPMU_FCCR;
		inline_wait(wait64, x12a);

		APMU_IMR = 0x2; j = APMU_CCR;
		APMU_CCR = 0x00c082c0; // set up dividers
		j = APMU_CCR;
		APMU_CCR = 0x30c082c0; // enable the dynamic changes
		j = APMU_CCR;
		APMU_CCR = 0x3Bc082c0; // request the frequency changes
		j = APMU_CCR;

		j = 0;
		while (!(APMU_ISR & 2) && j < 400) {
			j++;
		}
	}

	j = APMU_PLL_SEL_STATUS;

	/////////////////////////////////// Fix PLL2
	//        MPMU_PLL2CR    = MPMU_PLL2CR & ~(1<<8); j = MPMU_PLL2CR;
	MPMU_PLL2CR    = 0x00097200; inline_wait(wait64, 20); j = MPMU_PLL2CR; // Disable PLL2: good thing we switched to PLL1!
	inline_wait(wait64, 21);
	MPMU_PLL2_REG1 = 0x90020464; inline_wait(wait64, 22); j = MPMU_PLL2_REG1;
	inline_wait(wait64, 23);
	MPMU_PLL2_REG2 = 0x84000070; inline_wait(wait64, 24); j = MPMU_PLL2_REG2;
	inline_wait(wait64, 25);

	MPMU_PLL2CR    = 0x00097300; inline_wait(wait64, 26); j = MPMU_PLL2CR; // Enable PLL2
	inline_wait(wait64, 27);


	//////////////// bring us back to PLL2
	MPMU_FCCR      = 0x4000C290; inline_wait(wait64, 40); j = MPMU_FCCR;
	inline_wait(wait64, 42);
	MPMU_FCCR      = 0x4000C290; inline_wait(wait64, 40a); j = MPMU_FCCR;
	inline_wait(wait64, 42a);

	APMU_IMR = 0x2; j = APMU_CCR;
	APMU_CCR = 0x00c082c0; // set up dividers
	j = APMU_CCR;
	APMU_CCR = 0x30c082c0; // enable the dynamic changes
	j = APMU_CCR;
	APMU_CCR = 0x8Fc082c0; // request the frequency changes
	j = APMU_CCR;

	j = 0;
	while (!(APMU_ISR & 2) && j < 400) {
		j++;
	}

	j = APMU_PLL_SEL_STATUS;

	//////////////////////////// Recover DRAM timing
	// Resynchronize DDR PHY clock
	PHY_CNTRL_14 = 0x80000000;

	// Reset DLL
	SDRAM_CNTRL_1 = 0x40;

	// Send MRS command
	USER_INITIATED_COMMAND = 0x03000100;

	// Clean DLL_RESET bit
	SDRAM_CNTRL_1 = 0x00;

	// Set DLL_RESET_TIMER to 8
	PHY_CNTRL_13 = ((PHY_CNTRL_13) & 0x0fffffff) | 0x80000000;

	// Set PHY_DLL_RESET
	PHY_CNTRL_14 = 0x20000000;

	// Write bit 27 of PHY_CNTRL_14 and wait at least 512 clock cycles to
	// update DLL.
	PHY_CNTRL_14 = 0x08000000;
	asm("ldr r7, =512\r\n"
		"wait512:\r\n"
		"subs r7, r7, #2\r\n"
		"bne wait512\r\n");

	// Write bit 27 of PHY_CNTRL_14 again to restore, since this bit is a
	// write-1-to-toggle bit.
	PHY_CNTRL_14 = 0x08000000;

	// Bring DDR out of self-refresh, by writing 0 to SLP_SEQ.
	APMU_MC_SLP_REQ = 0x0;

	// Wait a few ticks.
	asm("mov r0, #2560\r\n"
		"loop2:\r\n"
		"subs r0, r0, #1\r\n"
		"bne loop2\r\n");
}
