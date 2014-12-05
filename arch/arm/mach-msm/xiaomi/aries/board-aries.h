/* Copyright (c) 2011-2012, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ARCH_ARM_MACH_MSM_BOARD_APQ8064_ARIES_H
#define __ARCH_ARM_MACH_MSM_BOARD_APQ8064_ARIES_H

void apq8064_aries_init(void);
void apq8064_aries_init_fb(void);
void apq8064_init_input(void);

void xiaomi_add_backlight_devices(void);
void xiaomi_add_mhl_devices(void);
void xiaomi_add_sound_devices(void);
void xiaomi_add_camera_devices(void);

#endif
