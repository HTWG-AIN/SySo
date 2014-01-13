TARGETS=mygpio
BUILDROOT_OUTPUT :=~/buildroot/output
KDIR := $(BUILDROOT_OUTPUT)/build/linux-custom
OPTIONS= -C $(KDIR) M=$(shell pwd)  ARCH=arm
TOOLCHAIN=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-raspbian
CC := $(TOOLCHAIN)/bin/arm-linux-gnueabihf-gcc
LD := $(TOOLCHAIN)/bin/arm-linux-gnueabihf-ld
EXTRA_CFLAGS += -DDEBUG

RPI_HOST_KERNEL_DIR:=/lib/modules/3.12.6/kernel

OBJECTS=$(addsuffix .o, $(TARGETS))
obj-m += $(OBJECTS)

all:
	make $(OPTIONS) modules

clean:
	make $(OPTIONS) clean

install: all
	ssh root@$(RPI_HOST) mkdir -p  $(RPI_HOST_KERNEL_DIR)/$(TARGETS)
	scp -r $(TARGETS).ko root@$(RPI_HOST):$(RPI_HOST_KERNEL_DIR)/$(TARGETS)
	ssh root@$(RPI_HOST) "/sbin/depmod -A; /sbin/modprobe -r $(TARGETS); /sbin/modprobe -a $(TARGETS)"
uninstall:
	ssh root@$(RPI_HOST) /sbin/modprobe -r $(TARGETS)

copyid: 
	  ([ ! -f ~/.ssh/id_rsa.pub ] && ssh-keygen); ssh-copy-id root@$(RPI_HOST)
