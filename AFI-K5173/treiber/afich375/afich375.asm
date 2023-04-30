; Copyright (c) 2022, Stefan Berndt
;
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without 
; modification, are permitted provided that the following conditions are met:
; 1. Redistributions of source code must retain the above copyright notice, 
;    this list of conditions and the following disclaimer.
; 2. Redistributions in binary form must reproduce the above copyright notice, 
;    this list of conditions and the following disclaimer in the documentation 
;    and/or other materials provided with the distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF 
; THE POSSIBILITY OF SUCH DAMAGE.

; Dieser Geraetetreiber fuer DOS steuert den CH375 Chip am KES Subsystem des A7150 an.
; Der Treiber ist fuer nichts anderes zu gebrauchen!

; Version 1.0

org 0

                dw 0FFFFh
                dw 0FFFFh
                dw 2802h
                dw offset Strategy
                dw offset Interrupt
                db 1,0,0,0,0,0,0,0

DevRequest      dd 0

BPBTable        dw offset BPB
                dw 0      ; Segment

BPB             dw   200h ; sector size in bytes
                db     4h ; sectors per cluster (allocation unit size)
                dw     1h ; number of reserved sectors
                db     2h ; number of FATs on disk
                dw   200h ; number of root directory entries (directory size)
                dw     0h ; number of total sectors; if partition > 32Mb then set to zero and dword at 15h contains the actual count
                db   0F8h ; media descriptor byte  (see MEDIA DESCRIPTOR)
                dw   0FAh ; sectors per FAT
                dw    20h ; sectors per track
                dw    40h ; number of heads
                dd    20h ; number of hidden sectors
                dd 0DE000000h; number of total sectors if offset 8 is zero
                db    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
DiskId          db    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

LBA_Offset      dd 0

Mailbox_0       db 0  ; zusaetzliche Mailbox zum KES subsystem
Mailbox_1       db 0
Mailbox_2       db 0
Mailbox_3       db 0
Mailbox_4       db 0
Mailbox_5       db 0
Mailbox_6       db 0
Mailbox_7       db 0
Mailbox_8       db 0
Mailbox_9       db 0
Mailbox_10      db 0
Mailbox_12      db 0

Buffer          db 200h dup 0

JumpTable       dw offset Setup
                dw offset MediaCheck
                dw offset Create_BPB
                dw offset Dummy_Error
                dw offset Driver_Read
                dw offset Dummy_Error
                dw offset Dummy_Error
                dw offset Dummy_Error
                dw offset Driver_Write
                dw offset Driver_Write
                dw offset Dummy_Error
                dw offset Dummy_Error
                dw offset Dummy_Error
                dw offset Driver_Open
                dw offset Dummy_OK
                dw offset Dummy_OK

Strategy        proc far
                mov     word ptr cs:DevRequest,bx
                mov     word ptr cs:DevRequest+2,es
                retf
Strategy        endp


Interrupt       proc far
                push    es
                push    ds
                push    ax
                push    cx
                push    dx
                push    bx
                push    bp
                push    si
                push    di
                mov     ax,cs
                mov     ds,ax
                les     bx,DevRequest
                mov     al,es:[bx+2]
                and     ax,0Fh
                mov     di,ax
                shl     di,1
                call    JumpTable[di]
                les     bx,DevRequest
                or      ah,1
                mov     es:[bx+3], ax
                pop     di
                pop     si
                pop     bp
                pop     bx
                pop     dx
                pop     cx
                pop     ax
                pop     ds
                pop     es
                retf
Interrupt       endp


MediaCheck      proc near
                mov     byte ptr es:[bx+0Eh], 1
                mov     al,0Ah
                call    Send_CMD
                mov     al,1Bh
                call    Send_Data
                call    Rec_Data
                and     al,7Fh
                jnz     short MediaCheck_1
                mov     byte ptr es:[bx+0Eh],0FFh
MediaCheck_1:   mov     word ptr es:[bx+0Fh],offset DiskId
                mov     word ptr es:[bx+11h],cs
                xor     ax,ax
                retn
MediaCheck      endp


