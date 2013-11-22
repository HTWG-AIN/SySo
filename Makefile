all:
	cd uptime-info && make && mv uptime-info uptime-info-static ../initramfs/bin && \
	cd ../sysinfo_printer && make && mv sysinfo-printer ../initramfs/bin && \
	cd .. && ./make-tarballs.sh

clean:
	cd uptime-info && make clean && cd ../sysinfo_printer && make clean && cd .. && rm initramfs/bin/* && rm tarballs/*
