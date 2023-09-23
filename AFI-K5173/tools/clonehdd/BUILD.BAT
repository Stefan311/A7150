@del clonehdd.exe
tpc ..\libs\kes.pas
@copy ..\libs\kes.tpu .
as80 -x detect.z80
bin2inc detect.bin detect.inc Detect
as80 -x ideinfo.z80
bin2inc ideinfo.bin ideinfo.inc IdeInfo
as80 -x inithdd.z80
bin2inc inithdd.bin inithdd.inc InitHDD
as80 -x sector.z80
bin2inc sector.bin sector.inc Sector
as80 -x rest_chb.z80
bin2inc rest_chb.bin rest_chb.inc ResetChB
tpc clonehdd.pas
@del kes.tpu
@del *.bin
@del *.inc
@del *.lst
