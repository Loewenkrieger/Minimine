# Pruefstand - Minimine 0.5.3

## Zeilenlicht nach CP400-Messung aktiviert

- Bench 1.2.0 ist vollstaendig ausgewertet: 15 Bloecke und 900 Messbilder,
  korrekter Quellstand, gleiche Bild-/Zustandshashes und nachgerechnete Zeiten.
  Zeilenlicht erreicht 11,48 statt 9,71 FPS im Lichttest sowie 11,31 statt
  9,59 FPS mit Mobs und Licht. Gewinn gegen 0.5.2: 18,2 beziehungsweise
  17,9 Prozent. [Messwerte und Original-Logs](../benchmarks/1.2.0/DEVICE_RESULTS.md).
- Die beiden gemessenen Funktionen fuer zusammenhaengende Bildzeilen und
  Pixelpaare sind unveraendert ins Spiel uebernommen. Der alte Pfad ist nur
  im Benchmarkbuild verfuegbar; dessen Aufteilung in drei Varianten bleibt erhalten.
- Die genaue 0.5.2-Rendererquelle und die vier Original-Logs sind bytegetreu
  archiviert. Alle bisherigen Spiel- und Benchmark-HH3-Dateien sind unveraendert.
- Alle acht Spieltestsuiten mit ASan/UBSan bestanden: Inventar, Crafting,
  Survival, Speicherfehler, Touch, Langzeitpruefungen, Boss und echter Programmeinstieg.
- 2.056 pixelgleiche Renderer-Vergleiche mit neuem Standard und altem Licht
  gegen die 0.5.0-Referenz. Mobtypen, Richtungen, Animation, Trefferzustaende,
  Blockarten, Lichtstufen, Kameraoffsets, Ausschnittsgrenzen und unveraenderter
  Spielzustand geprueft. Historische Welt und alle 540 Benchmarkschritte stimmen.
- Benchmarktests: 16 Varianten, fuenf Szenen, drei Momentaufnahmen, native/2x
  Bildpunkte und vollstaendige Ausgabe. Kurzlauf mit 15 Bloecken/900 Stichproben,
  Abbrueche und Schreibfehler weiterhin erfolgreich getestet.
- Spielkern und Speicherformat unveraendert. Game **32.196 Bytes**, Grafikcache
  **20.370 Bytes**, kein weiterer Vollbildpuffer und keine laufenden Speicheranforderungen.
- HH3: ELF32 Big Endian SuperH EXEC, Metadatenname Minimine, Version 0.5.3.
  Datei **176.436 Bytes**, RAM-Ladebereich **175.680 / 337.920 Bytes**.
  SHA-256 `bd0de67429f21f9916b14c077416c1bb980615cae6e55257c23a4009221bff56`.
  [Artefakt und Berichte](../releases/0.5.3).

Der gemessene Zeilenlichtpfad ist auf dem CP400 schneller. Die neue Spiel-HH3
selbst ist noch nicht am Geraet geprueft. Fuer den mit rund 9 FPS gemeldeten
Spielstand ist ein direkter Vergleich an derselben Position vorgesehen:
[Geraeteprotokoll](DEVICE_TEST.md). Kein bestimmter Spiel-FPS-Wert wird zugesagt.

Die folgenden Abschnitte dokumentieren die vorherigen Versionen.

## Korrektur nach dem CP400-Lauf von Bench 1.1.0

- Der Geraetelauf zeigt bei 0.5.1 rund 6,7 Prozent mehr FPS an der Oberflaeche,
  aber rund 9,1 beziehungsweise 6,1 Prozent weniger FPS in den Lichttests.
  Die alternative CPU-Vollbildausgabe brachte keinen messbaren Vorteil.
  [Auswertung und Rohdaten](../benchmarks/1.1.0/DEVICE_RESULTS.md).
- 0.5.2 stellt deshalb die alte zeilen-/pixelweise Beleuchtung aus 0.5.0 wieder
  her. Die Atlas-Auswahl und Mob-Blits von 0.5.1 bleiben erhalten. Spielkern,
  Inventar und Speicherformat bleiben gleich.
- Ein neuer Lichtansatz durchlaeuft zusammenhaengende Bildzeilen und verarbeitet
  Pixelpaare pro Blockabschnitt. Er ist ausschliesslich im Benchmark 1.2.0
  enthalten und im Produktionsbuild nicht einkompiliert.
