/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 */

#ifndef _ASM_ARCH_GPIO_H
#define _ASM_ARCH_GPIO_H

#include <asm/types.h>
#include <asm/arch/armada100.h>

#define GPIO_HIGH		1
#define GPIO_LOW		0

#define GPIO_TO_REG(gp)		(gp >> 5)
#define GPIO_TO_BIT(gp)		(1 << (gp & 0x1F))
#define GPIO_VAL(gp, val)	((val >> (gp & 0x1F)) & 0x01)
#define MV_GPIO_BASE		ARMD1_GPIO_BASE

#endif /* _ASM_ARCH_GPIO_H */
