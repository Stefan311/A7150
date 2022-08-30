tpc ..\libs\kes.pas
@copy ..\libs\kes.tpu .
as80 -x kesflash.z80
bin2inc kesflash.bin kesflash.inc Kesprog
tpc afiflash.pas
@del kes.tpu
@del kesflash.bin
@del kesflash.inc
@del kesflash.lst