- 2.056 pixelgleiche Vergleiche beider Lichtpfade gegen 0.5.0, einschliesslich
  aller Mobtypen, Richtungen, Laufbilder, Trefferzustaende, Blockarten,
  Lichtstufen, Kameraoffsets und Raender. Darstellung veraendert keine Spielwerte.
- Alle acht Spieltestsuiten mit ASan/UBSan bestanden. Inventar-/Touchtransfers,
  Speicherfehler, 540 historische Benchmarkschritte, Langzeit-, Survival-,
  Boss- und Programmeinstiegstests bleiben erfolgreich.
- Grafikpuffer weiterhin **20.370 Bytes**, Spielzustand **32.196 Bytes**;
  kein weiterer Vollbildpuffer, keine laufenden Speicheranforderungen.
- HH3: ELF32 Big Endian SuperH EXEC, Name Minimine, Version 0.5.2.
  Datei **176.276 Bytes**, RAM-Ladebereich **175.392 / 337.920 Bytes**.
  SHA-256 `2d6df461ff81aa2d703ec0e39386a9e8e903bb40a37b4515aa9360fc4db2bc20`.
  [Artefakt und Berichte](../releases/0.5.2).
- Bench 1.2.0: **238.276 Bytes**, RAM **236.752 / 337.920 Bytes**, ILRAM **592 Bytes**.
  Standard: drei Varianten, fuenf Szenen, eine Wiederholung, 15 Bloecke und
  900 Messbilder. Alle 16 optionalen Varianten, ganze Bildausgabe, Kurzablauf,
  Abbrueche und Logfehler getestet. Menuevorschau visuell kontrolliert.
  [Pruefbericht](../benchmarks/1.2.0/VERIFICATION.md).

Die neuen HH3-Dateien sind gebaut und geprueft; ihre echte Geschwindigkeit,
Darstellung und Datei-I/O auf dem CP400 sind noch offen. Der Geraetetest soll
auch den bekannten Spielstand bei gleicher Position vergleichen. Die Logs
von 1.0.0 und 1.1.0 sowie die bisherigen HH3-Dateien bleiben unveraendert.

Die folgenden Abschnitte dokumentieren die vorherigen Versionen.

## Renderer-Optimierung in 0.5.1

- Unveraenderter Spielkern und Speicherformat; Versionskennung 0.5.1.
- Mobgrafiken: einmaliges Zuschneiden, direkte maskierte Schreibvorgaenge und
  zusammenhaengende Kopien deckender Zeilen. Spiegelung und Trefferblitz bleiben
  pixelgleich. Keine neuen Grafikpuffer.
- Beleuchtung: helle Blockrechtecke auslassen, dunkle Abschnitte paarweise mit
  ausgerichteten 32-Bit-Zugriffen abdunkeln. Exakt dieselben RGB565-Werte.
- Atlas- und Maskenauswahl wird pro Block vor der Zeilenschleife berechnet.
- 1.028 neue Bildvergleiche gegen den bytegetreu gesicherten 0.5.0-Renderer,
  einschliesslich aller Mobtypen, Blickrichtungen, Laufbilder, Trefferzustaende,
  abgeschnittener Raender, Lichtstufen und aller Kamera-Unterblockpositionen.
- Vollstaendige bisherige Spieltests und neue Renderer-Pruefung mit ASan/UBSan.
  Der Grafikcache bleibt bei 20.370 Bytes, ein Spielzustand bei 32.196 Bytes.
- Datei **176.372 Bytes**, RAM-Ladebereich **175.616 / 337.920 Bytes**.
  SHA-256 `c1e5788f9299d9f553b7b3c41f97c1b62d5030839c2491934e9292738dc660cb`.
  Artefakt und Berichte: [releases/0.5.1](../releases/0.5.1).
- Ausgabe im Spiel weiterhin ueber gint. Die alternative zusammenhaengende
  CPU-Ausgabe wird in Minimine Bench 1.1.0 separat gegen beide Renderer getestet.
- Der kurze Vergleich ist voreingestellt: vier Varianten, fuenf Szenen, eine
  Wiederholung, 30 Aufwaerm- plus 60 verteilte Messbilder pro Block, 20 Bloecke.
  Neue Gesamtfortschrittsanzeige; Dateinummer und Wiederholung sind getrennt.

