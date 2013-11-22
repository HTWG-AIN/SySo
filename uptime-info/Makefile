#
# Makefile
#
# Autor: Stefano Di Martino

#CROSS_COMPILE=arm-none-linux-gnueabi-
#CC=gcc
# Options
CFLAGS = -W -Wall

STATIC = -static # For static lib binding.

C_FILES = $(wildcard *.c)
TARGETS = $(C_FILES:%.c=%)

.PHONY: all clean

all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(CC) $(CFLAGS) $@.c -o $@  && $(CC) $(STATIC) $(CFLAGS) $@.c -o $@-static
	
clean:
	$(RM) $(TARGETS) $(TARGETS)-static *.o core* tmp*
