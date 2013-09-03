obj-$(CONFIG_MACH_APQ8064_MAKO) += board-mako.o \
	board-mako-camera.o \
	board-mako-display.o \
	board-mako-gpiomux.o \
	board-mako-gpu.o \
	board-mako-input.o \
	board-mako-pmic.o \
	board-mako-regulator.o \
	board-mako-misc.o \
	board-mako-sound.o \
	board-mako-storage.o

ifdef CONFIG_EARJACK_DEBUGGER
	obj-$(CONFIG_MACH_APQ8064_MAKO) += board-mako-earjack-debugger.o
endif

ifdef CONFIG_BCM2079X
	obj-$(CONFIG_MACH_APQ8064_MAKO) += board-mako-nfc.o
endif

CFLAGS_board-mako-display.o += -Idrivers/video
