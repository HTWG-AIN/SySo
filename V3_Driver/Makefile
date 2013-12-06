buildroot_dir := ~/buildroot
gitarm_dir := ~/git-arm
driver_dir := $(gitarm_dir)/V3_Driver
ip := 192.168.29.154

CC := $(buildroot_dir)/output/host/usr/bin/arm-linux-gcc


all: install

install: rebuild modulecopy

rebuild:
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i all; \
	done


clean:
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i clean; \
	done

modulecopy:
	scp -r $(gitarm_dir)/V3_Driver/* root@$(ip):/lib/modules/3.10.18/kernel/drivers 

copyid: 
	([ ! -f ~/.ssh/id_rsa.pub ] && ssh-keygen); ssh-copy-id root@$(ip)

clean-modules:
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i clean; \
	done



