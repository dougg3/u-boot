// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#include <common.h>
#include <init.h>
#include <mvmfp.h>
#include <asm/global_data.h>
#include <asm/mach-types.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mfp.h>
#include <asm/arch/armada100.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	u32 mfp_cfg[] = {
		/* Enable Console on UART1 */
		MFP107_UART1_TXD,
		MFP108_UART1_RXD,

		MFP_EOC		/*End of configureation*/
	};
	/* configure MFP's */
	mfp_config(mfp_cfg);

	/* Copied from Chumby old u-boot; this makes sure UART1 input works properly by
	 * setting SPINOR_MODE low, which allows RXD to pass through the level shifter
	 * U103 properly. */
	*(u32 *)0xd4019154 = (1 << (109-96));

	return 0;
}

int board_init(void)
{
	/* arch number of Board */
	gd->bd->bi_arch_number = MACH_TYPE_SILVERMOON;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = armd1_sdram_base(0) + 0x100;
	return 0;
}
