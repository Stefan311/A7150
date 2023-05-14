{ *** globale Teilprozeduren fr alle Chiptypen ************************** }

procedure Flash_Kes(prog: array of byte);
begin
  Kes_IOPB^.buffer_ptr:=@puffer[i];
  Kes_IOPB^.requested_bytes_l:=(adr and $0fff) or $9000;
  Kes_IOPB^.requested_bytes_h:=adr shr 12;
  kes_load_run_const(prog);
  ok:=Kes_IOPB^.done_bytes_h=1;
  i:=i+Kes_IOPB^.done_bytes_l;
  adr:=adr+Kes_IOPB^.done_bytes_l;
end;


procedure EraseFailure(grund:string);
begin
  case sprache of 'D' : write('Chip Erase fehlgeschlagen');
                  'N' : write('Het wissen van de chip is mislukt');
                   else write('Chip Erase failure');
  end{case};
  if length(grund)>0 then write(' ',grund);
  writeln('.');
  write('Ignorieren (J/N)?');
  GetKey('');
  ok:=(d in ['j','J','y','Y']);
  writeln;
end{EraseFailure};

procedure EraseAMD1; { Am29Fxxx und kompatible }
{$I er_amd1.inc}
begin
  kes_load_run_const(er_amd1);
  ok:=Kes_IOPB^.done_bytes_h=$80;
end{EraseAMD1};

procedure WriteFailure(grund:string);
begin
  case sprache of 'D' : write('Schreiben fehlgeschlagen');
                  'N' : write('Schrijf fout');
                   else write('Write failure');
  end{case};
  if length(grund)>0 then write(' ',grund);
  writeln('.');
  write('Ignorieren (J/N)?');
  GetKey('');
  ok:=(d in ['j','J','y','Y']);
  writeln;
end{WriteFailure};

procedure ChipErase28F0x0;
{$I er_28f1.inc}
begin
  Kes_IOPB^.requested_bytes_h:=memsize shr 12;
  kes_load_run_const(er_28f1);
  ok:=(Kes_IOPB^.done_bytes_h and $ff)=(memsize shr 12);
  adr:=(Kes_IOPB^.done_bytes_l and $FFF)+((Kes_IOPB^.done_bytes_h and $ff) shl 12);
  if not ok then EraseFailure('Cycle limit exceeded at $'+ATH(adr));
end{ChipErase28F0x0};

procedure Erase28F001;
  {$I er_28f2.inc}
  procedure EraseBlock(a:longint);
  var t   : longint;
      wsm : byte; { Status Write State Machine }
  begin
    Kes_IOPB^.requested_bytes_l:=(a and $0fff) or $9000;
    Kes_IOPB^.requested_bytes_h:=a shr 12;
    kes_load_run_const(er_28f2);
    ok:=(Kes_IOPB^.done_bytes_l and $80)=$80;
    WSM:=Kes_IOPB^.done_bytes_l and $FF;
    if ((WSM and $38)<>$0) then ok:=false;
    if not ok then begin
      case sprache of 'D' : write(', Fehler:');
                      'N' : write(', Fout:');
                       else write(', Error:');
      end{case};
      if (WSM AND $08)=$08 then write(' Vpp Range (WSM=$',BTH(WSM),')');
      if (WSM AND $30)=$30 then write(' Command Sequence (WSM=$',BTH(WSM),')');
      if (WSM AND $20)=$20 then write(' Block Erase (WSM=$',BTH(WSM),')');
      if (WSM AND $38)=$0  then write(' Time-Out.');
    end;
    writeln;
    if (WSM AND $08)=$08 then writeln('Jumper JP1 = +12V?');
  end;
begin { Erase28F001 }
  write('Erase: Boot');   EraseBlock(boot);
  write('Erase: Para1');  EraseBlock(para1);
  write('Erase: Para2');  EraseBlock(para2);
  write('Erase: Main');   EraseBlock(main);
end{Erase28F001};

