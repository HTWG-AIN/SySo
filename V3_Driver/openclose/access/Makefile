# Author: Stefano Di Martino
# Created: 08.01.2012

CROSS_COMPILE=arm-buildroot-linux-uclibcgnueabi-
CC = gcc
CFLAGS = -W -Wall

TARGETS = access

.PHONY: all clean

all: $(TARGETS)

$(TARGETS): $(SOURCES)
	$(CROSS_COMPILE)$(CC) $(CFLAGS) $@.c -o $@

clean:
	$(RM) $(TARGETS)
