# CP400-Ergebnisse: Minimine Bench 1.1.0

Die vom Benutzer uebergebenen Dateien liegen unveraendert unter
[device-run0001](device-run0001). Ein vollstaendiger Kurzlauf mit 20 Bloecken,
1.200 Messbildern und Abschlussstatus `run_complete`.

Programmstand: Bench 1.1.0, Spielrenderer 0.5.1, Quellkennung
`a3a8ebac0b391af3d3d6bed22f0995aeca454a639f3913cbbc344ddefb0deeff`.
Gemeldeter CPU-/Bus-/Peripherietakt: 117.964.800 / 58.982.400 / 29.491.200 Hz.
Das Programm hat den Takt nicht geaendert. OS-/Hollyhock-Unterversion unbekannt.

## Originalrenderer gegen 0.5.1

Beide mit gint-Vollbildausgabe. FPS und Mittel aus den Rohdaten berechnet;
die CSV-Zusammenfassung schneidet ihre Ganzzahlwerte ab.

| Szene | FPS 0.5.0 | FPS 0.5.1 | FPS-Aenderung | Mittel alt ms | Mittel neu ms |
| --- | ---: | ---: | ---: | ---: | ---: |
| surface | 12.67 | 13.52 | +6.7 % | 78.920 | 73.989 |
| caves | 11.79 | 12.12 | +2.8 % | 84.842 | 82.526 |
| deep_stone | 10.78 | 11.06 | +2.6 % | 92.738 | 90.416 |
| lighting | 9.44 | 8.58 | -9.1 % | 105.986 | 116.569 |
| mobs_lighting | 9.03 | 8.48 | -6.1 % | 110.713 | 117.865 |

Die Beleuchtungsaenderung bringt hier einen Rueckschritt. Die Zeichenzeit des
Lichttests steigt von rund 84,984 auf 95,568 ms. Der Aufschlag fuer Mobs gegen
die Lichtszene schrumpft dagegen von rund 4,73 auf 1,30 ms Gesamtbildzeit.
Das spricht fuer die Moboptimierung, isoliert sie wegen moeglicher
Wechselwirkungen jedoch noch nicht vollstaendig.

Die alternative CPU-Vollbildausgabe liegt ebenso wie gint bei rund 20,95 ms
je Ausgabe. Die Unterschiede der Gesamtmittel liegen unter 0,03 Prozent.
Der Vorteil der CPU-Ausgabe kleiner Zeilenfragmente aus 1.0.0 uebertraegt sich
damit nicht auf die Vollbildausgabe. Eine Cache-/Pufferursache ist eine
Vermutung und durch diese Messung nicht bewiesen.

## Pruefungen und Folgerung

- Alle 20 Bildpruefungen stimmen; kein Renderfehler oder Abbruch gemeldet.
- Anzahl, FPS, Mittel, P95, Maximum und Teilzeiten aus allen 1.200 Einzelbildern
  nachgerechnet. `total_us = logic_us + draw_us + lcd_us` stimmt fuer jedes Bild.
- Die 60 Stichproben je Block entsprechen genau den Simulationsschritten
  `30 + sample * 149 / 59`.
- Zustandshashes stimmen ueber alle vier Varianten und mit den entsprechenden
  Schritten des archivierten 1.0.0-Geraetelaufs ueberein.
- Rohdateien bytegetreu mit dem uebergebenen Ordner `MinimineBench-v1.1`
  abgeglichen. Bisherige HH3-Dateien und Rendererreferenz bleiben unveraendert.

Nur ein Durchlauf; dies sind Variantenmessungen mit festen Szenen, keine
Vorhersage der FPS eines geladenen Spielstands. Reine erfasste Messzeit:
114,544 Sekunden, ohne Aufwaermen, Bildpruefungen und Dateizugriffe.

0.5.2 stellt den alten Lichtpfad wieder her und behaelt die Block-/Mobaenderungen.
Bench 1.2.0 trennt diese Kombination vom Original und vom neuen Zeilenlicht;
15 Bloecke mit je 60 Messbildern. Die echte Geschwindigkeit bleibt zu messen.
Die genaue 0.5.1-Rendererquelle ist in [render-reference.c](render-reference.c)
mit SHA-256 `61671d553bd5028265ced43eea0b173c599434713447da3f961471846e4a7331` gesichert.
