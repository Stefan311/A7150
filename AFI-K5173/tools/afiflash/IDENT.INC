{ *** Chip-Identifizierung *********************************************** }

procedure IdentAMD1;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($5555,$AA); { Reset/Read }
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(1);        { 15us warten }
  WMem($5555,$AA); { Autoselect }
  WMem($2AAA,$55);
  WMem($5555,$90);
  Pause(1);        { 15us warten }
  mfg:=RMem($0);
  dev:=RMem($1);
  if (mfg<>AMD) then mfg:=UnknownMfg;
  WMem($5555,$AA); { Reset/Read }
  WMem($2AAA,$55);
  WMem($5555,$F0);
  asm STI end; {enable}
end;

procedure IdentAMD2;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($0,$FF);    { Reset/Read }
  Pause(1);        { 15us warten }
  WMem($0,$90);    { Autoselect }
  Pause(1);        { 15us warten }
  mfg:=RMem($0);
  dev:=RMem($1);
  if (mfg<>AMD) then mfg:=UnknownMfg;
  WMem($0,$FF);    { Reset/Read }
  asm STI end; {enable}
end;

procedure IdentAS; { !!! } { AS29F002: Reset = Pin 1 = J5 = A18 = High setzen }
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($45555,$AA); { Reset }
  WMem($42AAA,$55);
  WMem($45555,$00);
  Pause(17);        { 250 us warten }
  WMem($45555,$AA); { ID Read Code }
  WMem($42AAA,$55);
  WMem($45555,$02);
  Pause(17);        { 250 us warten }
  mfg:=RMem($40000);
  dev:=RMem($40001);
  if (mfg<>AS) then mfg:=UnknownMfg;
  WMem($45555,$AA); { Reset }
  WMem($42AAA,$55);
  WMem($45555,$00);
  Pause(17);        { 250 us warten }
  asm STI end; {enable}
end;

procedure IdentAtmel; { AT49F001/2: Reset = Pin 1 = J5 = A18 = High setzen }
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($45555,$AA);
  WMem($42AAA,$55);
  WMem($45555,$90);
  Pause(10*67); { 10ms warten }
  mfg := RMem($0);
  dev := RMem($1);
  case dev of $05,$07,$17,$0B,$13 : lockout:=(RMem($2) AND 1)=1; { Boot Block Lockout? }
              $04 : lockout:=(RMem($1C002) AND 1)=1; { AT49F001T }
              $08 : lockout:=(RMem($3C002) AND 1)=1; { AT49F002T }
              $12 : lockout:=(RMem($7C002) AND 1)=1; { AT49F040T }
               else lockout:=false;
  end{case};
  if (mfg<>Atmel) then mfg:=UnknownMfg;
  WMem($45555,$AA);
  WMem($42AAA,$55);
  WMem($45555,$F0);
  Pause(10*67); { 10ms warten }
  asm STI end; {enable}
end;

procedure IdentCatalyst;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  WMem($0,$90); { Identify }
  mfg:=RMem($0);
  dev:=RMem($1);
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  if (mfg<>CSI) then mfg:=UnknownMfg;
  asm STI end; {enable}
end;

procedure IdentHyundai; { HY29F002: Reset = Pin 1 = J5 = A18 = High setzen }
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($45555,$AA); { Reset }
  WMem($42AAA,$55);
  WMem($45555,$F0);
  Pause(1);         { 10us warten }

  WMem($45555,$AA); { Chip-ID }
  WMem($42AAA,$55);
  WMem($45555,$90);
  Pause(1);         { 10us warten }
  mfg:=RMem($40000);
  dev:=RMem($40001);

  WMem($45555,$AA); { Reset }
  WMem($42AAA,$55);
  WMem($45555,$F0);
  Pause(1);         { 10us warten }

  if (mfg<>Hyundai) then mfg:=UnknownMfg;
  asm CLI end; {disable}
end;

procedure IdentIntel;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  WMem($0,$90); { Identify }
  mfg:=RMem($0);
  dev:=RMem($1);
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  if (mfg<>Intel) then mfg:=UnknownMfg;
  asm STI end; {enable}
end;

