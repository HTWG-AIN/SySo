# Author: Stefano Di Martino
# Created: 10.12.2013

CROSS_COMPILE=arm-buildroot-linux-uclibcgnueabi-
CC = gcc
CFLAGS = -W -Wall

TARGETS = nonblock_write nonblock_read

.PHONY: all clean

debug: CC += -DDEBUG -g
debug: all

all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $@.c -o $@

clean:
	$(RM) $(TARGETS)
