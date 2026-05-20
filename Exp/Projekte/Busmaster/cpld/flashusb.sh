#!/bin/bash
rm flash.xsvf
/opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64/impact -batch buildimpact.cmd
stty -F /dev/ttyACM0 raw -echo -echoe -echok
echo "5" > /dev/ttyACM0
base64 flash.xsvf > /dev/ttyACM0
echo "=" > /dev/ttyACM0