procedure IdentMV;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($5555,$AA);
  WMem($2AAA,$55);
  WMem($5555,$90);
  Pause(10*67); { 10ms warten }
  mfg:=RMem($0);
  dev:=RMem($1);
  if (mfg=MV) then
    case dev of $02 : protected:=(RMem($3C002)=1); { V29C5002T, Boot Block protected? }
                $A2 : protected:=(RMem($0C002)=1); { V29C5002B }
    end{case};
  if (mfg<>MV) then mfg:=UnknownMfg;
  WMem($5555,$AA);
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(10*67); { 10ms warten }
  asm STI end; {enable}
end;

procedure IdentMX1;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  WMem($0,$90); { Identify }
  mfg:=RMem($0);
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  WMem($0,$90); { Identify }
  dev:=RMem($1);
  WMem($0,$FF); { Reset }
  WMem($0,$FF); { Reset }
  if (mfg<>MX) then mfg:=UnknownMfg;
  asm STI end; {enable}
end;

procedure IdentMX2; { MX29F002: Reset = Pin 1 = J5 = A18 = High setzen }
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($45555,$F0); { Reset }
  Pause(1);       { 15us warten }
  WMem($45555,$AA); { Autoselect }
  WMem($422AA,$55);
  WMem($45555,$90);
  Pause(1);        { 15us warten }
  mfg:=RMem($0);
  dev:=RMem($1);
  if (mfg<>MX) then mfg:=UnknownMfg;
  WMem($555,$F0); { Reset }
  asm STI end; {enable}
end;

procedure IdentSST1;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($5555,$AA); { Reset }
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(1);        { 15us warten }

  WMem($5555,$AA); { Chip-ID }
  WMem($2AAA,$55);
  WMem($5555,$80);
  WMem($5555,$AA);
  WMem($2AAA,$55);
  WMem($5555,$60);
  Pause(1);        { 15us warten }
  mfg:=RMem($0);
  dev:=RMem($1);

  WMem($5555,$AA); { Reset }
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(1);        { 15us warten }

  if (mfg<>SST) then mfg:=UnknownMfg;
  asm STI end; {enable}
end;

procedure IdentSST2;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($0,$FF);    { Reset }
  Pause(1);        { 15us warten }

  WMem($0,$90);    { Identify }
  Pause(1);        { 15us warten }
  mfg:=RMem($0);
  dev:=RMem($1);

  WMem($0000,$FF); { Reset }
  Pause(1);        { 15us warten }

  if (mfg<>SST) then mfg:=UnknownMfg;
  asm STI end; {enable}
end;

procedure IdentSST3;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($5555,$AA);
  WMem($2AAA,$55);
  WMem($5555,$90);
  Pause(10*67); { 10ms warten }
  mfg:=RMem($0);
  dev:=RMem($1);
  if (mfg<>SST) then mfg:=UnknownMfg;
  WMem($5555,$AA);
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(10*67); { 10ms warten }
  asm STI end; {enable}
end;

procedure IdentST1;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($0,$90);    { Chip-ID }
  Pause(1);        { 15us warten }
  mfg:=RMem($0);
  dev:=RMem($1);
  WMem($0,$FF);    { Reset }
  Pause(1);        { 15us warten }
  if (mfg<>STM) then mfg:=UnknownMfg;
  asm STI end; {enable}
end;

procedure IdentST2; { M29F002: Reset = Pin 1 = J5 = A18 = High setzen }
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($45555,$AA); { Reset/Read }
  WMem($42AAA,$55);
  WMem($45555,$F0);
  Pause(1);       { 15us warten }
  WMem($45555,$AA); { Autoselect }
  WMem($42AAA,$55);
  WMem($45555,$90);
  Pause(1);       { 15us warten }
  mfg:=RMem($0);
  dev:=RMem($1);
  if (mfg<>STM) then mfg:=UnknownMfg;
  WMem($45555,$AA); { Reset/Read }
  WMem($42AAA,$55);
  WMem($45555,$F0);
  asm STI end; {enable}
end;

