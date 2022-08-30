
procedure Schreiben;

  procedure WriteAMD;
  begin { WriteAMD }
    case dev of $A2,$29,$20,$B0,$34,$A4 : WriteByteAMD;
                $A7,$2A                 : Write28F0x0;
    end;
  end;

  procedure WriteAS; { !!! }
  begin { WriteAS }
  end;

  procedure WriteAtmel;
  var s,pagesize : word;
    procedure WritePage;
    var n : word;
        t : longint;
    begin
      asm CLI end; {disable}
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$A0);
      for n:=1 to pagesize do begin
        WMem(adr,puffer[i]);
        inc(adr); inc(i);
      end;
      Pause(20); { 330 us warten }
      asm STI end; {enable}
      t:=tick+2; { ca. 50ms Time-Out fuer Page-Write }
      repeat until (RMem(adr-1)=puffer[i-1]) or (tick>t);
      if (tick>t) then ok:=false;
      asm CLI end; {disable}
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$F0);
      asm STI end; {enable}
    end;
    procedure WritePuffer;
    var t : longint;
    begin
      while ok and (i<s) do begin
        asm CLI end; {disable}
        WMem($5555,$AA);
        WMem($2AAA,$55);
        WMem($5555,$A0);
        WMem(adr,puffer[i]);
        asm STI end; {enable}
        t:=tick+2; { ca. 50ms Time-Out (jenseits von gut und boese) fuer Byte-Write }
        repeat until (RMem(adr)=puffer[i]) or (tick>t);
        if (tick>t) then ok:=false;
        inc(adr); inc(i);
      end;
    end;
  begin { WriteAtmel }
    case dev of $5D,$D5         : pagesize:=128; { AT29C512/C010A }
                $DA,$A4         : pagesize:=256; { AT29C020/40A }
    end{case};
    i:=sizeof(puffer); s:=sizeof(puffer);
    while ok and (adr<memsize) do begin
      if (i>=s) and (adr<=(memsize-s)) then begin
        fread;
        i:=0;
      end;
      write(ATH(adr));
      case dev of $5D,$D5,$DA,$A4 : WritePage;   { AT29C512/C010/20/40A }
                  $05,$04,$07,$08,
                  $17,$0B,$13,$12 : WritePuffer; { AT49F001/2, AT49F010/20/40 }
      end{case};
      write(#8#8#8#8#8#8);
    end;
    write(ATH(adr)); writeln;
    if not ok then WriteFailure('');
  end;

  procedure WriteCatalyst;
  begin { WriteCatalyst, same as WriteIntel, Second Source }
    case dev of $94,$95 : Write28F001;
                $B4,$BD : Write28F0x0;
    end;
  end;

  procedure WriteHyundai;
  begin { WriteHyundai }
    case dev of $B0,$34 : WriteByteAMD;
    end;
  end;

  procedure WriteIntel;
  begin { WriteIntel }
    case dev of $94,$95 : Write28F001;
                $B4,$BD : Write28F0x0;
    end;
  end;

  procedure WriteMacronix;
  begin { WriteMacronix }
    case dev of $1A,$2A,$3C         : Write28F0x0;
                $18,$19,$B0,$34,$A4 : WriteByteAMD;
    end;
  end;

  procedure WriteMV;
  var s : word;
    procedure WritePage;
    var t : longint;
        b : byte;
    begin
      while ok and (i<s) do begin
        b:=puffer[i];
        asm CLI end; {disable}
        WMem($5555,$AA);
        WMem($2AAA,$55);
        WMem($5555,$A0);
        WMem(adr,b);
        asm STI end; {enable}
        t:=tick+2; { ca. 50ms Time-Out fuer Byte-Write }
        repeat until (RMem(adr)=b) or (tick>t);
        asm CLI end; {disable}
        WMem($5555,$AA);
        WMem($2AAA,$55);
        WMem($5555,$F0);
        asm STI end; {enable}
        if (tick>t) then ok:=false;
        inc(adr); inc(i);
      end;
    end;
  begin { WriteMV }
    s:=sizeof(puffer); i:=s;
    while ok and (adr<memsize) do begin
      if (i>=s) and (adr<=(memsize-s)) then begin
        fread;
        i:=0;
      end;
      write(ATH(adr));
      WritePage;
      write(#8#8#8#8#8#8);
    end;
    write(ATH(adr)); writeln;
    if not ok then WriteFailure('');
  end;

  procedure WriteSST;
    procedure WritePage128;
    var n : word;
        t : longint;
    begin
      asm CLI end; {disable}
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$A0);
      for n:=0 to 127 do begin
        WMem(adr,puffer[i]);
        inc(adr); inc(i);
      end;
      Pause(20); { 330 us warten }
      asm STI end; {enable}
      t:=tick+2; { ca. 50ms Time-Out fuer Page-Write }
      repeat until (RMem(adr-1)=puffer[i-1]) or (tick>t);
      if (tick>t) then ok:=false;
      asm CLI end; {disable}
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$F0);
      asm STI end; {enable}
    end;
    procedure WritePuffer;
    var t : longint;
    begin
      while ok and (i<sizeof(puffer)) do begin
        asm CLI end; {disable}
        case dev of $04     : WMem(adr,$10); { Byte Program }
                    $B5,$B6,$B7 : begin
                                WMem($5555,$AA);
                                WMem($2AAA,$55);
                                WMem($5555,$A0);
                              end;
        end;
        WMem(adr,puffer[i]);
        asm STI end; {enable}
        t:=tick+2; { ca. 50ms Time-Out (jenseits von gut und boese) fuer Byte-Write }
        repeat until (RMem(adr)=puffer[i]) or (tick>t);
        if (tick>t) then ok:=false;
        inc(adr); inc(i);
      end;
    end;
  begin { WriteSST }
    i:=sizeof(puffer);
    while ok and (adr<memsize) do begin
      if (i>=sizeof(puffer)) and (adr<=(memsize-sizeof(puffer))) then begin
        fread;
        i:=0;
      end;
      write(ATH(adr));
      case dev of $07,$10     : WritePage128; { PH29EE010/020 }
                  $04,$B5,$B6,$B7 : WritePuffer;  { PH28SF040, PH39SF010/20 }
      end;
      write(#8#8#8#8#8#8);
    end;
    write(ATH(adr)); writeln;
    if not ok then WriteFailure('');
    if ok and (dev in [$04]) then begin
      dummy:=RMem($1823);   { Software Data Protect }
      dummy:=RMem($1820);
      dummy:=RMem($1822);
      dummy:=RMem($0418);
      dummy:=RMem($041B);
      dummy:=RMem($0419);
      dummy:=RMem($040A);
    end;
  end;

  procedure WriteSTM;
  begin { WriteSTM }
    case dev of $A8,$AA,$02,$07 : Write28F0x0;  { M28F256(A)/512, M28F101 }
                $B0,$34,$E2     : WriteByteAMD; { M29F002T/NT/B, M29F040 }
    end;
  end;

  procedure WriteWinbond;
  var pagesize : word;
    procedure WritePage;
    var n : word;
        t : longint;
    begin
      asm CLI end; {disable}
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$A0);
      for n:=1 to pagesize do begin
        WMem(adr,puffer[i]);
        inc(adr); inc(i);
      end;
      Pause(667); { 10ms warten }
      asm STI end; {enable}
      t:=tick+2; { ca. 50ms Time-Out fuer Page-Write }
      repeat until (RMem(adr-1)=puffer[i-1]) or (tick>t);
      if (tick>t) then ok:=false;
      asm CLI end; {disable}
      WMem($5555,$AA);
      WMem($2AAA,$55);
      WMem($5555,$F0);
      asm STI end; {enable}
    end;
    procedure WriteWinPage;
    begin
      case dev of $C1,$45 : pagesize:=128;
                  $46     : pagesize:=256;
      end{case};
      i:=sizeof(puffer);
      while ok and (adr<memsize) do begin
        if (i>=sizeof(puffer)) and (adr<=(memsize-sizeof(puffer))) then begin
          fread;
          i:=0;
        end;
        write(ATH(adr));
        WritePage;
        write(#8#8#8#8#8#8);
      end;
      write(ATH(adr)); writeln;
      if not ok then WriteFailure('');
    end;
  begin { WriteWinbond }
    case dev of $C1,$45,$46 : WriteWinPage;
                $0B         : WriteByteAMD; { W49F002 }
    end;
  end;

{ procedure Schreiben }
begin
  reset(infile);
  adr:=0;
  case sprache of 'D' : write('Programmiere:  $');
                  'N' : write('Programmeren:  $');
                   else write('Programming:   $');
  end{case};
  case mfg of AMD      : WriteAMD;
              Atmel    : WriteAtmel;
              AS       : WriteAS;
              CSI      : WriteCatalyst;
              Hyundai  : WriteHyundai;
              Intel    : WriteIntel;
              MX       : WriteMacronix;
              MV       : WriteMV;
              SST      : WriteSST;
              STM      : WriteSTM;
              Winbond  : WriteWinbond;
  end;
  close(infile);
end;
