TARGETS=lock
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

install: all
	ssh root@$(QEMU_HOST_ADRESS) mkdir -p  $(QEMU_HOST_KERNEL_DIR)/$(TARGETS)
	scp -r $(TARGETS).ko root@$(QEMU_HOST_ADRESS):$(QEMU_HOST_KERNEL_DIR)/$(TARGETS)
	ssh root@$(QEMU_HOST_ADRESS) "/sbin/depmod -A; /sbin/modprobe -r $(TARGETS); /sbin/modprobe -a $(TARGETS)"
uninstall:
	ssh root@$(QEMU_HOST_ADRESS) /sbin/modprobe -r $(TARGETS)
