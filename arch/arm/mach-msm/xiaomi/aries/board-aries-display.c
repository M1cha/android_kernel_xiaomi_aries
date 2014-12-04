/* Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
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

#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/bootmem.h>
#include <linux/msm_ion.h>
#include <asm/mach-types.h>
#include <mach/msm_memtypes.h>
#include <mach/board.h>
#include <mach/gpiomux.h>
#include <mach/ion.h>
#include <mach/msm_bus_board.h>
#include <mach/socinfo.h>

#if defined(CONFIG_LEDS_LM3530_ARIES)
#include <linux/led-lm3530-aries.h>
#endif

#if defined(CONFIG_FB_MSM_HDMI_MHL_9244)
#include "mhl_api.h"
#endif

#include "devices.h"
#include "board-8064.h"


/* power: ldo23 1.8  gpio11 vspvsn */
/* reset: gpio25 */
#define MI_RESET_GPIO		PM8921_GPIO_PM_TO_SYS(25)
#define MI_LCD_ID_GPIO		PM8921_GPIO_PM_TO_SYS(12)

static bool dsi_mi_power_on;
static int mi_panel_id = 0xF;

int mipanel_id(void)
{
	return mi_panel_id;
}

void mipanel_set_id(int id)
{
	mi_panel_id = id;
}

