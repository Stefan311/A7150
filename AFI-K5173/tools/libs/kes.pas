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

unit kes;

interface

type
  TKes_IOPB = record
    reserved1: word;
    reserved2: word;
    done_bytes_l: word;
    done_bytes_h: word;
    devicecode: word;
        { 0=HDD, 1=8*Floppy, 2=HDD, 3=5.1/4*Floppy }
    device_nr: byte;
        { Bit 0-2:Laufwerksnummer
          Bit 4=1:wechseldatentraeger }
    functioncode: byte;
        { 0=Initialize
          1=Request Status
          2=Format
          3=Read Sector Id
          4=Read Sector to Host
          5=Read Sector to KES Buffer
          6=Write Sector from Host
          7=Write Sector from KES Buffer
          8=Seek Cylinder
          9-11=Reserved
          12=Start User-Program in KES
          13=DMA between Host and KES-IO
          14=KES Buffer IO
          15=Diagrosis}
    modification: word;
    cylinder: word;
    head: byte;
    sector: byte;
    buffer_ptr: ^byte;
    requested_bytes_l: word;
    requested_bytes_h: word;
    general_ptr: pointer;
  end;
  PKes_IOPB = ^TKes_IOPB;

  TKes_CIB = record
    reserved1: byte;
    op_status: byte;
    reserved2: byte;
    status_semaphore: byte;
    csa_ptr: ^word;
    io_ptr: PKes_IOPB;
  end;
  PKes_CIB = ^TKes_CIB;

  TKes_CCB = record
    dummy01: byte;
    busy: byte;
    cib_ptr: PKes_CIB;
    reserved1: word;
    cb_ptr: ^word;
    cb: word;
  end;
  PKes_CCB = ^TKes_CCB;

  TKes_WUB = record
    dummy01: byte;
    reserved: byte;
    ccb_ptr: PKes_CCB;
  end;
  PKes_WUB = ^TKes_WUB;

const
  Kes_Wakeup_port = $4a;
  Kes_Maxbuffer = 4672;

var
  Kes_WUB: PKes_WUB;
  Kes_CCB: PKes_CCB;
  Kes_CIB: PKes_CIB;
  Kes_IOPB: PKes_IOPB;
  Kes_Data: array[0..Kes_Maxbuffer] of byte;

procedure kes_init;
function kes_load_exec_file(filename: string):word;
procedure kes_load_exec_const(data: array of byte);
procedure kes_buffer_transfer(to_kes: boolean; size: word);
procedure kes_exec;
procedure kes_buffer_clear;

implementation

procedure kes_init;
begin
  kes_wub:=Ptr(Kes_Wakeup_port,$00);
  kes_ccb:=kes_wub^.ccb_ptr;
  kes_cib:=kes_ccb^.cib_ptr;
  {$ifdef fpc}
  dec(pointer(kes_cib),4);
  {$else}
  dec(integer(kes_cib),4);
  {$endif}
  kes_iopb:=kes_cib^.io_ptr;
end;

procedure kes_wakeup;
begin
  kes_cib^.status_semaphore:=0;
  port[Kes_Wakeup_Port]:=0;
  port[Kes_Wakeup_Port]:=1;
end;

procedure kes_load_exec_const(data: array of byte);
var a: word;
begin;
  for a:=0 to sizeof(data)-1 do
    Kes_Data[a]:=data[a];
end;

function kes_load_exec_file(filename: string):word;
var
  f: file;
  l: word;
begin
  if filename='' then
    begin
      kes_load_exec_file:=1001;
      exit;
    end;
  {$I-}
  assign(f,filename);
  reset(f,1);
  if ioresult<>0 then
    begin
      kes_load_exec_file:=1002;
      exit;
    end;
  l:=filesize(f);
  if ioresult<>0 then
    begin
      kes_load_exec_file:=1002;
      close(f);
      exit;
    end;
  if l>Kes_Maxbuffer then
    begin
      kes_load_exec_file:=1003;
      close(f);
      exit;
    end;
  blockread(f,Kes_Data,l);
  if ioresult<>0 then
    begin
      kes_load_exec_file:=1002;
      close(f);
      exit;
    end;
  close(f);
  {$I+}
  kes_load_exec_file:=0;
end;

procedure kes_buffer_transfer(to_kes: boolean; size: word);
begin;
  with kes_iopb^ do
    begin
      devicecode:=0;
      device_nr:=0;
      functioncode:=$0e;
      modification:=0;
      cylinder:=$2100;
      if to_kes then head:=$ff else head:=$00;
      buffer_ptr:=@Kes_Data;
      requested_bytes_l:=size;
    end;
  kes_ccb^.busy:=$ff;
  kes_wakeup;
  repeat until kes_ccb^.busy=0;
  kes_cib^.status_semaphore:=0;
end;

procedure kes_exec;
begin;
  with kes_iopb^ do
    begin
      devicecode:=0;
      device_nr:=0;
      functioncode:=$0c;
      modification:=0;
      general_ptr:=Ptr(0,$2100);
    end;
  kes_ccb^.busy:=$ff;
  kes_wakeup;
  repeat until kes_ccb^.busy=0;
  kes_cib^.status_semaphore:=0;
end;

procedure kes_buffer_clear;
var
  i:word;
begin
  for i:=0 to Kes_Maxbuffer do Kes_Data[i]:=0;
end;

end.

