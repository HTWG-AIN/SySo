TARGETS=mygpio
BUILDROOT_OUTPUT :=~/buildroot/output
KDIR := $(BUILDROOT_OUTPUT)/build/linux-custom
OPTIONS= -C $(KDIR) M=$(shell pwd)  ARCH=arm
TOOLCHAIN=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-raspbian
CC := $(TOOLCHAIN)/bin/arm-linux-gnueabihf-gcc
LD := $(TOOLCHAIN)/bin/arm-linux-gnueabihf-ld
EXTRA_CFLAGS += -DDEBUG -Ofast

C_FILES = $(wildcard *.c)
TARGETS += $(C_FILES:%.c=%)


GENERIC_TOOLCHAIN=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-raspbian
GENERIC_CROSS_COMPILE=$(GENERIC_TOOLCHAIN)/bin/arm-linux-gnueabihf-
GENERIC_CC=gcc
# Options
GENERIC_ARCH=arm
GENERIC_CFLAGS = -W -Wall -g  -I $(GENERIC_TOOLCHAIN)/arm-linux-gnueabihf/include/ -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6 -mtune=arm1176jzf-s -marm -L /$(GENERIC_TOOLCHAIN)/arm-linux-gnueabihf/lib/ -lpthread -lrt


RPI_HOST_KERNEL_DIR:=/lib/modules/3.12.6/kernel

OBJECTS=$(addsuffix .o, $(TARGETS))
obj-m += $(OBJECTS)

all: led250 ledmax ledflip
	make $(OPTIONS) modules


$(TARGETS): $(SOURCES)
	$(GENERIC_CROSS_COMPILE)$(GENERIC_CC) $(GENERIC_CFLAGS) $@.c -o $@

led-clean:
	rm led250
	rm ledmax
	rm ledflip

led-install:  led250 ledmax ledflip
	scp -r led250 root@$(RPI_HOST):/bin
	scp -r ledmax root@$(RPI_HOST):/bin
	scp -r ledflip root@$(RPI_HOST):/bin


clean: led-clean
	make $(OPTIONS) clean

install: clean all
	ssh root@$(RPI_HOST) mkdir -p  $(RPI_HOST_KERNEL_DIR)/mygio
	scp -r mygpio.ko root@$(RPI_HOST):$(RPI_HOST_KERNEL_DIR)/mygio
	ssh root@$(RPI_HOST) "/sbin/depmod -A; /sbin/modprobe -r mygpio; /sbin/modprobe -a mygpio"
uninstall:
	ssh root@$(RPI_HOST) /sbin/modprobe -r mygpio

copyid: 
	  ([ ! -f ~/.ssh/id_rsa.pub ] && ssh-keygen); ssh-copy-id root@$(RPI_HOST)
