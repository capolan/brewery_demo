#!/bin/bash
modprobe analog_output

rm -rf /analog_outputs
mkdir -p /analog_outputs
cd /analog_outputs

ln -s /sys/devices/soc0/analog_out_pwm5/params0 aout1
ln -s /sys/devices/soc0/analog_out_pwm6/params0 aout2

cd /analog_outputs/aout1
echo 0-10V > out_type
echo 0:1000000 > voltage_calibration
echo 5000:537700 > voltage_calibration
echo 10000:78260 > voltage_calibration
echo 5000 > out_eng_units

cd /analog_outputs/aout2
echo 4-20mA > out_type
echo 4000:200000 > current_calibration
echo 20000:1000000 > current_calibration
echo 12000 > out_eng_units
