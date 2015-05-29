subdir-ccflags-$(CONFIG_ARCH_MSM) += -Iarch/arm/mach-msm

-include $(src)/Makefile.board

obj-$(CONFIG_PM8921_BMS_XIAOMI) += bms-batterydata-sony.o bms-batterydata-samsung.o bms-batterydata-lg.o bms-batterydata-lg-3000.o bms-batterydata-sony-3000.o bms-batterydata-samsung-3000.o bms-batterydata-samsung-2000.o bms-batterydata-lg-2000.o
obj-$(CONFIG_MACH_XIAOMI) += devices_xiaomi.o

CFLAGS_devices_xiaomi.o += -Idrivers/staging/android