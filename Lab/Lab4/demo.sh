#!/bin/sh

set -x
# set -e

rmmod -f mydev
insmod mydev.ko

./writer ABCDEF &
./reader 192.168.0.16 8899 /dev/mydev
