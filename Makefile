all:
	cd sysinfo_printer && make && mv sysinfo-printer ../initramfs/bin && \
	cd .. && ./make-tarballs.sh

clean:
	cd sysinfo_printer && make clean && cd .. && rm initramfs/bin/* && rm tarballs/*
