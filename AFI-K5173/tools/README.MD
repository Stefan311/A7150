# Kompilieren

Die Tools sind so angelegt, dass die auf dem A7150 selbst, oder auf einem PC in der Dosbox kompiliert werden können.

Dazu wird benötigt:
+ Turbo Pascal 7
+ AS80 Dos-Version (http://www.kingswood-consulting.co.uk/assemblers/index.html)

TP7 und AS80 sollten über die PATH-Umgebungsvariable erreichbar sein. (ggf. in die AUTOEXEC.BAT eintragen)

Jedes Tool kann einzeln mit der jeweiligen BUILD.BAT kompiliert werden, oder alle zusammen mit der BUILDALL.BAT. Zum Kompilieren mancher Tools werden noch BIN2INC und CRCTOOL benötigt. Diese sind deshalb als erstes zu kompilieren und verfügbar zu machen (ggf. ebenfalls in PATH eintragen oder die .EXE in das DOS-Verzeichnis kopieren).

# Tools
### Memorydump (KESDump)
Liest den Speicher des KES-Subsystems aus und speichert ihn in Dateien.

### Firmware-Tool (AFIFlash)
Zum auslesen und Programmieren des Flash-Speichers auf dem AFI-Modul. Dieses Programm basiert auf der Software des legendären CT-Flashers (https://www.heise.de/ct/artikel/Pannenhelfer-286064.html), und kann *theoretisch* eine große Anzahl von Flash-Typen progrmmieren. Welche Flash-Typen in der Praxis funktionieren, muss erst noch getestet werden.

### Hardware-Test (AFITest)
Interaktives Tool zum Ansteuern der AFI-Hardware-Funktionen.

### Z80-Programm-Tool (BIN2INC)
Wandelt Binärdateien, wie sie der AS80 ausspuckt in Pascal-Konstanten-Text um. Auf diese Weise können Z80-Programme direkt in DOS-Pascal-Programme eingebunden werden.

### CRC-Generator (CRCTOOL)
Erzeugt eine CRC16-Prüfsumme von Binärdateien. Bei den KES-Firmware-ROM's sind üblicherweise die letzten 2 Bytes die Prüfsumme, und es wird das Startpolynom $FFFF benutzt. Das CRCTOOL kann auch zum schreiben der CRC in die Binärdatei genutzt werden.

### MWINCH-Korrektion-Tool (FIXWINCH)
Ermöglicht das Einstellen der Festplattengeometrie über die Begrenzungen des MWINCH hinaus. Korregiert außerdem fehlerhafte Einstellungen, so dass auch Festplatten mit unüblichen Geometrien ein GO im ACT erreichen.

### Festplatten-Kopier-Tool (CLONEHDD)
Ermöglicht das kopieren ganzer Festplatten. (MFM-->IDE, IDE-->MFM, IDE-->IDE)
Hinweise dazu:
+ Beim Kopieren von MFM nach IDE-CHS und beim kopieren von IDE nach MFM muss die Quellplatte von den Parametern her in die Zielplatte passen. Sprich: die Zielplatte muss mehr oder gleich viele Köpfe, und mehr oder gleich viel Zylinder haben. Sektoren sind immer 17. Überzählige Zylinder, Köpfe und Sektoren bleiben ungenutzt. Wenn die Zielplatte eine IDE-Platte im LBA-Modus ist, dann brauchen diese Parameter nicht beachtet zu werden.
+ Festplatten, die nicht mit MWINCH eingerichtet worden sind, werden möglicherweise mit falschen Parametern angezeigt, z.B. C:0 H:0 S:17. Als Ziel beim kopieren gehen die trotzdem.
+ Es wird kein Verify gemacht. 
+ Defekte Sektoren werden 1:1 mit kopiert. 
+ Wenn die Zielfestplatte defekte Sektoren hat, dann gehen die Daten, die dort rein kopiert werden sollten, verloren. Anfällig sind hier MFM und sehr alte IDE-Platten. Neuere IDE-Platten benutzen in dem Fall automatisch Reserve-Sektoren.
+ Es ist normal, dass beim Kopieren die Run-LED durchleuchtet. Die Haupt-CPU muss nicht angehalten werden, da die Daten nur über den KES-Speicher gehen.
+ Da die MWINCH-Parameter-Daten mit kopiert werden, braucht die Zielplatte vorher nicht eingerichtet zu werden. Ausnahme ist, wenn die Zielplatte MFM ist, dann kann es sinnvoll sein, diese vorher LOW-Level zu formatieren.
