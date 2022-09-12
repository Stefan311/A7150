@del afiflash.exe
tpc ..\libs\kes.pas
@copy ..\libs\kes.tpu .
as80 -x kesflash.z80
bin2inc kesflash.bin kesflash.inc Kesprog
as80 -x sstflash.z80
bin2inc sstflash.bin sstflash.inc sstflash
tpc afiflash.pas
@del kes.tpu
@del sstflash.inc
@del kesflash.inc
@del *.bin
@del *.lst
