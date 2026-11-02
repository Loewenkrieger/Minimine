# Renderer-Vergleich: Minimine Bench

## Version 1.2.0: Block-/Moboptimierung und Zeilenlicht trennen

Standard ist ein einziger Durchlauf mit **15 Testbloecken**: drei Varianten
in denselben fuenf Szenen. Pro Block werden 30 Aufwaermbilder und 60 gleichmaessig
verteilte Messbilder gezeichnet. Insgesamt sind es 900 Messbilder statt der
27.000 des alten Standardlaufs. Alle 180 Simulationsschritte pro Szene werden
weiter ausgefuehrt; nur die Bildschirmzeichnung wird bei nicht ausgewaehlten
Schritten ausgelassen. Gemessen werden die Schritte
`30 + sample * 149 / 59`, mit ganzzahliger Division und `sample` von 0 bis 59.

| Variante | Renderer | Vollbildausgabe |
| --- | --- | --- |
| reference_1x | Gesicherter Originalrenderer 0.5.0 | gint dupdate |
| optimized_1x | Block-/Moboptimierungen, alte Beleuchtung (0.5.2) | gint dupdate |
| scanline_light_1x | Dieselben Block-/Moboptimierungen, neues Zeilenlicht | gint dupdate |

Die langsamere blockweise Beleuchtung von 0.5.1 ist zurueckgenommen.
Der neue Ansatz laeuft zeilenweise durch den Bildpuffer, liest die Lichtstufe
nur pro Blockabschnitt und dunkelt ausgerichtete Pixelpaare ab. Das vermeidet
Spruenge zwischen entfernten Bildzeilen. Beim Bau von 1.2.0 war er ausschliesslich
im Benchmark enthalten; Spiel 0.5.3 verwendet ihn nach der Geraetemessung als
Standard. Der bisherige Pfad bleibt fuer den Vergleich per Buildschalter erhalten. Beide
Varianten nutzen dasselbe Lichtfeld, dieselben Masken und denselben Bildpuffer;
sie erzeugen dieselben Pixel. Der Geraetelauf mit 900 Messbildern zeigt rund
18 Prozent mehr FPS in den beiden beleuchteten Szenen gegenueber 0.5.2.
[Auswertung und Rohdaten](../benchmarks/1.2.0/DEVICE_RESULTS.md). Die genaue
0.5.2-Rendererquelle ist unter `benchmarks/1.2.0/render-reference.c` gesichert.

Der Originalrenderer ist bytegetreu unter `benchmarks/1.0.0/render-reference.c`
gesichert (SHA-256 `3a103f4d480964582a63e9cebcc987d4d927c1880e29ca00a888925c7dc0cc10`).
Er wird im selben Programm wie der neue Renderer gebaut. Der Grafikcache ist
dadurch im Vergleichsprogramm zweimal enthalten, im normalen Spiel einmal.
Die urspruenglichen zwoelf Experimente und die beiden CPU-Vollbildvarianten
bleiben ueber "Alle Varianten (lang)" erreichbar; insgesamt sind es 16 Varianten.
`optimized_1x` bezeichnet ab 1.2.0 den zurueckgenommenen Lichtpfad; CSV-ID allein
identifiziert daher keinen Quellstand. Version und Quellpruefsumme mitlesen.

`TEST n/15` und Prozent zeigen den Gesamtfortschritt, `LOG n` nur die
Dateinummer. Nach Abschluss erscheint die Ergebnisansicht, ohne automatisch
eine weitere Wiederholung zu starten. Eine Wiederholung ist voreingestellt.
In `frames.csv` identifiziert die neue Spalte `simulation_frame` den echten
Simulationsschritt; fuer den Vergleich mit 1.0.0 gilt dort `simulation_frame =
frame + 30`. `frame` bleibt der fortlaufende Index innerhalb der Messreihe.
Metadaten nennen 60 statt 150 Messbilder, sobald der Kurzvergleich gewaehlt ist.

