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

  {$I fl_sst1.inc}
  {$I fl_at1.inc}
  {$I fl_at2.inc}

  procedure WriteAtmel;
  var s: word;
  begin { WriteAtmel }
    i:=sizeof(puffer); s:=sizeof(puffer);
    while ok and (adr<datsize) do begin
      if (i>=s) and (adr<=(memsize-s)) then begin
        fread;
        i:=0;
      end;
      write(ATH(adr));
      case dev of $5D,$D5 : Flash_Kes(fl_at1);   { AT29C512/C010 }
                  $DA,$A4 : Flash_Kes(fl_at2);   { AT29C20/40A }
                  $05,$04,$07,$08,
                  $17,$0B,$13,$12 : Flash_Kes(fl_sst1); { AT49F001/2, AT49F010/20/40 }
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
  {$I fl_mv.inc}
  begin { WriteMV }
    s:=sizeof(puffer); i:=s;
    while ok and (adr<datsize) do begin
      if (i>=s) and (adr<=(memsize-s)) then begin
        fread;
        i:=0;
      end;
      write(ATH(adr));
      Flash_Kes(fl_mv);
      write(#8#8#8#8#8#8);
    end;
    write(ATH(adr)); writeln;
    if not ok then WriteFailure('');
  end;

  procedure WriteSST;
  {$I fl_sst2.inc}
  {$I fl_sst3.inc}
  begin { WriteSST }
    i:=sizeof(puffer);
    while ok and (adr<datsize) do begin
      if (i>=sizeof(puffer)) and (adr<=(memsize-sizeof(puffer))) then begin
        fread;
        i:=0;
      end;
      write(ATH(adr));
      case dev of $07,$10     : Flash_Kes(fl_sst3); { PH29EE010/020 }
                  $04         : Flash_Kes(fl_sst2); { PH28SF040 }
                  $B5,$B6,$B7 : Flash_Kes(fl_sst1); { PH39SF010/20 }
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
    procedure WriteWinPage;
    begin
      end{case};
      i:=sizeof(puffer);
      while ok and (adr<datsize) do begin
        if (i>=sizeof(puffer)) and (adr<=(memsize-sizeof(puffer))) then begin
          fread;
          i:=0;
        end;
        write(ATH(adr));
        case dev of $C1,$45 : Flash_Kes(fl_at1);
                    $46     : Flash_Kes(fl_at2);
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

  procedure WriteMitsubi;
  begin
    case dev of $D0 : Write28F0x0; { M5M28F101 }
    end;
  end;

{ procedure Schreiben }
begin
  reset(infile,1);
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
              Mitsubi  : WriteMitsubi;
  end;
  close(infile);
end;