static int mipi_dsi_mipanel_power(int on)
{
	static struct regulator *reg_l23, *reg_l2, *reg_lvs7, *reg_vsp;
	static int reset_gpio, lcd_id_gpio;
	int rc;

	if (!dsi_mi_power_on) {
		reg_lvs7 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_vddio");
		if (IS_ERR_OR_NULL(reg_lvs7)) {
			pr_err("could not get 8921_lvs7, rc = %ld\n",
					PTR_ERR(reg_lvs7));
			return -ENODEV;
		}

		reg_l2 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi1_pll_vdda");
		if (IS_ERR_OR_NULL(reg_l2)) {
			pr_err("could not get 8921_l2, rc = %ld\n",
					PTR_ERR(reg_l2));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l2, 1200000, 1200000);
		if (rc) {
			pr_err("set_voltage l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}

		reg_l23 = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi_mi_vddio");
		if (IS_ERR_OR_NULL(reg_l23)) {
			pr_err("could not get 8921_l23, rc = %ld\n",
				PTR_ERR(reg_l23));
			return -ENODEV;
		}

		rc = regulator_set_voltage(reg_l23, 1800000, 1800000);
		if (rc) {
			pr_err("set_voltage l23 failed, rc=%d\n", rc);
			return -EINVAL;
		}

		reg_vsp = regulator_get(&msm_mipi_dsi1_device.dev,
				"dsi_mi_vsp");
		if (IS_ERR_OR_NULL(reg_vsp)) {
			pr_err("could not get VSP/VSN regulator, rc = %ld\n",
				PTR_ERR(reg_vsp));
			return -ENODEV;
		}

		reset_gpio = MI_RESET_GPIO;
		rc = gpio_request(reset_gpio, "disp_rst_n");
		if (rc) {
			pr_err("request pm8921 gpio 25 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		lcd_id_gpio = MI_LCD_ID_GPIO;
		rc = gpio_request(lcd_id_gpio, "disp_id_det");
		if (rc) {
			pr_err("request pm8921 gpio 12 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		dsi_mi_power_on = true;
	}

	if (on) {
		rc = regulator_set_optimum_mode(reg_l2, 100000);
		if (rc < 0) {
			pr_err("set_optimum_mode l2 failed, rc=%d\n", rc);
			return -EINVAL;
		}

		rc = regulator_enable(reg_l2);
		if (rc) {
			pr_err("enable l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_enable(reg_lvs7);
		if (rc) {
			pr_err("enable lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = regulator_enable(reg_l23);
		if (rc) {
			pr_err("enable l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		mdelay(1);

		rc = regulator_enable(reg_vsp);
		if (rc) {
			pr_err("enable vsp failed, rc=%d\n", rc);
			return -ENODEV;
		}
		mdelay(10);

		gpio_direction_output(reset_gpio, 1);
		mdelay(3);

		mi_panel_id = gpio_get_value(lcd_id_gpio);
	} else {
		gpio_direction_output(reset_gpio, 0);

		rc = regulator_disable(reg_vsp);
		if (rc) {
			pr_err("disable reg_vsp failed, rc=%d\n", rc);
			return -ENODEV;
		}
		mdelay(10);

		rc = regulator_disable(reg_lvs7);
		if (rc) {
			pr_err("disable reg_lvs7 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_disable(reg_l23);
		if (rc) {
			pr_err("disable reg_l23 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		rc = regulator_disable(reg_l2);
		if (rc) {
			pr_err("disable reg_l2 failed, rc=%d\n", rc);
			return -ENODEV;
		}
	}

	return 0;
}

static struct mipi_dsi_platform_data mipi_dsi_mi_pdata = {
	.dsi_power_save = mipi_dsi_mipanel_power,
};

static struct platform_device mipi_dsi_hitachi_panel_device = {
	.name = "mipi_renesas",
	.id = 0,
};

void __init apq8064_aries_init_fb(void)
{
	platform_device_register(&mipi_dsi_hitachi_panel_device);
	msm_fb_register_device("mipi_dsi", &mipi_dsi_mi_pdata);
}

#if defined(CONFIG_LEDS_LM3530_ARIES)
#define LM3530_EN_GPIO			PM8921_GPIO_PM_TO_SYS(13)

static struct lm3530_platform_data lm3530_40_pdata = {
#if defined(CONFIG_FB_MSM_MIPI_DSI_CABC)
	.mode = LM3530_BL_MODE_I2C_PWM,
#else
	.mode = LM3530_BL_MODE_MANUAL,
#endif
	.max_current = 0x5,
	.pwm_pol_hi = 0,
	.brt_ramp_law = 0x1,		/* linear */
	.brt_ramp_fall = 0,
	.brt_ramp_rise = 0,
	.brt_val = 0,
	.bl_en_gpio = LM3530_EN_GPIO,
	.regulator_used = 0,
};

static struct i2c_board_info lm3530_board_info[] __initdata = {
	{
		I2C_BOARD_INFO("lm3530-led", 0x38),
		.platform_data = &lm3530_40_pdata,
	},
};
#endif

static struct i2c_registry apq8064_i2c_backlight_device[] __initdata = {
#if defined(CONFIG_LEDS_LM3530_ARIES)
	{
		I2C_FFA,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		lm3530_board_info,
		ARRAY_SIZE(lm3530_board_info),
	},
#endif
};

void __init xiaomi_add_backlight_devices(void)
{
	int i;

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(apq8064_i2c_backlight_device); ++i) {
		i2c_register_board_info(apq8064_i2c_backlight_device[i].bus,
					apq8064_i2c_backlight_device[i].info,
					apq8064_i2c_backlight_device[i].len);
	}
}

#if defined(CONFIG_FB_MSM_HDMI_MHL_9244)
#define MITWO_GPIO_MHL_RESET		PM8921_GPIO_PM_TO_SYS(22)
#define MITWO_GPIO_MHL_INT		23
#define MITWO_GPIO_MHL_WAKEUP		PM8921_GPIO_PM_TO_SYS(16)

static int sii9244_power_setup(int on)
{
	int rc;
	static bool mhl_power_on;
	int mhl_1v8_gpio = PM8921_GPIO_PM_TO_SYS(14);
	int mhl_3v3_gpio = PM8921_GPIO_PM_TO_SYS(19);
	int hdmi_1v8_3v3_gpio = PM8921_GPIO_PM_TO_SYS(21);

	if (!mhl_power_on) {
		rc = gpio_request(mhl_1v8_gpio, "mhl_1v8_gpio");
		if (rc) {
			pr_err("request pm8921 gpio 14 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = gpio_request(mhl_3v3_gpio, "mhl_3v3_gpio");
		if (rc) {
			pr_err("request pm8921 gpio 19 failed, rc=%d\n", rc);
			return -ENODEV;
		}
		rc = gpio_request(hdmi_1v8_3v3_gpio, "hdmi_1v8_3v3_gpio");
		if (rc) {
			pr_err("request pm8921 gpio 21 failed, rc=%d\n", rc);
			return -ENODEV;
		}

		mhl_power_on = true;
	}

	if (on) {
		gpio_direction_output(mhl_1v8_gpio, 1);
		gpio_direction_output(mhl_3v3_gpio, 1);
		gpio_direction_output(hdmi_1v8_3v3_gpio, 1);
	} else {
		gpio_direction_output(mhl_1v8_gpio, 0);
		gpio_direction_output(mhl_3v3_gpio, 0);
		gpio_direction_output(hdmi_1v8_3v3_gpio, 0);
	}

	return 0;
}

static void sii9244_reset(int on)
{
	int rc;
	static bool mhl_first_reset;
	int mhl_gpio_reset = MITWO_GPIO_MHL_RESET;

	if (!mhl_first_reset) {
		rc = gpio_request(mhl_gpio_reset, "mhl_rst");
		if (rc) {
			pr_err("request pm8921 gpio 22 failed, rc=%d\n", rc);
			return;
		}
		mhl_first_reset = true;
	}

	if (on) {
		gpio_direction_output(mhl_gpio_reset, 0);
		msleep(10);
		gpio_direction_output(mhl_gpio_reset, 1);
	} else {
		gpio_direction_output(mhl_gpio_reset, 0);
	}
}

#if defined(CONFIG_FB_MSM_HDMI_MHL_RCP)
static int sii9244_key_codes[] = {
	KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
	KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
	KEY_SELECT, KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT,
	KEY_MENU, KEY_EXIT, KEY_DOT, KEY_ENTER,
	KEY_CLEAR, KEY_SOUND,
	KEY_PLAY, KEY_PAUSE, KEY_STOP, KEY_FASTFORWARD, KEY_REWIND,
	KEY_EJECTCD, KEY_FORWARD, KEY_BACK,
	KEY_PLAYCD, KEY_PAUSECD, KEY_STOP,
};
#endif

static struct mhl_platform_data mhl_sii9244_pdata = {
	.mhl_gpio_reset = 	MITWO_GPIO_MHL_RESET,
	.mhl_gpio_wakeup = 	MITWO_GPIO_MHL_WAKEUP,
	.power_setup = 		sii9244_power_setup,
	.reset =		sii9244_reset,
#if defined(CONFIG_FB_MSM_HDMI_MHL_RCP)
	.mhl_key_codes =	sii9244_key_codes,
	.mhl_key_num = 		ARRAY_SIZE(sii9244_key_codes),
#endif
};

static struct i2c_board_info mhl_sii9244_board_info[] = {
	{
		I2C_BOARD_INFO("mhl_Sii9244_page0", 0x39),		//0x72
		.platform_data = &mhl_sii9244_pdata,
		.irq = MSM_GPIO_TO_INT(MITWO_GPIO_MHL_INT),
	},
	{
		I2C_BOARD_INFO("mhl_Sii9244_page1", 0x3D),		//0x7A
	},
	{
		I2C_BOARD_INFO("mhl_Sii9244_page2", 0x49),		//0x92
	},
	{
		I2C_BOARD_INFO("mhl_Sii9244_cbus", 0x64),		//0xC8
	},
};
#endif

static struct i2c_registry apq8064_i2c_mhl_device[] __initdata = {
#if defined(CONFIG_FB_MSM_HDMI_MHL_9244)
	{
		I2C_FFA,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		mhl_sii9244_board_info,
		ARRAY_SIZE(mhl_sii9244_board_info),
	},
#endif
};

void __init xiaomi_add_mhl_devices(void)
{
	int i;

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(apq8064_i2c_mhl_device); ++i) {
		i2c_register_board_info(apq8064_i2c_mhl_device[i].bus,
					apq8064_i2c_mhl_device[i].info,
					apq8064_i2c_mhl_device[i].len);
	}
}
