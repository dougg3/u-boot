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
#include <dm/device_compat.h>

#ifndef MV_MAX_GPIO
#define MV_MAX_GPIO	128
#endif

struct mv_gpio_priv {
	u32 base;
	u32 count;
};

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

static struct mv_gpio_priv mv_gpio = {
	.base = MV_GPIO_BASE,
	.count = MV_MAX_GPIO,
};

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

static int _gpio_direction_input(struct mv_gpio_priv const *priv, unsigned int gpio)
{
	struct mvgpio_reg *regs;
	regs = get_gpio_base(priv->base, GPIO_TO_REG(gpio));
	writel(GPIO_TO_BIT(gpio), &regs->gcdr);
	return 0;
}

static int _gpio_get_value(struct mv_gpio_priv const *priv, unsigned int gpio)
{
	u32 gpio_val;
	struct mvgpio_reg *regs;
	regs = get_gpio_base(priv->base, GPIO_TO_REG(gpio));
	gpio_val = readl(&regs->gplr);
	return GPIO_VAL(gpio, gpio_val);
}

static int _gpio_set_value(struct mv_gpio_priv const *priv, unsigned int gpio, int value)
{
	struct mvgpio_reg *regs;
	regs = get_gpio_base(priv->base, GPIO_TO_REG(gpio));
	if (value)
		writel(GPIO_TO_BIT(gpio), &regs->gpsr);
	else
		writel(GPIO_TO_BIT(gpio), &regs->gpcr);

	return 0;
}

static int _gpio_direction_output(struct mv_gpio_priv const *priv, unsigned int gpio, int value)
{
	struct mvgpio_reg *regs;
	regs = get_gpio_base(priv->base, GPIO_TO_REG(gpio));
	writel(GPIO_TO_BIT(gpio), &regs->gsdr);
	_gpio_set_value(priv, gpio, value);
	return 0;
}

#if !CONFIG_IS_ENABLED(DM_GPIO)

int gpio_direction_input(unsigned int gpio)
{
	return _gpio_direction_input(&mv_gpio, gpio);
}

int gpio_direction_output(unsigned int gpio, int value)
{
	return _gpio_direction_output(&mv_gpio, gpio, value);
}

int gpio_get_value(unsigned int gpio)
{
	return _gpio_get_value(&mv_gpio, gpio);
}

int gpio_set_value(unsigned int gpio, int value)
{
	return _gpio_set_value(&mv_gpio, gpio, value);
}

#else /* DM_GPIO */

static int _gpio_get_dir(struct mv_gpio_priv const *priv, unsigned int gpio)
{
	u32 dir_val;
	struct mvgpio_reg *regs;
	regs = get_gpio_base(priv->base, GPIO_TO_REG(gpio));
	dir_val = readl(&regs->gpdr);
	return GPIO_VAL(gpio, dir_val);
}

static int mv_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct mv_gpio_priv *priv = dev_get_priv(dev);
	return _gpio_get_value(priv, offset);
}

static int mv_gpio_set_value(struct udevice *dev, unsigned int offset, int value)
{
	struct mv_gpio_priv *priv = dev_get_priv(dev);
	return _gpio_set_value(priv, offset, value);
}

static int mv_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct mv_gpio_priv *priv = dev_get_priv(dev);
	return _gpio_direction_input(priv, offset);
}

static int mv_gpio_direction_output(struct udevice *dev, unsigned int offset, int value)
{
	struct mv_gpio_priv *priv = dev_get_priv(dev);
	return _gpio_direction_output(priv, offset, value);
}

static int mv_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	int dir;
	struct mv_gpio_priv *priv = dev_get_priv(dev);
	dir = _gpio_get_dir(priv, offset);
	return dir ? GPIOF_OUTPUT : GPIOF_INPUT;
}

static const struct dm_gpio_ops gpio_mv_ops = {
	.direction_input = mv_gpio_direction_input,
	.direction_output = mv_gpio_direction_output,
	.get_value = mv_gpio_get_value,
	.set_value = mv_gpio_set_value,
	.get_function = mv_gpio_get_function,
};

static int mv_gpio_probe(struct udevice *dev)
{
	struct mv_gpio_priv *priv = dev_get_priv(dev);
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	uc_priv->gpio_count = priv->count;
	uc_priv->bank_name = dev->name;

	return 0;
}

static int mv_gpio_of_to_plat(struct udevice *dev)
{
	struct mv_gpio_priv *priv = dev_get_priv(dev);

	priv->base = dev_read_addr(dev);
	if (priv->base == FDT_ADDR_T_NONE) {
		dev_err(dev, "mvgpio: unable to map registers\n");
		return -EINVAL;
	}

	priv->count = 128;

	return 0;
}

static const struct udevice_id mv_gpio_ids[] = {
	{ .compatible = "marvell,mmp-gpio" },
	{ }
};

U_BOOT_DRIVER(mv_gpio) = {
	.name	= "mv_gpio",
	.id		= UCLASS_GPIO,
	.ops	= &gpio_mv_ops,
	.of_match = mv_gpio_ids,
	.probe	= mv_gpio_probe,
	.of_to_plat = mv_gpio_of_to_plat,
	.priv_auto	= sizeof(struct mv_gpio_priv),
};

#endif
