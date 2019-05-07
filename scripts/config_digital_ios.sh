#!/bin/bash

cd /sys/class/gpio/

echo 136 > export
echo 27 > export
echo 35 > export
echo 9 > export

echo 19 > export
echo 112 > export
echo 12 > export
echo 110 > export

echo out > gpio136/direction
echo out > gpio27/direction
echo out > gpio35/direction
echo out > gpio9/direction

rm -rf /digital_ios
mkdir -p /digital_ios
cd /digital_ios

ln -s /sys/class/gpio/gpio136 dout1
ln -s /sys/class/gpio/gpio27 dout2
ln -s /sys/class/gpio/gpio35 dout3
ln -s /sys/class/gpio/gpio9 dout4

ln -s /sys/class/gpio/gpio19 din1
ln -s /sys/class/gpio/gpio112 din2
ln -s /sys/class/gpio/gpio12 din3
ln -s /sys/class/gpio/gpio110 din4

echo 0 > dout1/value
echo 0 > dout2/value
echo 0 > dout3/value
echo 1 > dout4/value
