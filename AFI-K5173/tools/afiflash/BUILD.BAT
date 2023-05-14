@del afiflash.exe
tpc ..\libs\kes.pas
@copy ..\libs\kes.tpu .
as80 -x identz80.z80
bin2inc identz80.bin identz80.inc IdentZ80
as80 -x kesflash.z80
bin2inc kesflash.bin kesflash.inc Kesprog
as80 -x fl_sst1.z80
bin2inc fl_sst1.bin fl_sst1.inc fl_sst1
as80 -x fl_sst2.z80
bin2inc fl_sst2.bin fl_sst2.inc fl_sst2
as80 -x fl_sst3.z80
bin2inc fl_sst3.bin fl_sst3.inc fl_sst3
as80 -x fl_mv.z80
bin2inc fl_mv.bin fl_mv.inc fl_mv
as80 -x fl_amd1.z80
bin2inc fl_amd1.bin fl_amd1.inc fl_amd1
as80 -x fl_amd2.z80
bin2inc fl_amd2.bin fl_amd2.inc fl_amd2
as80 -x fl_28f1.z80
bin2inc fl_28f1.bin fl_28f1.inc fl_28f1
as80 -x fl_28f2.z80
bin2inc fl_28f2.bin fl_28f2.inc fl_28f2
as80 -x fl_at1.z80
bin2inc fl_at1.bin fl_at1.inc fl_at1
as80 -x fl_at2.z80
bin2inc fl_at2.bin fl_at2.inc fl_at2
as80 -x er_at1.z80
bin2inc er_at1.bin er_at1.inc er_at1
as80 -x er_28f1.z80
bin2inc er_28f1.bin er_28f1.inc er_28f1
as80 -x er_28f2.z80
bin2inc er_28f2.bin er_28f2.inc er_28f2
as80 -x er_win.z80
bin2inc er_win.bin er_win.inc er_win
as80 -x er_sst1.z80
bin2inc er_sst1.bin er_sst1.inc er_sst1
as80 -x er_sst2.z80
bin2inc er_sst2.bin er_sst2.inc er_sst2
as80 -x er_as.z80
bin2inc er_as.bin er_as.inc er_as
as80 -x er_amd1.z80
bin2inc er_amd1.bin er_amd1.inc er_amd1
as80 -x er_amd2.z80
bin2inc er_amd2.bin er_amd2.inc er_amd2
tpc afiflash.pas
@echo off
@del kes.tpu
@del fl_sst1.inc
@del fl_sst2.inc
@del fl_sst3.inc
@del fl_mv.inc
@del fl_amd1.inc
@del fl_amd2.inc
@del fl_28f1.inc
@del fl_28f2.inc
@del fl_at1.inc
@del fl_at2.inc
@del er_at1.inc
@del er_28f1.inc
@del er_28f2.inc
@del er_win.inc
@del er_sst1.inc
@del er_sst2.inc
@del er_as.inc
@del er_amd1.inc
@del er_amd2.inc
@del identz80.inc
@del kesflash.inc
@del *.bin
@del *.lst
