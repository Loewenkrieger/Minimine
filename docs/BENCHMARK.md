# Benchmark-Vergleich

## Version 0.6.0

Die urspruengliche Benchmarkwelt und alle 540 Spieler-/Kameraschritte bleiben
unveraendert gegen 0.1.0 geprueft. Schwierigkeit, Mobs und Diamanten sind dort
ausgeschaltet. Die alte zeilenweise Lichtoptimierung bleibt im Spiel aktiv.
Die separaten Renderer-Vergleiche verwenden nur die bisherigen Blocktypen;
Diamanterze werden in diesen Testwelten durch ihren Basaltuntergrund ersetzt.
Die archivierten Bench-1.0/1.1/1.2-Dateien und Messlogs bleiben unveraendert.

Neue Kollisions- und Schwierigkeitsregeln veraendern die Survival-Simulation.
Zustandshashes neuer Survival-Laeufe sind deshalb nicht als identisch mit alten
Laeufen zu erwarten. 6.000 zusaetzliche dichte Kampfschritte auf allen Stufen
pruefen Ueberlappungsfreiheit und gueltige Zustaende. Neue CP400-FPS sind noch
zu messen; die PC-Tests sind keine Geraetegeschwindigkeitsmessung.

## Messgrundlage: Version 0.5.3 und Kurzvergleich 1.2.0

Aus dem ersten Geraetelauf von Minimine Bench 1.0.0 sind alle 60 Erstlauf-Bloecke
vollstaendig erhalten. Die Einzelmessungen, Zusammenfassungen und Pruefwerte
stimmen ueberein. [Auswertung und Messdateien](../benchmarks/1.0.0/DEVICE_RESULTS.md).

Der Kurzlauf von 1.1.0 ist ausgewertet: 0.5.1 war an der Oberflaeche rund
6,7 Prozent schneller, in den beiden Lichttests aber rund 9,1 beziehungsweise
6,1 Prozent langsamer. Die alternative Vollbild-CPU-Ausgabe brachte keinen
messbaren Vorteil. [Messwerte und Rohdaten](../benchmarks/1.1.0/DEVICE_RESULTS.md).

0.5.2 nahm deshalb die langsamere blockweise Beleuchtung zurueck und behielt
die Block-/Moboptimierungen. Der vollstaendige Kurzlauf von 1.2.0 vergleicht
Originalrenderer, diese Kombination und Zeilenlicht: 15 Bloecke, 900 Messbilder.
Zeilenlicht erreicht 11,48 statt 9,71 FPS im Lichttest und 11,31 statt 9,59 FPS
mit Mobs und Licht. Das sind 18,2 beziehungsweise 17,9 Prozent mehr FPS bei
gleichen Bildpunkten und Zustandshashes. Die LCD-Ausgabe bleibt bei rund 21 ms.
[Auswertung und Rohdaten](../benchmarks/1.2.0/DEVICE_RESULTS.md).

0.5.3 uebernimmt diesen Lichtpfad ins Spiel. Die ersten drei Tests von 1.2.0
haben die Beleuchtung ausgeschaltet und bleiben praktisch gleich schnell.
Fuer den vom Benutzer mit rund 9 FPS gemeldeten Spielstand ist daher ein
direkter Vergleich an derselben Position vorgesehen. Ein bestimmter FPS-Wert
wird daraus nicht vorausgesagt. Der normale Spielbenchmark behaelt seine Welt
und 540 Schritte. [Kurzvergleich](../bench/README.md).

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


## Version 0.4.0

Survival, Mobs, Beute und Oefen sind im Vergleichsbenchmark ausgeschaltet.
Kohle wird nur in der normalen Spielwelt ergaenzt; die Benchmark-Welt und alle
540 alten Spieler-/Kameraschritte bleiben mit 0.1.0 identisch. Das wird im
Regressionstest direkt geprueft. Der Grafikcache einschliesslich Mobs ist auf
17.056 Byte begrenzt. Die LCD-Uebertragung bleibt vollstaendig.

Neue Systeme werden separat mit anfangs 16 Mobs und 32 aktiven Oefen ueber
18.000 Spielschritte getestet. Das misst keine Geschwindigkeit des CP400.
Eine Messung von 0.4.0 auf dem echten Geraet steht weiterhin aus.


## Version 0.4.1

Touchbedienung und halbe Stapel betreffen nur pausierte Menues. Der alte
Vergleichsbenchmark bleibt unveraendert und ist weiterhin gegen 0.1.0 geprueft.
Messungen auf dem CP400 stehen noch aus.


## Version 0.4.2

Zielen unter die Fuesse wurde fuer das Hochbauen korrigiert. Die alte
Benchmarkroute verwendet dieses Ziel nicht und bleibt unveraendert.
Die 540 Schritte werden weiterhin direkt gegen 0.1.0 geprueft.
Messungen auf dem CP400 stehen noch aus.


## Version 0.5.0

Ruinen, Beleuchtung, Zubehoer und Boss sind im alten Vergleichsbenchmark
ausgeschaltet. Die urspruengliche Welt und alle 540 Kamera-/Spielerschritte
bleiben gegen 0.1.0 geprueft. Neue Inhalte werden separat mit einem Durchlauf
bis zum Boss und den maximalen Objektpools getestet. Der Grafikcache liegt
mit blockweisem Licht bei 20.370 Byte. Echte CP400-FPS stehen noch aus.
