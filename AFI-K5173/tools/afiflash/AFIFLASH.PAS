program afiflash;   

{ (C) Heise Zeitschriftenverlag GmbH & Co. KG, Hannover, Germany }
{ Beachten Sie unbedingt die Hinweise in LIZENZ.TXT und LIESMICH.TXT }

{ EINBAUEN: AMIC 29040 (VID: $37 DID: $86) == AMD 29F040 }
{ EINBAUEN: AMIC 290021 == AT29C020} 

{ EINBAUEN: AS29F010/002/040, MX29F001/004/020, V29C5001/004, W49F020 }
{ einbauen:  = ATMEL AT29C020 mit J5 auf 12V, J4 normal }

{ BAUSTELLE: AT49F010/020 (Muster fehlen) }

{ mit neuen Chips checken: Pin 1 = /RESET? -> Ident-Funktion! }

{$DEFINE noDEBUG} { Debug-Informationen anzeigen }

{ noch nicht mit Chips getestet: 
  HY29F002T/B,
  V29C51002B, 
  M28F512, 
  W29C010, kompatibel zu W29EE011, 
  AT49F001/2/40T, kompatibel zu /B-Typen
  AMIC 29040 (== AMD 29F040) }

{$A+$B+$D-$E-$F+$G+$I-$N+$O-$R-$S-$V-$X-}

{ neu in 1.64: Macronix MX29F040, STM M29F040 }
{ neu in 1.63: Winbond  W49F002 }

uses CRT, KES {,GO32}; { Readkey, I/O-Zugriff }

const version = '1.64a, ct-ea-20021023';

const UnknownMfg =   0; { unbekannter Hersteller }
      UnknownDev =   0; { unbekannter Bausteintyp }
      AMD        = $01;
      AS         = $52; { Alliance Semiconductor }
      Atmel      = $1F;
      CSI        = $31; { Catalyst Semiconductor Inc. }
      Hyundai    = $AD;
      Intel      = $89;
      MV         = $40; { Mosel-Vitelic }
      MX         = $C2; { Macronix International Co. Ltd. }
      SST        = $BF; { Silicon Storage Technology }
      STM        = $20; { SGS-Thomson Microelectronics}
	  Winbond    = $DA;
	  Mitsubi    = $1C; { Mitsubishi Electronics }
      filler     = $FF; { Fuellbyte, wenn Datei kuerzer als Baustein }

type page = array[0..2047] of byte;

