TARGETS=mygpio
BUILDROOT_OUTPUT :=~/buildroot/output
KDIR := $(BUILDROOT_OUTPUT)/build/linux-custom
OPTIONS= -C $(KDIR) M=$(shell pwd)  ARCH=arm
TOOLCHAIN=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-raspbian
CC := $(TOOLCHAIN)/bin/arm-linux-gnueabihf-gcc
LD := $(TOOLCHAIN)/bin/arm-linux-gnueabihf-ld
EXTRA_CFLAGS += -DDEBUG

OBJECTS=$(addsuffix .o, $(TARGETS))
obj-m += $(OBJECTS)

all:
	make $(OPTIONS) modules

clean:
	make $(OPTIONS) clean

install: all
	ssh root@$(RPI_HOST) mkdir -p  $(QEMU_HOST_KERNEL_DIR)/$(TARGETS)
	scp -r $(TARGETS).ko root@$(RPI_HOST):$(QEMU_HOST_KERNEL_DIR)/$(TARGETS)
	ssh root@$(RPI_HOST) "/sbin/depmod -A; /sbin/modprobe -r $(TARGETS); /sbin/modprobe -a $(TARGETS)"
uninstall:
	ssh root@$(RPI_HOST) /sbin/modprobe -r $(TARGETS)

