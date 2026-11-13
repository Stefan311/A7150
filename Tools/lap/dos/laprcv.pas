program laprcv;

var
  a: byte;
  b: integer;
  c: array[0..127] of byte;
  d: integer;
  e: byte;
  ofile: file;
  fname: string[16];
  blocks: longint;
  rest: word;

const
  SymSSync1 = $23;
  SymSSync2 = $76;
  SymSSend = $42;
  SymSStop = $84;
  SymRSync1 = $83;
  SymRSync2 = $37;
  SymRReady = $47;
  SymRRepeat = $68;
  crctable: array[0..255] of word =
  ($0000,$1021,$2042,$3063,$4084,$50a5,$60c6,$70e7,
   $8108,$9129,$a14a,$b16b,$c18c,$d1ad,$e1ce,$f1ef,
   $1231,$0210,$3273,$2252,$52b5,$4294,$72f7,$62d6,
   $9339,$8318,$b37b,$a35a,$d3bd,$c39c,$f3ff,$e3de,
   $2462,$3443,$0420,$1401,$64e6,$74c7,$44a4,$5485,
   $a56a,$b54b,$8528,$9509,$e5ee,$f5cf,$c5ac,$d58d,
   $3653,$2672,$1611,$0630,$76d7,$66f6,$5695,$46b4,
   $b75b,$a77a,$9719,$8738,$f7df,$e7fe,$d79d,$c7bc,
   $48c4,$58e5,$6886,$78a7,$0840,$1861,$2802,$3823,
   $c9cc,$d9ed,$e98e,$f9af,$8948,$9969,$a90a,$b92b,
   $5af5,$4ad4,$7ab7,$6a96,$1a71,$0a50,$3a33,$2a12,
   $dbfd,$cbdc,$fbbf,$eb9e,$9b79,$8b58,$bb3b,$ab1a,
   $6ca6,$7c87,$4ce4,$5cc5,$2c22,$3c03,$0c60,$1c41,
   $edae,$fd8f,$cdec,$ddcd,$ad2a,$bd0b,$8d68,$9d49,
   $7e97,$6eb6,$5ed5,$4ef4,$3e13,$2e32,$1e51,$0e70,
   $ff9f,$efbe,$dfdd,$cffc,$bf1b,$af3a,$9f59,$8f78,
   $9188,$81a9,$b1ca,$a1eb,$d10c,$c12d,$f14e,$e16f,
   $1080,$00a1,$30c2,$20e3,$5004,$4025,$7046,$6067,
   $83b9,$9398,$a3fb,$b3da,$c33d,$d31c,$e37f,$f35e,
   $02b1,$1290,$22f3,$32d2,$4235,$5214,$6277,$7256,
   $b5ea,$a5cb,$95a8,$8589,$f56e,$e54f,$d52c,$c50d,
   $34e2,$24c3,$14a0,$0481,$7466,$6447,$5424,$4405,
   $a7db,$b7fa,$8799,$97b8,$e75f,$f77e,$c71d,$d73c,
   $26d3,$36f2,$0691,$16b0,$6657,$7676,$4615,$5634,
   $d94c,$c96d,$f90e,$e92f,$99c8,$89e9,$b98a,$a9ab,
   $5844,$4865,$7806,$6827,$18c0,$08e1,$3882,$28a3,
   $cb7d,$db5c,$eb3f,$fb1e,$8bf9,$9bd8,$abbb,$bb9a,
   $4a75,$5a54,$6a37,$7a16,$0af1,$1ad0,$2ab3,$3a92,
   $fd2e,$ed0f,$dd6c,$cd4d,$bdaa,$ad8b,$9de8,$8dc9,
   $7c26,$6c07,$5c64,$4c45,$3ca2,$2c83,$1ce0,$0cc1,
   $ef1f,$ff3e,$cf5d,$df7c,$af9b,$bfba,$8fd9,$9ff8,
   $6e17,$7e36,$4e55,$5e74,$2e93,$3eb2,$0ed1,$1ef0);

{ Byte empfangen }
function recive: byte;
var x:byte;
begin
  repeat until (port[$cc] and $04)=$04;
  x:=port[$c8] and $0f;
  port[$ca]:=$10 or port[$ca];
  repeat until (port[$cc] and $04)=$00;
  recive:=(port[$c8] shl 4) or x;
  port[$ca]:=$ef and port[$ca];
end;

{ gleichzeitig Byte senden und empfangen }
function xchange(o: byte):byte;
var x: byte;
begin
  repeat until (port[$cc] and $04)=$04;
  x:=port[$c8] and $0f;
  port[$ca]:=(o and $0f) or $10;
  repeat until (port[$cc] and $04)=$00;
  x:=(port[$c8] shl 4) or x;
  port[$ca]:=(o shr 4) and $0f;
  xchange:=x;
end;

begin

  { init port }
  port[$ce]:=$91;
  port[$ca]:=$00;

  { verhindert Sync-Probleme, wenn die CP/M-Seite zuerst gestartet wird }
  e:=xchange(0);

  { Synchronisieren }
  repeat until xchange(SymRSync1)=SymSSync1;
  write('#');
  repeat until xchange(SymRSync2)=SymSSync2;

  { Dateiname empfangen }
  fname[0]:=chr(recive and 15);
  for a:=1 to ord(fname[0]) do fname[a]:=chr(recive);
  if ParamCount=1 then fname:=ParamStr(1);
  writeln(fname);

  { Dateigroesse empfangen }
  rest:=recive;
  blocks:=recive;
  blocks:=blocks or (recive shl 8);
  blocks:=blocks or (recive shl 16);
  blocks:=(blocks shl 1) or ((rest shr 7) and 1);
  rest:=rest and $7f;

  { Datei oeffnen }
  assign(ofile,fname);
  rewrite(ofile,1);
  write('[');

  e:=SymRReady;

  while (xchange(e)=SymSSend) or (e=SymRRepeat) do
    begin
	  d:=0;
      b:=0;
      { Datenblock empfangen }
      for a:=0 to 127 do
	    begin
	      b:=recive;
          c[a]:=b;
          d:=(d shl 8) xor crctable[((d shr 8) xor b) and $ff];
        end;
      { CRC empfangen }
	  b:=recive;
	  b:=b or (recive shl 8);

      { wenn CRC korrekt, dann speichern und naechsten Block bestellen }
      if b=d then
	    begin
          if blocks=0 then
            begin
              blockwrite(ofile, c, rest);
            end
          else
            begin
              blockwrite(ofile, c, 128);
              dec(blocks);
            end;
          write('.');
	      e:=SymRReady;
        end
      else { CRC inkorrekt, Transfer wiederholen }
        begin
          write('!');
	      e:=SymRRepeat;
        end;
    end;

  { Druckerport in den default-Zustand setzen }
  port[$ca]:=$10;

  { Programm beenden }
  writeln(']');
  close(ofile);  
end.

