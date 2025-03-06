{
Copyright (c) 2022, Stefan Berndt

All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, 
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, 
   this list of conditions and the following disclaimer in the documentation 
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
THE POSSIBILITY OF SUCH DAMAGE.
}

program clonehdd;

uses kes,crt;

var 
    jpide: word;
    mfmaktiv: boolean;
    mfmonline: boolean;
    mfmcylinder: word;
    mfmheads: byte;

    pideaktiv: boolean;
    pideonline: boolean;
    pidecylinder: word;
    pideheads: byte;

    sideaktiv: boolean;
    sideonline: boolean;
    sidecylinder: word;
    sideheads: byte;

    a, b: word;
    s: string;
    src,dst: byte;
    cylinders: word;
    heads: byte;
    retrys: word;

	oldchb: word;
	
    cyl: word;
    hd: byte;
    sect: byte;
    readerr: word;
    writeerr: word;
    retrycnt: word;

    error: byte;

function inttostr(i: word):string;
var o: string;
begin
  str(i,o);
  inttostr:=copy(o+'    ',1,5);
end;  

const DiskNames: array[1..3] of string = ('MFM','1. IDE','2. IDE');
{$I detect.inc}    { Detect }
{$I sector.inc}    { Sector }
{$I inithdd.inc}   { InitHDD }
{$I rest_chb.inc}  { ResetChB }
{$I ideinfo.inc}   { IdeInfo }

