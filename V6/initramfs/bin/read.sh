#!/bin/sh


for i in 1 2 3 4 5 6
do
  head -n 1 /dev/t12buf_threaded 
  sleep  $(($RANDOM % 6 + 5))
done
