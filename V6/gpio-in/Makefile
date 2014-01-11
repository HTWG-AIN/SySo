#
# Makefile
#
# Autor: Stefano Di Martino

TOOLCHAIN=/opt/toolchains/gcc-linaro-arm-linux-gnueabihf-raspbian
CROSS_COMPILE=$(TOOLCHAIN)/bin/arm-linux-gnueabihf-
CC=gcc
# Options
ARCH=arm
CFLAGS = -W -Wall -g  -I $(TOOLCHAIN)/arm-linux-gnueabihf/include/ -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6 -mtune=arm1176jzf-s -marm -L /$(TOOLCHAIN)/arm-linux-gnueabihf/lib/


STATIC = -static # For static lib binding.

C_FILES = $(wildcard *.c)
TARGETS = $(C_FILES:%.c=%)

.PHONY: all clean

all: $(TARGETS)

# libc has to be installed as a static lib on your system!
# Look for 'glibc-static' in your distributor's repository!
#
#


$(TARGETS): $(SOURCES)
	$(CROSS_COMPILE)$(CC) $(CFLAGS)  $@.c -o $@

clean:
	$(RM) $(TARGETS) *.o core* tmp*

install: clean all
	scp switch root@$(RPI_HOST):/bin/


copyid: 
	  ([ ! -f ~/.ssh/id_rsa.pub ] && ssh-keygen); ssh-copy-id root@$(RPI_HOST)

