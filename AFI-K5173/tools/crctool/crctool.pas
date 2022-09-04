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

program crctool;

function NTH(data:Byte):char;
var h:char;
begin
  case data of
    0..9:h:=chr(48+data);
    else h:=chr(55+data);
  end;
  NTH:=h;
end;

function BTH(data:Byte):string;
var h:string;
begin
  h:=NTH(data shr 4)+NTH(data and $F);
  BTH:=h;
end;

var
  f: file;
  crc, i, b: word;
  buf: array[0..1023] of byte;
  l, start, ende, posit, writepos: longint;
  

begin
  crc:=0;
  start:=0;
  ende:=-1;
  writepos:=-1;

  if paramcount=0 then
    begin
      writeln('Hilfsprogramm f'+#129+'r Pr'+#129+'fsummen (CRC-CCITT, CRC-16)');
      writeln('Aufruf:'+#13+#10+'CRCTOOL dateiname {parameter}');
      writeln('Parameter:'+#13+#10+'-s Nummer : Startposition (Default=0)');
      writeln('-e Nummer : Endposition, erstes nicht zu pr'+#129+'fendes Byte (Default=ganze Datei)');
      writeln('-p Nummer : Startpolynom (Default=0)');
      writeln('-w Nummer : CRC in diese Dateiposition schreiben (Default=Bildschirm)');
      writeln('Als Nummer sind Dezimalzahlen und Hex-Zahlen mit $ davor erlaubt.');
      writeln('Beispiele:'+#13+#10+'CRCTOOL bios.bin');
      writeln('CRCTOOL firmware.bin -p -1 -s 2048 -e 4095 -w 4095');
      writeln('CRCTOOL firmware.bin -p $ffff -s 0 -e $07fe -w $07fe');
      halt;
    end;

  if paramcount>=3 then
    for i:=1 to (paramcount div 2) do
      begin
        if paramstr(i*2)='-s' then val(paramstr(i*2+1),start,b);
        if paramstr(i*2)='-e' then val(paramstr(i*2+1),ende,b);
        if paramstr(i*2)='-p' then val(paramstr(i*2+1),crc,b);
        if paramstr(i*2)='-w' then val(paramstr(i*2+1),writepos,b);
      end;

  assign(f,paramstr(1));
  filemode:=0;
  reset(f,1);

  if ende=-1 then
    ende:=filesize(f)
  else
    if ende>filesize(f) then
      begin
        writeln('Endposition ist gr'+#148+#225+'er als Dateil'+#132+'nge!');
        halt;
      end;

  if start>=ende then 
    begin
      writeln('Startposition liegt hinter Endposition!');
      halt;
    end;

  if (writepos>0) and (writepos>filesize(f)) then
    begin
      writeln('Schreibposition ist gr'+#148+#225+'er als Dateil'+#132+'nge!');
      halt;
    end;

  posit:=start;
  seek(f,posit);
  while posit<ende do
    begin
      l:=ende-posit;
      if l>1024 then l:=1024;
      blockread(f,buf,l);
      for i:=0 to l-1 do
        begin
          crc:=crc xor (buf[i] shl 8);
         for n:=1 to 8 do
           if (crc and $8000) <> 0 then
             crc:=(crc shl 1) xor $1021
           else
             crc:=crc shl 1;
        end;
      posit:=posit+l;
    end;
  close(f);

  if writepos>0 then
    begin
      filemode:=2;
      reset(f,1);
      seek(f,writepos);
      b:=crc shr 8;
      blockwrite(f,b,1);
      b:=crc and $ff;
      blockwrite(f,b,1);
      close(f);
    end
  else
    writeln('CRC:'+BTH(crc shr 8)+BTH(crc and $ff));
end.
