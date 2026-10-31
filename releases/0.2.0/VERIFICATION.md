# Prüfstand — Version 0.2.0, 31. Oktober 2026

## Bestanden

- Nativer Bau mit GCC, strengen Warnungen und Fehlerbehandlung
  (`-Wall -Wextra -Werror -Wshadow -Wconversion -pedantic`).
- AddressSanitizer und UndefinedBehaviorSanitizer ohne Befunde.
- Deterministische Welt, Startpunkt, Block-IDs, geschütztes Grundgestein,
  Boden-, Wand-, Decken- und Weltgrenzenkollisionen.
- Acht manuelle Zielrichtungen, Vorrang gegenüber Bewegung und automatischer
  Seitenwechsel auch bei blockierter Bewegung.
- Sprung; Sneak mit 14/22 Pixel Höhe, fester Fußposition und Kamera, halbem
  Maximaltempo, niedrigem Gang, verhindertem Aufrichten, beiden Kantenrichtungen,
  Springen über die Kante und Schwerkraft bei fehlender Auflage.
- Stapel verschieben, zusammenführen mit Rest, tauschen, leere Plätze und
  volle Stapel; 5.000 Transaktionen mit Erhaltung aller Materialmengen.
- Reihenfolge beim Aufsammeln, direkte Hotbarwahl, leere Hotbar,
  Platzierungsverbrauch und Schutz vor Platzierung im Körper.
- Exakte Abbauzeiten aller Materialien, unabbaubares Grundgestein,
  Rücksetzung bei Loslassen/Zielwechsel/Materialwechsel, volles Inventar
  und Abbau eines tragenden Bodenblocks.
- Einsekündiger Steinabbau bei simulierten 10, 15, 30, 60 und 120 Bildern/s:
  stets genau 60 Physikschritte.
- 20.000 gemischte Spielschritte mit laufenden Kollisions-, Kamera- und
  Stapelprüfungen.
- Welt und alle 540 Benchmark-Schritte direkt gegen den eingefrorenen
  Spielkern von 0.1.0 verglichen: Blockwelt, Spielerposition/-geschwindigkeit,
  Bodenkontakt und Kamera stimmen überein.
- Pixelgenauer Vergleich der Weltzeichnung mit dem alten Renderer:
  alle Blockarten/Varianten, Transparenzen, Ausschnittsränder und Kamerafahrten.
- Schutzbereiche vor/hinter dem Test-Bildpuffer und Prüfung, dass die
  Weltzeichnung außerhalb des Spielausschnitts nichts verändert.
- HUD-Invalidierung nach Menüseiten sowie Vorschauen von Spiel, Inventar,
  Sneak/Abbau, Hilfe, drei Benchmark-Szenen und Ergebnis visuell geprüft.
  Der Testadapter verwendet eine andere Schrift als das Gerät.
- Echter Programmeinstieg mit simulierten Tasten und Timer: Sprungereignisse
  für Hoch/SHIFT, Sneak ohne Wiederholung bei gehaltenen Tasten und
  nachgeholten Physikschritten, mehrere Tastendrücke, pausierte Inventarnavigation,
  EXE-Auswahl und Abbruch, Rücksetzung eines laufenden Abbaus beim Menüöffnen.
- Sneak-Erhalt durch Inventar/Hilfe, keine nachgeholte Menüzeit und keine
  versehentlichen DEL-/EXE-Aktionen beim Fortsetzen.
- Vollständiger Benchmark mit genau 150 Messbildern je Szene, Erhalt der
  Spielwelt, Timerüberlauf und Rückkehr zum Launcher.
- Syntaxprüfung gegen die echten ClassPad-gint-Header am Quellstand
  `120cffd9b8093c96047f10858c75a2adae62347b`.
- Vollständiger Gerätebau über den Windows-Helfer mit SuperH GCC 14.1.0,
  fxSDK 2.11.0, gint 2.11.0-91 und dem festgehaltenen Container-Digest.
- ELF32 big endian, ausführbares statisches SuperH-Programm; Einstiegspunkt
  `0x8c058428`, Maschinenflags `0x6` (SH4AL-DSP).
- Alle vier Hollyhock-Metadatenfelder vorhanden; Name „Casio Sandbox“,
  Version „0.2.0“. Keine dynamischen Ladesegmente, keine überlappenden oder
  außerhalb der ClassPad-Region liegenden Ladebereiche.
- Unabhängige ELF-Prüfung mit sh-elf-readelf; fertige Quellprüfsummen stimmen
  mit dem Buildbericht überein. Sicherung von 0.1.0 unverändert.

## Datei und Speicher

[build-cp/CasioSandbox.hh3](CasioSandbox.hh3), **113.796 Byte**.

SHA-256:

```text
8e241f9c4066f08326fcf062412e1713ffb3d240e67f5339941e8ee9113384d3
```

| Bereich | Größe |
| --- | ---: |
| Beide Spielzustände | 50.080 Byte |
| RGB565-Atlas | 13.824 Byte |
| Transparenzmasken | 864 Byte |
| Hintergrundkontur | 1.280 Byte |
| Zusätzliche Grafikpuffer gesamt | 15.968 Byte, unter 20 KiB |
| Hauptladebereich belegt | 113.040 von 337.920 Byte |
| Hauptladebereich frei | 224.880 Byte |

Kein weiterer Vollbildpuffer und keine Speicheranforderungen des Projektcodes
während der Spielschleife. Die Hauptladebelegung ist kein vollständiger
Gerätespeicherverbrauch: Stack, System-VRAM und OS-Heap sind darin nicht
vollständig erfasst. Vergleichswert 0.1.0: 92.400 Byte im Hauptladebereich.

Der [Buildbericht](build-info.json) enthält Metadaten, Ladebereiche,
Quellprüfsummen und Werkzeugketten-Digest. Ungestrippte ELF-Datei, Linkerkarte,
ELF-Bericht und Abschnittsgrößen liegen ebenfalls in build-cp.

## Noch auf dem Gerät zu prüfen

Version 0.1.0 wurde vom Benutzer gestartet und mit **14,5 / 11,9 / 11,0 FPS**
gemessen; die Werte stehen in [BENCHMARK.md](../../docs/BENCHMARK.md).
**Version 0.2.0 wurde noch nicht auf dem CP400 ausgeführt.**
Echte Tastenmatrix, Darstellung und Performance müssen dort geprüft werden.
Vorlage: [DEVICE_TEST.md](../../docs/DEVICE_TEST.md).

Die simulierten Zeitwerte testen die Messlogik. Sie liefern keine
Performance-Aussage über die Hardware.

