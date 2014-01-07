#!/bin/sh


for i in 1 2 3 4 5 6
do
  cat /dev/t12buf_threaded 
  sleep  $(($RANDOM % 6 + 5))
done





