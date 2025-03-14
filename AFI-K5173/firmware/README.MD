# Firmware-Versionen

---
[Rom-Floppy:](rom_disk/)

Diese Firmware-Version implementiert nur einen kleinen Teil der geplanten Funktionen.
Sie ist eher als Demonstrations,- und Lernobjekt zu sehen, als ein fertiges Produkt.

Features:
+ Stellt den Inhalt des Flashrom als Diskettenlaufwerk 0 zur Verf�gung
+ Booten von ROM-Disk
+ Verschiebt die anderen Diskettenlaufwerke
+ keine IDE-Funktionen

---
[Einfach IDE:](ide/)

Firmware zum Ansteuern von IDE-Festplatten ohne Schnickschnack

Features:
+ Schalterstellungen: 0=MFM, 1=Primary IDE, 2=Secundary IDE
+ IDE-Adressmodus: LBA28 (nicht geeignet f�r sehr alte IDE-Platten)
+ CHS zu LBA Umrechnung mit einstellbaren Parametern
+ Datentransfer mit Z80-DMA in den KES-Buffer

---
[IDE im CHS-Modus:](ide_chs/)

Firmware zum Ansteuern von IDE-Festplatten im CHS-Modus. F�r die meisten Anwendungen empfohlen.

Hinweise:
+ der CHS-Modus ist nicht mehr Bestandteil der ATA-Spezifikation, allerdings scheinen auch moderne IDE-Platten diesen Modus immer noch zu unterst�tzen.
+ da die aufw�ndige CHS-Umrechnung durch die IDE-Platte selber gemacht wird, ist der CHS-Modus etwas schneller als der LBA-Modus.
+ Wird eine CHS-Parametrierung gew�hlt, die die Festplatte nicht unterst�tzt, dann wird automatisch zum LBA-Modus gewechselt. Die meisten Platten unterst�tzen jedoch alle CHS-Kombinationen.
+ Durch die CHS-Translation sind damit Beschriebene Platten relativ gut PC-Kompatibel
+ Platten, die mit Version 0.1 beschrieben worden sind, sind ab 0.2 nicht mehr lesbar.

Features:
+ Schalterstellungen: 0=MFM, 1=Primary IDE, 2=Secundary IDE
+ IDE-Adressmodus: CHS
+ Datentransfer mit Z80-DMA direkt in den Hostspeicher (~380KB/s)
+ Kanal B-Support f�r die CH375 USB-Platine
+ MFM-Mover-Option: Paralell zur IDE-Festplatte werden auch die K�pfe der MFM-Festplatte bewegt, es werden aber auf MFM keine Daten gelesen oder geschrieben. F�r Leute, die den Sound von MFM-Festpalten m�gen. Diese Option muss im Quelltext aktiviert, und neu compiliert werden.

---
[Iomega ZIP100 Atapi:](atapi/)

Spezial-Firmware fuer ATAPI-Diskettenlaufwerke Iomega ZIP100 (Atapi)

Hinweise:
+ NICHT f�r IDE-Festplatten
+ NICHT f�r CD-ROM-Laufwerke
+ Es gibt 2 Varianten ZIP100 mit IDE-Schnittstelle: "IDE" und "ATAPI". Unterst�tzt wird nur ATAPI.
+ bisher nur mit Iomega ZIP100 Atapi getestet
+ Die ZIP-Disketten k�nnen im laufenden Betrieb ausgeworfen und wieder eingelegt werden. ABER: DCP/DOS erkennt das nicht. Diskettenwechsel w�rend des Betriebs k�nnen deshalb zu Problemen f�hren.

Features:
+ Schalterstellungen: 0=MFM, 1=Primary ATAPI, 2=Secundary ATAPI
+ Datentransfer mit Z80-DMA in den KES-Buffer
+ Kanal B-Support f�r die CH375 USB-Platine