Die damalige Geraetemessung wurde mit Bench 1.1.0 nachgeholt und ist oben
verlinkt. Der Ausgangslauf von Bench 1.0.0
ist unter [benchmarks/1.0.0/device-run0001](../benchmarks/1.0.0/device-run0001)
gesichert und in [DEVICE_RESULTS.md](../benchmarks/1.0.0/DEVICE_RESULTS.md)
ausgewertet. Abhaengig von diesen Messungen wurde gezielt der bisherige
Renderer optimiert. Der Beleuchtungsversuch wurde wegen des anschliessend
gemessenen Rueckschritts in 0.5.2 zurueckgenommen.

Die folgenden Abschnitte dokumentieren den bisherigen Pruefstand von 0.5.0.

## Neu in 0.5.0

- Neue Weltgenerierung mit Basaltschicht, Kristallerz, fuenf Minenraeumen,
  drei eindeutigen Relikten, sieben persistenten Kisten und versiegelter Arena.
  Gleicher Seed erzeugt gleiche Welten; mehrere Seeds vollstaendig validiert.
- Kisten mit zwoelf Slots, ganze/halbe Touchtransfers, Quickselect und atomarer
  Rueckgabe des gesamten Inhalts beim Abbau. Geteilte Relikte ueber Crafting
  werden ebenfalls fuer den Fortschritt erfasst.
- Paare aus Tuerhaelften und Bettteilen, Platzierung mit Bodenpruefung,
  Kollisionsschutz beim Schliessen, Bett-Rueckkehrpunkt und blockierter Rueckweg.
- Fackeln, Kiste, Tuer, Bett, Eisenspitzhacke, Stiefel der Eile und nach dem Sieg
  freigeschaltetes Sprungamulett als Rezepte. Fackeln im 2x2-Feld herstellbar.
- Eisenspitzhacke mit 256 Nutzungen und neuen Abbauzeiten; Basalt und Kristallerz
  bleiben mit falschem Werkzeug bestehen. Kammersaeume sind geschuetzt.
- Zwei typisierte Zubehoerplaetze, doppelte Ausruestung abgewiesen. Jeder fuenfte
  Abbau ohne Werkzeugverschleiss, 15 Prozent Laufbonus mit Sneak und genau ein
  weiterer Sprung pro Luftphase geprueft.
- Drei Helligkeitsstufen, Radius von platzierten Fackeln sowie Verhinderung neuer
  feindlicher Spawns im Licht. Handfackel und Naehelicht als getrennte Quellen.
- Fledermaus mit Warten, Vorwarnung, geradem Flug und Erholung; Zombievorwarnung,
  Trefferblitz und Rueckstoss. Gegner bleiben im begrenzten vorhandenen Pool.
- Vollstaendiger Questablauf: drei Relikte aus Kisten, dauerhaftes Oeffnen des
  Siegels, Bossstart, Vorwarnung/Ansturm/Bodenwelle, Ausweichen, Tod/Neustart,
  17 erfolgreiche Eisenschwerttreffer bis zum Sieg und einmalige Belohnung.
- Laufender Boss und Siegstatus erfolgreich gespeichert/zurueckgeladen.
  Volle Belohnungskiste verliert nichts; Belohnung wird nach Freigabe zugestellt.
- Gemeinsamer Maximalzustand mit 16 Kisten, 64 Fackeln, 32 Oefen, 16 Mobs und
  64 Beutestapeln gespeichert, geladen und simuliert. Zusaetzlich 18.000 Schritte
  in der neuen Welt mit wiederholter vollstaendiger Zustandspruefung.
- Format 2 speichert Kisten, Fackeln, Bett, Zubehoer, Gegnerphasen, Relikte,
  Bossphasen und vorgemerkte Belohnung. Neue Werte und Objektverknuepfungen
  werden vor dem Austausch des laufenden Spiels geprueft.
- Echte Format-1-Testdatei mit historischem 0.4.2-Kern erzeugt, geladen und ins
  neue Format ueberfuehrt; vorherige A/B-Datei bleibt unveraendert. Alte Welten
  werden nicht nachtraeglich umgebaut. Fixture ist synthetisch und liegt in tests/fixtures.
- Vorschauen von dunkler Mine, Kiste, sechs Ausruestungsslots, Basis, Basalt,
  sichtbarem Steinwaechter und Questfortschritt visuell geprueft.

## Bisherige Funktionen einschliesslich Hochbaufix

