#
# Makefile
#
# Autor: Stefano Di Martino

CROSS_COMPILE=/opt/toolchains/Sourcery-CodeBench-ARM-2013.05/bin/arm-none-linux-gnueabi-
CC=gcc
# Options
CFLAGS = -W -Wall

STATIC = -static # For static lib binding.

C_FILES = $(wildcard *.c)
TARGETS = $(C_FILES:%.c=%)

.PHONY: all clean

all: $(TARGETS)

# libc has to be installed as a static lib on your system!
# Look for 'glibc-static' in your distributor's repository!

$(TARGETS): $(SOURCES)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $@.c -o $@

clean:
	$(RM) $(TARGETS) *.o core* tmp*
