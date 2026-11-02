# Pruefbericht: Minimine Bench 1.2.0

- HH3: ELF32 Big Endian, SuperH EXEC, Metadatenversion 1.2.0.
- Datei: **238.276 Bytes**.
- SHA-256: `a996ca4eb2226d57b8ee71ea2151a67a9552c83e6e1553fa154c7441c9a3511c`.
- Statischer RAM-Ladebereich: **236.752 / 337.920 Bytes**.
- ILRAM: **592 / 4.096 Bytes**. Azur-Queue einmalig 65.536 Heap-Bytes.
- Referenzrenderer: unveraenderte 0.5.0-Quelle mit SHA-256
  `3a103f4d480964582a63e9cebcc987d4d927c1880e29ca00a888925c7dc0cc10`.
- Die alte und neue Grafikkopie sind nur im Vergleichsprogramm gemeinsam
  enthalten. Das Spiel behaelt **20.370 Bytes** Grafikpuffer.

Erfolgreiche ASan-/UBSan-Pruefungen am PC:

- Alle 16 optionalen Varianten in fuenf Szenen und drei Momentaufnahmen gegen
  die Referenz: native/2x Bildpunkte, ganze Ausgabe und letztes 2x-Fragment.
- 2.056 pixelgleiche Renderer-Vergleiche: Block-/Moboptimierung mit altem Licht
  und neuem Zeilenlicht, alle Mobtypen, Richtungen, Laufbilder, Trefferzustaende,
  Kameraoffsets, Blocktypen, Lichtstufen und Bildschirmraender. Zustandsdaten
  bleiben unveraendert, Bildpuffergrenzen und Grafikbudget werden geprueft.
- Echter Menueablauf: drei Varianten als Standard, eine Wiederholung,
  15 vollstaendige Bloecke, 900 Stichproben an den vorgegebenen Simulationsschritten.
  Beide CPU-Vollbildvarianten sind aus diesem Kurzlauf ausgeschlossen.
- Gesamtlauf einer einzelnen Variante, Abbruch vor/während der Messung,
  gesicherte Teilmessungen, Abschlussstatus und Beenden.
- CSV-Zeiten, FPS/P95, kurze Schreibvorgaenge, Platzmangel und Schliessfehler.
- Saemtliche acht Spieltestsuiten einschliesslich Langzeit-, Inventar-,
  Speicher-, Touch-, Boss- und Programmeinstiegstests bestanden.
- Menuevorschau visuell geprueft: Version 1.2.0, drei Varianten,
  eine Wiederholung und Kurzvergleich als sichtbare Voreinstellung.

Keine neue Spielmechanik, kein zusaetzlicher Vollbildpuffer und keine laufenden
Speicheranforderungen. Das neue Zeilenlicht ist nur mit dem Buildschalter
`MINIMINE_BENCH_LIGHT` enthalten; das normale Spiel verwendet den alten Pfad.

Die Hardwareausgabe ist im Hosttest simuliert. Echte Bedienung, Datei-I/O,
Darstellung und Geschwindigkeit von 1.2.0 muessen auf dem CP400 gemessen werden.
Es wird kein neuer FPS-Gewinn behauptet.

[Buildmetadaten](build-info.json) · [ELF-Bericht](elf-info.txt) ·
[Abschnittsgroessen](section-sizes.txt)
