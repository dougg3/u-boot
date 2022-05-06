// SPDX-License-Identifier: GPL-2.0+
/*
 * Video driver for Marvell PXA168 SoC
 * Very similar to pxa168_lcd.c driver, but slight register differences.
 * Currently hardcoded for settings needed for Chumby 8.
 *
 * Initialization of LCD interface and setup of SPLASH screen image
 */

#include <common.h>
#include <dm.h>
#include <part.h>
#include <video.h>
#include <asm/cache.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/mbus.h>
#include <asm/io.h>
#include <asm/arch/cpu.h>

#define APMU_BASE					0xD4282800
#define APMU_LCD_CLK_RES_CTRL		0x004C

#define LCD_CFG_GRA_START_ADDR0		0x00F4
#define LCD_CFG_GRA_START_ADDR1		0x00F8
#define LCD_CFG_GRA_PITCH			0x00FC
#define LCD_SPU_GRA_HPXL_VLN		0x0104
#define LCD_SPU_GZM_HPXL_VLN		0x0108
#define LCD_SPU_V_H_TOTAL			0x0114
#define LCD_SPU_V_H_ACTIVE			0x0118
#define LCD_SPU_H_PORCH				0x011C
#define LCD_SPU_V_PORCH				0x0120
#define LCD_SPU_DMA_CTRL0			0x0190
#define LCD_SPU_DMA_CTRL1			0x0194
#define LCD_CFG_SCLK_DIV			0x01A8
#define LCD_SPU_CONTRAST			0x01AC
#define LCD_SPU_SATURATION			0x01B0
#define LCD_SPU_CBSH_HUE			0x01B4
#define LCD_SPU_DUMB_CTRL			0x01B8

enum {
	/* Maximum LCD size we support */
	LCD_MAX_WIDTH		= 800,
	LCD_MAX_HEIGHT		= 600,
	LCD_MAX_LOG2_BPP	= VIDEO_BPP32,
};

struct pxa168_lcd_info {
	u32 fb_base;
	struct display_timing timing;
};

struct pxa168_video_priv {
	uintptr_t regs;
};