Das kurze Menue und die 15 Bloecke, die 900 Stichproben, die Abtastpositionen
und die Abbruchfaelle werden zusaetzlich automatisiert geprueft. Die Bildpruefung
vergleicht neue Varianten ebenfalls direkt mit dem gesicherten 0.5.0-Renderer.
Der erste Geraetelauf von 1.0.0 ist unter
`benchmarks/1.0.0/device-run0001` erhalten. Seine 60 abgeschlossenen Erstlauf-
Bloecke enthalten keine gemeldeten Bild- oder Zustandshash-Abweichungen.

## Archiv: Ergebnis von Version 1.1.0

1.1.0 verglich 0.5.0 und 0.5.1 mit gint sowie alternativer Vollbild-CPU-Ausgabe:
20 Bloecke, 1.200 Messbilder. Der vollstaendige Geraetelauf ist unter
`benchmarks/1.1.0/device-run0001` erhalten. Beleuchtete Szenen wurden mit 0.5.1
langsamer; die Vollbild-CPU-Ausgabe blieb bei rund 20,95 ms. Deshalb enthaelt
der neue Kurzvergleich nur noch die drei oben genannten Varianten.
[Auswertung](../benchmarks/1.1.0/DEVICE_RESULTS.md). Die genaue 0.5.1-Rendererquelle
ist unter `benchmarks/1.1.0/render-reference.c` gesichert (SHA-256
`61671d553bd5028265ced43eea0b173c599434713447da3f961471846e4a7331`).

## Archiv: Messverfahren von Version 1.0.0

Bedienung und Log-Uebergabe: [bench/README.md](../bench/README.md).
Das Programm ist eine eigene HH3-Anwendung. Kein Spielstandzugriff, keine
Gameplay-Aenderung und keine Umstellung des normalen Spiels auf einen neuen
Renderer. Der bisherige Spielbenchmark bleibt erhalten.

## Varianten

| CSV-ID | Umsetzung |
| --- | --- |
| reference_1x | Originales `render_world()` aus Minimine 0.5.0 und gint `dupdate()` |
| packed_2x | Welt direkt mit 160x200 Abtastpunkten im vorhandenen VRAM zeichnen; Pixel/Zeilen bei der Ausgabe verdoppeln |
| chunks_ram_1x | Gemeinsame Szenenbefehle in Bloecke von acht Zeilen zeichnen und sofort per CPU ausgeben |
| chunks_xram_1x | Identische Bloecke, Ausgabepuffer in XRAM bei 0xe5007020 |
| chunks_yram_1x | Identische Bloecke, Ausgabepuffer in YRAM bei 0xe5017020 |
| chunks_ilram_1x | RAM-Zeilenpuffer; derselbe Zeilen-Blitkernel aus ILRAM statt RAM |
| chunks_xram_ilram_1x | XRAM-Zeilenpuffer und ILRAM-Blitkernel kombinieren |
| dma_full_2b_1x | Originalrenderer, anschliessend kompletter VRAM per 2-Byte-DMA |
| dma_full_32b_1x | Originalrenderer, anschliessend kompletter VRAM per 32-Byte-DMA |
| dma_chunks_2b_1x | Zeilenbloecke in normalem RAM, jeweils per 2-Byte-DMA ausgeben |
| azur_1x | Echte Azur-Befehlssortierung und Fragmentausgabe; eigener Shader fuer die gemeinsamen Minimine-Zeichenbefehle |
| azur_2x | Dieselbe Azur-Pipeline mit Skalierungsfaktor zwei |

Das sind gezielte Prototypen, keine vollstaendige Produktumstellung. XRAM/YRAM
testen den **Zeilenpuffer**, nicht einen verlegten Stack. ILRAM testet den
**Zeilen-Blitkernel**, nicht die gesamte Spiellogik. DMA verwendet ausschliesslich
normale RAM-Quellen, Kanal 0, Cache-Writeback und vollstaendiges Warten auf den
Abschluss. Es gibt keine Ueberlappung zwischen DMA und Zeichnen; diese koennte
spaeter separat untersucht werden. Ein nicht abgeschlossener Transfer wird
nach 500 ms abgebrochen und als Renderfehler protokolliert.

