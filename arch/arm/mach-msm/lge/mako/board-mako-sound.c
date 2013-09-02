/* Copyright (c) 2012, LGE Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/mfd/pm8xxx/pm8921.h>
#include <linux/regulator/consumer.h>
#include <linux/platform_data/hds_fsa8008.h>
#include <linux/console.h>
#include <mach/board_lge.h>
#include "devices.h"

#ifdef CONFIG_SND_SOC_ES310
#include <sound/es310.h>
#endif

#include "board-mako.h"
#include "board-mako-earjack-debugger.h"

#define ES310_ADDRESS (0x3E)

#define GPIO_EAR_MIC_BIAS_EN        PM8921_GPIO_PM_TO_SYS(20)
#define GPIO_EAR_SENSE_N            82
#define GPIO_EAR_SENSE_N_REV11      81
#define GPIO_EAR_MIC_EN             PM8921_GPIO_PM_TO_SYS(31)
#define GPIO_EARPOL_DETECT          PM8921_GPIO_PM_TO_SYS(32)
#define GPIO_EAR_KEY_INT            83

#ifdef CONFIG_SND_SOC_ES310
static int es310_power_setup(int on)
{
	int rc;
	struct regulator *es310_vdd;

	es310_vdd = regulator_get(NULL, "8921_l15");

	if (IS_ERR(es310_vdd)) {
		printk(KERN_ERR "%s: Unable to get regulator es310_vdd\n",
		       __func__);
		return -1;
	}

	rc = regulator_set_voltage(es310_vdd, 2800000, 2800000);
	if (rc) {
		printk(KERN_ERR "%s: regulator set voltage failed\n", __func__);
		return rc;
	}

	rc = regulator_enable(es310_vdd);
	if (rc) {
		printk(KERN_ERR
		       "%s: Error while enabling regulator for es310 %d\n",
		       __func__, rc);
		return rc;
	}

	return 0;
}

#define MITWO_GPIO_ES310_CLK		34
#define MITWO_GPIO_ES310_RESET		37
#define MITWO_GPIO_ES310_MIC_SWITCH	38
#define MITWO_GPIO_ES310_WAKEUP		51
static struct es310_platform_data audience_es310_pdata = {
	.gpio_es310_reset = MITWO_GPIO_ES310_RESET,
	.gpio_es310_clk = MITWO_GPIO_ES310_CLK,
	.gpio_es310_wakeup = MITWO_GPIO_ES310_WAKEUP,
	.gpio_es310_mic_switch = MITWO_GPIO_ES310_MIC_SWITCH,
	.power_on = es310_power_setup,
};
#endif

static struct i2c_board_info msm_i2c_audiosubsystem_info[] = {
#ifdef CONFIG_SND_SOC_ES310
	{
		I2C_BOARD_INFO("audience_es310", ES310_ADDRESS),
		.platform_data = &audience_es310_pdata,
	}
#endif
};

static struct i2c_registry msm_i2c_audiosubsystem __initdata = {
	/* Add the I2C driver for Audio Amp */
	I2C_FFA,
	APQ_8064_GSBI1_QUP_I2C_BUS_ID,
	msm_i2c_audiosubsystem_info,
	ARRAY_SIZE(msm_i2c_audiosubsystem_info),
};

static void __init lge_add_i2c_es310_devices(void)
{
	/* Run the array and install devices as appropriate */
	i2c_register_board_info(msm_i2c_audiosubsystem.bus,
				msm_i2c_audiosubsystem.info,
				msm_i2c_audiosubsystem.len);
}

