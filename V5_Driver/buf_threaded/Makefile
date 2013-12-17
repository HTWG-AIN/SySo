TARGETS=buf_threaded
BUILDROOT_OUTPUT :=~/buildroot/output
KDIR := $(BUILDROOT_OUTPUT)/build/linux-3.10.18
OPTIONS= -C $(KDIR) M=$(shell pwd)  ARCH=arm
CC := $(BUILDROOT_OUTPUT)/host/usr/bin/arm-linux-gcc
LD := $(BUILDROOT_OUTPUT)/host/usr/bin/arm-linux-ld
EXTRA_CFLAGS += -DDEBUG

OBJECTS=$(addsuffix .o, $(TARGETS))
obj-m += $(OBJECTS)

all:
	make $(OPTIONS) modules

clean:
	make $(OPTIONS) clean

install:
	make $(OPTIONS) INSTALL_MOD_PATH=$(BUILDROOT_OUTPUT)/target
INSTALL_MOD_STRIP=1 modules_install
