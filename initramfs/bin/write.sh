#!/bin/sh

for i in 1 2 3 4 5 6
do
  echo "Write instanz 0 : write $i" > /dev/t12buf_threaded 
  sleep  $(($RANDOM % 5 + 1))
done