procedure IdentWin;
begin
  Pause(10*67); { 10ms warten } { ASK, NEW: wait first, otherwise no autodetection! }
  asm CLI end; {disable}
  WMem($5555,$AA); { Reset }
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(1);        { 10us warten }

  WMem($5555,$AA); { Chip-ID }
  WMem($2AAA,$55);
  WMem($5555,$80);
  WMem($5555,$AA);
  WMem($2AAA,$55);
  WMem($5555,$60);
  Pause(1);        { 10us warten }
  mfg:=RMem($0);
  dev:=RMem($1);

{ !!! Boot-Block-Lockout-Erkennung einbauen }

  WMem($5555,$AA); { Reset }
  WMem($2AAA,$55);
  WMem($5555,$F0);
  Pause(1);        { 10us warten }

  if (mfg<>Winbond) then mfg:=UnknownMfg;
  asm CLI end; {disable}
end;

procedure EvalSize(m,d:byte);
begin
  memsize:=0; typ:='Unbekannt'; J5:=false; J4:=false; RESPIN:=false;
  case m of AMD      : case d of $A7 : begin memsize:=128; typ:='Am28F010'; J5:=true;                end;
                                 $A2 : begin memsize:=128; typ:='Am28F010A'; J5:=true;               end;
                                 $2A : begin memsize:=256; typ:='Am28F020'; J5:=true;                end;
                                 $29 : begin memsize:=256; typ:='Am28F020A'; J5:=true;               end;
                                 $20 : begin memsize:=128; typ:='Am29F010';                          end;
                                 $B0 : begin memsize:=256; typ:='Am29F002(N)T';                      end;
                                 $34 : begin memsize:=256; typ:='Am29F002(N)B';                      end;
                                 $A4 : begin memsize:=512; typ:='Am29F040B';                         end;
                       end;
            AS       : case d of $03, 
                                 $04,
                                 $06 : begin memsize:=128; typ:='AS29F010';                          end;
                       end;
            Atmel    : case d of $5D : begin memsize:= 64; typ:='AT29C512';                          end;
                                 $D5 : begin memsize:=128; typ:='AT29C010A';                         end;
                                 $DA : begin memsize:=256; typ:='AT29C020';                          end;
                                 $A4 : begin memsize:=512; typ:='AT29C040A';                         end;
                                 $05 : begin memsize:=128; typ:='AT49F001(N)'; J5:=true; RESPIN:=true; end;
                                 $04 : begin memsize:=128; typ:='AT49F001(N)T'; J5:=true; RESPIN:=true; end;
                                 $07 : begin memsize:=256; typ:='AT49F002(N)'; J5:=true; RESPIN:=true; end;
                                 $08 : begin memsize:=256; typ:='AT49F002(N)T'; J5:=true; RESPIN:=true; end;
                                 $17 : begin memsize:=128; typ:='AT49F010';                          end;
                                 $0B : begin memsize:=256; typ:='AT49F020';                          end;
                                 $13 : begin memsize:=512; typ:='AT49F040';                          end;
                                 $12 : begin memsize:=512; typ:='AT49F040T';                         end;
                       end;
            CSI      : case d of $94 : begin memsize:=128; typ:='CAT28F001Bx-T'; J5:=true; J4:=true; end;
                                 $95 : begin memsize:=128; typ:='CAT28F001Bx-B'; J5:=true; J4:=true; end;
                                 $B4 : begin memsize:=128; typ:='CAT28F010'; J5:=true;               end;
                                 $BD : begin memsize:=256; typ:='CAT28F020'; J5:=true;               end;
                       end;
            Hyundai  : case d of $B0 : begin memsize:=256; typ:='HY29F002T'; RESPIN:=true;           end;
                                 $34 : begin memsize:=256; typ:='HY29F002B'; RESPIN:=true;           end;
                       end;
            Intel    : case d of $94 : begin memsize:=128; typ:='i28F001Bx-T'; J5:=true; J4:=true;   end;
                                 $95 : begin memsize:=128; typ:='i28F001Bx-B'; J5:=true; J4:=true;   end;
                                 $B4 : begin memsize:=128; typ:='i28F010'; J5:=true;                 end;
                                 $BD : begin memsize:=256; typ:='i28F020'; J5:=true;                 end;
                       end;
            MX       : case d of $1A : begin memsize:=128; typ:='MX28F1000(P)'; J5:=true;            end;
                                 $2A : begin memsize:=256; typ:='MX28F2000P'; J5:=true;              end;
                                 $3C : begin memsize:=256; typ:='MX28F2000T'; J5:=true;              end;
                                 $18 : begin memsize:=128; typ:='MX29F001T';                         end;
                                 $19 : begin memsize:=128; typ:='MX29F001B';                         end;
                                 $B0 : begin memsize:=256; typ:='MX29F002(N)T'; RESPIN:=true;        end;
                                 $34 : begin memsize:=256; typ:='MX29F002(N)B'; RESPIN:=true;        end;
                                 $A4 : begin memsize:=512; typ:='MX29F040';                          end;
                       end;
            MV       : case d of $02 : begin memsize:=256; typ:='F/V29C51002T';                      end;
                                 $A2 : begin memsize:=256; typ:='F/V29C51002B';                      end;
                       end;
            SST      : case d of $07 : begin memsize:=128; typ:='PH29EE010';                         end;
                                 $10 : begin memsize:=256; typ:='PH29EE020';                         end;
                                 $04 : begin memsize:=512; typ:='PH28SF040';                         end;
                                 $B5 : begin memsize:=128; typ:='PH39SF010';                         end;
                                 $B6 : begin memsize:=256; typ:='PH39SF020';                         end;
                                 $B7 : begin memsize:=512; typ:='PH39SF040';                         end;
                       end;
            STM      : case d of $07 : begin memsize:=128; typ:='M28F101'; J5:=true;                 end;
                                 $A8 : begin memsize:= 32; typ:='M28F256'; J5:=true;                 end;
                                 $AA : begin memsize:= 32; typ:='M28F256A'; J5:=true;                end;
                                 $02 : begin memsize:= 64; typ:='M28F512'; J5:=true;                 end;
                                 $B0 : begin memsize:=256; typ:='M29F002T/NT'; RESPIN:=true;         end;
                                 $34 : begin memsize:=256; typ:='M29F002B'; RESPIN:=true;            end;
                                 $E2 : begin memsize:=512; typ:='M29F040B';                          end;
                 end;

            Winbond  : case d of $C1 : begin memsize:=128; typ:='W29EE011/W29C010/11';               end;
                                 $45 : begin memsize:=256; typ:='W29C020';                           end;
                                 $46 : begin memsize:=512; typ:='W29C040';                           end;
                                 $0B : begin memsize:=256; typ:='W49F002'; J5:=true;                 end;
                       end;
  end;
  memsize:=memsize SHL 10;