var sprache  : char;         { 'D'eutsch/Default, 'E'nglish, 'N'ederlands }
    filename : string;       { Dateiname zu schreibende Binaerdatei }
    lesen    : boolean;      { nur auslesen }
    datsize  : longint;      { Groesse Binaerdatei }
    mfg,dev  : byte;         { Hersteller- und Typcode }
    lockout  : boolean;      { Boot Block Lockout bei manchen Chips }
    memsize  : longint;      { Kapazitaet des Bausteins in Byte }
    typ      : string;       { Typenbezeichnung des Bausteins }
    infile   : file;         { Datei fuer's Einlesen... }
    outfile  : file of page; { ...und fuer's Ausgeben }
    puffer   : page;         { und Puffer fuer BIOS-Inhalt }
	adr      : longint;      { fortlaufende Zieladresse im Flash }
	info     : word;
    ok,                      { solange alles gutgeht... }
    autodetect,              { Baustein automatisch erkennen? }
    autoident,               { wurde auch automatisch erkannt }
    protected,               { Boot Block ist geschuetzt }
    verify,
    J4,J5,RESPIN
             : boolean;      { 12-V-Rangierung, true -> Jx=12V, bzw. Pin 1 = /Reset }
    dummy    : byte;
    i        : word;
    d        : char;

    boot,para1,para2,main : longint; { bausteinspezifische Variablen }

function Tick:longint;
begin
  Tick:=meml[$40:$6C];
end;

procedure fread;
var i : word;
begin
  if adr+2048<=datsize then 
    blockread(infile,puffer,2048)
  else 
    begin
      for i:=0 to 2047 do puffer[i]:=255;
      if adr<datsize then
        blockread(infile,puffer,datsize-adr);
    end;    
end;

procedure Pause(n:longint); { wartet n * 15,6 us }
var i,j : longint;
begin
  j:=n shr 1;
  for i:=1 to j do;
end;

function NTH(data:Byte):char;
var h:char;
begin
  case data of
    0..9:h:=chr(48+data);
    else h:=chr(55+data);
  end;
  NTH:=h;
end; {NTH}

function BTH(data:Byte):string;
var h:string;
begin
  h:=NTH(data shr 4)+NTH(data and $F);
  BTH:=h;
end; {BTH}

function WTH(data:word):string;
var h:string;
begin
  H:=BTH(Byte(data shr 8))+BTH(Byte(data and $FF));
  WTH:=h;
end; {WTH}

function ATH(data:longint):string;
var h:string;
begin
  H:=BTH(Byte(data shr 16))+WTH(Word(data and $FFFF));
  ATH:=h;
end; {ATH}

function HTN(data:char):byte;
var c : byte;
begin
  if (data<'0') or (data>'F') or ((data>'9') and (data<'A')) then begin
    case sprache of 'D' : writeln('Ung'#129'ltiges Hex-Zeichen: ',data); 
                    'N' : writeln('Ongeldig hexadecimaal karakter: ',data);
                     else writeln('Invalid hex character: ',data);
    end{case};
    ok:=false;
  end;
  if ok then begin
    c:=Ord(data)-$30;
    if (c>$9) then c:=c-$7;
    end
   else c:=0;
  HTN:=c;
end;

function HTB(data:string):byte;
begin
  HTB:=HTN(data[1]) SHL 4 + HTN(data[2]);
end;

function HTW(data:string):word;
begin
  HTW:=HTB(copy(data,1,2)) SHL 8 + HTB(copy(data,3,2));
end;

procedure SetAdr(a:longint);
begin
  Mem[$4A:9]:=a SHR 12;
  Mem[$4A:7]:=a AND $FF;
  Mem[$4A:8]:=((a SHR 8) AND $0F) OR $90;   { <-- Flash-Segment im KES }
end;

procedure WMem(a:longint;d:byte);
begin
  SetAdr(a);
  Mem[$4A:10]:=d;
  Mem[$4A:6]:=1;
  Port[$4B]:=1;
  repeat until Mem[$4A:6]=0;
end;

function RMem(a:longint):byte;
begin
  SetAdr(a);
  Mem[$4A:6]:=2;
  Port[$4B]:=1;
  repeat until Mem[$4A:6]=0;
  RMem:=Mem[$4A:10];
end;

procedure GetKey(s:string);
begin
  repeat until KeyPressed;
  d:=ReadKey; writeln(s,d); if d=#0 then begin d:=ReadKey; d:=#0; end;
end;

{$I KESFLASH.INC}

{$I IDENTZ80.INC}
{$I IDENTPAS.INC}

{$I ERASE.INC}

procedure Leertest;
begin
  case sprache of 'D' : write('Leer-Test:     $000000');
                  'N' : write('Wis-controle:  $000000');
                   else write('Erasure check: $000000');
  end{case};
  adr:=0;
  while (adr<memsize) and ok do begin
    write(#8#8#8#8#8#8,ATH(adr));
    Mem[$4A:9]:=adr SHR 12;     { Segment }
    Memw[$4A:7]:=$9000;         { Startaddresse Flash-Bereich in der KES }
    Mem[$4A:6]:=3;              { Kommando: Leertest }
    Port[$4B]:=1;               { KES Wakeup Chan 2 }
    repeat until Mem[$4A:6]=0;  { Warten bis die KES fertig ist }
    ok:=(Memw[$4A:7]=$A000);    { Bis zum Ende durchgelaufen? }
    if ok then adr:=adr+$1000 
          else adr:=(adr and $fffff000) or (Memw[$4A:7] and $0fff);
  end;
  write(#8#8#8#8#8#8,ATH(adr)); writeln;
  if not ok then 
     case sprache of 
       'D' : writeln('Baustein ist nicht komplett gel'+#148+'scht, bitte nochmal versuchen.');
       'N' : writeln('Chip is niet geheel gewist, probeer het nog eens aub.');
       else  writeln('Chip didn''t erase completely, please try again.');
     end;
end;

{$I WRITE.INC}

procedure Pruefen;
begin
  reset(infile,1);
  verify:=true;
  adr:=0;
  case sprache of 'D' : write('Verifiziere:   $');
                  'N' : write('Controleren:   $');
                   else write('Verifying:     $');
  end{case};
  while (adr<datsize) and verify do begin
    fread;
    write(ATH(adr));
    i:=0;
    SetAdr(adr);
    Mem[$4A:6]:=4;              { Kommando: Blockread }
    Port[$4B]:=1;               { KES Wakeup Chan 2 }
    repeat until Mem[$4A:6]=0;  { Warten bis die KES fertig ist }
    Kes_Buffer_Transfer(false, $800);
    while (i<sizeof(puffer)) and verify do begin
      verify:=(Kes_Data[i]=puffer[i]);
      inc(adr); inc(i);
      if not verify then dec(adr);
    end;
    write(#8#8#8#8#8#8);
  end;
  write(ATH(adr)); writeln;
  if not verify then case sprache of 'D' : writeln('Verify fehlgeschlagen, bitte nochmal probieren.');
                                     'N' : writeln('Controle mislukt, probeer het nog eens aub.');
                                      else writeln('Verify mismatch, please try again.');
                     end{case};
  close(infile);
end;

procedure Auslesen;
begin
  assign(outfile,filename);
  rewrite(outfile);
  adr:=0;
  case sprache of 'D' : write('Auslesen:  $',ATH(adr));
                  'N' : write('Lezen:     $',ATH(adr));
                   else write('Reading:   $',ATH(adr));
  end{case};
  while (adr<memsize) do begin
    i:=0;
    write(#8#8#8#8#8#8,ATH(adr)); 
    SetAdr(adr);
    Mem[$4A:6]:=4;              { Kommando: Blockread }
    Port[$4B]:=1;               { KES Wakeup Chan 2 }
    repeat until Mem[$4A:6]=0;  { Warten bis die KES fertig ist }
    Kes_Buffer_Transfer(false, $800);
    while (i<sizeof(puffer)) do begin
      puffer[i]:=Kes_Data[i];
      inc(i); inc(adr);
    end;
    write(outfile,puffer);
  end;
  write(#8#8#8#8#8#8,ATH(adr));
  close(outfile);
  writeln;
  case sprache of 'D' : writeln('Fertig.');
                  'N' : writeln('Klaar.');
                   else writeln('Ready.');
  end{case};
end;

begin { Hauptprogramm }

  autodetect:=true;
  autoident:=true;
  sprache:='D';
  if (ParamCount>0) then begin
    for i:=ParamCount downto 1 do begin
      filename:=ParamStr(i);
      if filename[1] in ['-','/'] then begin
        d:=filename[2];
        if d in ['n','N'] then sprache:='N';
        if d in ['e','E'] then sprache:='E';
        if d in ['m','M'] then autodetect:=false;
      end;
    end{for};
  end;

  writeln('AFIFLASH -- Robotron A7150 (CM1910) IDE-Controller Firmware Tool');
  case sprache of 'D' : writeln('basierend auf CTFLASH -- c''t-Flash-Schreiber, Version ',version);
                  'N' : writeln('gebaseerd op CTFLASH -- c''t-Flash-Programmer, Versie ',version);
                   else writeln('based on CTFLASH -- c''t-Flash-Writer, Version ',version)
  end{case};
  case sprache of 'D' : writeln('Niederl'+#132+'ndische '+#154+'bersetzung+Verbesserung: A.S. Kerkmeester, www.flashbios.org');
                  'N' : writeln('Nederlandse vertaling en verbeteringen: A.S. Kerkmeester, www.flashbios.org');
                   else writeln('Dutch translation and improvement: A.S. Kerkmeester, www.flashbios.org')
  end{case};
  writeln('A7150-Port 2022-25 von Stefan Berndt');

  if (ParamCount>=2) and (ParamCount<=4) then begin
    ok:=true;

    Lesen:=(ParamStr(1)<>'W') and (ParamStr(1)<>'w');
    filename:=ParamStr(2);
    datsize:=0;

    { Init Flash-Handler Program in KES-System }
    Kes_Init;
    Kes_Buffer_Clear;
    Kes_Load_Exec_Const(Kesprog);
    Kes_Buffer_Transfer(true, sizeof(Kesprog));
    Kes_Exec;
    
    if not Lesen then begin
      assign(infile,filename);
      {$I-}
      reset(infile,1);
      {$I+}
      if (IOResult<>0) then begin
        ok:=false;
        case sprache of 'D' : writeln('Konnte Datei ',filename,' nicht '+#148+'ffnen.');
                        'N' : writeln('Kon het bestand ',filename,' niet openen.');
                        else writeln('Couldn''t open file ',filename,'.')
        end{case};
        end
       else begin
        datsize:=filesize(infile);
        close(infile);
      end;
    end;

    if ok then begin
      mfg:=UnknownMfg; dev:=UnknownDev;

    if autodetect then Identify;

    if (mfg=UnknownMfg) or (dev=UnknownDev) or (memsize=0) then begin

        if autodetect and (mfg=UnknownMfg) then begin
        autoident:=false;
        case sprache of 'D' : begin
                                write('Baustein wurde nicht automatisch erkannt: ');
                                writeln('Hersteller=$',BTH(mfg),', Device=$',BTH(dev));
                                writeln('Manche Bausteine ben'+#148+'tigen +12V, damit die automatische Erkennung');
                                writeln('funktioniert.');
                                end;
                        'N' : begin
                                write('Kon de chip niet automatisch detecteren: ');
                                writeln('Fabrikant=$',BTH(mfg),', Device=$',BTH(dev));
                                writeln('Sommige chips hebben +12V nodig om automatisch te kunnen worden herkend.');
                                end;
                            else begin
                                write('Could''nt identify chip automatically: ');
                                writeln('Manufacturer=$',BTH(mfg),', Device=$',BTH(dev));
                                writeln('Some chips need +12V in order to be automatically recognizable.');
                                end;
        end{case};
        end
        else if autodetect then begin
        case sprache of 'D' : write('Baustein wurde nicht vollst'+#132+'ndig erkannt: Hersteller ',MfgStr(mfg),' ');
                        'N' : write('De chip is niet geheel herkent: Merk ',MfgStr(mfg),' ');
                            else write('Chip wasn''t recognized completely: Manufacturer ',MfgStr(mfg),' ')
        end{case};
        writeln(', Device=$',BTH(dev));
        end;
        if not Lesen then ManualSelect else begin
        case sprache of 'D' : writeln('Bitte geben Sie die Kapazit'+#132+'t vor:');
                        'N' : writeln('Geef de te lezen capaciteit op:');
                            else writeln('Please select capacity to be read:')
        end{case};
        writeln('(1)  32 KByte');
        writeln('(2)  64 KByte');
        writeln('(3) 128 KByte');
        writeln('(4) 256 KByte');
        writeln('(5) 512 KByte');
        GetKey('');
        case d of '1' : memsize:= 32*1024;
                    '2' : memsize:= 64*1024;
                    '3' : memsize:=128*1024;
                    '4' : memsize:=256*1024;
                    '5' : memsize:=512*1024;
                    else memsize:=0;
        end;
        mfg:=UnknownMfg; dev:=0;
        end;
    end;

    if (mfg<>UnknownMfg) and (dev<>UnknownDev) then
        case sprache of 'D' : writeln('Bausteintyp ',typ,' von ',MfgStr(mfg),', Kapazit'+#132+'t: ',memsize SHR 10,' KByte');
                        'N' : writeln('Chip type ',typ,' van ',MfgStr(mfg),', capaciteit: ',memsize SHR 10,' KByte');
                        else writeln('Chip type ',typ,' from ',MfgStr(mfg),', capacity: ',memsize SHR 10,' KByte')
        end{case};

    if (mfg=Atmel) then
      begin
        if (info and $ff) = $fe then writeln('Low Bootblock programmable.');
        if (info and $ff) = $ff then writeln('Low Bootblock locked!');
        if (info and $ff00) = $fe00 then writeln('High Bootblock programmable.');
        if (info and $ff00) = $ff00 then writeln('High Bootblock locked!');
      end;

    if ok and not Lesen and (mfg<>UnknownMfg) and (memsize>0) then begin

        if (J5 or J4) and not autoident then begin
        case sprache of 'D' : write('Beim ',typ,' ');
                        'N' : write('Bij het ',typ,' ');
                            else write('With the ',typ,' ')
        end{case};
        if J5 and J4 then case sprache of 'D' : write('m'+#129+'ssen ');
                                            'N' : write('moeten ');
                            end{case}


            else case sprache of 'D' : write('muss ');
                                'N' : write('moet ');
                end{case};
        if J5 then write('J5 ');
        if J5 and J4 then case sprache of 'D' : write('und ');
                                            'N' : write('en ');
                                            else write('and ')
                            end{case};
        if J4 then write('J4 ');
        case sprache of 'D' : writeln('auf +12V gesteckt sein.');
                        'N' : writeln('op +12V gezet zijn.');
                            else begin 
                                if J4 then write('have ') else write('has ');
                                writeln('to be set to +12V.');
                                end
        end{case};
        case sprache of 'D' : write('Fortfahren (J/N)? ');
                        'N' : write('Verder gaan (J/N)? ');
                            else write('Continue (Y/N)? ')
        end{case};
        GetKey('');
        if not (d in ['J','j','y','Y']) then ok:=false;
        end;

        if ok then Loeschen;
        if ok then Leertest;
        if ok then Schreiben;
        if ok then Pruefen;
        if ok then case sprache of 'D' : writeln('Fertig.');
                                    'N' : writeln('Klaar.');
                                    else writeln('Complete.')
                    end{case};
        if J5 or J4 then case sprache of 'D' : writeln('Vergessen Sie nicht, J5/J4 zur'+#129+'ckzustecken.');
                                        'N' : writeln('Vergeet niet J5/J4 terug te zetten op hun originele posities.');
                                        else writeln('Don''t forget to set J5/J4 back to their original positions.')
                            end{case};

    end;

    if ok and (memsize>0) and Lesen then Auslesen;

    end;
   end

   else begin
      case sprache of 'D' : writeln('AFIFLASH schreibt eine Bin'+#132+'rdatei in den Firmware-Chip des AFI-Modules.');
                      'N' : writeln('AFIFLASH schrijft een binair bestand in de firmwarechips van de AFI-Module.');
                       else writeln('AFIFLASH writes a binary file into the firmware chips of the AFI-Module.')
      end{case};

      case sprache of 'D' : begin
      writeln('Aufruf: afiflash R/W FILE [/M] [/N|E]');
      writeln('                 R/W                 - ''R'': auslesen, ''W'': schreiben');
      writeln('                     FILE            - kompletter Pfad zur Bin'+#132+'rdatei');
      writeln('                           /M        - manuelle Auswahl des Bausteintyps');
      writeln('                                /N|E - Nederlands|English');
      writeln('Also beispielsweise: afiflash W A:\VER0108.BIN');
      writeln('Diese Version beherrscht folgende EEPROM- bzw. Flash-Chips:');
      end;

      'N' : begin
      writeln('Gebruik:  afiflash R/W FILE [/M] [/N|E]');
      writeln('                   R/W                 - ''R'': uitlezen, ''W'': beschrijven');
      writeln('                       FILE            - volledig path van binaire bestand');
      writeln('                             /M        - forceer handmatige chip selectie');
      writeln('                                  /N|E - Nederlands|English');
      writeln('Voorbeeld: afiflash W A:\VER0108.BIN');
      writeln('Deze versie kan de volgende EEPROM en flash chips aan:');
      end;

      else begin
      writeln('Usage:  afilash R/W FILE [/M] [/N|E]');
      writeln('                R/W                 - ''R'': read into, ''W'': write from file');
      writeln('                    FILE            - full path to binary file');
      writeln('                          /M        - force manual chip selection');
      writeln('                               /N|E - Nederlands|English');
      writeln('For example: afiflash W A:\VER0108.BIN');
      writeln('This version handles the following EEPROM and flash chips:');
      end
      end{case};

      writeln('AMD            Am28F010(A)/20(A), Am29F010/02/40(=A29040)');
      writeln('Atmel          AT29C512/010A/20(=A290021)/40A, AT49F001/02/40');
      writeln('Catalyst (CSI) CAT28F001Bx-T/B, CAT28F010/20');
      writeln('Hyundai        HY29F002T/B');
      writeln('Intel          i28F001Bx-T/B, i28F010(Z28F010)/20');
      writeln('Macronix       MX28F1000P (=H.T.M21xxx), MX28F2000P/T, MX29F002(N)T/B/040');
      writeln('Mosel-Vitelic  F/V29C51002T/B (Boot Block unprotected!)');
      writeln('SST            PH29EE010/20, PH28SF040, PH39SF010/20');
      writeln('ST Microelec.  M28F256(A)/512, M28F101, M29F002T/NT/B/040');
      writeln('Winbond        W29EE011 (=AE29F1008), W29C010/11/20(=AE29F2008)/40, W49F002');
      writeln('Mitsubishi     M5M28F101');
    end;
end.

