buildroot_dir := ~/buildroot
gitarm_dir := ~/git-arm
driver_dir := $(gitarm_dir)/V3_Driver
ip := 192.168.29.48

CC := $(buildroot_dir)/output/host/usr/bin/arm-linux-gcc


all: install

install: rebuild modulcopy

rebuild:
	$(MAKE) -C $(driver_dir)/* all


modulecopy:
	scp -r $(gitarm_dir)/V3_Driver/* root@$(ip):/lib/modules/3.10.18/kernel/drivers

copyid: 
	ssh-copy-id root@$(ip)

clean-modules:
	$(MAKE) -C $(driver_dir)/*  clean