Create_BPB      proc near
                call    Parse_MBR
                les     bx,DevRequest
                mov     word ptr es:[bx+12h],offset BPB
                mov     word ptr es:[bx+14h],cs
                jz      short Create_BPB_1
                xor     ax,ax
                retn
Create_BPB_1:   mov     ax,8002h
                retn
Create_BPB      endp


Dummy_Error     proc near
                mov     ax,8003h
                retn
Dummy_Error     endp


Set_LBA_Addr    proc near
                mov     ax,es:[bx+14h]
                xor     dx,dx
                cmp     ax,0FFFFh
                jnz     short Set_LBA_Addr_1
                mov     ax,es:[bx+1Ah]
                mov     dx,es:[bx+1Ch]
Set_LBA_Addr_1: add     ax,word ptr ds:LBA_Offset
                mov     word ptr Mailbox_6,ax
                adc     dx,word ptr ds:LBA_Offset+2
                mov     word ptr Mailbox_8,dx
                mov     al,es:[bx+12h]
                mov     byte ptr Mailbox_5,al      ; Anzahl Sektoren
                les     di,es:[bx+0Eh]
                mov     word ptr Mailbox_1,di      ; Offset Data
                mov     word ptr Mailbox_3,es      ; Segment Data
                retn
Set_LBA_Addr    endp


Driver_Read     proc near
                mov     cl,3
Driver_Read_1:  call    Set_LBA_Addr
                mov     Mailbox_0,25        ; Kommando: Sektor lesen
                call    KES_WakeupHLT
                mov     ax, word ptr Mailbox_1
                cmp     ax,0
                jnz     short Driver_Read_2
                retn
Driver_Read_2:  dec     cl
                jnz     short Driver_Read_1
                retn
Driver_Read     endp


Driver_Write    proc near
                mov     cl,3
Driver_Write_1: call    Set_LBA_Addr
                mov     Mailbox_0,24        ; Kommando: Sektor schreiben
                call    KES_WakeupHLT
                mov     ax,word ptr Mailbox_1
                cmp     ax,0
                jnz     short Driver_Write_2
                retn
Driver_Write_2: dec     cl
                jnz     short Driver_Write_1
                retn
Driver_Write    endp


Driver_Open     proc near
                mov     Mailbox_0,26        ; Kommando: Disk initialisieren
                call    KES_Wakeup
                cmp     Mailbox_1,14h       ; 14h=Successful
                jnz     short Driver_Open_1
                xor     ax,ax
                retn
Driver_Open_1:  mov     ax,8002h
                retn
Driver_Open     endp


Dummy_OK        proc near
                xor     ax, ax
                retn
Dummy_OK        endp

; Byte auf dem Kommando-Kanal zum CH375 senden
Send_CMD        proc near
                mov     Mailbox_0,20
                mov     Mailbox_1,al
                jmp     short KES_Wakeup
Send_CMD        endp

; Byte auf dem Daten-Kanal zum CH375 senden
Send_Data       proc near
                mov     Mailbox_0,22
                mov     Mailbox_1,al
                jmp     short KES_Wakeup
Send_Data       endp

; Byte auf dem Daten-Kanal vom CH375 holen
Rec_Data        proc near
                mov     Mailbox_0,23
                call    KES_Wakeup
                mov     al,Mailbox_1
                retn
Rec_Data        endp

; KES Subsystem aufwecken, und auf Ausfuehrung des Auftrags warten
KES_Wakeup      proc near
                mov     al,1
                out     4Bh,al              ; fire KES channel B wakeup
KES_Wakeup_1:   cmp     Mailbox_0,0
                jnz     KES_Wakeup_1
                retn
KES_Wakeup      endp

; KES Subsystem aufwecken, und auf Ausfuehrung des Auftrags warten, CPU so lange in HALT
KES_WakeupHLT   proc near
                mov     al,1
                out     4Bh,al              ; fire KES channel B wakeup
KES_Wakeup_2:   hlt                         ; KES does wake up the main CPU if ready
                cmp     Mailbox_0,0         ; check KES done, also timer can wake us up
                jnz     KES_Wakeup_2
                mov     al,0
                out     4Bh,al              ; ack KES interrupt
                retn
KES_WakeupHLT   endp

