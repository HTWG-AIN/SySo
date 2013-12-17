buildroot_dir := ~/buildroot
gitarm_dir := ~/git-arm
driver_dir := $(gitarm_dir)/V5_Driver
ip := 192.168.29.154

CC := $(buildroot_dir)/output/host/usr/bin/arm-linux-gcc


all: install

install: rebuild modulecopy setup-modules

rebuild:
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i all; \
	done


clean:
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i clean; \
	done

modulecopy:
	ssh root@$(ip) mkdir -p /lib/modules/3.10.18/kernel/drivers
	scp -r $(driver_dir)/* root@$(ip):/lib/modules/3.10.18/kernel/drivers 

copyid: 
	([ ! -f ~/.ssh/id_rsa.pub ] && ssh-keygen); ssh-copy-id root@$(ip)


setup-modules: 
	ssh root@$(ip) /sbin/depmod -A
	ssh root@$(ip) /sbin/modprobe -r buf hello2 tasklet kthread
	ssh root@$(ip) /sbin/modprobe -a buf hello2 tasklet kthread


rm-nod:
	ssh root@$(ip) rm /dev/t12*
