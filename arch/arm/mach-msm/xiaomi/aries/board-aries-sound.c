/* Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
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

#include <linux/kernel.h>
#include <linux/gpio.h>
#include <mach/board.h>

#if defined(CONFIG_AUDIENCE_ES310)
#include <sound/es310.h>
#endif

#include "board-8064.h"

#if defined(CONFIG_AUDIENCE_ES310)
static int es310_power_setup(int on)
{
	int rc;
	struct regulator *es310_vdd;

	/* Read more to understand whether we can disable the regulator */
        es310_vdd = regulator_get(NULL, "8921_l15");

        if (IS_ERR(es310_vdd)) {
                printk(KERN_ERR "%s: Unable to get regulator es310_vdd\n", __func__);
                return -1;
        }

        rc = regulator_set_voltage(es310_vdd, 2800000, 2800000);
        if (rc) {
                printk(KERN_ERR "%s: regulator set voltage failed\n", __func__);
		return rc;
	}

        rc = regulator_enable(es310_vdd);
        if (rc) {
                printk(KERN_ERR "%s: Error while enabling regulator for es310 %d\n", __func__, rc);
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
	.power_setup = es310_power_setup,
};

static struct i2c_board_info audience_es310_board_info[] = {
	{
		I2C_BOARD_INFO("audience_es310", 0x3E),
		.platform_data = &audience_es310_pdata,
	},
};
#endif

static struct i2c_registry apq8064_i2c_audio_device[] __initdata = {
#if defined(CONFIG_AUDIENCE_ES310)
	{
		I2C_SURF | I2C_FFA | I2C_LIQUID,
		APQ_8064_GSBI1_QUP_I2C_BUS_ID,
		audience_es310_board_info,
		ARRAY_SIZE(audience_es310_board_info),
	},
#endif
};

void __init xiaomi_add_sound_devices(void)
{
	int i;

	/* Run the array and install devices as appropriate */
	for (i = 0; i < ARRAY_SIZE(apq8064_i2c_audio_device); ++i) {
		i2c_register_board_info(apq8064_i2c_audio_device[i].bus,
					apq8064_i2c_audio_device[i].info,
					apq8064_i2c_audio_device[i].len);
	}
}
