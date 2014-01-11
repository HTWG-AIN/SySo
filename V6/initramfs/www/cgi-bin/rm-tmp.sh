#!/bin/sh
echo "Content-type: text/html"
echo ""

rm -rf /tmp/*
echo "/tmp wurde gelöscht. Neuer Inhalt:\n`ls -la /tmp`"
