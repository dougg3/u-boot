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
#include <serial.h>
#include <dm.h>
#include <linux/delay.h>

DECLARE_GLOBAL_DATA_PTR;

int board_early_init_f(void)
{
	u32 mfp_cfg[] = {
		/* MMC2 - WiFi */
		(MFP90  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC2_DAT3 */
		(MFP91  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC2_DAT2 */
		(MFP92  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC2_DAT1 */
		(MFP93  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC2_DAT0 */
		(MFP94  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC2_CMD */
		(MFP95  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC2_CLK */
		/* MMC3 - SD card */
		(MFP0   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_D7 */
		(MFP1   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_D6 */
		(MFP2   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_D5 */
		(MFP3   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_D4 */
		(MFP4   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_D3 */
		(MFP5   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_D2 */
		(MFP6   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_D1 */
		(MFP7   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_D0 */
		(MFP8   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC3_CLK */
		(MFP9   | MFP_AF6 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC3_CMD */
		/* LCD */
		/* Note: This block of pins is special. MFP56-59 drive strength settings are applied to
		 * the entire block of pins from 56 to 85. */
		(MFP56  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_FCLK_RD */
		(MFP57  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_LCLK_A0 */
		(MFP58  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_PCLK_WR */
		(MFP59  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DENA_BIAS */
		/* Note: MFP60 through 62 are special and also control the voltage domains (VDD_IO0-4).
		 * We want VDD_IO0, 1, 2, and 4 to be 3.3V, and VDD_IO3 to be 1.8V. This, we want:
		 * MFPR_60[10], MFPR_60[11], MFPR_61[10], and MFPR_62[11] to be 1,
		 * and MFPR_61[11] to be 0. This corresponds to the following settings:
		 * MFP60 = fast (11:10 = 3), MFP61 = slow (11:10 = 1), MFP62 = medium (11:10 = 2).
		 * Based on looking at old code, I believe the meanings of these bis was reversed on
		 * prerelease S0 silicon. I'm only worrying about supporting A0 and newer silicon. */
		(MFP60  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD0 */
		(MFP61  | MFP_AF1 | MFP_DRIVE_SLOW   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD1 */
		(MFP62  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD2 */
		(MFP63  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD3 */
		(MFP64  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD4 */
		(MFP65  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD5 */
		(MFP66  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD6 */
		(MFP67  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD7 */
		(MFP68  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD8 */
		(MFP69  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD9 */
		(MFP70  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD10 */
		(MFP71  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD11 */
		(MFP72  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD12 */
		(MFP73  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD13 */
		(MFP74  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD14 */
		(MFP75  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD15 */
		(MFP76  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD16 */
		(MFP77  | MFP_AF1 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_DD17 */
		(MFP84  | MFP_AF0 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_PWM */
		(MFP85  | MFP_AF0 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* LCD_VBUS */
		/* I2C */
		(MFP105 | MFP_AF1 | MFP_DRIVE_SLOW   | 0                 | MFP_PULL_HIGH), /* I2C_SDA */
		(MFP106 | MFP_AF1 | MFP_DRIVE_SLOW   | 0                 | MFP_PULL_HIGH), /* I2C_SCL */
		/* UART1 */
		(MFP107 | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* UART1_TXD */
		(MFP108 | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* UART1_RXD */
		/* UART3 */
		(MFP98  | MFP_AF2 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* UART3_TXD */
		(MFP99  | MFP_AF2 | MFP_DRIVE_MEDIUM | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* UART3_RXD */
		/* Misc */
		(MFP16  | MFP_AF0 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* GPIO16 */

#if 0
		/* Misc, from Chumby original u-boot */
		(MFP19  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nCE1 */
		(MFP20  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nCE2 */
		(MFP22  | MFP_AF0 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nOE */
		(MFP23  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nALE */
		(MFP25  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nRESET */
		(MFP26  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* ND_RnB1 */
		(MFP27  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* ND_RnB2 */
		(MFP28  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_RDY */
		(MFP29  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_STSCH */
		(MFP30  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nREG */
		(MFP31  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nIOIS16 */
		(MFP32  | MFP_AF5 | MFP_DRIVE_FAST   | 0                 | MFP_PULL_NONE), /* CF_nCD1 */
		(MFP33  | MFP_AF5 | MFP_DRIVE_FAST   | 0                 | MFP_PULL_NONE), /* CF_nCD2 */
		(MFP34  | MFP_AF2 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nSPKR */
		(MFP35  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_INPACK */
		(MFP36  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* CF_nWAIT */
		(MFP42  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MSP_BS */
		(MFP44  | MFP_AF2 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MSP_DAT3 */
		(MFP50  | MFP_AF3 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MSP_SCLK */
		(MFP102 | MFP_AF3 | MFP_DRIVE_FAST   | 0                 | MFP_PULL_HIGH), /* SSP4_CLK */
		(MFP103 | MFP_AF3 | MFP_DRIVE_FAST   | 0                 | MFP_PULL_HIGH), /* SSP4_FRM */
		(MFP104 | MFP_AF4 | MFP_DRIVE_FAST   | 0                 | MFP_PULL_HIGH), /* SSP4_TXD */
		/* MMC1 (unused?) */
		(MFP37  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC1_DAT7 */
		(MFP38  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC1_DAT6 */
		(MFP40  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC1_DAT1 */
		(MFP41  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC1_DAT0 */
		(MFP43  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC1_CLK */
		(MFP46  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC1_WP */
		(MFP48  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC1_DAT4 */
		(MFP49  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC1_CMD */
		(MFP51  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC1_DAT3 */
		(MFP52  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_HIGH), /* MMC1_DAT2 */
		(MFP53  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC1_CD */
		(MFP54  | MFP_AF1 | MFP_DRIVE_FAST   | MFP_LPM_EDGE_NONE | MFP_PULL_NONE), /* MMC1_DAT5 */
#endif

		MFP_EOC		/*End of configureation*/
	};
	/* configure MFP's */
	mfp_config(mfp_cfg);

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

static void write_coproc_data(struct udevice *dev, const char *cmd, int ch_delay)
{
	struct dm_serial_ops *ops = serial_get_ops(dev);
	int err;

	while (*cmd)
	{
		udelay(ch_delay);
		do {
			err = ops->putc(dev, *cmd);
		} while (err == -EAGAIN);
		cmd++;
	}

	while (ops->pending(dev, false));
}

static void send_coproc_command(const char *cmd)
{
	u32 serial_config = SERIAL_DEFAULT_CONFIG;
	struct udevice *dev = NULL;
	struct dm_serial_ops *ops = NULL;
	int err;
	int q_found;

	/* Power on UART3 with 14.7456 MHz clock */
	struct armd1apb1_registers *apb1clkres =
		(struct armd1apb1_registers *)ARMD1_APBC1_BASE;
	writel(APBC_APBCLK | APBC_FNCLK | APBC_FNCLKSEL(1), &apb1clkres->uart3);

	if (uclass_get_device_by_seq(UCLASS_SERIAL, 2, &dev)) {
		log_err("unable to find coprocessor UART\n");
		return;
	}

	ops = serial_get_ops(dev);
	if (!ops) {
		log_err("coproc UART missing ops\n");
		return;
	}

	/* Initialize UART3 to 115200, 8N1 */
	ops->setconfig(dev, serial_config);
	ops->setbrg(dev, 115200);

	/* Try several times; with no flow control, command could get lost */
	for (int i = 0; i < 8; i++) {
		/* Flush UART */
		do {
			err = ops->getc(dev);
		} while (err != -EAGAIN);

		/* Try a few times: Send "!!!!", listen for '?' */
		q_found = 0;
		for (int j = 0; j < 4 && !q_found; j++) {
			write_coproc_data(dev, "!!!!", 0);
			for (int k = 0; k < 10 && !q_found; k++) {
				if (ops->getc(dev) == '?') {
					q_found = 1;
					break;
				}
				udelay(300);
			}
		}

		/* Now send the actual command */
		write_coproc_data(dev, cmd, 1000);
	}
}

void reset_misc(void)
{
	/* In order to reset the chumby, we have to send a command on UART3 */
	send_coproc_command("RSET\n\r");
	/* if we get here, the coprocessor didn't react */
	log_err("reboot request failed");
}

void do_poweroff(void)
{
	/* Poweroff is similar to reset, with a different command */
	send_coproc_command("DOWN\n\r");
	/* if we get here, the coprocessor didn't react */
	log_err("poweroff request failed");
}