; Sektor von USB in den internen Buffer lesen
Internal_Read   proc near
                mov     cl, 3
Int_Read_1:     mov     word ptr LBA_Offset, ax
                mov     word ptr LBA_Offset+2, dx
                mov     word ptr Mailbox_6,ax
                mov     word ptr Mailbox_8,dx
                mov     byte ptr Mailbox_5,1        ; sector count
                mov     word ptr Mailbox_1,offset Buffer ; offset data
                mov     word ptr Mailbox_3,cs       ; segmend data
                mov     Mailbox_0,25                ; command: block read
                call    KES_WakeupHLT
                cmp     word ptr Mailbox_1,0        ; check error result
                jnz     short Int_Read_2
                retn
Int_Read_2:     dec     cl
                jnz     short Int_Read_1
                or      al,01h
                retn
Internal_Read   endp

; MBR von USB lesen, Partition suchen, BPB dieser Partition einlesen und Sektor-Offset merken
Parse_MBR       proc near
                xor     ax, ax
                xor     dx, dx
                call    Internal_Read
                jnz     short Parse_MBR_4
                mov     di, offset Buffer
                cmp     word ptr [di+01FEh],0AA55h
                jnz     short Parse_MBR_4   
                cmp     byte ptr [di],0E9h
                jz      short Parse_MBR_5
                cmp     byte ptr [di],0EBh
                jz      short Parse_MBR_5
                add     di,1BEh
Parse_MBR_1:    mov     ax,[di+8]
                mov     dx,[di+0Ah]
                or      ax,dx
                jz      short Parse_MBR_2
                mov     al,[di+4]
                cmp     al,1         ; FAT12
                jz      short Parse_MBR_3
                cmp     al,4         ; FAT16<32MB
                jz      short Parse_MBR_3
                cmp     al,6         ; FAT16>32MB
                jz      short Parse_MBR_3
                cmp     al,0Ch       ; FAT32 (LBA)
                jz      short Parse_MBR_3
                cmp     al,0Eh       ; FAT16>32MB (LBA)
                jz      short Parse_MBR_3
Parse_MBR_2:    add     di,10h
                cmp     di,offset Buffer+1FFh
                jb      short Parse_MBR_1
                jmp     short Parse_MBR_4
Parse_MBR_3:    mov     ax,[di+8]
                mov     dx,[di+0Ah]
                call    Internal_Read
                jnz     short Parse_MBR_4
                mov     di,offset Buffer
                cmp     byte ptr [di],0E9h
                jz      short Parse_MBR_5
                cmp     byte ptr [di],0EBh
                jz      short Parse_MBR_5
Parse_MBR_4:    xor     ax,ax
                retn
Parse_MBR_5:    mov     ax,cs
                mov     si,offset Buffer+0Bh
                mov     es,ax
                mov     di,offset BPB
                mov     cx,29h
                cld
                rep     movsb
                mov     di,offset Buffer
                mov     si,offset Buffer+27h
                cmp     byte ptr [di+26h],29h
                jz      short Parse_MBR_7
                cmp     byte ptr [di+42h],29h
                jnz     short Parse_MBR_7
                mov     si,offset Buffer+43h
Parse_MBR_7:    mov     di,offset DiskId
                mov     cx,15
                cld
                rep     movsb
                mov     al,1
                or      al,al
                retn
Parse_MBR       endp

; Treiber-Setup
Setup           proc near
                mov     al, es:[bx+16h]
                add     al, 41h
                mov     byte ptr ds:DriveText, al
                mov     dx, offset ModVersion
                call    PrintString
; Testet auf signatur "A 7150" im BIOS(ACT) ROM. Bei verschiedenen ACT-Versionen ist der an unterschiedlichen Poitionen, deshalb ganzen Bereich absuchen
                push    es
                mov     ax,0FBA8h       ; BIOS / ACT Segment
                mov     es,ax
                mov     di,0
Setup_1:        cmp     word ptr es:[di],2041h     
                jnz     Setup_2
                cmp     word ptr es:[di+2],3137h
                jnz     Setup_2
                cmp     word ptr es:[di+4],3035h
                jz      Setup_3
