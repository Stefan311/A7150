{ *** globale Teilprozeduren fr alle Chiptypen ************************** }

procedure EraseFailure(grund:string);
begin
  case sprache of 'D' : write('Chip Erase fehlgeschlagen');
                  'N' : write('Het wissen van de chip is mislukt');
                   else write('Chip Erase failure');
  end{case};
  if length(grund)>0 then write(' ',grund);
  writeln('.');
end{EraseFailure};

procedure EraseAMD1; { Am29Fxxx und kompatible }
var t : longint;
begin
  Pause(10*67); { 10ms warten }
  asm CLI end; {disable}

  WMem($5555,$AA); { Reset/Read }
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(1);        { 15us warten }

  WMem($5555,$AA); { Chip Erase }
  WMem($2AAA,$55);
  WMem($5555,$80);
  WMem($5555,$AA);
  WMem($2AAA,$55);
  WMem($5555,$10);
  asm STI end; {enable}
  t:=tick+541; { ca. 30s Time-Out fuer ChipErase }
  repeat until ((RMem($0000) AND $80)=$80) or (tick>t);
  if (tick>t) then ok:=false;
  Pause(1);        { 15us warten }

  WMem($5555,$AA); { Reset/Read }
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(1);        { 15us warten }
end{EraseAMD1};

procedure WriteFailure(grund:string);
begin
  case sprache of 'D' : write('Schreiben fehlgeschlagen');
                  'N' : write('Schrijf fout');
                   else write('Write failure');
  end{case};
  if length(grund)>0 then write(' ',grund);
  writeln('.');
end{WriteFailure};

procedure ChipErase28F0x0;
var cyc     : word;
    EraseOK : boolean;
begin
  cyc:=0;
  adr:=0;
  repeat
    asm CLI end; {disable}
    WMem(adr,$20);   { Erase Setup }
    WMem(adr,$20);   { Erase Start }
    asm STI end; {enable}
    Pause(667);      { 10ms warten }
    repeat
      asm CLI end; {disable}
      WMem(adr,$A0); { Erase Verify }
      Pause(1);      { 15us warten  }
      EraseOK:=(RMem(adr)=$FF);
      if EraseOK then inc(adr) else inc(cyc);
      asm STI end; {enable}
    until (adr>=memsize) or not(EraseOK);
  until (adr>=memsize) or (cyc>1000);
  ok:=not(cyc>1000);
  if not ok then EraseFailure('Cycle limit exceeded at $'+ATH(adr));
  WMem($0,$00); { Read }
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
end{ChipErase28F0x0};

procedure Erase28F001;
  procedure EraseBlock(a:longint);
  var t   : longint;
      wsm : byte; { Status Write State Machine }
  begin
    asm CLI end; {disable}
    WMem(a,$FF); { Reset }
    WMem(a,$50); { Clear Status Register }
    WMem(a,$20); { EraseSetup }
    WMem(a,$D0); { EraseConfirm }
    asm STI end; {enable}
    t:=tick+379; { Time-Out max. 21s }
    repeat
      asm CLI end; {disable}
      WSM:=RMem(a); { Read Status Register }
      asm STI end; {enable}
    until ((WSM and $80)=$80) or (tick>t);
    asm CLI end; {disable}
    WSM:=RMem(a); { Read Status Register }
    WMem(a,$FF);  { Reset }
    asm STI end; {enable}
    if ((WSM and $38)<>$0) then ok:=false;
    if (tick>t) then ok:=false;
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
    if (WSM AND $08)=$08 then writeln('Jumper J4 & J5 = +12V?');
  end;
begin { Erase28F001 }
  write('Erase: Boot');   EraseBlock(boot);
  write('Erase: Para1');  EraseBlock(para1);
  write('Erase: Para2');  EraseBlock(para2);
  write('Erase: Main');   EraseBlock(main);
end{Erase28F001};

procedure WriteByte28F0x0(a:longint;d:byte);
var o       : byte;
    cyc,max : word;
begin
  asm CLI end; {disable}
  cyc:=0;
  max:=25;       { max. Anzahl Schreibversuche }
  repeat
    WMem(a,$40); { Program Setup  }
    WMem(a,d);   { Write Data     }
    Pause(1);    { 15us warten    }
    WMem(a,$C0); { Program Verify }
    Pause(1);    { 15us warten    }
    o:=RMem(a);
    inc(cyc);
  until (o=d) or (cyc>max);
  asm STI end; {enable}
  ok:=not(cyc>max);
  WMem(0,0);     { Read }
end{WriteByte28F0x0};