- Regressionstest schlaegt vor der Korrektur fehl: Bei nicht blockbuendiger
  Fusshoehe lag das Ziel beim Zielen mit 2 in einer vom Koerper belegten Reihe.
- Direkt nach unten wird jetzt in Q8 auf die erste vollstaendige Blockreihe
  unter den Fuessen aufgerundet. Die seitlichen Fusshoehenziele bleiben erhalten.
- Alle 4.097 Subpixelpositionen entlang einer Blockhoehe mit normaler und
  geduckter Figur geprueft: 8.194 Positionen ohne Koerperueberlappung des Ziels.
- Drei aufeinanderfolgende Hochbauspruenge mit kurzem und gehaltenem EXE,
  stehend/geduckt bei 10/15/30/60/120 simulierten Bildraten. Vollstaendige
  Endzustaende stimmen je Variante ueberein, Materialverbrauch entspricht den
  gesetzten Bloecken und die Figur landet jeweils hoeher und ausserhalb von Bloecken.
- Belegter Boden, leere Hotbar und Platzierungen innerhalb des Koerpers bleiben
  gesperrt. Die echte Eingabebelegung fuer gehaltenes 2 + EXE ist geprueft.
- Vollstaendiger bisheriger Testlauf inklusive Speicher-, Touch-, Langzeit- und
  Benchmarktests bestanden. Vorschauen von Platzierung und Landung erzeugt.

## Touchbedienung seit 0.4.1

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
  UndefinedBehaviorSanitizer ohne Befund in allen sieben Testprogrammen.
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
  0x8c05efa4 im ausfuehrbaren Ladesegment. Ein geprueftes RAM-Ladesegment.
  Hollyhock-Metadaten Name
  Minimine, Version 0.5.0; keine dynamischen oder ungueltigen Ladesegmente.

## Auslieferung und Speicher

[CasioSandbox.hh3](../releases/0.5.0/CasioSandbox.hh3), **175.924 Byte**.

SHA-256:

```text
508d642d8334f690313ec2ecdcdee63da331a26c8718b8d23ec81c406d0ae2c0
```

| Bereich | Groesse |
| --- | ---: |
| Ein Spielzustand | 32.196 Byte |
| Spiel + Benchmark/Ladezustand | 64.392 Byte |
| RGB565-Tileatlas | 11.264 Byte |
| Tilemasken | 704 Byte |
| Mobgrafiken, vier Typen mit zwei Bildern | 6.144 Byte |
| Mobmasken | 384 Byte |
| Hintergrundkontur | 1.280 Byte |
| Lichtfeld fuer sichtbare Bloecke | 594 Byte |
| Grafikpuffer gesamt | **20.370 Byte**, unter 20 KiB |
| Hauptladebereich belegt | 175.168 von 337.920 Byte |
| Hauptladebereich frei | 162.752 Byte |

Die Groessen der Spielzustaende und Grafikpuffer wurden auch aus den Symbolen
im fertigen Geraete-ELF gelesen. Kein weiterer Vollbildpuffer; keine laufenden
Speicheranforderungen im Projektcode. Speichern verwendet einen 512-Byte-Puffer
und den vorhandenen Benchmarkzustand als Ladepruefzustand. Stack, System-VRAM
und OS-Heap sind nicht vollstaendig in der Hauptladebelegung enthalten.

[Buildbericht](../releases/0.5.0/build-info.json),
[ELF-Bericht](../releases/0.5.0/elf-info.txt) und
[Abschnittsgroessen](../releases/0.5.0/section-sizes.txt) sind beigelegt.
Quellpruefsummen werden mit dem ausgelieferten Stand abgeglichen.
Ungestrippte ELF-Datei und Linkerkarte entstehen lokal unter build-cp.

## Echte Hardware noch offen

Version 0.5.0 wurde noch nicht auf dem CP400 gestartet. Beleuchtungsleistung,
Gegner-/Bossbalancing, Touch-Kalibrierung und
Bedienung mit gehaltenem DIV, Dateizugriffe ueber Fugue, Haltbarkeit der
Sicherungen bei Stromunterbrechung, Tastenmatrix,
Darstellung und Performance muessen dort noch geprueft werden:
[DEVICE_TEST.md](DEVICE_TEST.md). Die Messungen von 0.1.0 stehen in
[BENCHMARK.md](BENCHMARK.md). Host-Zeitwerte sind keine CP400-FPS.