end;

function MfgStr(m:byte):string;
var s : string;
begin
  case m of AMD     : s:='AMD';
            AS      : s:='Alliance';
            Atmel   : s:='Atmel';
            CSI     : s:='Catalyst';
            Hyundai : s:='Hyundai';
            Intel   : s:='Intel';
            MV      : s:='Mosel-Vitelic';
            MX      : s:='Macronix';
            SST     : s:='SST';
            STM     : s:='STM';
            Winbond : s:='Winbond';
                 else s:='unbekannt';
  end;
  MfgStr:=s;
end;

procedure Identify;
begin
  IdentAMD1;
  if (mfg=UnknownMfg) then IdentAMD2;
{  if (mfg=UnknownMfg) then IdentAS; }
  if (mfg=UnknownMfg) then IdentAtmel;
  if (mfg=UnknownMfg) then IdentCatalyst;
  if (mfg=UnknownMfg) then IdentHyundai;
  if (mfg=UnknownMfg) then IdentIntel;
  if (mfg=UnknownMfg) then IdentMX1;
  if (mfg=UnknownMfg) then IdentMX2;
  if (mfg=UnknownMfg) then IdentMV;
  if (mfg=UnknownMfg) then IdentSST1;
  if (mfg=UnknownMfg) then IdentSST2;
  if (mfg=UnknownMfg) then IdentSST3;
  if (mfg=UnknownMfg) then IdentST1;
  if (mfg=UnknownMfg) then IdentST2;
  if (mfg=UnknownMfg) then IdentWin;
  EvalSize(mfg,dev);
{$IFDEF DEBUG}
        write('Identify: mfg=$',BTH(mfg),' dev=$',BTH(dev));
        if lockout then write(' LOCKOUT');
        writeln(' Memsize=',memsize);
{$ENDIF}
end;

