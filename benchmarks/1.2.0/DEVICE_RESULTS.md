# CP400-Ergebnisse: Minimine Bench 1.2.0

Die vier vom Benutzer uebergebenen Dateien liegen bytegetreu unter
[device-run0001](device-run0001). Der Kurzlauf ist vollstaendig: 15 Bloecke,
900 Messbilder, eine Wiederholung, Abschlussstatus `run_complete`.

Bench 1.2.0, Spielversion der Quellen 0.5.2; Quellkennung
`bc17a14dbbe0e9924df5cb707c6f85e3187b6ac4ba756e5ef6d4e736bc40f91c`.
Sie stimmt mit dem ausgelieferten Buildbericht ueberein. Gemeldeter
CPU-/Bus-/Peripherietakt: 117.964.800 / 58.982.400 / 29.491.200 Hz,
unveraendert gegenueber den vorherigen Laeufen. OS-/Hollyhock-Unterversion unbekannt.

## Ergebnisse

Alle drei Varianten verwenden normale Aufloesung und gint-Vollbildausgabe.
FPS aus Anzahl geteilt durch gesamte Rohbildzeit, auf zwei Stellen gerundet.

| Szene | Original 0.5.0 | Block-/Mobs mit altem Licht (0.5.2) | Zeilenlicht | FPS-Gewinn gegen 0.5.2 |
| --- | ---: | ---: | ---: | ---: |
| surface | 12.67 | 13.51 | 13.51 | praktisch gleich |
| caves | 11.79 | 12.12 | 12.12 | praktisch gleich |
| deep_stone | 10.78 | 11.06 | 11.06 | praktisch gleich |
| lighting | 9.41 | 9.71 | 11.48 | +18.2 % |
| mobs_lighting | 9.02 | 9.59 | 11.31 | +17.9 % |

Die ersten drei Szenen haben Beleuchtung und Survival ausgeschaltet. Ihre
Unterschiede zwischen 0.5.2 und Zeilenlicht liegen unter 0,03 Prozent.
In den beiden beleuchteten Szenen spart der neue Pfad 15,826 beziehungsweise
15,806 ms je Bild. Die Zeichenzeit sinkt von 81,941 auf 66,108 ms und mit Mobs
von 83,039 auf 67,214 ms. Die mittlere LCD-Zeit ueber alle Varianten bleibt
bei 20,951 ms. Der Gewinn kommt aus der Zeichnung.

Auch P95 verbessert sich: 103,201 auf 88,322 ms im Lichttest und 104,753 auf
89,679 ms mit Mobs. Gegen den vorherigen 0.5.1-Lichtversuch aus Bench 1.1.0
liegt der FPS-Gewinn bei rund 33 Prozent; die Tabelle vergleicht mit 0.5.2.

## Pruefung und Grenzen

- Alle 15 Bloecke melden `ok`; Bildpruefsummen stimmen mit der Referenz ueberein.
- Alle 900 Rohzeilen geprueft: Anzahl, FPS, Mittel, P95, Maximum und Teilzeiten
  ergeben exakt die gespeicherten Zusammenfassungen.
- `total_us = logic_us + draw_us + lcd_us` stimmt fuer jedes Bild.
- Stichproben entsprechen den 60 Simulationsschritten `30 + sample * 149 / 59`.
- Zustandshashes stimmen zwischen allen drei Varianten und den entsprechenden
  Schritten der Original-Logs von 1.0.0 und 1.1.0 ueberein.
- Der Quellstand und die Speicherangaben passen zum ausgelieferten Build.
  Die vier Eingabedateien wurden unveraendert ins Archiv kopiert.

Reine erfasste Messzeit: 81,013 Sekunden. Aufwaermen, Bildpruefungen,
Menues und Datei-I/O sind darin nicht enthalten; die komplette Laufzeit
laesst sich aus diesen Logs nicht bestimmen. Nur eine Wiederholung.
Die Bildpruefung liest den physischen LCD-Inhalt nicht zurueck.

0.5.3 uebernimmt den gemessenen Zeilenlicht-Pfad. Die rund 18 Prozent sind
Szenenmessungen; der vom Benutzer mit rund 9 FPS gemeldete Spielstand muss
an derselben Position separat verglichen werden. Keine bestimmte Spiel-FPS
wird vorausgesagt. Der Grafikcache bleibt bei 20.370 Bytes und es wird kein
weiterer Vollbildpuffer angelegt.

Die genaue Rendererquelle des gemessenen Builds ist in
[render-reference.c](render-reference.c) gesichert, SHA-256:
`2f5e62f3b71713b28292df5dc5f5c9b19a43d7b605b7c4d629d60b536422f5d2`.
Die bisher ausgelieferte Bench-1.2.0-HH3 bleibt unveraendert.
