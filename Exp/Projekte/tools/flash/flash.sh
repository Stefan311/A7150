#!/bin/bash
rm flash.xsvf
/opt/Xilinx/14.7/ISE_DS/ISE/bin/lin64/impact -batch buildimpact.cmd
curl -X POST --data-binary @flash.xsvf 192.168.178.90:80/xapp058/upload_flash
