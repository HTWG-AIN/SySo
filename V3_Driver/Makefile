buildroot_dir := ~/buildroot
gitarm_dir := ~/git-arm
driver_dir := $(gitarm_dir)/V3_Driver
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
	scp -r $(gitarm_dir)/V3_Driver/* root@$(ip):/lib/modules/3.10.18/kernel/drivers 

copyid: 
	([ ! -f ~/.ssh/id_rsa.pub ] && ssh-keygen); ssh-copy-id root@$(ip)


setup-modules: 
	ssh root@$(ip) /sbin/depmod -A
	ssh root@$(ip) /sbin/modprobe -r null openclose zero hello
	ssh root@$(ip) /sbin/modprobe -a null openclose zero hello

make-nod: 
	ssh root@$(ip) /bin/mknod /dev/t12hello c 116 0
	ssh root@$(ip) /bin/mknod /dev/t12null c 115 0
	ssh root@$(ip) /bin/mknod /dev/t12zero c 113 0
	ssh root@$(ip) /bin/mknod /dev/t12openclose c 112 0

rm-nod:
	ssh root@$(ip) rm /dev/t12*


clean-modules:
	for i in $(driver_dir)/*; do \
		$(MAKE) -C $$i clean; \
	done