/* Initialize LCD registers */
static void pxa168_lcd_register_init(struct pxa168_lcd_info *lcd_info,
				    uintptr_t regs)
{
	/* Local variable for easier handling */
	int x = lcd_info->timing.hactive.typ;
	int y = lcd_info->timing.vactive.typ;
	int x_total = lcd_info->timing.hactive.typ +
	              lcd_info->timing.hfront_porch.typ +
	              lcd_info->timing.hback_porch.typ +
	              lcd_info->timing.hsync_len.typ;
	int y_total = lcd_info->timing.vactive.typ +
	              lcd_info->timing.vfront_porch.typ +
	              lcd_info->timing.vback_porch.typ +
	              lcd_info->timing.vsync_len.typ;

	/* Enable LCD controller clock */
	writel(0x007F, APMU_BASE + APMU_LCD_CLK_RES_CTRL);

	/* Ensure DMA controller is disabled */
	writel(0x00000000, regs + LCD_SPU_DMA_CTRL0);

	/*
	 * Set the LCD_CFG_GRA_START_ADDR0/1 Registers
	 * Window 1 will be disabled; only bother filling out window 0
	 */
	writel(lcd_info->fb_base, regs + LCD_CFG_GRA_START_ADDR0);
	writel(0, regs + LCD_CFG_GRA_START_ADDR1);

	/*
	 * Set the LCD_CFG_GRA_PITCH Register
	 * (Line length * 4 bytes per line)
	 * Backlight control in this register is unused.
	 */
	writel(4 * x, regs + LCD_CFG_GRA_PITCH);

	/*
	 * Set the LCD_SPU_GRA_HPXL_VLN Register
	 * Bits 31-16: Vertical size of graphical overlay
	 * Bits 15-00: Horizontal size of graphical overlay
	 * Values before zooming
	 */
	writel((y << 16) | x, regs + LCD_SPU_GRA_HPXL_VLN);

	/*
	 * Set the LCD_SPU_GZM_HPXL_VLN Register
	 * Bits 31-16: Vertical size of graphical overlay
	 * Bits 15-00: Horizontal size of graphical overlay
	 * Values after zooming
	 */
	writel((y << 16) | x, regs + LCD_SPU_GZM_HPXL_VLN);

	/*
	 * Set the LCD_SPU_V_H_ACTIVE Register
	 * Bits 31-16: Screen active vertical lines
	 * Bits 15-00: Screen active horizontal pixels
	 */
	writel((y << 16) | x, regs + LCD_SPU_V_H_ACTIVE);

	/*
	 * Set the LCD_SPU_V_H_TOTAL Register
	 * Bits 31-16: Screen total vertical lines (including blanking)
	 * Bits 15-00: Screen total horizontal pixels (including blanking)
	 */
	writel((y_total << 16) | x_total, regs + LCD_SPU_V_H_TOTAL);

	/*
	 * Set the LCD_SPU_H_PORCH Register
	 * Bits 31-16: Screen horizontal backporch
	 * Bits 15-00: Screen horizontal frontporch
	 */
	writel((lcd_info->timing.hback_porch.typ << 16) |
	       lcd_info->timing.hfront_porch.typ,
	       regs + LCD_SPU_H_PORCH);

	/*
	 * Set the LCD_SPU_V_PORCH Register
	 * Bits 31-16: Screen vertical backporch
	 * Bits 15-00: Screen vertical frontporch
	 */
	writel((lcd_info->timing.vback_porch.typ << 16) |
	       lcd_info->timing.vfront_porch.typ,
	       regs + LCD_SPU_V_PORCH);

	/*
	 * Set the LCD_SPU_DMA_CTRL1 Register
	 * Bit     31: Manual DMA Trigger = 0
	 * Bits 30-28: DMA Trigger Source: 0x2 VSYNC
	 * Bit     27: VSYNC_INV: 0=Rising Edge
	 * Bits 26-24: Color Key Mode: 0=disable
	 * Bit     23: Fill low bits: 0=fill with zeroes
	 * Bit     22: Disable line buffer
	 * Bit     21: Gated Clock: 0=disable
	 * Bit     20: Power Save enable: 0=disable
	 * Bits 19-18: Scaling: 0=No scaling down
	 * Bits 17-16: Configure Video/Graphic Path: 0x1: Graphic path alpha.
	 * Bits 15-08: Configure Alpha: 0xFF.
	 * Bits 07-00: Pixel command, should be set to 0x81.
	 */
	writel(0x2001FF81, regs + LCD_SPU_DMA_CTRL1);

	/*
	 * Set LCD_SPU_SATURATION
	 * Bits 31-16: Multiplier signed 3.13 fixed point value
	 * Bits 15-00: Saturation signed 3.13 fixed point value
	 */
	writel(0x20004000, regs + LCD_SPU_SATURATION);

	/*
	 * Set LCD_SPU_CONTRAST
	 * Bits 31-16: Brightness sign ext. 8-bit value +255 to -255
	 * Bits 15-00: Contrast signed 2.14 fixed point value
	 */
	writel(0x00004000, regs + LCD_SPU_CONTRAST);

	/*
	 * Set LCD_SPU_HUE
	 * Bits 31-16: Sine signed 2.14 fixed point value
	 * Bits 15-00: Cosine signed 2.14 fixed point value
	 */
	writel(0x00004000, regs + LCD_SPU_CBSH_HUE);

	/*
	 * Set LCD_SPU_DUMB_CTRL
	 * Bits 31-28: LCD Type: 2=18 bit RGB on LDD[17:0]
	 * Bits 27-20: GPIO output data (disabled)
	 * Bits 19-12: GPIO data enable (disabled)
	 * Bit     11: Reserved
	 * Bit     10: Don't swap DENA and BIAS
	 * Bit      9: Don't disable I/O pads
	 * Bit      8: Dumb backlight control pin off
	 * Bit      7: Don't reverse RGB bit order
	 * Bit      6: Don't invert composite blank signal
	 * Bit      5: Don't invert composite sync signal
	 * Bit      4: Don't invert pixel valid enable
	 * Bit      3: Invert VSYNC
	 * Bit      2: Invert HSYNC
	 * Bit      1: Invert Pixel Clock
	 * Bit      0: Enable LCD Panel
	 */
	writel(0x2000000F, regs + LCD_SPU_DUMB_CTRL);

	/*
	 * Set the LCD_CFG_SCLK_DIV Register
	 * Bit     31: SCLK source: 1 = 312 MHz PLL clock
	 * Bit     30: Not used when 312 MHz PLL is selected
	 * Bit     29: SCLK updates after vsync
	 * Bit     28: Not used when 312 MHz PLL is selected
	 * Bits 27-16: Clock fraction divider = 0
	 * Bits 15-00: Clock Divider = 7. So clock is 312/7 = 44.571428571 MHz
	 * See page 475 in section 28.5.
	 */
	writel(0x80000007, regs + LCD_CFG_SCLK_DIV);

	/*
	 * Set the LCD_SPU_DMACTRL 0 Register
	 * Bit     31: Overlay blending enabled
	 * Bit     30: Gamma correction disabled
	 * Bit     29: Video Contrast/Saturation/Hue Adjust disabled
	 * Bit     28: Color palette disabled
	 * Bit     27: DMA AXI Arbiter enable
	 * Bit     26: HW cursor setting not used because disabled
	 * Bit     25: HW Cursor 2-bit setting not used because disabled
	 * Bit     24: HW Cursor disabled
	 * Bits 23-20: Video Memory Color Format: 0x4=RGBA888
	 * Bits 19-16: Graphics Memory Color Format:    0x4=RGBA888
	 * Bit     15: Memory Toggle between frame 0 and 1: 0=disable
	 * Bit     14: Graphics horizontal scaling enable: 0=disable
	 * Bit     13: Graphics test mode: 0=disable
	 * Bit     12: Graphics SWAP R and B: 1=enable
	 * Bit     11: Graphics SWAP U and V: 0=disable
	 * Bit     10: Graphics SWAP Y and U/V: 0=disable
	 * Bit     09: Graphic YUV to RGB Conversion: 0=disable
	 * Bit     08: Graphic Transfer: 1=enable
	 * Bit     07: Memory Toggle: 0=disable
	 * Bit     06: Video horizontal smooth enable: 0=disable
	 * Bit     05: Video test mode: 0=disable
	 * Bit     04: Video SWAP R and B: 1=enable
	 * Bit     03: Video SWAP U and V: 0=disable
	 * Bit     02: Video SWAP Y and U/V: 0=disable
	 * Bit     01: Video YUV to RGB Conversion: 0=disable
	 * Bit     00: Video  Transfer: 1=enable
	 */
	writel(0x08441111, regs + LCD_SPU_DMA_CTRL0);
}

