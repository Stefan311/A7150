# Hardware-Tests mit der AFI-K5173
Zum Testen der Grundfunktion ist das Programm [AFITEST](../tools/afitest) vorgesehen.

---
### Frontpanel
- LED 1 ein: `o b1 40`
- LED 2 ein: `o b1 80`
- LEDs aus: `o b1 00`
- Schalter lesen: `i b1`

Die Schalterbits sind bei der 1. Platinen-Version seitenverkehrt. Bit 0 = 80, 1 = 40, 2 = 20, 3 = 10

---
### IDE Signale
Am besten ein IDE-Kabel ohne Festplatte anstecken und an einem freien Stecker messen.

Alle Steuerpins Low: `o b1 00`
- PIN1 (-Reset): High `o b1 20`
- PIN38 (-Sel1): High `o b1 10`
- PIN37 (-Sel0): High `o b1 08`
- PIN36 (A2): High `o b1 04`
- PIN33 (A1): High `o b1 02`
- PIN35 (A0): High `o b1 01`

Die folgenden Pins geben nur kurze Impulse aus. Am besten Oszi mit Triggerfunktion oder Logikanalyzer benutzen.
- PIN25 (-RD): Low-Puls: `i b2` und danach `i b0`
- PIN23 (-WR): Low-Puls: 2 mal `o b0 00`

Lesen und Schreiben auf den Port $B0 bedient abwechselnd die Datenleitungen D0-7 und D8-D15 der Festplatte. Der Lese-Impuls an die Platte wird bei den unteren Datenbits ausgelöst, und der Schreib-Impuls bei den oberen Datenbits.
Das Lesen auf Port $B2 setzt den Bit-Umschalter auf low, so dass der darauf folgende Zugriff auf $B0 immer auf die Datenleitungen D0-D7 geht.
- PIN17 (D0): High-Puls: `i b2`, `o b0 01`, `o b0 00`
- PIN15 (D1): High-Puls: `i b2`, `o b0 02`, `o b0 00`
- PIN13 (D2): High-Puls: `i b2`, `o b0 04`, `o b0 00`
- PIN11 (D3): High-Puls: `i b2`, `o b0 08`, `o b0 00`
- PIN9 (D4): High-Puls: `i b2`, `o b0 10`, `o b0 00`
- PIN7 (D5): High-Puls: `i b2`, `o b0 20`, `o b0 00`
- PIN5 (D6): High-Puls: `i b2`, `o b0 40`, `o b0 00`
- PIN3 (D7): High-Puls: `i b2`, `o b0 80`, `o b0 00`
- PIN4 (D8): High-Puls: `i b2`, `o b0 00`, `o b0 01`
- PIN6 (D9): High-Puls: `i b2`, `o b0 00`, `o b0 02`
- PIN8 (D10): High-Puls: `i b2`, `o b0 00`, `o b0 04`
- PIN10 (D11): High-Puls: `i b2`, `o b0 00`, `o b0 08`
- PIN12 (D12): High-Puls: `i b2`, `o b0 00`, `o b0 10`
- PIN14 (D13): High-Puls: `i b2`, `o b0 00`, `o b0 20`
- PIN16 (D14): High-Puls: `i b2`, `o b0 00`, `o b0 40`
- PIN18 (D15): High-Puls: `i b2`, `o b0 00`, `o b0 80`

Zum Testen der Lesefunktion die Datenleitungen auf High legen (am besten mit einem Widerstand ca. 1K mit PIN27 verbinden).
- PIN17 (D0): `i b2`, `i b0` = 01, `i b0` = 00
- PIN15 (D1): `i b2`, `i b0` = 02, `i b0` = 00
- PIN13 (D2): `i b2`, `i b0` = 04, `i b0` = 00
- PIN11 (D3): `i b2`, `i b0` = 08, `i b0` = 00
- PIN9 (D4): `i b2`, `i b0` = 10, `i b0` = 00
- PIN7 (D5): `i b2`, `i b0` = 20, `i b0` = 00
- PIN5 (D6): `i b2`, `i b0` = 40, `i b0` = 00
- PIN3 (D7): `i b2`, `i b0` = 80, `i b0` = 00
- PIN4 (D8): `i b2`, `i b0` = 00, `i b0` = 01
- PIN6 (D9): `i b2`, `i b0` = 00, `i b0` = 02
- PIN8 (D10): `i b2`, `i b0` = 00, `i b0` = 04
- PIN10 (D11): `i b2`, `i b0` = 00, `i b0` = 08
- PIN12 (D12): `i b2`, `i b0` = 00, `i b0` = 10
- PIN14 (D13): `i b2`, `i b0` = 00, `i b0` = 20
- PIN16 (D14): `i b2`, `i b0` = 00, `i b0` = 40
- PIN18 (D15): `i b2`, `i b0` = 00, `i b0` = 80

*Tip 1*: Wenn man weiß ob gerade D0-D7 oder D8-D15 dran sind, dann kann man das erste `i b2` weglassen.

*Tip 2*: Eine leere Eingabe wiederholt den letzten Befehl. Man braucht also nur 2x Enter zum Testen eines Pins zu drücken.

