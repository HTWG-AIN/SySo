ifneq ($(KERNELRELEASE),)
obj-m	:= template.o
 
else
KDIR	:= ~/buildroot/output/build/linux-3.10.18
PWD	:= $(shell pwd)
 
default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
endif

clean:
	$(RM) $(TARGETS) *.o core* tmp* *.ko *.symvers *.order *.mod.c
