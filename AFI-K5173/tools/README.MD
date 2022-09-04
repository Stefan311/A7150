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

