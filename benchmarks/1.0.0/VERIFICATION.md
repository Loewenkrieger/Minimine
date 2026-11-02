# Pruefbericht: Minimine Bench 1.0.0

Gebaut am 01.11.2026 mit der vorhandenen gepinnten ClassPad-Werkzeugkette.

- HH3: ELF32, Big Endian, SuperH EXEC, Name `Minimine Bench`, Version `1.0.0`.
- Datei: **207.172 Bytes**.
- SHA-256: `34ab5a60150db8cf38251c471f17190be4539c62e2b0f22811be7324ec9a515a`.
- Statischer RAM-Ladebereich: **205.664 / 337.920 Bytes**.
- ILRAM-Ladebereich: **592 / 4.096 Bytes**.
- Azur-Queue: einmalig **65.536 Heap-Bytes**, zusaetzlich zur statischen Belegung.
- Schnelle Datenpuffer: Azur bis 10.240 Bytes; XRAM-/YRAM-Tests jeweils 5.120
  Bytes. Diese Bereiche werden nacheinander verwendet.
- Kein weiterer Vollbildpuffer; der OS-VRAM wird wiederverwendet.

`build-info.json` dokumentiert Quellstand, Abhaengigkeiten, Ladebereiche,
Werkzeugkettenimage und Laufzeitpuffer. Elf- und Abschnittsberichte sowie das
ungestrippte ELF liegen im lokalen Ausgabeordner `build-bench`.

## Erfolgreiche PC-Pruefungen

- Alle zwoelf Render-/Ausgabevarianten in fuenf Szenen, jeweils bei den
  Schritten 0, 89 und 179, gegen denselben Originalrenderer geprueft.
- Pixelgleichheit bei nativer Aufloesung, passende Abtastung bei 2x,
  vollstaendige LCD-Pixelfolge einschliesslich nicht gleichfoermiger
  Randanzeigen und des letzten halben Azur-Fragments.
- Deterministische Spielzustandspruefsummen, gueltige Belastungsszenen und
  16 tatsaechlich aktive Mobs zu Beginn der Mob-Szene.
- AddressSanitizer, UndefinedBehaviorSanitizer und Bildpuffergrenzen.
- CSV-Struktur, Einzelmessungen, FPS und P95 anhand bekannter Messreihen.
- Kurze Schreibvorgaenge, Platzmangel und Schliessfehler; neue Laufnummern
  erhalten fruehere Logs.
- Echte Programmsteuerung mit einem vollstaendigen Fuenf-Szenen-Lauf,
  Abbruch vor den Messbildern und Abbruch mit bereits erfassten Messbildern.
- Menue und Ergebnisansicht als Vorschau gerendert und visuell geprueft.
- Spielquellen, regulaerer Spielbuild und vorhandene Spiel-Releases bleiben
  unveraendert. Der gemeinsame ELF-Pruefer akzeptiert den Benchmarknamen nur
  bei ausdruecklicher Angabe; seine bisherige Voreinstellung bleibt erhalten.

## Noch auf dem CP400 zu pruefen

- Start, Tasten, Dateianlage und Rueckkopieren der vier Log-Dateien.
- Physische LCD-Ausgabe, XRAM/YRAM und ILRAM sowie DMA-Betrieb.
- Tatsaechliche Bildzeiten und Unterschiede zwischen den Varianten.

Die PC-Pruefung simuliert Hardwaretransport und Speicheradressen. Sie belegt
Bildgleichheit und Kontrollablaeufe, keine Geraete-FPS. Eine direkte Ruecklesung
des physischen LCD ist nicht enthalten.
