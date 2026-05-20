setMode -bs
addDevice -p 1 -file Main.jed
setcable -p xsvf -file flash.xsvf
program -e -v -p 1
closecable
exit
