#
# Makefile
#
# Autor: Stefano Di Martino

CC=arm-linux-gnueabi-gcc
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
	$(CC) $(CFLAGS) $@.c -o $@

clean:
	$(RM) $(TARGETS) *.o core* tmp*
