buildroot_dir := /home/`whoami`/buildroot
gitarm_dir := /home/`whoami`/git-arm
driver_dir := $(gitarm_dir)/V3_Driver
ip := 192.168.29.48

all:  rebuild modulecopy

rebuild:
	$(MAKE) -C $(driver_dir)/* 


modulecopy:
	scp -r $(gitarm_dir)/V3_Driver/* root@$(ip):/lib/modules/3.10.18/kernel/drivers

copyid: 
	ssh-copy-id root@$(ip)

clean-modules:
	$(MAKE) -C $(driver_dir)/*  clean