Die unveraenderten Spielgrafiken werden direkt aus `src/render.c` eingebunden.
Der alternative Renderer verwendet dessen Atlas, Masken und Figurenfunktionen.
Er zeichnet keine bereits fertig gerenderten Weltbilder ab. Die Szenenbefehle
werden in jedem Bild neu aufgebaut, ihre Kosten sind in den Messwerten enthalten.

## Szenen und Vergleichbarkeit

Die ersten drei Szenen verwenden die historische Benchmarkwelt mit Oberflaeche,
Hoehlen und Tiefengestein sowie die bisherigen 180 Kamera-/Simulationsschritte.
Survival und Beleuchtung sind dort deaktiviert. Die beiden weiteren Szenen
verwenden eine feste Hoehlenkammer mit neun Fackeln; die Belastungsszene startet
zusaetzlich mit 16 verletzten Mobs, deren KI und Lebensleisten aktiv sind.
Die Tiere und Gegner werden fuer diesen Test gezielt eingesetzt, unabhaengig
von den natuerlichen Spawnregeln. Spieler-Schutz verhindert einen Respawn
waehrend der drei simulierten Sekunden.

Jeder Block beginnt mit demselben Seed und Spielzustand. Ein Bild entspricht
einem Physikschritt von 1/60 Sekunde, unabhaengig von den tatsaechlichen FPS.
Nach 30 Aufwaermbildern werden 150 Bilder aufgezeichnet. Der Gesamtlauf umfasst
12 Varianten x 5 Szenen x 3 Wiederholungen, also 27.000 Messbilder. Die
Variantenreihenfolge rotiert zwischen den Wiederholungen. Eine einzelne Variante
und ein Durchlauf sind ebenfalls waehlbar.

Alle Varianten uebertragen das ganze 320x528-Bild. Die Welt behaelt dieselbe
sichtbare Flaeche. Beide 2x-Varianten verwenden dieselbe Abtastung (oberer linker
Pixel jedes 2x2-Quadrats); auch die statischen Randanzeigen werden waehrend
dieser Messungen entsprechend skaliert. Die Auswahl- und Ergebnisansichten
bleiben nativ. 1x und 2x werden im Log ausdruecklich unterschieden.

Vor jedem Block werden drei Bildzustaende (Schritte 0, 89, 179) gegen den
Originalrenderer geprueft. Bei 2x wird mit der entsprechenden Abtastung
verglichen. Ein Fehler verhindert die Zeitmessung fuer diesen Block; andere
Varianten laufen weiter. Die Bildpruefung kontrolliert RAM-Ausgaben und bei
Azur die echten Fragmente vor der Uebertragung. Sie liest das physische LCD
**nicht** zurueck; falsche Hardwareausgabe muss beim Geraetetest gemeldet werden.

Diese Messung zeichnet nur die Spielwelt neu; die Test-Randanzeigen sind pro
Block statisch. Die Ergebnisse sind daher ein neuer Variantenvergleich und
nicht unmittelbar dieselben FPS wie der alte Benchmark oder normales Spielen.

## Zeitmessung und Logs

Gints Profiler liefert Mikrosekunden. `total_us` ist Logikzeit plus gesamte
Render-/Ausgabezeit; Eingabeabfrage, Zustandspruefsummen, Bildpruefungen und
Dateizugriffe liegen ausserhalb. `lcd_us` enthaelt die gemessenen Ausgabeabschnitte,
einschliesslich DMA-Vorbereitung und Warten. `draw_us` ist die verbleibende
Renderzeit; bei Azur umfasst sie auch Befehlsaufbau, Sortierung und Verwaltung.
Azur-Sortier- und Shaderzeiten werden zusaetzlich einzeln aufgezeichnet. Sie
sind Teilmengen, keine weiteren Summanden zur Bildzeit.

