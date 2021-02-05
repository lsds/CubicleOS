#!/bin/bash
#for k in {10..500..10}
for ((k=1; k<=32*1024*1024;k*=2))
do
    dd if=/dev/zero of=$k  bs=$k  count=1
done
