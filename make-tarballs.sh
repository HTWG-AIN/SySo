#!/bin/sh
cd uptime-info
tar czfv uptime-info.tar.gz uptime-info.c Makefile && mv uptime-info.tar.gz ../tarballs
