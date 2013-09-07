/* arch/arm/mach-msm/lge/lge_kcal_ctrl.c
 *
 * Interface to calibrate display color temperature.
 *
 * Copyright (C) 2012 LGE
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <vos_types.h>
#include <linux/lge_primaconfig.h>

v_MACADDR_t custom_mac_address = VOS_MAC_ADDR_ZERO_INITIALIZER;
#define MAC_ADDR_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_ADDRESS_STR "%02x:%02x:%02x:%02x:%02x:%02x"

static ssize_t mac_address_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	unsigned int newmac[6];

	if(sscanf(buf, MAC_ADDRESS_STR,
      &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],&newmac[5])==6) {
      custom_mac_address.bytes[0] = newmac[0];
      custom_mac_address.bytes[1] = newmac[1];
      custom_mac_address.bytes[2] = newmac[2];
      custom_mac_address.bytes[3] = newmac[3];
      custom_mac_address.bytes[4] = newmac[4];
      custom_mac_address.bytes[5] = newmac[5];
   }
   else printk("%s: error setting mac address\n", __func__);

   return count;
}

static ssize_t mac_address_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return sprintf(buf, MAC_ADDRESS_STR"\n", MAC_ADDR_ARRAY(custom_mac_address.bytes));
}

static DEVICE_ATTR(mac_address, 0644, mac_address_show, mac_address_store);

static int primaconfig_probe(struct platform_device *pdev)
{
	int rc = 0;

	rc = device_create_file(&pdev->dev, &dev_attr_mac_address);
	if(rc !=0) {
      printk("%s: error creating device_file\n", __func__);
		return -1;
   }

	return 0;
}
static struct platform_driver primaconfig_driver = {
	.probe  = primaconfig_probe,
	.driver = {
		.name   = "primaconfig",
	},
};

int __init primaconfig_init(void)
{
	printk(KERN_INFO "%s\n", __func__);
	return platform_driver_register(&primaconfig_driver);
}

device_initcall(primaconfig_init);
