# AFI-K5173 - IDE Controller für A7150 (CM1910)
AFI ist einen Platine, mit dem IDE-Festplatten an das Datenträger-Subsystem des A7150 ageschlossen werden können.
### Welche Features unterstützt AFI-K5173? (bzw. wird es unterstützen?)
* 1 IDE Kanal für 2 Geräte
* DMA-Fähig
* Boot-Fähig
* Firmware Flashrom im System beschreibbar
### Wie ist der Projektstatus?

Schaltungsentwurf: *fertig*

Test auf Steckbrett: *fertig*

PCB Layout: *fertig*

PCB Prototyp: *beim Zoll*

Firmware: *begonnen*

[Tools:](tools/)
+ Memorydump (KESDump): *fertig*
+ Firmware-Tool (AFIFlash): *fertig*
+ Hardware-Test (AFITest): *fertig*
+ Diskinfo-Tool (AFIInfo): *noch nicht begonnen*
+ SMART-Info (AFISmart): *noch nicht begonnen*
+ Z80-Programm-Tool (BIN2INC): *fertig*
+ CRC-Generator (CRCTOOL): *fertig*
