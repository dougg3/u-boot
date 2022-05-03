/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 * Written-by: Prafulla Wadaskar <prafulla@marvell.com>
 * Contributor: Mahavir Jain <mjain@marvell.com>
 */

#ifndef __CONFIG_ASPENITE_H
#define __CONFIG_ASPENITE_H

/*
 * High Level Configuration Options
 */
#define CONFIG_SHEEVA_88SV331xV5	1	/* CPU Core subversion */
#define CONFIG_ARMADA100		1	/* SOC Family Name */
#define CONFIG_ARMADA168		1	/* SOC Used on this Board */
#define CONFIG_SKIP_LOWLEVEL_INIT	/* disable board lowlevel_init */

/*
 * Stack pointer deserves some explanation here. All of my research indicates
 * that the Chumby first stage bootloader loads u-boot at 0x00F00000. This is
 * true, but it also seems to load a copy at 0x0, and that's the one that it
 * actually jumps to. Therefore, we need to set CONFIG_SYS_TEXT_BASE to 0 so
 * that u-boot agrees with where it's actually executing. Put the stack at
 * this location, which is between the two u-boots in the DRAM.
 */
#define CONFIG_SYS_INIT_SP_ADDR		0x00D00000

#include "mv-common.h"

/*
 * Environment variables configurations
 */

#endif	/* __CONFIG_ASPENITE_H */
