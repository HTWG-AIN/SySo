buildroot_dir := ~/buildroot
gitarm_dir := ~/git-arm
driver_dir := $(gitarm_dir)/V5_Driver
IP := $(QEMU_HOST_ADRESS)

CC := $(buildroot_dir)/output/host/usr/bin/arm-linux-gcc


all: install

install: 
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i install; \
	done

uninstall: 
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i uninstall; \
	done

rebuild:
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i all; \
	done


clean:
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i clean; \
	done

copyid: 
	([ ! -f ~/.ssh/id_rsa.pub ] && ssh-keygen); ssh-copy-id root@$(IP)



rm-nod:
	ssh root@$(IP) rm /dev/t12*
