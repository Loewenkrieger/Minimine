# Pruefstand - Minimine 0.4.1, 1. November 2026

## Neu in 0.4.1

- Ganze Stapel per Quelle/Ziel antippen oder ziehen; Transfer erst beim Loslassen.
- DIV + Touch bewegt die aufgerundete Haelfte. Mengen 1, 17, 32, 33 und 64,
  volle/teilvolle Ziele, verbotene Halbtauschversuche und Haltbarkeit geprueft.
- 10.000 weitere gemischte ganze/halbe Touchtransfers erhalten alle Mengen und
  Haltbarkeiten sowie den reservierten Platz fuer Crafting-Zutaten.
- Ziehen nach ausserhalb, auf Ergebnisfelder und Schaltflaechen sowie Unterbrechen
  durch Tasten/Menuwechsel kann keine Items verlieren oder Vorschauen duplizieren.
- Ruestung/Schliessen antippbar; SHIFT bleibt Ruestungswechsel, DIV teilt Stapel.
- Start, Pause, Hilfe, Ergebnisse, Speicherplaetze, Meldungen und Ja/Nein per Touch.
  Zeichnung und Treffertest verwenden gemeinsame Rechtecke.
- Echte Hauptschleife verarbeitet simulierte Touch-Down/Drag/Up-Ereignisse und
  die gehaltene DIV-Taste. Mehrfaches Loslassen und der Wechsel auf eine neue
  Seite loesen keine zweite Aktion aus. Menuezeiten bleiben pausiert.
- Beenden mit ungespeichertem Spiel erfolgt direkt per Touch und Tastatur.
  Speicher-/Ladebestaetigungen bleiben bestehen. Kein automatisches Speichern.
- Touch-API und Ereigniserzeugung der tatsaechlich eingesetzten gint-Version
  in der vorhandenen Werkzeugkette gelesen; echte CP400-Header erfolgreich gebaut.
- Halbstapel-, Zieh-, Menue- und Rueckfrageansichten als Hostbilder geprueft.

## Bestanden

- Nativer GCC-Bau: strenge Warnungen als Fehler, AddressSanitizer und
  UndefinedBehaviorSanitizer ohne Befund in allen sechs Testprogrammen.
- Bestehende Physik, Kollisionen, Sneak, Kantenbremse, Spruenge und Kameragrenzen;
  20.000 gemischte Spielschritte.
- Angrenzende Zielspalten bei 32 Positionen an Blockgrenzen, sechs seitliche
  Zielhoehen sowie Zielen unter die Fuesse und ueber den Kopf mit/ohne Sneak.
- Alte Welt und alle 540 Benchmark-Schritte direkt gegen den eingefrorenen Kern
  von 0.1.0 verglichen. Benchmark ohne Survival und Kohle; unveraendert
  30 Aufwaerm- und 150 Messbilder je Szene.
- Pixelidentische bisherige Weltgrafik gegen den alten Renderer; geschuetzter
  Bildpuffer, Baumtransparenz, neue Itemgrafiken und Lebensleisten aller vier
  Mobtypen an allen Ausschnittsraendern geprueft. Darstellung aendert keine Mengen.
- Vorschauen von Spiel, allen vier Mobs, Ruestung, Ofen, Crafting, Hotbar und
  Start-/Pause-/Speichermenues erzeugt und visuell geprueft.
- 5.000 Inventar- und 5.000 gemischte Inventar-/Craftingtransaktionen erhalten
  Mengen und Haltbarkeiten. 64er-Grenzen, Einzelstapel fuer dauerhafte Items,
  Quickmove mit Rest, Tausch, ungueltige Ziele und Zutatenrueckgabe geprueft.
- Alle sichtbaren Slots sind ueber Pfeiltasten erreichbar, auch Ruestung und Ofen.
  Cursor/aktive Hotbar bleiben bei X/Y/Z erhalten. SHIFT bewahrt Zutaten.
- Rezepte fuer Bretter, Stoecke, Werkbank, beide Spitzhacken, drei Schwerter,
  Ofen und vier Eisenruestungsteile; falsche und verschobene Muster, voller
  Rucksack, frische Haltbarkeit und atomare Zutaten-/Ergebnistransaktionen.
- Deterministische Kohleverteilung; Hand/Schwert gelten nicht als Spitzhacke.
  Kohle- und Eisenerz-Drops und Abbauzeiten fuer jede Werkzeugstufe geprueft.
  Letzte Werkzeugnutzung liefert noch Material; Abbrueche kosten nichts.
- Naechster einzelner Mob, Reichweite, Waende, Angriffspausen, Schwertbruch,
  Beute und EXE-Menueereignisse. Gleicher Kampfverlauf bei 10/15/30/60/120
  simulierten Bildraten; gehaltener Angriff oeffnet kein Menue.
- Voller Eisenruestungssatz reduziert 2 Schaden auf 0,8 Lebenspunkte;
  zerbrechende Teile schuetzen noch. Schutzzeit, Hunger, Zusatzverbrauch,
  Regeneration, Hunger-Schaden, alle sechs Nahrungsmittel und Respawn mit
  erhaltenem Verschleiss geprueft.
- Mobgrenzen, geschuetzter Startbereich, Hoehlen, Zombie-Sprung, Tierflucht,
  Distanzentfernung sowie Beute bei vollem Inventar, Zusammenfuehren,
  Ablauf und Ersetzung des aeltesten Stapels.