static int pxa168_video_probe(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);
	struct video_priv *uc_priv = dev_get_uclass_priv(dev);
	struct pxa168_video_priv *priv = dev_get_priv(dev);
	struct pxa168_lcd_info lcd_info;
	u32 fb_start, fb_end;
	int ret;

	priv->regs = dev_read_addr(dev);
	if (priv->regs == FDT_ADDR_T_NONE) {
		dev_err(dev, "failed to get LCD address\n");
		return -ENXIO;
	}

	ret = ofnode_decode_display_timing(dev_ofnode(dev), 0, &lcd_info.timing);
	if (ret) {
		dev_err(dev, "failed to get any display timings\n");
		return -EINVAL;
	}

	lcd_info.fb_base = plat->base;

	/* Initialize the LCD controller */
	pxa168_lcd_register_init(&lcd_info, priv->regs);

	/* Enable dcache for the frame buffer */
	fb_start = plat->base & ~(MMU_SECTION_SIZE - 1);
	fb_end = plat->base + plat->size;
	fb_end = ALIGN(fb_end, 1 << MMU_SECTION_SHIFT);
	mmu_set_region_dcache_behaviour(fb_start, fb_end - fb_start,
					DCACHE_WRITEBACK);
	video_set_flush_dcache(dev, true);

	uc_priv->xsize = lcd_info.timing.hactive.typ;
	uc_priv->ysize = lcd_info.timing.vactive.typ;
	uc_priv->bpix = VIDEO_BPP32;	/* Uses RGBA32 format */

	return 0;
}

static int pxa168_video_bind(struct udevice *dev)
{
	struct video_uc_plat *plat = dev_get_uclass_plat(dev);

	plat->size = LCD_MAX_WIDTH * LCD_MAX_HEIGHT *
		(1 << LCD_MAX_LOG2_BPP) / 8;

	return 0;
}

static const struct udevice_id pxa168_video_ids[] = {
	{ .compatible = "marvell,pxa168-lcd" },
	{ }
};

U_BOOT_DRIVER(pxa168_video) = {
	.name	= "pxa168_video",
	.id	= UCLASS_VIDEO,
	.of_match = pxa168_video_ids,
	.bind	= pxa168_video_bind,
	.probe	= pxa168_video_probe,
	.priv_auto	= sizeof(struct pxa168_video_priv),
};