begin
  Kes_Init;
  Kes_Buffer_Clear;
  { Festplatten erkennen, und Parameter aus dem MBR lesen }
  Kes_Load_Exec_Const(Detect);
  Kes_Buffer_Transfer(true, sizeof(Detect));
  Kes_Exec;
  Kes_Buffer_Transfer(false, 512);
  mfmaktiv:=(Kes_Data[$1d0]=1);
  oldchb:=Kes_Data[$1e1] or (Kes_Data[$1e2] shl 8);
  pideaktiv:=(not mfmaktiv) and (Kes_Data[$1d1]=0);
  sideaktiv:=(not mfmaktiv) and (Kes_Data[$1d1]=$10);
  jpide:=Kes_Data[$1d2] or (Kes_Data[$1d3] shl 8);
  mfmonline:=(Kes_Data[$1dc]=1);
  mfmcylinder:=Kes_Data[$1de] or (Kes_Data[$1df] shl 8);
  mfmheads:=Kes_Data[$1e0];
  pideonline:=(Kes_Data[$1d4]=1);
  pidecylinder:=Kes_Data[$1d5] or (Kes_Data[$1d6] shl 8);
  pideheads:=Kes_Data[$1d7];
  sideonline:=(Kes_Data[$1d8]=1);
  sidecylinder:=Kes_Data[$1d9] or (Kes_Data[$1da] shl 8);
  sideheads:=Kes_Data[$1db];

  { Ergebnisse anzeigen }
  a:=0;
  write('(M) MFM-Festplatte: ');
  if mfmonline then
    begin
      write('online, ');
      if mfmaktiv then write('aktiv, ');
      write('Cylinder:'+inttostr(mfmcylinder));
      write(' Heads:'+inttostr(mfmheads));
      writeln(' Sect:17');
      inc(a);
    end
  else
    writeln('offline');

  write('(P) 1. IDE-Festplatte: ');
  if pideonline then
    begin
      write('online, ');
      if pideaktiv then write('aktiv, ');
      write('Cylinder:'+inttostr(pidecylinder));
      write(' Heads:'+inttostr(pideheads));
      writeln(' Sect:17');
      inc(a);
    end
  else
    writeln('offline');

  write('(S) 2. IDE-Festplatte: ');
  if sideonline then
    begin
      write('online, ');
      if sideaktiv then write('aktiv, ');
      write('Cylinder:'+inttostr(sidecylinder));
      write(' Heads:'+inttostr(sideheads));
      writeln(' Sect:17');
      inc(a);
    end
  else
    writeln('offline');

  if a<2 then
    begin
      if a=0 then writeln('Es wurde keine Festplatte erkannt, es kann also keine kopiert werden.');
      if a=1 then writeln('Es wurde nur eine Festplatte erkannt, zum kopieren werden mindestens zwei ben'+#148+'tigt.');
      exit;
    end;              

  { Festplatten-Auswahl }
  write('Welche Festplatte soll die Quelle sein? (M/P/S):');
  readln(s);
  src:=0;
  if ((s='m') or (s='M')) and mfmonline then
    begin
      cylinders:=mfmcylinder;
      heads:=mfmheads;
      src:=1;
    end;
  if ((s='p') or (s='P')) and pideonline then
    begin
      cylinders:=pidecylinder;
      heads:=pideheads;
      src:=2;
    end;
  if ((s='s') or (s='S')) and sideonline then
    begin
      cylinders:=sidecylinder;
      heads:=sideheads;
      src:=3;
    end;
  if src=0 then 
    begin
      writeln('Diese Eingabe war ung'+#129+'ltig.');
      exit;
    end;

  write('Welche Festplatte soll das Ziel sein? (M/P/S):');
  readln(s);
  dst:=0;
  if ((s='m') or (s='M')) and mfmonline then dst:=1;
  if ((s='p') or (s='P')) and pideonline then dst:=2;
  if ((s='s') or (s='S')) and sideonline then dst:=3;
  if (dst=0) or (src=dst) then 
    begin
      writeln('Diese Eingabe war ung'+#129+'ltig.');
      exit;
    end;

  if src=1 then
    begin
      write('Wie oft soll bei Lesefehlern das Lesen wiederholt werden? (zahl):');
      readln(s);
      val(s,retrys,b);
      if b<>0 then
        begin
          writeln('Diese Eingabe war ung'+#129+'ltig.');
          exit;
        end;
    end;

  { Letzte Warnung! }
  writeln('Die '+DiskNames[src]+'-Festplatte soll auf die '+DiskNames[dst]+'-Festplatte kopiert werden?');
  writeln('ACHTUNG! Die Daten auf der '+DiskNames[dst]+'-Festplatte werden '+#129+'berschrieben!');
  write('Tippe "start" zum Beginn des Kopiervorgangs:');
  readln(s);
  if s<>'start' then 
    begin
      writeln('Diese Eingabe war ung'+#129+'ltig.');
      exit;
    end;

  { Festplatten-Parameter IDE setzen }
  Kes_Buffer_Clear;
  Kes_Load_Exec_Const(InitHDD);
  Kes_Data[$1e0]:=jpide and 255;
  Kes_Data[$1e1]:=(jpide shr 8) and 255;
  Kes_Data[$1e2]:=cylinders and 255;
  Kes_Data[$1e3]:=(cylinders shr 8) and 255;
  Kes_Data[$1e4]:=heads;
  if (src=3) or (dst=3) then Kes_Data[$1e5]:=$10 else Kes_Data[$1e5]:=0;
  Kes_Buffer_Transfer(true, 512);
  Kes_Exec;

  { Festplatten-Parameter MFM setzen }
  Kes_Buffer_Clear;
  Kes_Load_Exec_Const(InitHDD);
  Kes_Data[$1e0]:=0;
  Kes_Data[$1e1]:=$10;
  Kes_Data[$1e2]:=cylinders and 255;
  Kes_Data[$1e3]:=(cylinders shr 8) and 255;
  Kes_Data[$1e4]:=heads;
  Kes_Buffer_Transfer(true, 512);
  Kes_Exec;

  { Festplatten-Info von IDE holen }
  Kes_Buffer_Clear;
  Kes_Load_Exec_Const(IdeInfo);
  Kes_Buffer_Transfer(true, 512);
  Kes_Exec;
  Kes_Buffer_Transfer(false, 1280);

  { testen, ob die aktuelle CHS-Translation den geforderten Platten-Parametern entspricht }
  if (((src=2) or (dst=2)) and ((heads<>Kes_Data[$100+55*2]+(Kes_Data[$101+55*2] shl 8))
       or (Kes_Data[$100+56*2]<>17) or (Kes_Data[$101+56*2]<>0))) or
       (((src=3) or (dst=3)) and ((heads<>Kes_Data[$300+55*2]+(Kes_Data[$301+55*2] shl 8))
       or (Kes_Data[$300+56*2]<>17) or (Kes_Data[$301+56*2]<>0))) then
    begin
      writeln('Warnung: CHS-Translations-Parameter der IDE-Festplatte sind nicht korrekt gesetzt.');
      writeln('F'+#129+'r LBA-Firmware ist das OK, bei CHS-Firmware wird die Kopie unbrauchbar sein.');
      write('Trotzdem starten? (j/n):');
      readln(s);
      if (s<>'j') and (s<>'J') then 
        begin
          writeln('Ja. Besser nicht.');
          exit;
        end;
      end;

  { Chan B-Handler installieren }
  Kes_Buffer_Clear;
  Kes_Load_Exec_Const(Sector);
  Kes_Buffer_Transfer(true, 100);
  Kes_Exec;

  { Kopieren }
  writeln('Kopiere:');
  cyl:=0;
  hd:=0;
  sect:=0;
  error:=0;
  retrycnt:=0;
  repeat
    if (cyl=cylinders-1) and (hd=heads-1) then error:=50; { letzte Sektoren einzeln kopieren }
  { Sektor in KES-Buffer lesen }
    b:=retrys;
    repeat
      Mem[$4A:6]:=5;  { Lese-Kommando }
      if src=1 then MemW[$4A:7]:=$1000 else MemW[$4A:7]:=jpide;  
                            { Sprungvektor zur Kontroller-Firmware, entweder MFM oder IDE }
      if src=2 then Mem[$4A:9]:=0 else Mem[$4A:9]:=$10; { bei IDE: Primary oder Secundary? }
      MemW[$4A:10]:=cyl;  { Cylinder }
      Mem[$4A:12]:=hd;  { Kopf }
      Mem[$4A:13]:=sect; { Sektor }
      if error=0 then Mem[$4A:14]:=16 else Mem[$4A:14]:=2; { Anzahl Bytes (high) }
      Port[$4B]:=1; { KES-Kanal B Wakeup }
      write(#13+'Cylinder:'+inttostr(cyl)+'Head:'+inttostr(hd)+'Sektor:'+inttostr(sect+1));
      repeat until Mem[$4A:6]<>5;  { Warten bis fertig-Meldung }
      if Mem[$4A:6]<>0 then
        begin
          error:=20;
          if b>1 then
            begin;
              dec(b);
              inc(retrycnt);
            end
          else
            begin;
              inc(readerr);  { Lesefehler zaehlen }
              b:=0;
            end;
        end
      else
        b:=0;
    until b=0;

  { Sektor aus KES-Buffer schreiben }
    Mem[$4A:6]:=7; { Schreib-Kommando }
    if dst=1 then MemW[$4A:7]:=$1000 else MemW[$4A:7]:=jpide;  { Sprungvektor zur Kontroller-Firmware, entweder MFM oder IDE }
    if dst=2 then Mem[$4A:9]:=0 else Mem[$4A:9]:=$10; { bei IDE: Primary oder Secundary? }
    if error=0 then Mem[$4A:14]:=16 else Mem[$4A:14]:=2; { Anzahl Bytes (high) }
    Port[$4B]:=1; { KES-Kanal B Wakeup }
    write('Lesef.:'+inttostr(readerr));
    if src=1 then write('Wdh:'+inttostr(retrycnt));
    repeat until Mem[$4A:6]<>7;  { Warten bis fertig-Meldung }
    if Mem[$4A:6]<>0 then
      begin
        inc(writeerr);  { Schreibfehler zaehlen }
        error:=20;
      end;
    write('Schreibf.:'+inttostr(writeerr));

  { CHS hochzaehlen }
    if error=0 then 
      inc(sect,8)  { Fehlerfreier Durchgang: 8 Sektoren }
    else
      begin
        inc(sect); { Fehlerhafter Durchgang: 1 Sektor }
        dec(error);
      end;
    if sect>16 then
      begin
        inc(hd);
        sect:=sect-17;
      end;
    if hd>=heads then
      begin
        inc(cyl);
        hd:=0;
      end;
  until cyl>=cylinders; { bis zum letzten Zylinder wiederholen }

  { vorherigen Chan B-Handler wiederherstellen }
  Kes_Buffer_Clear;
  Kes_Load_Exec_Const(ResetChB);
  Kes_Data[1]:=oldchb and 255;
  Kes_Data[2]:=(oldchb shr 8) and 255;
  Kes_Buffer_Transfer(true, 10);
  Kes_Exec;
  
  writeln(#13+#10+'Kopiervorgang beendet.');
end.