#ifdef CONFIG_SWITCH_FSA8008
static void enable_external_mic_bias(int status)
{
	static struct regulator *reg_mic_bias = NULL;
	static int prev_on;
	int rc = 0;

	if (status == prev_on)
		return;

	if (lge_get_board_revno() > HW_REV_C) {
		if (!reg_mic_bias) {
			reg_mic_bias = regulator_get(NULL, "mic_bias");
			if (IS_ERR(reg_mic_bias)) {
				pr_err("%s: could not regulator_get\n",
						__func__);
				reg_mic_bias = NULL;
				return;
			}
		}

		if (status) {
			rc = regulator_enable(reg_mic_bias);
			if (rc)
				pr_err("%s: regulator enable failed\n",
						__func__);
			pr_debug("%s: mic_bias is on\n", __func__);
		} else {
			rc = regulator_disable(reg_mic_bias);
			if (rc)
				pr_warn("%s: regulator disable failed\n",
						__func__);
			pr_debug("%s: mic_bias is off\n", __func__);
		}
	}

	if (lge_get_board_revno() < HW_REV_1_0)
		gpio_set_value_cansleep(GPIO_EAR_MIC_BIAS_EN, status);
	prev_on = status;
}

static int console_enabled = 1;
int mako_console_stopped(void)
{
	return !console_enabled;
}

static void set_uart_console(int enable)
{
	static struct console *uart_con = NULL;

	console_enabled = enable;

	if (!uart_con) {
		struct console *con;
		for_each_console(con) {
			if (!strcmp(con->name, "ttyHSL")) {
				uart_con = con;
				break;
			}
		}
	}

	if (!uart_con) {
		pr_debug("%s: no uart console\n", __func__);
		return;
	}

	if (enable)
		console_start(uart_con);
	else
		console_stop(uart_con);
}

static struct fsa8008_platform_data lge_hs_pdata = {
	.switch_name = "h2w",
	.keypad_name = "hs_detect",

	.key_code = KEY_MEDIA,

	.gpio_detect = GPIO_EAR_SENSE_N,
	.gpio_mic_en = GPIO_EAR_MIC_EN,
	.gpio_mic_bias_en = GPIO_EAR_MIC_BIAS_EN,
	.gpio_jpole  = GPIO_EARPOL_DETECT,
	.gpio_key    = GPIO_EAR_KEY_INT,

	.latency_for_detection = 75,
	.set_headset_mic_bias = enable_external_mic_bias,
	.set_uart_console = set_uart_console,
};

static __init void mako_fixed_audio(void)
{

	if (lge_get_board_revno() >= HW_REV_1_0)
		lge_hs_pdata.gpio_mic_bias_en = -1;
	if (lge_get_board_revno() > HW_REV_1_0) {
		lge_hs_pdata.gpio_detect = GPIO_EAR_SENSE_N_REV11;
		lge_hs_pdata.gpio_detect_can_wakeup = 1;
	}
}

static struct platform_device lge_hsd_device = {
	.name = "fsa8008",
	.id   = -1,
	.dev = {
		.platform_data = &lge_hs_pdata,
	},
};
#else
int mako_console_stopped(void)
{
	return 0;
}
#endif

#ifdef CONFIG_EARJACK_DEBUGGER
#define GPIO_EARJACK_DEBUGGER_TRIGGER       PM8921_GPIO_PM_TO_SYS(13)
static struct earjack_debugger_platform_data earjack_debugger_pdata = {
	.gpio_trigger = GPIO_EARJACK_DEBUGGER_TRIGGER,
	.set_uart_console = set_uart_console,
};

static struct platform_device earjack_debugger_device = {
	.name = "earjack-debugger-trigger",
	.id = -1,
	.dev = {
		.platform_data = &earjack_debugger_pdata,
	},
};
#endif

static struct platform_device *sound_devices[] __initdata = {
#ifdef CONFIG_SWITCH_FSA8008
	&lge_hsd_device,
#endif
#ifdef CONFIG_EARJACK_DEBUGGER
	&earjack_debugger_device,
#endif
};

void __init lge_add_sound_devices(void)
{
#ifdef CONFIG_SWITCH_FSA8008
	mako_fixed_audio();
#endif
	lge_add_i2c_es310_devices();
	platform_add_devices(sound_devices, ARRAY_SIZE(sound_devices));
}
