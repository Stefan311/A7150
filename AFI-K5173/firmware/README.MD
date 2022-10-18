# Firmware-Versionen

---
[Rom-Floppy:](rom_disk/)

Diese Firmware-Version implementiert nur einen kleinen Teil der geplanten Funktionen.
Sie ist eher als Demonstrations,- und Lernobjekt zu sehen, als ein fertiges Produkt.

Features:
+ Stellt den Inhalt des Flashrom als Diskettenlaufwerk 0 zur Verfügung
+ Booten von ROM-Disk
+ Verschiebt die anderen Diskettenlaufwerke
+ keine IDE-Funktionen

---
[Einfach IDE:](ide/)

Firmware zum Ansteuern von IDE-Festplatten ohne Schnickschnack

Features:
+ Schalterstellungen: 0=MFM, 1=Primary IDE, 2=Secundary IDE
+ IDE-Adressmodus: LBA28 (nicht geeignet für sehr alte IDE-Platten)
+ CHS zu LBA Umrechnung mit einstellbaren Parametern
+ Datentransfer mit Z80-DMA in den KES-Buffer

---
[IDE im CHS-Modus:](ide_chs/)

Firmware zum Ansteuern von sehr alten IDE-Festplatten, die nur den CHS-Modus können.
Hinweise:
+ Festplatten, die mit dem A7150 (CM1910) benutzt werden, koennen nur am PC benutzt werden, wenn dort exakt die selben CHA-Parameter eingestellt werden.
+ Da der A7150 auf 17 Sektoren/Track limitiert ist, kann nur ein Bruchteil der Festplattenkapazitaet genutzt werden.

Features:
+ Schalterstellungen: 0=MFM, 1=Primary IDE, 2=Secundary IDE
+ IDE-Adressmodus: CHS
+ Datentransfer mit Z80-DMA in den KES-Buffer
