#
# Makefile
#
# Autor: Stefano Di Martino

CC=arm-buildroot-linux-uclibcgnueabi-gcc
# Options
CFLAGS = -g -W -Wall

STATIC = -static # For static lib binding.

C_FILES = $(wildcard *.c)
TARGETS = $(C_FILES:%.c=%)

.PHONY: all clean

all: $(TARGETS)

# libc has to be installed as a static lib on your system!
# Look for 'glibc-static' in your distributor's repository!

$(TARGETS): $(SOURCES)
	$(CC) $(CFLAGS) -static $@.c -o $@

clean:
	$(RM) $(TARGETS) *.o core* tmp*
