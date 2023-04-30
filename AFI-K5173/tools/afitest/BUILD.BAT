tpc ..\libs\kes.pas
@copy ..\libs\kes.tpu .
as80 -x kesprog.z80
bin2inc kesprog.bin kesprog.inc Kesprog
tpc afitest.pas
@del kes.tpu
rem @del kesprog.bin
@del kesprog.inc
@del kesprog.lst
