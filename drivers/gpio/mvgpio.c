// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2011
 * eInfochips Ltd. <www.einfochips.com>
 * Written-by: Ajay Bhargav <contact@8051projects.net>
 *
 * (C) Copyright 2010
 * Marvell Semiconductor <www.marvell.com>
 */

#include <common.h>
#include <dm.h>
#include <fdtdec.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/errno.h>
#include "mvgpio.h"
#include <asm/gpio.h>
#include <dt-bindings/gpio/gpio.h>

#ifndef MV_MAX_GPIO
#define MV_MAX_GPIO	128
#endif

static inline void *get_gpio_base(u32 base, int bank)
{
	const unsigned int offset[4] = {0, 4, 8, 0x100};
	/* gpio register bank offset - refer Appendix A.36 */
	return (struct mvgpio_reg *)(base + offset[bank]);
}

#if !CONFIG_IS_ENABLED(DM_GPIO)
#include <asm/arch/armada100.h>
#if !defined(MV_GPIO_BASE)
#error You cannot build this driver for the selected architecture.
#endif

int gpio_request(unsigned int gpio, const char *label)
{
	if (gpio >= MV_MAX_GPIO) {
		printf("%s: Invalid GPIO requested %d\n", __func__, gpio);
		return -1;
	}
	return 0;
}

int gpio_free(unsigned gpio)
{
	return 0;
}
#endif /* DM_GPIO */

static int _gpio_direction_input(u32 base, unsigned int gpio)
{
	struct mvgpio_reg *regs;
	regs = get_gpio_base(base, GPIO_TO_REG(gpio));
	writel(GPIO_TO_BIT(gpio), &regs->gcdr);
	return 0;
}

static int _gpio_get_value(u32 base, unsigned int gpio)
{
	u32 gpio_val;
	struct mvgpio_reg *regs;
	regs = get_gpio_base(base, GPIO_TO_REG(gpio));
	gpio_val = readl(&regs->gplr);
	return GPIO_VAL(gpio, gpio_val);
}

static int _gpio_set_value(u32 base, unsigned int gpio, int value)
{
	struct mvgpio_reg *regs;
	regs = get_gpio_base(base, GPIO_TO_REG(gpio));
	if (value)
		writel(GPIO_TO_BIT(gpio), &regs->gpsr);
	else
		writel(GPIO_TO_BIT(gpio), &regs->gpcr);

	return 0;
}

static int _gpio_direction_output(u32 base, unsigned int gpio, int value)
{
	struct mvgpio_reg *regs;
	regs = get_gpio_base(base, GPIO_TO_REG(gpio));
	writel(GPIO_TO_BIT(gpio), &regs->gsdr);
	_gpio_set_value(base, gpio, value);
	return 0;
}

#if !CONFIG_IS_ENABLED(DM_GPIO)

int gpio_direction_input(unsigned int gpio)
{
	return _gpio_direction_input(MV_GPIO_BASE, gpio);
}

int gpio_direction_output(unsigned int gpio, int value)
{
	return _gpio_direction_output(MV_GPIO_BASE, gpio, value);
}

int gpio_get_value(unsigned int gpio)
{
	return _gpio_get_value(MV_GPIO_BASE, gpio);
}

int gpio_set_value(unsigned int gpio, int value)
{
	return _gpio_set_value(MV_GPIO_BASE, gpio, value);
}

#else /* DM_GPIO */

static int _gpio_get_dir(u32 base, unsigned int gpio)
{
	u32 dir_val;
	struct mvgpio_reg *regs;
	regs = get_gpio_base(base, GPIO_TO_REG(gpio));
	dir_val = readl(&regs->gpdr);
	return GPIO_VAL(gpio, dir_val);
}

#error Implement this next...

#endif