procedure Zap28F0x0; { Baustein mit $00 vollschreiben }
begin { ZapI28F0x0 }
  adr:=0;
  while ok and (adr<memsize) do begin
    write(ATH(adr));
    i:=0;
    while ok and (i<sizeof(puffer)) do begin
      WriteByte28F0x0(adr,$00);
      inc(i); inc(adr);
      if not ok then dec(adr);
    end;
    write(#8#8#8#8#8#8);
  end;
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  write(ATH(adr)); writeln;
  if not ok then case sprache of 'D' : writeln('$00-Schreiben fehlgeschlagen (Cycle limit exceeded).');
                                 'N' : writeln('$00-Schrijf fout (Cyclus limiet overschreden).');
                                  else writeln('$00-Write failure (Cycle limit exceeded).');
                 end{case};
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

procedure WriteByte28F0x0A(a:longint;d:byte);
var t       : longint;
begin
  asm CLI end; {disable}
  WMem(a,$50); { Program Setup  }
  WMem(a,d);   { Write Data     }
  Pause(1);    { 15us warten    }
  asm STI end; {enable}
  t:=tick+2;   { ca. 50ms Time-Out, jenseits von gut und boese... }
  repeat until (RMem(a)=d) or (tick>t);
  if (tick>t) then ok:=false;
end{WriteByte28F0x0A};

procedure WriteByteAMD; { Am28Fxxx; Am29Fxxx und kompatible }
{ Achtung: Anpassung der Device-Typen in [] !!! }
var timex,bytewise,byte28f0 : boolean;
    msb : byte;
  procedure WriteByte(a:longint;d:byte);
  var t : longint;
  begin
    msb:=(d and $80);
    asm CLI end; {disable}
    WMem($5555,$AA);
    WMem($2AAA,$55);
    WMem($5555,$A0);
    WMem(a,d);
    Pause(1); { kleine Schreibpause }
    if (mfg=Winbond) and (dev=$0B) then Pause(3); { W49F002 braucht Schreibpause }
    asm STI end; {enable}
    t:=tick+2; { ca. 50ms Time-Out }
    while not (((RMem(a) and $80)=msb) or (tick>t) or timex) do begin
      if ((RMem(a) and $20)=$20) then timex:=((RMem(a) and $80)<>msb);
    end;
    if (tick>t) then ok:=false;
  end;
begin { WriteByteAMD }
  timex:=false;
  while ok and (adr<memsize) do begin
    if (adr<=(memsize-sizeof(puffer))) then fread;
    write(ATH(adr));
    i:=0;
    bytewise:=dev in [$20,$B0,$34,$A4,$0B,$E2];
    byte28f0:=dev in [$A2,$29];
    while ok and (i<sizeof(puffer)) do begin
      if bytewise then WriteByte(adr,puffer[i]);
      if byte28f0 then WriteByte28F0x0A(adr,puffer[i]);
      inc(i); inc(adr);
      if not ok then dec(adr);
    end;
    write(#8#8#8#8#8#8);
  end;
  write(ATH(adr)); writeln;
  if not ok then if timex then WriteFailure('(Time limit exceeded)') else WriteFailure('(Time-Out)');
end{WriteByteAMD};

procedure Write28F001;
  procedure WriteByte28F001(a:longint;d:byte);
  var t   : longint;
      WSM : byte;
  begin
    asm CLI end; {disable}
    WMem(a,$40);
    WMem(a,d);
    asm STI end; {enable}
    t:=tick+2; { ca. 50ms Time-Out }
    repeat
      asm CLI end; {disable}
      WSM:=RMem(a); { Read Status Register }
      asm STI end; {enable}
    until ((WSM and $80)=$80) or (tick>t);
    asm CLI end; {disable}
    WSM:=RMem(a); { Read Status Register }
    WMem(a,$FF);  { Reset }
    asm STI end; {enable}
    if ((WSM and $38)<>$0) then ok:=false;
    if (tick>t) then ok:=false;
    if not ok then begin
      case sprache of 'D' : write(', Fehler:');
                      'N' : write(', Fout:');
                       else write(', Error:');
      end{case};
      if (WSM AND $08)=$08 then write(' Vpp Range (WSM=$',BTH(WSM),')');
      if (WSM AND $30)=$30 then write(' Command Sequence (WSM=$',BTH(WSM),')');
      if (WSM AND $20)=$20 then write(' Block Erase (WSM=$',BTH(WSM),')');
      if (WSM AND $38)=$0  then write(' Time-Out.');
      writeln;
      if (WSM AND $08)=$08 then writeln('Jumper J4 & J5 = +12V?');
    end;
  end;
begin { Write28F001 }
  WMem(adr,$FF); { Reset }
  WMem(adr,$50); { Clear Status Register }
  while ok and (adr<memsize) do begin
    if (adr<=(memsize-sizeof(puffer))) then fread;
    write(ATH(adr));
    i:=0;
    while ok and (i<sizeof(puffer)) do begin
      WriteByte28F001(adr,puffer[i]);
      inc(i); inc(adr);
      if not ok then dec(adr);
    end;
    write(#8#8#8#8#8#8);
  end;
  write(ATH(adr)); writeln;
  if not ok then WriteFailure('(Time-Out)');
end{Write28F001};

procedure Write28F0x0;
begin { Write28F0x0 }
  while ok and (adr<memsize) do begin
    if (adr<=(memsize-sizeof(puffer))) then fread;
    write(ATH(adr));
    i:=0;
    while ok and (i<sizeof(puffer)) do begin
      WriteByte28F0x0(adr,puffer[i]);
      inc(i); inc(adr);
      if not ok then dec(adr);
    end;
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
    procedure ChipErase; { Am28F010A/020A }
    var t : longint;
    begin
      asm CLI end; {disable}
      WMem($0,$FF);      { Reset }
      Pause(1);          { 15 us warten }
      WMem($0,$30);      { Chip Erase }
      WMem($0,$30);
      asm STI end; {enable}
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
    procedure ChipErase;
    begin
      asm CLI end; {disable}
      WMem($5555,$AA); { Chip Erase }
      WMem($2AAA,$55);
      WMem($5555,$80);
      WMem($0000,$FF);
      Pause(1333);     { 20ms warten }
      asm STI end; {enable}
    end;
  begin
    writeln('Chip-Erase...');
    ChipErase;
  end;

  procedure EraseAtmel;
    procedure ChipErase;
    var t : longint;
    begin
      asm CLI end; {disable}
      WMem($5555,$AA); { Chip Erase }
      WMem($2AAA,$55);
      WMem($5555,$80);
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$10);
      Pause(1333);     { 20ms warten }
      asm STI end; {enable}
      t:=tick+181;     { ca. 10s Time-Out fuer Chip-Erase }
      repeat until ((RMem($0000) AND $80)=$80) or (tick>t);
      if (tick>t) then ok:=false;
      if not ok then EraseFailure('Time-Out');
    end;
  begin
    writeln('Chip-Erase...');
    ChipErase;
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


  procedure EraseHyundai;
    procedure ChipErase; { HY29F002T/B }
    var t : longint;
    begin
      asm CLI end; {disable}
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$80);
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$10);
      asm STI end; {enable}
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
    procedure ChipErase;
    begin
      asm CLI end; {disable}
      WMem($5555,$AA); { Chip Erase }
      WMem($2AAA,$55);
      WMem($5555,$80);
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$10);
      Pause(133300);   { 2s warten }
      asm STI end; {enable}
    end;
  begin
    if not protected then begin
      writeln('Chip-Erase...');
      ChipErase;
      end
     else begin
      ok:=false;
      case sprache of 'D' : writeln('Boot Block ist gesch'+#129+'tzt, kann nicht l'+#148+'schen -- siehe Datenblatt.');
                      'N' : writeln('Boot block is beveiligd, wissen lukt niet -- zie datasheet.');
                       else writeln('Boot block is protected, can''t erase -- see datasheet.');
      end{case};
    end;
  end;


  procedure EraseSST;
    procedure ChipErase1;
    begin
      asm CLI end; {disable}
      WMem($5555,$AA); { Chip Erase }
      WMem($2AAA,$55);
      WMem($5555,$80);
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$10);
      Pause(6666);      { 100ms warten statt 20ms wg. 39SF020A }
      asm STI end; {enable}
    end;
    procedure ChipErase2;
    begin
      asm CLI end; {disable}
      WMem($0,$FF);     { Reset }
      dummy:=RMem($1823);   { Software Data Unprotect }
      dummy:=RMem($1820);
      dummy:=RMem($1822);
      dummy:=RMem($0418);
      dummy:=RMem($041B);
      dummy:=RMem($0419);
      dummy:=RMem($041A);
      WMem($0,$30);     { Chip Erase }
      WMem($0,$30);
      Pause(1333);      { 20ms warten }
      asm STI end; {enable}
    end;
  begin
    writeln('Chip-Erase...');
    if dev in [$07,$10,$B5,$B6,$B7] then ChipErase1;
    if dev in [$04] then ChipErase2;
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
    procedure ChipErase;
    begin
      asm CLI end; {disable}
      if (dev<>$0B) then begin
        WMem($5555,$AA); { Software Data Protection Disable }
        WMem($2AAA,$55);
        WMem($5555,$80);
        WMem($5555,$AA);
        WMem($2AAA,$55);
        WMem($5555,$20);
      end;
      Pause(667);      { 10 ms Pause }
      WMem($5555,$AA); { Chip Erase }
      WMem($2AAA,$55);
      WMem($5555,$80);
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$10);
      Pause(66700);     { 1s warten (W49F002) }
      asm STI end; {enable}
    end;
  begin
    writeln('Chip-Erase...');
    ChipErase;
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
  end;
end;

