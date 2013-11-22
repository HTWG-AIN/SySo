#
# Makefile
#
# Autor: Stefano Di Martino

CROSS_COMPILE=arm-none-linux-gnueabi-
CC=gcc
# Options
CFLAGS = -W -Wall

STATIC = -static # For static lib binding.

C_FILES = $(wildcard *.c)
TARGETS = $(C_FILES:%.c=%)

.PHONY: all clean

all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $@.c -o $@  && $(CROSS_COMPILE)$(CC) $(STATIC) $(CFLAGS) $@.c -o $@-static
	
clean:
	$(RM) $(TARGETS) *.o core* tmp*
