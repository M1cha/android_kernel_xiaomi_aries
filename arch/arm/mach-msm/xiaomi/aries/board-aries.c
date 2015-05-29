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

#include <linux/init.h>
#include <linux/bug.h>
#include <linux/mfd/pm8xxx/pm8xxx-adc.h>
#include <mach/board_xiaomi.h>

#include "board-aries.h"
#include "board-aries-pmic.h"

void __init apq8064_aries_init(void) {
	pm8xxx_set_adcmap_btm_threshold(adcmap_btm_threshold,
			ARRAY_SIZE(adcmap_btm_threshold));

	apq8064_aries_init_fb();
	apq8064_init_input();

	xiaomi_add_ramconsole_devices();
	xiaomi_add_backlight_devices();
	xiaomi_add_mhl_devices();
	xiaomi_add_sound_devices();
	xiaomi_add_camera_devices();
}
