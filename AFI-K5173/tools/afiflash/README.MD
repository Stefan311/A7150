# AFIFLASH
Mit diesem Programm kann die Firmware in den Flash-Chip des IDE-Kontrollers übertragen (geflasht) werden.

Während des Flash-Vorgangs sind natürlich IDE-Festplatte und USB-Stick nicht verfügbar. Deshalb muss das neue Firmware-Image auf Diskette, MFM-Festplatte oder RAM-Disk vorliegen.
Nach dem Flash-Vorgang wird die neue Firmware erst nach einem Reboot wirksam.

Einige Flash-IC's brauchen +12V wärend des Flash-Vorgangs. Für diese IC's muss der Jumper JP1 auf +12V stecken. Für alle anderen muss der Jumper JP1 auf A18 stecken!

Als Vorlage für AFIFLASH diente die Software des CT'Flashers. Zusätzlich zu den Chip-Typen den CT-Flashers kann AFIFLASH auch noch den PH39SF040 und M5M28F101.

### Kommandozeilenparameter
|afiflash|R/W|FILE|[/M]|[/N /E]|   |
|--------|---|----|----|-------|---|
|        |R/W|    |    |       |R: auslesen, W: schreiben|
|        |   |FILE|    |       |kompletter Pfad zur Binärdatei|
|        |   |    |/M  |       |Optional: manuelle Auswahl des Bausteintyps|
|        |   |    |    |/N /E  |Optional: Nederlands oder English (Alternativ zu Deutsch)|


### Unterstützte Chips
|Hersteller|IC-Typ|
|----------|------|
|AMD       |Am28F010(A)/20(A), Am29F010/02/40(=A29040)|
|Atmel         |AT29C512/010A/20(=A290021)/40A, AT49F001/02/40|
|Catalyst (CSI)|CAT28F001Bx-T/B, CAT28F010/20|
|Hyundai       |HY29F002T/B|
|Intel         |i28F001Bx-T/B, i28F010(Z28F010)/20|
|Macronix      |MX28F1000P (=H.T.M21xxx), MX28F2000P/T, MX29F002(N)T/B/040|
|Mosel-Vitelic |F/V29C51002T/B (Boot Block unprotected!)|
|SST           |PH29EE010/20, PH28SF040, PH39SF010/20/40|
|ST Microelec. |M28F256(A)/512, M28F101, M29F002T/NT/B/040|
|Winbond       |W29EE011 (=AE29F1008), W29C010/11/20(=AE29F2008)/40, W49F002|
»Mitsubishi    |M5M28F101|


### Getestete, funktionierende Chips
|Hersteller|IC-Typ|
|----------|------|
|Atmel     |AT29C010A|
|Macronix  |MX28F1000P|
|SST       |PH39SF040|
|ST Microelec.|M28F101|
|Mitsubishi|M5M28F101|

