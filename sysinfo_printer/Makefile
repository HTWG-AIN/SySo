#
# Makefile
#
# Autor: Stefano Di Martino

CROSS_COMPILE=arm-buildroot-linux-uclibcgnueabi-
CC=gcc
# Options
CFLAGS = -W -Wall -g

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
