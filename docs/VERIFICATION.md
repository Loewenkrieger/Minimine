# Pruefstand - Version 0.3.0, 31. Oktober 2026

## Bestanden

- Nativer GCC-Bau mit strengen Warnungen als Fehler, AddressSanitizer und
  UndefinedBehaviorSanitizer ohne Befunde.
- Bestehende Physik, Sneak, Kollisionen, acht Zielrichtungen und Kameragrenzen.
  20.000 gemischte Spielschritte ohne ungueltige Koerperposition oder Stapel.
- Welt und alle 540 Benchmark-Schritte stimmen mit dem eingefrorenen Kern
  von 0.1.0 ueberein; weiterhin 30 Aufwaerm- plus 150 Messbilder je Szene.
- Pixelgenauer Vergleich der bisherigen Weltgrafik mit dem alten Renderer.
  Neue Stamm-/Werkbankgrafiken an Ausschnittsraendern und geschuetzter
  Bildpuffer; Vorschauen von Hotbar, 2x2-/3x3-Crafting und Werkzeugen visuell geprueft.
- Materialien maximal 64, Werkzeuge einzeln; Stapel verschieben,
  zusammenfuehren, tauschen und einzelne Zutaten verteilen.
- 5.000 bisherige Inventartransaktionen plus 5.000 gemischte Inventar-/
  Craftingtransaktionen: alle Mengen und Werkzeughaltbarkeiten bleiben erhalten.
- Baumstamm zu vier Brettern, zwei Bretter zu vier Stoecken, vier Bretter zur
  Werkbank; verschobene Muster, falsche Muster und versteckte Zusatzfelder.
- Beide Spitzhacken nur im 3x3-Feld, richtige Mengen, frische Haltbarkeit
  und getrennte Werkzeugslots bei wiederholtem Herstellen.
- Ergebnisse werden nur vollstaendig eingefuegt, Zutaten erst bei Erfolg
  verbraucht. Fehlender Platz fuer Ergebnis oder uebrige Zutaten aendert nichts.
  Die Kapazitaetspruefung verhindert ein spaeter nicht schliessbares Craftingmenue.
- Uebrige Zutaten werden beim Schliessen gemeinsam zurueckgegeben; bei
  Platzmangel bleiben Inventar und Zutaten unveraendert.
- Von Hand kein Stein-/Erz-Drop, auch bei vollem Inventar; Werkzeug im Rucksack
  reicht nicht. Aktive Holz-/Steinspitzhacke liefert Stein.
- Holzspitzhacke genau 64, Steinspitzhacke genau 128 erfolgreiche Blockabbaue.
  Letzter Drop wird gutgeschrieben, bevor das Werkzeug zerbricht.
  Abbruch, Grundgestein und fehlender Platz kosten keine Nutzung.
- Steinspitzhacke baut Stein/Erz in 30 statt 60 Physikschritten ab.
  Werkzeug-/Zielwechsel setzen Fortschritt zurueck.
- Platzierbare Werkbank und Baumstaemme: Kollision, Abbau und passende Drops.
- Echter Programmeinstieg mit simulierten Tasten: 2x2-Crafting,
  Werkbank per EXE oeffnen, im 3x3-Feld einzelne Zutaten verteilen,
  Holzspitzhacke herstellen und ohne erneutes Oeffnen durch gehaltenes EXE fortsetzen.
- Menuepfeile steuern nur die Auswahl; Sneak bleibt erhalten. Keine nachgeholte
  Menuezeit, mehrfachen Spruenge oder Sneak-Wiederholungen bei Nachholschritten.
- Vollstaendiger Benchmark, Erhalt der Spielwelt, Timerueberlauf und Beenden.
- Echte ClassPad-gint-Header geprueft; vollstaendiger SuperH-Geraetebau mit
  GCC 14.1.0, fxSDK 2.11.0 und gint 2.11.0-91.
- Statisches ELF32 big endian SuperH EXEC, Maschinenflags 0x6 (SH4AL-DSP),
  Einstiegspunkt 0x8c0590bc in einem ausfuehrbaren Ladesegment.
- Vier Hollyhock-Metadatenfelder vorhanden, Name Casio Sandbox, Version 0.3.0.
  Keine dynamischen, ueberlappenden oder ausserhalb der Geraeteregionen
  liegenden Ladesegmente; unabhaengiger readelf-Bericht vorhanden.

## Auslieferung und Speicher

[CasioSandbox.hh3](../releases/0.3.0/CasioSandbox.hh3), **120.580 Byte**.

SHA-256:

```text
d41aee8da0783dafb2edcbda3c9e475f25409993e80f485100d507d9eac36878
```

| Bereich | Groesse |
| --- | ---: |
| Beide Spielzustaende | 50.240 Byte |
| RGB565-Atlas | 16.896 Byte |
| Transparenzmasken | 1.056 Byte |
| Hintergrundkontur | 1.280 Byte |
| Grafikpuffer gesamt | 19.232 Byte, unter 20 KiB |
| Hauptladebereich belegt | 119.824 von 337.920 Byte |
| Hauptladebereich frei | 218.096 Byte |

Kein weiterer Vollbildpuffer und keine Speicheranforderungen des Projektcodes
in der Spielschleife. Kleine Transaktionskopien liegen auf dem Stack.
Die Hauptladebelegung ist kein vollstaendiger Geraetespeicherverbrauch:
Stack, System-VRAM und OS-Heap sind darin nicht vollstaendig erfasst.

[Buildbericht](../releases/0.3.0/build-info.json),
[ELF-Bericht](../releases/0.3.0/elf-info.txt) und
[Abschnittsgroessen](../releases/0.3.0/section-sizes.txt) sind beigelegt.
Die Quellpruefsummen wurden mit den ausgelieferten Quellen abgeglichen.
Ungestrippte ELF-Datei und Linkerkarte entstehen lokal unter build-cp.

## Geraetetest noch offen

Version 0.3.0 wurde noch nicht auf dem echten CP400 gestartet. Tastenmatrix,
Darstellung und Performance muessen dort geprueft werden:
[DEVICE_TEST.md](DEVICE_TEST.md). Die bisherigen Werte von 0.1.0 stehen
in [BENCHMARK.md](BENCHMARK.md). Simulierte Zeitwerte liefern keine CP400-FPS.

