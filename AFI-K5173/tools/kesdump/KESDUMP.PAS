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

program kesdump;

uses kes;

const
  kes_copy_memory: array[0..11] of byte = ($21,$00,$00,$11,$00,$22,$01,$00,$10,$ED,$B0,$C9);
{
        ld      hl,0
        ld      de,2200h
        ld      bc,1000h
        ldir
        ret
}

var
  a,b: word;
  f: file;
  s: string;
  i: boolean;

begin
  writeln('KESDUMP A7150 (CM1910) -- Liest den Speicher des KES-Moduls aus.'#13#10'2022 von Stefan Berndt');
  write('Init...'#13);
  kes_init;
  for a:=0 to 11 do
    begin
      writeln('Memory Segment '+copy('0123456789ABCFDEF',a+1,1)+'000-'+copy('0123456789ABCFDEF',a+1,1)+'FFF:');
      write('Buffer clear...'#13);
      kes_buffer_clear;
      kes_load_exec_const(kes_copy_memory);
      kes_data[2]:=a shl 4;  { Segment-Nummer in das 'ld hl,0' schreiben }
      write('Lade Z80 Program in KES...'#13);
      kes_buffer_transfer(true, $12);
      write('Starte Z80 Program...       '#13);
      kes_exec;
      write('Transfer Data aus dem KES...'#13);
      kes_buffer_transfer(false, $1100);
      write('Transfer beendet.           '#13);
      if a=2 then
        begin
          for b:=$200 to $1100 do kes_data[b]:=0;  { Transfer-Buffer im KES loeschen }
          assign(f,'RAM_20.BIN');
          rewrite(f,1);
          blockwrite(f,kes_data[$100],$1000);
          writeln('Segment (sollte RAM sein) in RAM_20.BIN gespeichert.');
        end
      else if a=3 then
        begin
          for b:=$100 to $200 do kes_data[b]:=0;  { Transfer-Buffer im KES loeschen }
          blockwrite(f,kes_data[$100],$1000);
          close(f);
          writeln('Segment (sollte RAM sein) auch in RAM_20.BIN gespeichert.');
        end
      else
        begin
          { Erkennung von leeren Segmenten }
          i:=true;
          for b:=0 to $1000 do if kes_data[$100+b]<>kes_data[$100] then i:=false;
          if i then
            begin
              writeln('Segment ist leer, nichts zu speichern.');
              continue;
            end;

          { 1. ROM-Chip identifizieren }
          s:='UNK';
          case kes_data[$8FD] of
            $21: s:='KES1';
            $23: s:='KES3';
            $31: s:='AFS1';
            $41: s:='AFP1';
            $51: s:='AFI1';
          end;
          assign(f,s+'_'+copy('0123456789ABCFDEF',a+1,1)+'0.BIN');
          rewrite(f,1);
          blockwrite(f,kes_data[$100],$800);
          close(f);
          if s='UNK' then
            write('Segment enth'#$84'lt unbekannte Daten')
          else
            write('Segment enth'#$84'lt '+s+' ROM');
          writeln(', gespeichert in '+s+'_'+copy('0123456789ABCFDEF',a+1,1)+'0.BIN');

          { 2. ROM-Chip identifizieren }
          s:='UNK';
          case kes_data[$10FD] of
            $22: s:='KES2';
            $24: s:='KES4';
            $32: s:='AFS2';
            $42: s:='AFP2';
            $52: s:='AFI2';
          end;
          assign(f,s+'_'+copy('0123456789ABCFDEF',a+1,1)+'8.BIN');
          rewrite(f,1);
          blockwrite(f,kes_data[$900],$800);
          close(f);
          if s='UNK' then
            write('Segment enth'#$84'lt unbekannte Daten')
          else
            write('Segment enth'#$84'lt '+s+' ROM');
          writeln(', gespeichert in '+s+'_'+copy('0123456789ABCFDEF',a+1,1)+'8.BIN');
        end;
    end;
  writeln('Habe fertig.');
end.
