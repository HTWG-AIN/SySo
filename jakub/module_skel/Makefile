ifneq ($(KERNELRELEASE),)
	obj-$(CONFIG_MODULE_SKEL_DRIVER) += treiber.o
	 
else
KDIR    := /lib/modules/$(shell uname -r)/build
PWD     := $(shell pwd)
 
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
endif
