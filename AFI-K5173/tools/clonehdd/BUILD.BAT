@del clonehdd.exe
tpc ..\libs\kes.pas
@copy ..\libs\kes.tpu .
as80 -x detect.z80
bin2inc detect.bin detect.inc Detect
as80 -x sector.z80
bin2inc sector.bin sector.inc Sector
tpc clonehdd.pas
@del kes.tpu
@del detect.bin
@del detect.inc
@del detect.lst
@del sector.bin
@del sector.inc
@del sector.lst