{$I fl_28f1.inc}
procedure Zap28F0x0; { Baustein mit $00 vollschreiben }
begin { Zap28F0x0 }
  adr:=0;
  for i:=0 to sizeof(puffer)-1 do puffer[i]:=0;
  while ok and (adr<memsize) do begin
    write(ATH(adr));
    i:=0;
    Flash_Kes(fl_28f1);
    write(#8#8#8#8#8#8);
  end;
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  write(ATH(adr)); writeln;
  if not ok then 
    begin
      case sprache of 'D' : writeln('$00-Schreiben fehlgeschlagen (Cycle limit exceeded).');
                      'N' : writeln('$00-Schrijf fout (Cyclus limiet overschreden).');
                      else writeln('$00-Write failure (Cycle limit exceeded).');
      end{case};
      write('Ignorieren (J/N)?');
      GetKey('');
      ok:=(d in ['j','J','y','Y']);
      writeln;
    end;
end{Zap28F0x0};

procedure Erase28F0x0;
begin
  case sprache of 'D' : write('$00-Schreiben: $'); 
                  'N' : write('$00-Schrijven: $');
                   else write('$00-Writing: $');
  end{case};
  Zap28F0x0;
  if ok then begin
    writeln('Chip-Erase...');
    ChipErase28F0x0;
  end;
end{Erase28F0x0};

{$I fl_amd1.inc}
{$I fl_amd2.inc}

procedure WriteByteAMD; { Am28Fxxx; Am29Fxxx und kompatible }
{ Achtung: Anpassung der Device-Typen in [] !!! }
begin { WriteByteAMD }
  while ok and (adr<datsize) do begin
    if (adr<=(memsize-sizeof(puffer))) then fread;
    write(ATH(adr));
    i:=0;
    if dev in [$20,$B0,$34,$A4,$0B,$E2] then Flash_Kes(fl_amd2);
    if dev in [$A2,$29] then Flash_Kes(fl_amd1);
    write(#8#8#8#8#8#8);
  end;
  write(ATH(adr)); writeln;
  if not ok then WriteFailure('(Time-Out)');
end{WriteByteAMD};

procedure Write28F001;
var WSM : byte;
{$I fl_28f2.inc}
begin { Write28F001 }
  WMem(adr,$FF); { Reset }
  WMem(adr,$50); { Clear Status Register }
  while ok and (adr<datsize) do begin
    if (adr<=(memsize-sizeof(puffer))) then fread;
    write(ATH(adr));
    i:=0;
    Flash_Kes(fl_28f2);
    if not ok then begin
      case sprache of 'D' : write(', Fehler:');
                      'N' : write(', Fout:');
                       else write(', Error:');
      end{case};
      WSM:=Kes_IOPB^.done_bytes_h;
      if (WSM AND $08)=$08 then write(' Vpp Range (WSM=$',BTH(WSM),')');
      if (WSM AND $30)=$30 then write(' Command Sequence (WSM=$',BTH(WSM),')');
      if (WSM AND $20)=$20 then write(' Block Erase (WSM=$',BTH(WSM),')');
      if (WSM AND $38)=$0  then write(' Time-Out.');
      writeln;
      if (WSM AND $08)=$08 then writeln('Jumper JP1 = +12V?');
    end;
    write(#8#8#8#8#8#8);
  end;
  write(ATH(adr)); writeln;
  if not ok then WriteFailure('(Time-Out)');
end{Write28F001};

procedure Write28F0x0;
begin { Write28F0x0 }
  while ok and (adr<datsize) do begin
    if (adr<=(memsize-sizeof(puffer))) then fread;
    write(ATH(adr));
    i:=0;
    Flash_Kes(fl_28f1);
    write(#8#8#8#8#8#8);
  end;
  write(ATH(adr)); writeln;
  if not ok then WriteFailure('(Time-Out)');
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
end{Write28F0x0};

{ *** Ende globale Teilprozeduren **************************************** }



procedure Loeschen;

  procedure EraseAMD;
  {$I er_amd2.inc}
    procedure ChipErase; { Am28F010A/020A }
    var t : longint;
    begin
      kes_load_run_const(er_amd2);
      t:=tick+181;       { ca. 10s Time-Out fuer ChipErase }
      repeat until ((RMem($0000) AND $80)=$80) or (tick>t);
      if (tick>t) then ok:=false;
    end;
  begin
    if not(dev in [$A7,$2A]) then writeln('Chip-Erase...');
    if dev in [$20,$B0,$34,$A4] then EraseAMD1;
    if dev in [$A7,$2A] then Erase28F0x0;
    if dev in [$A2,$29] then ChipErase;
  end;

  procedure EraseAS; { !!! }
  {$I er_as.inc}
  begin
    writeln('Chip-Erase...');
    kes_load_run_const(er_as);
  end;

  procedure EraseAtmel;
  var t : longint;
  {$I er_at1.inc}
  begin
    writeln('Chip-Erase...');
    kes_load_run_const(er_at1);
    t:=tick+181;     { ca. 10s Time-Out fuer Chip-Erase }
    repeat until ((RMem($0000) AND $80)=$80) or (tick>t);
    if (tick>t) then ok:=false;
    if not ok then EraseFailure('Time-Out');
  end;


  procedure EraseCatalyst;
  begin { EraseCatalyst }
    { Block-Adressen setzen }
    case dev of $94 : begin boot:=$1E000; para1:=$1C000; para2:=$1D000; main:=$0; end;
                $95 : begin main:=$04000; para1:=$02000; para2:=$01000; boot:=$0; end;
    end;
    case dev of $94,$95 : Erase28F001;
                $B4,$BD : Erase28F0x0;
    end;
  end;


  {$I er_sst1.inc}
  procedure EraseHyundai;
    procedure ChipErase; { HY29F002T/B }
    var t : longint;
    begin
      kes_load_run_const(er_sst1);
      t:=tick+1900; { ca. 105s Time-Out fuer ChipErase }
      repeat until ((RMem($40000) AND $80)=$80) or (tick>t);
      if (tick>t) then ok:=false;
    end;
  begin { EraseHyundai }
    case dev of $B0,$34 : begin { HY29F002T/B }
                            writeln('Chip-Erase...');
                            ChipErase;
                          end;
    end;
  end;


  procedure EraseIntel;
  begin { EraseIntel }
    { Block-Adressen setzen }
    case dev of $94 : begin boot:=$1E000; para1:=$1C000; para2:=$1D000; main:=$0; end;
                $95 : begin main:=$04000; para1:=$02000; para2:=$01000; boot:=$0; end;
    end;
    case dev of $94,$95 : Erase28F001;
                $B4,$BD : Erase28F0x0;
    end;
  end;


  procedure EraseMacronix;
  begin { EraseMacronix }
    case dev of $1A,$2A,$3C : Erase28F0x0; { MX28F1000/2000[P|T] }
                $18,$19,$B0,$34,$A4 : begin    { MX29F001/2(N)T/B, MX29F040 }
                                        writeln('Chip-Erase...');
                                        EraseAMD1;
                                      end;
    end;
  end;


  procedure EraseMV;
  var t : longint;
  begin
    writeln('Chip-Erase...');
    kes_load_run_const(er_sst1);
    t:=tick+36;
    repeat until (tick>t);
  end;


  procedure EraseSST;
  {$I er_sst2.inc}
  begin
    writeln('Chip-Erase...');
    if dev in [$07,$10,$B5,$B6,$B7] then kes_load_run_const(er_sst1);
    if dev in [$04] then kes_load_run_const(er_sst2);
  end;

  procedure EraseSTM;
  begin
    case dev of $A8,$AA,$02,$07 : begin
                                    case sprache of 'D' : write('$00-Schreiben: $');
                                                    'N' : write('$00-Schrijven: $');
                                                     else write('Writing $00:   $');
                                    end{case};
                                    Zap28F0x0;
                                    if ok then begin
                                      writeln('Chip-Erase...');
                                      ChipErase28F0x0;
                                    end;
                                  end;
                $B0,$34,$E2     : begin { M29F002/040 }
                                    writeln('Chip-Erase...');
                                    EraseAMD1;
                                  end;
    end;
  end;

  procedure EraseWinbond;
  {$I er_win.inc}
  begin
    writeln('Chip-Erase...');
    Kes_IOPB^.requested_bytes_l:=dev;
    kes_load_run_const(er_win);
  end;

  procedure EraseMitsubi;
  begin
    case dev of $D0 : Erase28F0x0; { M5M28F101 }
    end;
  end;

{ procedure Loeschen }
begin
  Pause(100*67); { 100ms warten }
  case mfg of AMD      : EraseAMD;
              AS       : EraseAS;
              Atmel    : EraseAtmel;
              CSI      : EraseCatalyst;
              Intel    : EraseIntel;
              MX       : EraseMacronix;
              MV       : EraseMV;
              SST      : EraseSST;
              STM      : EraseSTM;
              Winbond  : EraseWinbond;
              Mitsubi  : EraseMitsubi;
  end;
end;