FPS werden aus Anzahl geteilt durch gesamte gemessene Zeit berechnet. Die
CSV-Spalte `fps_x100` ist FPS mal 100. P95 ist die aufgerundete 95%-Position
der sortierten Bildzeiten. Jede Zeile in `frames.csv` hat einen stabilen
Zustandshash ohne C-Padding; gleiche Szene und Bildnummer muessen ueber alle
Varianten denselben Hash liefern.

Dateien werden ausschliesslich zwischen Bloecken geschrieben und jeweils
geschlossen. Vor einem Test wird `block_started` gesichert, danach folgen die
Einzelmessungen, Zusammenfassung und Abschlussstatus. CLEAR waehrend einer
laufenden Anwendung sichert auch eine Teilmessung. Bei Stromverlust oder
Neustart bleiben abgeschlossene Bloecke erhalten; der letzte ungeschriebene
Block kann nicht rekonstruiert werden. Schreib- und Schliessfehler stoppen den
Lauf. `O_EXCL` reserviert neue Laufnummern ohne alte Daten zu ueberschreiben.

Die Metadaten dokumentieren den Quellstand, Azur-Pin, Werkzeugkettenimage,
gemeldete CPU-/Bus-/Peripherietakte und gemeinsame Speicherbelegung. Es wird
kein Takt eingestellt. OS- und Hollyhock-Version bitte bei Bedarf ergaenzen.

`working_ram_buffers` und `onchip_data_buffers` beschreiben die fuer den jeweiligen
Prototyp benoetigten Pufferkapazitaeten. Alle Varianten befinden sich im selben
Programm und teilen Arbeitsbereiche, daher sind diese Werte **keine** zusaetzlich
zur gesamten statischen RAM-Belegung zu addierenden Zahlen. Die Azur-Queue
reserviert einmalig 65.536 Heap-Bytes; innerhalb der Bildschleife wird kein
Speicher angefordert. Der VRAM des Betriebssystems wird wiederverwendet, es
gibt keinen weiteren Vollbildpuffer. Diese Benchmark-Prototypen sind noch nicht
auf das 20-KiB-Grafikbudget des normalen Spiels reduziert.

## Abhaengigkeiten und Pruefung

Azur stammt aus [TheRainbowPhoenix/Azur](https://github.com/TheRainbowPhoenix/Azur),
Commit `70e92c47f5d35c6d6b1a0dde73c8346966b8d7a6`, auf Basis von Lephe's Azur.
Die verwendeten Quellen tragen den MIT-Hinweis: "Designed by Lephe' and the
Planete Casio community"; [MIT-Bedingungen](https://opensource.org/licenses/MIT).
Nur die C-Renderpipeline und die CP-LCD-Ausgaberoutinen werden gebaut, keine
C++-Engine oder fremden Grafikassets. Download und temporaere Anpassungen sind
vom normalen Spielbuild getrennt. Das Containerimage bleibt unveraendert.

`tools/prepare_bench.py` nimmt im temporaeren Quellverzeichnis nachvollziehbare
Kompatibilitaetskorrekturen vor: LCD-Datenbefehl nach Setzen des Fensters,
korrekte Hoehe des letzten halben Fragments bei 2x, begrenztes Lesen des
Befehlsindex und zur Deklaration passender `size_t`-Parametertyp. Der Renderer
selbst bleibt die Azur-Pipeline. Die vom Benchmark genutzten XRAM-/YRAM-Bereiche
entsprechen der [CP-Speicherkarte](https://classpaddev.github.io/wiki/dev/notes/).

Native Tests mit AddressSanitizer und UndefinedBehaviorSanitizer pruefen alle
zwoelf Varianten gegen dieselben Bildpunkte in fuenf Szenen und drei
Momentaufnahmen, einschliesslich der vollstaendigen Ausgabe und des letzten
2x-Fragments. Sie pruefen deterministische/gueltige Szenen, echte Menueablaeufe,
Abbruch vor und waehrend der Messung, kurze Schreibvorgaenge, Platzmangel und
Schliessfehler. Die hardwareabhaengigen LCD-/DMA-/RAM-Zugriffe sind im PC-Test
simuliert; deren echte Funktion und Geschwindigkeit bleiben Geraetearbeit.
