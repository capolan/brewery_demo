#!/bin/bash
modprobe analog_input

rm -rf /analog_ios
mkdir -p /analog_ios
cd /analog_ios

ln -s  /sys/devices/soc0/soc/2100000.aips-bus/21f8000.i2c/i2c-3/3-0008 ain1
ln -s  /sys/devices/soc0/soc/2100000.aips-bus/21f8000.i2c/i2c-3/3-000a ain2
ln -s  /sys/devices/soc0/soc/2100000.aips-bus/21f8000.i2c/i2c-3/3-000c ain3
ln -s  /sys/devices/soc0/soc/2100000.aips-bus/21f8000.i2c/i2c-3/3-000e ain4

# Configure thermocouple type T on input 1
cd /analog_ios/ain1
echo 0 > deep_sleep
echo 100 > cj1_factor
echo 100 > cj2_factor
echo 242 > cj_offset
echo 2 > type
echo 0 > unit
echo 1 > filter
echo 0 > inll
echo 6 > window_size
echo 36 > period_samples
echo 1 > mains_freq
echo 1 > opmode
echo 1 > anti_noise
echo 0 > offset
echo 1 > channel_check
echo 1 > reconfig
echo 1 > store_config
echo 1 > store_config

# Configure PT100 on input 2
cd /analog_ios/ain2
echo 0 > deep_sleep
echo 100 > cj1_factor
echo 100 > cj2_factor
echo 242 > cj_offset
echo 8 > type
echo 0 > unit
echo 1 > filter
echo 0 > inll
echo 6 > window_size
echo 36 > period_samples
echo 1 > mains_freq
echo 1 > opmode
echo 1 > anti_noise
echo 0 > offset
echo 1 > channel_check
echo 1 > reconfig
echo 1 > store_config
echo 1 > store_config

# Configure 0-10V on input 3
cd /analog_ios/ain3
echo 0 > deep_sleep
echo 13 > type
echo 0 > unit
echo 1 > filter
echo 0 > inll
echo 10000 > inhl
echo 6 > window_size
echo 36 > period_samples
echo 1 > mains_freq
echo 1 > opmode
echo 1 > anti_noise
echo 0 > offset
echo 1 > channel_check
echo 1 > reconfig
echo 1 > store_config
echo 1 > store_config

# Configure 4-20mA on input 4
cd /analog_ios/ain4
echo 0 > deep_sleep
echo 15 > type
echo 0 > unit
echo 1 > filter
echo 0 > inll
echo 10000 > inhl
echo 6 > window_size
echo 36 > period_samples
echo 1 > mains_freq
echo 1 > opmode
echo 1 > anti_noise
echo 0 > offset
echo 1 > channel_check
echo 1 > reconfig
echo 1 > store_config
echo 1 > store_config
