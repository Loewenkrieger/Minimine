# Benchmark-Vergleich

## Ausgangswert: Version 0.1.0

Ein Gerätelauf auf dem Casio ClassPad II fx-CP400 mit Hollyhock 3.
Vom Benutzer übermittelt als Foto am 31.10.2026
(Datei: 20261031_134742.jpg). Genaue Hollyhock-Unterversion, OS-Version,
Takt und sonstige Geräteeinstellungen wurden nicht angegeben.

| Szene | FPS | Mittel ms | P95 ms | Max ms | Logik us | Zeichnen us | LCD us |
| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| Oberfläche | 14,5 | 68,7 | 73,7 | 75,8 | 42 | 47.525 | 20.956 |
| Höhlen | 11,9 | 83,8 | 87,5 | 89,2 | 40 | 62.248 | 20.959 |
| Tiefengestein | 11,0 | 90,1 | 94,6 | 96,0 | 41 | 68.593 | 20.949 |

Die Zeichnung beanspruchte etwa 69–76 % der mittleren Bildzeit.
Die LCD-Übertragung lag bei ungefähr 21 ms. Die Logik war deutlich kleiner.
Darauf richtet sich die Optimierung der Darstellung in Version 0.2.0.

Das geprüfte Original liegt unter [releases/0.1.0](../releases/0.1.0/).
SHA-256 der ursprünglichen .hh3:

`0464f0abecc0afc7a38f5dd347ea3e209d230e8be684da2233111ea2bc36d1da`

## Version 0.2.0

**Eine Messung auf dem CP400 steht noch aus. Es wird kein FPS-Gewinn behauptet.**
Die Tests am PC messen keine Geschwindigkeit des Taschenrechners.

Welt, Startwert, Spielerbewegung und Kamerafahrt wurden direkt gegen den
gesicherten Quellcode von 0.1.0 geprüft. Alle 540 Benchmark-Schritte stimmen
überein. Unverändert: drei Szenen, je 30 Aufwärmbilder und 150 Messbilder,
ein Physikschritt je Benchmark-Bild, vollständige LCD-Übertragung.
Sneak ist aus, das Inventar geschlossen. Die Spielwelt ist getrennt.

Für einen Vergleich die neue .hh3 bei denselben Geräteeinstellungen starten,
**0** drücken und die Ergebnisansicht fotografieren. Möglichst drei Läufe
ausführen. Vorlage: [DEVICE_TEST.md](DEVICE_TEST.md).

Nach der Messung FPS und Mittel/P95/Max sowie Logik/Zeichnen/LCD vergleichen.
Eine schnellere Zeichnung kann die Gesamtrate nur bis zur Grenze der weiterhin
vollständigen LCD-Übertragung verbessern.


## Version 0.3.0

Crafting, Werkbank und Werkzeuge sind im Benchmark inaktiv. Welt und alle
540 Schritte werden weiterhin direkt gegen 0.1.0 getestet. Die normale
Spielsteuerung enthaelt nun Namen und Werkzeuganzeigen; Aufwaerm-/Messbilder
und die vollstaendige LCD-Uebertragung bleiben unveraendert.

Eine Messung von 0.3.0 auf dem echten CP400 steht aus.
