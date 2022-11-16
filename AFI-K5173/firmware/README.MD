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

Firmware zum Ansteuern von IDE-Festplatten im älteren CHS-Modus.
Hinweise:
+ der CHS-Modus ist nicht mehr Bestandteil der ATA-Spezifikation, allerdings scheinen auch moderne IDE-Platten diesen Modus immer noch zu unterstützen.
+ da die aufwändige CHS-Umrechnung durch die IDE-Platte selber gemacht wird, ist der CHS-Modus etwas schneller als der LBA-Modus.
+ Es sind nur CHS-Parameter Kombinationen erlaubt, die durch die IDE-Interne CHS-Translation erlaubt sind. Welche das sind, hängt von den eingesetzten Festplatten ab. Die Platten, die ich bisher getestet habe, fressen ALLES.
+ Durch die CHS-Translation sind damit Beschriebene Platten relativ gut PC-Kompatibel
+ Platten, die mit Version 0.1 beschrieben worden sind, sind mit 0.2 nicht mehr lesbar.
Features:
+ Schalterstellungen: 0=MFM, 1=Primary IDE, 2=Secundary IDE
+ IDE-Adressmodus: CHS
+ Datentransfer mit Z80-DMA in den KES-Buffer