---
### IDE mit angeschlossener Festplatte
Wenn bisher alles geklappt hat, dann kann man eine Festplatte anschließen.
- Festplatte resetten: `o b1 18` und `o b1 38`. Der Reset sollte in der Platte eine kurze hörbare Reaktion auslösen. Bei 2 meiner Platten ist diese allerdings sehr leise. Wenn die Platte nicht richtig gejumpert (Master/Single/Slave) ist, dann kann es bis zu 30 Sekunden dauern ehe die Platte danach wieder reagiert.

Zum weiteren Testen wird das AFITEST in den IDE-Testmodus versetzt: `d`
- Anwahl ob Master oder Slave: `dp` bzw. `ds`.
- Alle IDE-Register lesen: `a`. Es sollte so etwas wie `Err=01 Count=00 SecNr=01 CylLow=00 CylHi=00 Dev=00 Status=50` anzeigen. Besonders wichtig ist hier der Status=50, was bedeutet dass die Platte bereit für Kommandos ist.
- Dann geben wir doch mal ein Kommando: `w 17 ec`. W heißt ein IDE-Register zu beschreiben, 17 ist das Kommandoregister, ec ist der Befehl für "Identifiziere dich!".
- Nun müsste sich das Statusregister geändert haben: `a` --> `Err=01 Count=00 SecNr=01 CylLow=00 CylHi=00 Dev=00 Status=58`. Die Status=58 bedeutet dass die Platte Nutzdaten senden oder empfangen will.
- Dann rufen wir doch mal diese Daten ab: `brd`. Es sollte ein Hexdump erscheinen, in dem man im oberen Bereich die Typbezeichnung der Festplatte sehen kann.

Zum Basis-Testmodus kann man mit `x` zurückkehren

---
### USB-Board
Sollte der Kontroller mit einer USB-Platine bestückt sein, so kann diese auch getestet werden

Als erstes wird das AFITEST in den USB-Testmodus versetzt: `u`

Dann den USB-Kontroller resetten: `c 05`

Echo-Test:
- `c 06`
- `o 01`
- `i` 
Als Antwort müsste nun die Zahl von oben negiert, also 'FE' kommen. Das kann man ruhig mit anderen Zahlen probieren.
- `c 06`  `o 0f'  `i` Ergebnis: 'F0'
- `c 06`  `o f8`  `i` Ergebnis: '07'

Einstellung Kontroller-Modus:
- `c 15`
- `o 06`
06 ist der Modus als USB-Host mit automatischem Erzeugen von SOF Packages. Fragt mich nicht, was das heißt - ich weiß es auch nicht. So steht es im Datenblatt des CH375B.
Nur in diesem Modus lässt sich die Verbindung mit USB-Sticks einfach testen.

Mit `s` können wir den Status abfragen. Solange kein USB-Stick angesteckt ist, sollte das '16(USB_INT_DISCONNECT)' sein.

Stecken wir einen USB-Stick an, dann sollte der Status zu '15(USB_INT_CONNECT)' wechseln.

Mit `c 51`kann nun der USB-Stick am Kontroller angemeldet werden. Der Status sollte dann zu '14(USB_INT_SUCCESS)' wechseln. Der USB-Stick wäre nun theoretisch für Lese,-und Schreibkommandos bereit.

Die Eingabeaufforderung 'Int>' signalisiert, dass der CH375B die Interrupt-Leitung gesetzt hat. Das sollte nach Abfrage des Status wieder weg gehen.

---
### Firmware-Flashrom testen
Wer ein Flashrom mit bekanntem Inhalt hat, der kann dieses zum Testen nutzen. Ich gehe mal davon aus dass ihr für das Flashrom einen IC-Sockel verwendet.
- Byte aus Flashrom lesen (Firmware-Bereich): `r 5000` sollte das erste Byte ausgeben, `r 5001` das zweite usw. 
Wenn der Jumper "Firmware" auf "AUS" steht, dann sollte hier nicht der Inhalt des Flashroms kommen, sondern irgendetwas.
- Byte aus Flashrom lesen (Programmierbereich): `o b2 00` und `r 9000`. Hier sollte wieder das erste Byte des Flashroms kommen. Mit `o b2 01` und `r 9000` sollte das 4096ste byte kommen, mit `o b2 02` und `r 9000` das 8192ste byte, usw.

Zum vollständigen lesen und (eventuell) schreiben des Flashroms kann das Tool [AFIFLASH](../tools/afiflash) benutzt werden.

HINWEIS: Onboard flash funktioniert bisher nur mit dem SST39SF040. Ich habe auch ST28F101, M5M28F101, AT29C010A getestet. Die lassen sich alle auslesen (und demzufolge auch als Firmwarebaustein nutzen), aber NICHT onboard flashen. Das liegt vermutlich daran, dass die Programmier-Sequenzen durch die Übertragung in die KES zu langsam sind. Abhilfe könnte hier geschaffen werden, indem der Flash-Algorithmus direkt in der KES ausgeführt wird. Ich habe vor, das zumindest für die genannten Typen zu programmieren. Bis dahin verwendet bitte einen Chip der SST39F-Serie oder einen exteren Flash-Programmierer.