Setup_2:        inc     di
                cmp     di,255
                jnz     Setup_1
                mov     dx, offset aCH375Incompat
                pop     es
                jmp     short Setup_7
; der USB-Kontroller braucht Kanal-B Firmware-Support
Setup_3:        mov     ax,04Ah          ; setze "Get Version" in die primaere Mailbox des Kanal B
                mov     es,ax
                mov     al,1
                mov     es:06h,al        ; Kommando-Register
                out     4Bh,al           ; KES Kanal B aufwecken
                mov     cx,1000
Setup_4:        cmp     byte ptr es:06,0 ; die KES setzt das Kommando auf 0, wenn sie fertig ist (Die KES ist temperamentvoll, deshalb "Sie")
                jz      Setup_5
                loop    Setup_4
                mov     dx, offset KesChanBOffline ; die Zeit ist um!
                pop     es
                jmp     short Setup_7
; Fuer die Bequemlichkeit setzen wir uns eine zusaetzliche B-Kanal-Mailbox direkt in unser Code-Segment
Setup_5:        mov     byte ptr es:06,30 ; setze Kommando "register additional mailbox"
                mov     word ptr es:07,offset Mailbox_0 ; und unsere Mailbox Adresse
                mov     ax,cs
                mov     word ptr es:09,ax
                mov     al,1
                out     4Bh,al           ; KES Kanal B aufwecken
Setup_6:        cmp     byte ptr es:06,0 ; Warten bis die KES fertig ist
                jnz     Setup_6          ; kein Timeout, wir wissen, dass die KES reagiert
                pop     es
; CH375 Echo Test
                mov     al,6
                call    Send_CMD
                mov     al,55h
                call    Send_Data
                call    Rec_Data
                cmp     al,0AAh
                jnz     short Setup_12
                mov     al,6
                call    Send_CMD
                mov     al,0AAh
                call    Send_Data
                call    Rec_Data
                cmp     al,55h
                jz      short Setup_8
Setup_12:       mov     dx, offset ChipOffline
Setup_7:        call    PrintString
                mov     ax,800Ch
                retn
; CH375 Kontoller initialisieren
Setup_8:        mov     al,15h         ; Setze Modus
                call    Send_CMD
                mov     al,06h         ; Host-Modus, Automatisches SOF
                call    Send_Data
                mov     ax,0
Setup_13:       dec     ax             ; Warten...
                jnz     Setup_13
                call    Rec_Data
                mov     Mailbox_0,26        ; Kommando: disk init
                call    KES_Wakeup
                cmp     Mailbox_1,14h       ; Ergebnis
                jz      short Setup_10
                mov     dx, offset DiskNotFound
Setup_9:        call    PrintString
                jmp     short Setup_11
Setup_10:       call    Parse_MBR
                jnz     short Setup_11
                mov     dx,offset BootSektorError
                call    PrintString
; Treiber installieren
Setup_11:       les     bx,ds:DevRequest
                mov     ax,1
                mov     word ptr ds:JumpTable,offset Dummy_Error
                mov     es:[bx+0Dh],al
                mov     word ptr es:[bx+0Eh],offset Setup+2
                mov     word ptr es:[bx+10h],cs
                mov     di, offset BPBTable
                mov     es:[bx+12h],di
                mov     word ptr [di+2],cs
                mov     word ptr es:[bx+14h],cs
                mov     es:[bx+17h],ax
                xor     ax,ax
                retn
Setup           endp


PrintString     proc near
                mov     ax,900h
                int     21h
                retn
PrintString     endp


ModVersion      db 'CH375 Treiber, A7150(CM1910)-KES Version V1.0, Laufwerk ' 
DriveText       db ' :',0Dh,0Ah,'$'
aCH375Incompat  db 'Fatal: Das hier ist kein A7150!',0Dh,0Ah,'$'
KesChanBOffline db 'Fatal: KES Channel B antwortet nicht!',0Dh,0Ah,'$'
ChipOffline     db 'Fatal: CH375 Chip antwortet nicht',0Dh,0Ah,'$'
DiskNotFound    db 'Warnung: Disk nicht gefunden',0Dh,0Ah,'$'
BootSektorError db 'Warnung: Fehler im Bootsektor',0Dh,0Ah,'$'

end
