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

program fixwinch;

const DISKTABLES: array[0..9] of byte = ($44,$49,$53,$4b,$20,$54,$41,$42,$4c,$45);

var 
  a: string;
  i: byte;
  mbr: array[0..511] of byte;
  dts: array[0..511] of byte;
  x: word;
  f: file;
  mbrset: boolean;
  mbrcyl: word;
  mbrhd: byte;
  dtsset: boolean;
  dtscyl1: word;
  dtscyl2: word;
  dtshd1: byte;
  dtshd2: byte;

function readHDD(s: word; d: array of byte): boolean; assembler;
asm
    mov         ax,$0201
    mov         cx,s
    les         bx,d
    mov         dx,$0080
    int         $13
end;

function writeHDD(s: word; d: array of byte): boolean; assembler;
asm
    mov         ax,$0301
    mov         cx,s
    les         bx,d
    mov         dx,$0080
    int         $13
end;

function inttostr(a: word):string;
var b:string;
begin
  str(a,b);
  inttostr:=b;
end;

begin
  writeln('Mit diesem Programm ist es m'#148'glich auf dem Robotron A7150 (CM1910)');
  writeln('die Festplattenparameter zu '#132'ndern. F'#129'r IDE-Festplatten am ');
  writeln('Kontroller AFI-K5173 k'#148'nnen auch Parameter oberhalb der Grenzen von');
  writeln('MWINCH gesetzt werden.');
  writeln;
  writeln('Soll vorher eine Sicherheitskopie vom MBR und DISKTABLE-Sektor (DTS)');
  write('erstellt werden? (J/N) (Default Abbruch):');
  readln(a);
  i:=0;
  if (a='j') or (a='J') then i:=1;
  if (a='n') or (a='N') then i:=2;
  if i=0 then exit;
  if readHDD(1,mbr) then
    begin
      writeln('Fehler beim lesen des MBR');
      exit;
    end;
  if readHDD(8,dts) then
    begin
      writeln('Fehler beim lesen des DTS');
      exit;
    end;
  if i=1 then
    begin
      write('Dateiname f'#129'r Kopie des MBR (Default A:\MBR.BIN):');
      readln(a);
      if a='' then a:='A:\MBR.BIN';
      assign(f,a);
      rewrite(f,1);
      blockwrite(f,mbr,512);
      close(f);
      a:='';
      write('Dateiname f'#129'r Kopie des DTS (Default A:\DTS.BIN):');
      readln(a);
      if a='' then a:='A:\DTS.BIN';
      assign(f,a);
      rewrite(f,1);
      blockwrite(f,dts,512);
      close(f);
    end;
  if (mbr[$1fe]=$55) and (mbr[$1ff]=$aa) then
    writeln('OK: Der MBR einth'#132'lt eine Bootsektor-Signatur (55AA).');
    
  if (mbr[$1c2]<>0) or (mbr[$1d2]<>0) or (mbr[$1e2]<>0) or (mbr[$1f2]<>0) then
    writeln('OK: Der MBR einth'#132'lt mindestens eine Partition.');
   
  mbrset:=false;
  mbrcyl:=mbr[$1b0]+(mbr[$1b1]*256);
  mbrhd:=mbr[$1b2];
  for x:=$19c to $1b7 do if mbr[x]<>0 then mbrset:=true;
  if mbrset then
    begin 
      writeln('Der MBR enth'#132'lt Festplatten-Parameter.');
      writeln('Sektorl'#132'nge1:'+inttostr(mbr[$19c]+(mbr[$19d] shl 8)));
      writeln('Anzahl der Zylinder:'+inttostr(mbrcyl));
      writeln('Anzahl der K'#148'pfe:'+inttostr(mbrhd));
      writeln('Anzahl der Sektoren pro Spur:'+inttostr(mbr[$1b4]));
      writeln('Sektorl'#132'nge2:'+inttostr(mbr[$1b5]+(mbr[$1b6] shl 8)));
      if (mbr[$19c]<>0) or (mbr[$19d]<>2) then 
        begin
          mbrset:=false;
          writeln('Nicht OK: Sektorl'#132'nge1 ist nicht 512');
        end;
      if (mbr[$1b5]<>0) or (mbr[$1b6]<>2) then 
        begin
          mbrset:=false;
          writeln('Nicht OK: Sektorl'#132'nge2 ist nicht 512');
        end;
      if mbr[$1b4]<>17 then 
        begin
          mbrset:=false;
          writeln('Nicht OK: Anzahl der Sektoren pro Spur ist nicht 17');
        end;
      if not mbrset then
        begin
          writeln('Das ist ein bekannter Fehler vom MWINCH f'#129'r einige Festplattentypen.');
          writeln('Die k'#148'nnen zum NOGO beim ACT f'#129'hren.');
          write('Sollen diese Parameter korregiert werden? (J/N):');
          readln(a);
          if (a='j') or (a='J') then 
            begin
              mbr[$19c]:=0;
              mbr[$19d]:=2;
              mbr[$1b5]:=0;
              mbr[$1b6]:=2;
              mbr[$1b4]:=17;
              if writeHDD(1,mbr) then
                begin;
                  writeln('Fehler beim Schreiben des MBR!');
                  exit;
                end;
              writeln('Der MBR wurde geschrieben.');
              mbrset:=true;
            end;
        end;
       
    end
  else
    begin
      writeln('Der MBR enth'#132'lt KEINE Festplatten-Parameter. Das ist OK, solange der DTS korrekt ist.');
    end;
  
  dtsset:=true;
  for x:=0 to 9 do if DISKTABLES[x]<>dts[x] then dtsset:=false;
  if dtsset then
    begin
      writeln('Der DTS enth'#132'lt Festplatten-Parameter.');
      dtscyl1:=dts[$18]+(dts[$19] shl 8);
      dtscyl2:=dts[$38]+(dts[$39] shl 8);
      dtshd1:=dts[$1a];
      dtshd2:=dts[$3a];
      writeln('Anzahl der Zylinder:'+inttostr(dtscyl1)+' und '+inttostr(dtscyl2));
      writeln('Anzahl der K'#148'pfe:'+inttostr(dtshd1)+' und '+inttostr(dtshd2));
      writeln('Anzahl der Sektoren pro Spur:'+inttostr(dts[$26])+' und '+inttostr(dts[$46]));
      if (dtscyl1=dtscyl2) and (dtscyl1=mbrcyl) then 
        writeln('OK: die Anzahl der Zylinder ist in MBR und DTS gleich.')
      else
        writeln('Nicht OK: die Anzahl der Zylinder ist in MBR und DTS unterschiedlich.');
      if (dtshd1=dtshd2) and (dtshd1=mbrhd) then
        writeln('OK: die Anzahl der K'#148'pfe ist in MBR und DTS gleich.')
      else
        writeln('Nicht OK: die Anzahl der K'#148'pfe ist in MBR und DTS unterschiedlich.');
    end
  else
    begin
      writeln('Der DTS enth'#132'lt KEINE "DISK TABLES" Signatur. Das ist OK, solange der MBR korrekt ist.');
    end;
    
  if (not mbrset) and (not dtsset) then
    begin
      writeln('Weder MBR noch DTS enthalten Festplatten-Parameter. Bitte konfigurieren die die Festplatte zuerst mit MWINCH.');
      exit;
    end;
  
  write('Sollen die Festplatten-Parameter ge'#132'ndert werden? (J/N):');
  readln(a);
  if (a='j') or (a='J') then
    begin
      if dtsset then 
        begin
          mbrcyl:=dtscyl1;
          mbrhd:=dtshd1;
        end;
      repeat
        write('Neue Anzahl der Zylinder? (Default '+inttostr(mbrcyl)+'):');
        readln(a);
        x:=0;
        if a<>'' then
          begin
            val(a,dtscyl1,x);
            if x<>0 then 
              writeln('Das verstehe ich nicht!')
            else
              mbrcyl:=dtscyl1;
          end;
      until x=0;    
      repeat
        write('Neue Anhahl der K'#148'pfe? (Default '+inttostr(mbrhd)+'):');
        readln(a);
        x:=0;
        if a<>'' then
          begin
            val(a,dtshd1,x);
            if x<>0 then 
              writeln('Das verstehe ich nicht!')
            else
              mbrhd:=dtshd1;
          end;
      until x=0;    
      writeln('Neue Zylinder:'+inttostr(mbrcyl)+' Neue Anzahl K'#148'pfe:'+inttostr(mbrhd));
      if mbrset and dtsset then write('Soll das in MBR und DTS geschrieben werden? (J/N):')
      else if mbrset then write('Soll das in den MBR geschrieben werden? (J/N):')
      else write('Soll das in den DTS geschrieben werden? (J/N):');
      readln(a);
      if (a='j') or (a='J') then
        begin
          dts[$18]:=mbrcyl and 255;
          dts[$19]:=mbrcyl shr 8;
          dts[$38]:=mbrcyl and 255;
          dts[$39]:=mbrcyl shr 8;
          dts[$1a]:=mbrhd;
          dts[$3a]:=mbrhd;
          mbr[$1b0]:=mbrcyl and 255;
          mbr[$1b1]:=mbrcyl shr 8;
          mbr[$1b2]:=mbrhd;
          if mbrset then 
            if writeHDD(1,mbr) then
              begin;
                writeln('Fehler beim Schreiben des MBR!');
                exit;
              end
            else
              writeln('Der MBR wurde geschrieben.');
          if dtsset then  
            if writeHDD(8,dts) then
              begin;
                writeln('Fehler beim Schreiben des DTS!');
                exit;
              end
            else
              writeln('Der DTS wurde geschrieben.');
        end;
    end;
  writeln('Das wars. Einen sch'#148'nen Tag noch.');
end.
