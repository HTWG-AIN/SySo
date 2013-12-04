ifneq ($(KERNELRELEASE),)
obj-m	:= zero.o
 
else
KDIR	:= ~/buildroot/output/build/linux-3.10.18
PWD	:= $(shell pwd)

all: default

debug: CXX += -DDEBUG -g
debug: CC += -DDEBUG -g
debug: default

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
endif

clean:
	$(RM) $(TARGETS) *.o core* tmp* *.ko *.symvers *.order *.mod.c
