# AFI-K5173 - IDE Controller für A7150 (CM1910)
AFI ist einen Platine, mit dem IDE-Festplatten an das Datenträger-Subsystem des A7150 angeschlossen werden können.
### Welche Features unterstützt AFI-K5173?
* 1 IDE Kanal für 2 Geräte
* DMA-Fähig
* Boot-Fähig
* Firmware Flashrom im System beschreibbar
* Iomega ZIP100 Atapi Laufwerke als Festplatte nutzbar
* Unterstützung Arduino-USB-Board auf Basis vom CH375B zur Verwendung von USB-Sticks (Achtung: Platine muss auf 5V Signalpegel umbebaut werden!)

### Welche Features sind in Vorbereitung?

* Firmware mit DMA-Übertragung direkt in den Host-Speicher für höhere Übertragungsrate
* Firmware "All in One"

### Teile des Projekts:

[Schaltung und PCB Layout in KiCad und PDF](schaltplan/)

[Materialliste](schaltplan/V1/Materialliste.txt)

[Firmware](firmware/)

[Tools](tools/)

[Treiber](treiber/)

[Anleitungen: Hardware-Test](knowhow/Hardwaretest.MD)