procedure ManualSelect;
begin
  if (mfg=UnknownMfg) then begin
    writeln;
    case sprache of 'D' : writeln('Bitte w'+#132+'hlen Sie den Hersteller:');
                    'N' : writeln('Selecteer het merk aub:');
                     else writeln('Please select the manufacturer:');
    end{case};
    writeln('(1) AMD');
{ !!! } {    writeln('(2) Alliance'); }
    writeln('(2) Atmel');
    writeln('(3) CSI');
    writeln('(4) Hyundai');
    writeln('(5) Intel');
    writeln('(6) Macronix');
    writeln('(7) Mosel-Vitelic');
    writeln('(8) SST');
    writeln('(9) ST Microelectronics');
    writeln('(A) Winbond');
    GetKey(' ');
    if (d in ['a'..'z']) then d:=chr(ord(d)-32);
    case d of '1' : mfg:=AMD;
{              '2' : mfg:=AS; }
              '2' : mfg:=Atmel;
              '3' : mfg:=CSI;
              '4' : mfg:=Hyundai;
              '5' : mfg:=Intel;
              '6' : mfg:=MX;
              '7' : mfg:=MV;
              '8' : mfg:=SST;
              '9' : mfg:=STM;
              'A' : mfg:=Winbond;
              else  mfg:=UnknownMfg;
    end;
  end;

  dev:=UnknownDev;
  if (mfg<>UnknownMfg) then begin
    writeln;
    case sprache of 'D' : writeln('Bitte w'+#132+'hlen Sie den Bausteintyp:');
                    'N' : writeln('Selecteer het type chip aub:');
                     else writeln('Please select the chip type:');
    end{case};
    case mfg of AMD   : begin
                          writeln('(1) Am28F010');
                          writeln('(2) Am28F010A');
                          writeln('(3) Am28F020');
                          writeln('(4) Am28F020A');
                          writeln('(5) Am29F010');
                          writeln('(6) Am29F002(N)T/B');
                          writeln('(7) Am29F040');
                        end;
                AS    : begin
                          writeln('(1) AS29F010');
                        end;
                Atmel : begin
                          writeln('(1) AT29C512');
                          writeln('(2) AT29C010A');
                          writeln('(3) AT29C020');
                          writeln('(4) AT29C040A');
                          writeln('(5) AT49F001(N)[T]');
                          writeln('(6) AT49F002(N)[T]');
(*                          writeln('(7) AT49F010'); *)
(*                          writeln('(7) AT49F020'); *)
                          writeln('(7) AT49F040[T]');
                        end;
                CSI   : begin
                          writeln('(1) CAT28F001Bx-T');
                          writeln('(2) CAT28F001Bx-B');
                          writeln('(3) CAT28F010');
                          writeln('(4) CAT28F020');
                        end;
              Hyundai : begin
                          writeln('(1) HY29F002T');
                          writeln('(2) HY29F002B');
                        end;
                Intel : begin
                          writeln('(1) i28F001Bx-T');
                          writeln('(2) i28F001Bx-B');
                          writeln('(3) i28F010/Z28F010');
                          writeln('(4) i28F020');
                        end;
                MX    : begin
                          writeln('(1) M28F1000P');
                          writeln('(2) M28F2000P');
                          writeln('(3) M28F2000T');
                          writeln('(4) MX29F001(N)T/B');
                          writeln('(5) MX29F002(N)T/B');
                          writeln('(6) MX29F040');
                        end;
                MV    : begin
                          writeln('(1) F/V29C51002T');
                          writeln('(2) F/V29C51002B');
                        end;
                SST   : begin
                          writeln('(1) PH29EE010');
                          writeln('(2) PH29EE020');
                          writeln('(3) PH28SF040');
                          writeln('(4) PH39SF010');
                          writeln('(5) PH39SF020');
                          writeln('(6) PH39SF040');
                        end;
                STM   : begin
                          writeln('(1) M28F256(A)');
                          writeln('(2) M28F512');
                          writeln('(3) M28F101');
                          writeln('(4) M29F002T/NT/B');
                          writeln('(5) M29F040B');
                        end;
              Winbond : begin
                          writeln('(1) W29EE011, W29C010/11');
                          writeln('(2) W29C020');
                          writeln('(3) W29C040');
                          writeln('(4) W49F002');
                        end;
    end;
    GetKey(' ');
    if (d in ['a'..'z']) then d:=chr(ord(d)-32);
    case mfg of AMD   : case d of '1' : dev:=$A7; { Am28F010 }
                                  '2' : dev:=$A2; { Am28F010A }
                                  '3' : dev:=$2A; { Am28F020 }
                                  '4' : dev:=$29; { Am28F020A }
                                  '5' : dev:=$20; { Am29F010 }
                                  '6' : dev:=$B0; { Am29F002NT, NB=$34 }
                                  '7' : dev:=$A4; { Am29F040 }
                        end;
                AS    : case d of '1' : dev:=$04; { AS29F010 }
                        end;
                Atmel : case d of '1' : dev:=$5D; { AT29C512 }
                                  '2' : dev:=$D5; { AT29C010A }
                                  '3' : dev:=$DA; { AT29C020 }
                                  '4' : dev:=$A4; { AT29C040A }
                                  '5' : dev:=$05; { AT49F001 }
                                  '6' : dev:=$07; { AT49F002 = F002T }
(*                                  '7' : dev:=$17; { AT49F010 } *)
(*                                  '7' : dev:=$0B; { AT49F020 } *)
                                  '7' : dev:=$13; { AT49F040 = F040T }
                        end;
                CSI   : case d of '1' : dev:=$94; { CAT28F001Bx-T }
                                  '2' : dev:=$95; { CAT28F001Bx-B }
                                  '3' : dev:=$B4; { CAT28F010 }
                                  '4' : dev:=$BD; { CAT28F020 }
                        end;
              Hyundai : case d of '1' : dev:=$B0; { HY29F002T }
                                  '2' : dev:=$34; { HY29F002B }
                        end;
                Intel : case d of '1' : dev:=$94; { i28F001Bx-T }
                                  '2' : dev:=$95; { i28F001Bx-B }
                                  '3' : dev:=$B4; { i28F010 }
                                  '4' : dev:=$BD; { i28F020 }
                        end;
                MX    : case d of '1' : dev:=$1A; { MX28F1000P }
                                  '2' : dev:=$2A; { MX28F2000P }
                                  '3' : dev:=$3C; { MX28F2000T }
                                  '4' : dev:=$18; { MX29F001(N)T, B=$19 }
                                  '5' : dev:=$B0; { MX29F002(N)T, B=$34 }
                                  '6' : dev:=$A4; { MX29F040 }
                        end;
                MV    : case d of '1' : dev:=$02; { F/V29C51002T }
                                  '2' : dev:=$A2; { F/V29C51002B }
                        end;
                SST   : case d of '1' : dev:=$07; { PH29EE010 }
                                  '2' : dev:=$10; { PH29EE020 }
                                  '3' : dev:=$04; { PH28SF040 }
                                  '4' : dev:=$B5; { PH39SF010 }
                                  '5' : dev:=$B6; { PH39SF020 }
                                  '6' : dev:=$B7; { PH39SF040 }
                        end;
                STM   : case d of '1' : dev:=$A8; { M28F256 }
                                  '2' : dev:=$02; { M28F512 }
                                  '3' : dev:=$07; { M28F101 }
                                  '4' : dev:=$B0; { M29F002T/NT, B=$34 }
                                  '5' : dev:=$E2; { M29F040B }
                        end;
              Winbond : case d of '1' : dev:=$C1; { W29EE011/W29C010/11 }
                                  '2' : dev:=$45; { W29C020 }
                                  '3' : dev:=$46; { W29C040 }
                                  '4' : dev:=$0B; { W49F002 }
                        end;
    end;
    if (mfg<>UnknownMfg) and (dev<>UnknownDev) then EvalSize(mfg,dev);
    writeln;
  end;
end;

