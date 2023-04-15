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

program afitest;

uses kes,crt;

var
  ok: boolean;
  s,t: string;
  cmd: byte;
  ad: word;
  va: byte;
  endless: boolean;
  a,b,c: word;
  f:file;

function upper(ip:string):string;
var a: byte;
    i: string;
begin
  i:=ip;
  for a:=1 to length(i) do
    begin
      if (i[a]>='a') and (i[a]<='z') then
        i[a]:=chr(ord(i[a])-$20);
    end;
  upper:=i;
end;


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

function WTH(data:word):string;
var h:string;
begin
  H:=BTH(Byte(data shr 8))+BTH(Byte(data and $FF));
  WTH:=h;
end;

function ATH(data:longint):string;
var h:string;
begin
  H:=BTH(Byte(data shr 16))+WTH(Word(data and $FFFF));
  ATH:=h;
end;

function HTN(data:char):byte;
var c : byte;
begin
  if (data<'0') or (data>'F') or ((data>'9') and (data<'A')) then
    begin
      writeln('Ung'#129'ltiges Hex-Zeichen: ',data);
      ok:=false;
    end;
  if ok then 
    begin
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

procedure KOut(p,v:byte);
begin
  Mem[$4A:6]:=4;
  Mem[$4A:7]:=p;
  Mem[$4A:9]:=v;
  Port[$4B]:=1;
  repeat until Mem[$4A:6]=0;
end;

function KIn(p:byte):byte;
begin;
  Mem[$4A:6]:=3;
  Mem[$4A:7]:=p;
  Port[$4B]:=1;
  repeat until Mem[$4A:6]=0;
  KIn:=Mem[$4A:9];
end;

procedure IDEOut(p:byte;v:word);
var a: byte;
begin
  KOut($B1,(p and $F) or $30);
  a:=KIn($B2);
  KOut($B0, v and $ff);
  KOut($B0, v shr 8);
end;

function IDEIn(p:byte):word;
var x,y:byte;
begin
  KOut($B1,(p and $F) or $30);
  x:=KIn($B2);
  x:=KIn($B0);
  y:=KIn($B0);
  IDEIn:=x+(y shl 8);
end;

procedure USBCmd(c:byte);
begin
  KOut($B1,$31);
  KOut($B3,c);
end;

procedure USBOut(c:byte);
begin
  KOut($B1,$30);
  KOut($B3,c);
end;

function USBIn: byte;
begin
  KOut($B1,$30);
  USBIn:=KIn($B3);
end;

{$I kesprog.inc}

procedure basefunct;
begin
  writeln('**** Basistest-Modus ****');
  writeln('E         - Endlos (prefix)');
  writeln('R xxxx    - Speicher lesen');
  writeln('W xxxx xx - Speicher schreiben');
  writeln('I xx      - Port lesen');
  writeln('O xx xx   - Port schreiben');
  writeln('D         - IDEtest Modus');
  writeln('U         - USBtest Modus');
  writeln('Q         - Beenden');
  cmd:=0;

  repeat;
    write('>');
    readln(s);
    s:=upper(s);
    if s<>'' then
      begin
        cmd:=0;
        endless:=false;
        ad:=0;
        va:=0;
        ok:=true;
        if s[1]='E' then
          begin
            endless:=true;
            s:=copy(s,2,length(s)-1);
          end;
        case s[1] of
          'R': begin
                 if length(s)<6 then
                   begin
                     writeln('Parameter zu kurz');
                     continue;
                   end;
                 cmd:=1;
                 ad:=HTW(copy(s,3,4));
                 va:=0;
               end;
          'W': begin
                 if length(s)<9 then
                   begin
                     writeln('Parameter zu kurz');
                     continue;
                   end;
                 cmd:=2;
                 ad:=HTW(copy(s,3,4));
                 va:=HTB(copy(s,8,2));
               end;
          'I': begin
                 if length(s)<4 then
                   begin
                     writeln('Parameter zu kurz');
                     continue;
                   end;
                 cmd:=3;
                 ad:=HTB(copy(s,3,2));
                 va:=0;
               end;
          'O': begin
                 if length(s)<7 then
                   begin
                     writeln('Parameter zu kurz');
                     continue;
                   end;
                 cmd:=4;
                 ad:=HTB(copy(s,3,2));
                 va:=HTB(copy(s,6,2));
               end;
          'D': begin
                 cmd:=102;
                 ok:=false;
               end;
          'U': begin
                 cmd:=103;
                 ok:=false;
               end;
          'Q': begin
                 cmd:=100;
                 ok:=false;
               end;
          '?': begin
                 cmd:=101;
                 ok:=false;
               end;
        end;
      end;
    if ok and (cmd>0) then
      begin
        repeat
          Mem[$4A:6]:=cmd;
          Memw[$4A:7]:=ad;
          Mem[$4A:9]:=va;
          Port[$4B]:=1;
          repeat until Mem[$4A:6]=0;
          if keypressed then endless:=false;
          if (cmd = 1) or (cmd = 3) then
            begin
              write(BTH(Mem[$4A:9]));
              if endless then 
                write(#13)
              else
                write(#13+#10);
            end;
        until not endless;
        writeln('ok');
      end;
  until cmd>50;
end;

procedure diskfunct;
begin
  writeln('**** IDEtest-Modus ****');
  writeln('S         - Statusregister lesen');
  writeln('A         - Alle Registers lesen');
  writeln('W XX XX   - Ein IDE Register schreiben');
  writeln('DP        - Primary (Master) Auswahl');
  writeln('DS        - Secundary (Slave) Auswahl');
  writeln('BRC datei - Block lesen mit CPU');
  writeln('BWC datei - Block schreiben mit CPU');
  writeln('BRD datei - Block lesen mit Z80-DMA');
  writeln('BWD datei - Block schreiben mit Z80-DMA');
  writeln('(Block lesen ohne Dateiname gibt auf Bildschirm aus.)');
  writeln('(Block schreiben ohne Dateiname benutzt ein Testpattern.)');
  writeln('X         - Basistest-Modus');
  writeln('Q         - Beenden');
  cmd:=0;
  repeat
    write('>');
    readln(s);
    s:=upper(s);
    ok:=true;
    case s[1] of
      'S': writeln('Status:'+BTH(IDEIn(7)));
      'A': begin
             write('Err='+BTH(IDEIn(1))+' Count='+BTH(IDEIn(2)));
             write(' SecNr='+BTH(IDEIn(3))+' CylLow='+BTH(IDEIn(4)));
             write(' CylHi='+BTH(IDEIn(5))+' Dev='+BTH(IDEIn(6)));
             writeln(' Status='+BTH(IDEIn(7)));
           end;
      'W': begin
             if length(s)<7 then
               begin
                 writeln('Parameter zu kurz');
                 continue;
               end;
             ad:=HTB(copy(s,3,2));
             va:=HTB(copy(s,6,2));
             IDEOut(ad,va);
             writeln('OK');
           end;
      'D': begin
             case s[2] of
               'P': begin
                      IDEOut(6,IDEIn(6) and $EF);
                      writeln('OK');
                    end;
               'S': begin
                      IDEOut(6,IDEIn(6) or $10);
                      writeln('OK');
                    end;
               else writeln('Fehler!');
             end;  
           end;
      'B': begin
             Kes_Buffer_Clear;
             if s[2]='W' then
               begin
                 if length(s)>3 then
                   begin
                     assign(f,copy(s,5,length(s)-4));
                     reset(f,1);
                     blockread(f,kes_data[0],512);
                     close(f);
                   end
                 else
                   begin
                     for a:=0 to 255 do
                       begin
                         kes_data[a*2]:=a;
                         if (a and 1) = 0 then
                           kes_data[a*2+1]:=$55
                         else
                           kes_data[a*2+1]:=$AA;
                       end;
                   end;
               end;
             Kes_Buffer_Transfer(true, 512);
             KOut($B1,$30);
             a:=KIn($B2);
             Mem[$4A:6]:=0;
             t:=copy(s,1,3);
             if t='BRC' then Mem[$4A:6]:=5;
             if t='BWC' then Mem[$4A:6]:=6;
             if t='BRD' then Mem[$4A:6]:=7;
             if t='BWD' then Mem[$4A:6]:=8;
             if Mem[$4A:6]=0 then continue;
             Memw[$4A:7]:=$2100;
             Mem[$4A:9]:=$B0;
             Port[$4B]:=1;
             repeat until Mem[$4A:6]=0;
             Kes_Buffer_Transfer(false, 512);
             if s[2]='R' then
               begin
                 if length(s)>3 then
                   begin
                     assign(f,copy(s,5,length(s)-4));
                     rewrite(f,1);
                     blockwrite(f,kes_data[0],512);
                     close(f);
                   end
                 else
                   begin
                     for a:=0 to 511 do 
                       begin
                         if (a and 15=0) then 
                           begin
                             write(wth(a));
                             write(': ');
                           end;
                         write(bth(kes_data[a]));
                         if (a and 15=15) then 
                           begin
                             write(' ');
                             for b:=0 to 15 do
                               begin
                                 va:=kes_data[(a and $fff0) + b];
                                 if va<32 then write('.') else write(chr(va));
                               end;
                             writeln;
                           end
                         else
                           write(' ');
                       end;
                     writeln;
                   end;
               end;
             writeln('OK');
           end; 
      'X': cmd:=101;
      '?': cmd:=102;
      'Q': cmd:=100;   
    end;
  until cmd>50;
end;

procedure usbfunct;
begin
  writeln('**** USBtest-Modus ****');
  writeln('C XX      - Kommando senden');
  writeln('O XX (XX) - Daten senden (mehrere Bytes moeglich)');
  writeln('I (XX)    - Daten empfangen (XX ist Anzahl Bytes wenn angegeben)');
  writeln('S         - Status lesen');
  writeln('BR datei  - Block lesen');
  writeln('BW datei  - Block schreiben (64 Bytes)');
  writeln('(Block lesen ohne Dateiname gibt auf Bildschirm aus.)');
  writeln('(Block schreiben ohne Dateiname benutzt ein Testpattern.)');
  writeln('X         - Basistest-Modus');
  writeln('Q         - Beenden');
  cmd:=0;
  repeat
    if (KIn($B1) and $08) = 0 then 
	  write('Int');
    write('>');
    readln(s);
    s:=upper(s);
    ok:=true;
    case s[1] of
      'C': begin
             if length(s)<4 then
               begin
                 writeln('Parameter zu kurz');
                 continue;
               end;
             a:=HTB(copy(s,3,2));
             USBCmd(a);
             writeln('OK');
           end;
      'O': begin
             if length(s)<4 then
               begin
                 writeln('Parameter zu kurz');
                 continue;
               end;
			 a:=3;
             repeat
               b:=HTB(copy(s,a,2));
			   USBOut(b);
               a:=a+3;
			 until a>=(length(s)+1);
             writeln('OK');
           end;
      'I': begin
	         a:=1;
             if length(s)>=4 then
               a:=HTB(copy(s,3,2));
			 for b:=1 to a do
               write(bth(USBIn)+' ');
             writeln(#13#10'OK');
           end; 
	  'S': begin
	         USBCmd($22);
			 a:=USBIn;
			 t:='Unbekannt';
			 case a of
			   $14: t:='USB_INT_SUCCESS';
			   $15: t:='USB_INT_CONNECT';
			   $16: t:='USB_INT_DISCONNECT';
			   $17: t:='USB_INT_BUF_OVER';
			   $1D: t:='USB_DISK_READ';
			   $1E: t:='USB_DISK_WRITE';
			   $1F: t:='USB_DISK_ERROR';
			 end;
             writeln('Status: '+bth(a)+'('+t+')');
             writeln('OK');
		   end;
	   'B': begin
             Kes_Buffer_Clear;
             if s[2]='W' then
               begin
                 if length(s)>3 then
                   begin
                     assign(f,copy(s,5,length(s)-4));
                     reset(f,1);
                     blockread(f,kes_data[0],64);
                     close(f);
                   end
                 else
                   begin
                     for a:=0 to 31 do
                       begin
                         kes_data[a*2]:=a;
                         if (a and 1) = 0 then
                           kes_data[a*2+1]:=$55
                         else
                           kes_data[a*2+1]:=$AA;
                       end;
                   end;
				 USBCmd($2B);
				 USBOut(64);
                 for a:=0 to 63 do
                   USBOut(kes_data[a]);
               end
			 else
			   begin
				 USBCmd($28);
				 b:=USBIn;
				 if b=0 then
				   begin
				     writeln('Keine Daten!');
				     continue;
				   end;
                 for a:=0 to b do
                   kes_data[a]:=USBIn;
                 if length(s)>3 then
                   begin
                     assign(f,copy(s,5,length(s)-4));
                     rewrite(f,1);
                     blockwrite(f,kes_data[0],b);
                     close(f);
                   end
                 else
                   begin
                     for a:=0 to b-1 do 
                       begin
                         if (a and 15=0) then 
                           begin
                             write(wth(a));
                             write(': ');
                           end;
                         write(bth(kes_data[a]));
                         if (a and 15=15) then 
                           begin
                             write(' ');
                             for c:=0 to 15 do
                               begin
                                 va:=kes_data[(a and $fff0) + c];
                                 if va<32 then write('.') else write(chr(va));
                               end;
                             writeln;
                           end
                         else
                           write(' ');
                       end;
                     writeln;
                   end;
			   end;
             writeln('OK');
           end; 
      'X': cmd:=101;
      '?': cmd:=103;
      'Q': cmd:=100;   
    end;
  until cmd>50;
end;

begin
  Kes_Init;
  Kes_Buffer_Clear;
  Kes_Load_Exec_Const(Kesprog);
  Kes_Buffer_Transfer(true, sizeof(Kesprog));
  Kes_Exec;

  cmd:=101;
  ok:=false;

  repeat
    if cmd=101 then basefunct;
    if cmd=102 then diskfunct;
    if cmd=103 then usbfunct;
  until cmd=100;
end.