- Ofen: 600 Schritte je Rezept, alle Brennstoffe, voller Ausgang,
  Eingangswechsel, Restbrennzeit, Entnahme und sichere Rueckgabe beim Abbau.
  33. Ofenplatzierung verbraucht kein Item.
- Separater Belastungstest mit anfangs 16 Mobs und 32 aktiven Oefen ueber
  18.000 Schritte. Regelmaessige Zustandspruefung und 30 fertige Barren je Ofen.
- Echter Programmeinstieg mit simulierten Tasten: Sprung-/Sneak-Ereignisse
  werden trotz Nachholschritten einmal verarbeitet. Menuezeit erzeugt keinen
  Nachlauf. Pfeile/SHIFT im Inventar veraendern keine Bewegung. EXE/DEL sind
  nach Pausen gesperrt, bis sie losgelassen wurden.
- Start-/Pausenmenues, Laden, Ueberschreiben/Verwerfen mit standardmaessigem Nein,
  Abbrechen, Werkbankinteraktion, vollstaendiger Benchmark und Beenden.
- Alle drei Speicherplaetze mit maximalen Mob-/Beute-/Ofenpools und verschlissenen
  Gegenstaenden vollstaendig rundgetestet; kanonische erneute Kodierung stimmt
  mit den urspruenglichen Nutzdaten ueberein. Kurzlesen/-schreiben mit sieben
  Byte je Aufruf wird korrekt fortgesetzt.
- A/B-Generationen, teilweises Schreiben und Speichermangel an sieben Positionen,
  defekte neueste Kopie und Wiederherstellung. Die gueltige alte Datei bleibt
  bei Schreibfehlern unveraendert. Zuruecklesen nach Schreiben ist erforderlich.
- Header-/Nutzdaten-/CRC-Fehler, abgeschnittene und zu lange Dateien, unbekannte
  Versionen, ungueltige Items/Stapel/Weltwerte mit gueltiger CRC sowie 300
  weitere veraenderte Dateien geprueft. Laden erfolgt im getrennten Zustand.
- Fehler bei mkdir/open/read/write/close und Zuruecklesen; ausgeglichene
  Dateideskriptoren. Echte POSIX-Dateizugriffe fuer alle drei Slots am Host getestet.
- Vollstaendiger SuperH-Bau mit GCC 14.1.0, fxSDK 2.11.0 und gint 2.11.0-91
  einschliesslich der echten ClassPad-Dateischnittstelle.
- ELF32 big endian SuperH EXEC, Maschinenflags 0x6, Einstiegspunkt
  0x8c05ef9c im ausfuehrbaren Ladesegment. Zwei gepruefte RAM-Ladesegmente.
  Hollyhock-Metadaten Name
  Minimine, Version 0.4.1; keine dynamischen oder ungueltigen Ladesegmente.

## Auslieferung und Speicher

[CasioSandbox.hh3](../releases/0.4.1/CasioSandbox.hh3), **71.780 Byte**.

SHA-256:

```text
7e875a2469f4d1920d9d16f2747c0270137eb8df3b017a8e8bb49f2fcfb4247c
```

| Bereich | Groesse |
| --- | ---: |
| Ein Spielzustand | 29.444 Byte |
| Spiel + Benchmark/Ladezustand | 58.888 Byte |
| RGB565-Tileatlas | 8.704 Byte |
| Tilemasken | 544 Byte |
| Mobgrafiken, vier Typen mit zwei Bildern | 6.144 Byte |
| Mobmasken | 384 Byte |
| Hintergrundkontur | 1.280 Byte |
| Grafikpuffer gesamt | **17.056 Byte**, unter 20 KiB |
| Hauptladebereich belegt | 152.944 von 337.920 Byte |
| Hauptladebereich frei | 184.976 Byte |

Die Groessen der Spielzustaende und Grafikpuffer wurden auch aus den Symbolen
im fertigen Geraete-ELF gelesen. Kein weiterer Vollbildpuffer; keine laufenden
Speicheranforderungen im Projektcode. Speichern verwendet einen 512-Byte-Puffer
und den vorhandenen Benchmarkzustand als Ladepruefzustand. Stack, System-VRAM
und OS-Heap sind nicht vollstaendig in der Hauptladebelegung enthalten.

[Buildbericht](../releases/0.4.1/build-info.json),
[ELF-Bericht](../releases/0.4.1/elf-info.txt) und
[Abschnittsgroessen](../releases/0.4.1/section-sizes.txt) sind beigelegt.
Quellpruefsummen werden mit dem ausgelieferten Stand abgeglichen.
Nicht initialisierte .bss-Daten belegen RAM, werden in dieser ELF-Datei aber
nicht als Nullbytes gespeichert; die Dateigroesse ist daher kleiner als bei 0.4.0.
Ungestrippte ELF-Datei und Linkerkarte entstehen lokal unter build-cp.

## Echte Hardware noch offen

Version 0.4.1 wurde noch nicht auf dem CP400 gestartet. Touch-Kalibrierung und
Bedienung mit gehaltenem DIV, Dateizugriffe ueber Fugue, Haltbarkeit der
Sicherungen bei Stromunterbrechung, Tastenmatrix,
Darstellung und Performance muessen dort noch geprueft werden:
[DEVICE_TEST.md](DEVICE_TEST.md). Die Messungen von 0.1.0 stehen in
[BENCHMARK.md](BENCHMARK.md). Host-Zeitwerte sind keine CP400-FPS.
