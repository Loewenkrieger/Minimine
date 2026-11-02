# Erster CP400-Lauf von Minimine Bench 1.0.0

Unveraenderte Eingabedateien: `device-run0001/run0001-*`.
Gemeldeter CPU-Takt: 117.964.800 Hz. Ein vollstaendiger erster Durchlauf
(60 Bloecke x 150 Bilder); drei weitere Bloecke der zweiten Wiederholung
und anschliessend ein Abbruch vor den Messbildern des naechsten Blocks.

Die Analyse verwendet nur die 9.000 Bilder der ersten Wiederholung.
Insgesamt enthalten die Dateien 9.450 Bildmessungen. Summen, Mittelwerte,
P95, Maxima, Bildpruefsummen und Zustandsvergleiche wurden nachgerechnet;
keine Abweichungen. Der Quellenfingerabdruck entspricht dem ausgelieferten
1.0.0-Paket. Physische LCD-Pixel werden nicht zurueckgelesen.

| Szene | Referenz | Eigenes 2x | Azur 1x | Azur 2x |
| --- | ---: | ---: | ---: | ---: |
| Oberflaeche | 12,66 | 13,64 | 7,50 | 14,16 |
| Hoehlen | 11,79 | 14,44 | 8,92 | 15,46 |
| Tiefengestein | 10,78 | 15,38 | 10,55 | 16,45 |
| Licht | 9,41 | 9,14 | 4,81 | 9,44 |
| Mobs und Licht | 9,01 | 7,45 | 3,93 | 6,84 |

FPS sind aus den Rohdaten berechnet und gerundet. Zeilenbloecke liegen je nach
Szene deutlich dahinter. XRAM/YRAM verbessern den entsprechenden Zeilenprototyp
um etwa 10 bis 33 Prozent, erreichen aber die Referenz nicht. Der getestete
ILRAM-Blitkernel liefert praktisch keinen weiteren Vorteil. Beide synchron
abgewarteten DMA-Vollbildvarianten sind langsamer als die Referenzausgabe.

Die Referenz benoetigt etwa 20,95 ms fuer die LCD-Ausgabe; CPU-Ausgaben der
Fragmentvarianten etwa 14,7 ms. Dieser Unterschied darf wegen unterschiedlicher
Puffer-/Cacheverhaeltnisse nicht ohne separaten Test auf das Spiel uebertragen
werden. Die Spiellogik liegt zwischen etwa 0,05 und 0,34 ms pro Bild.

Die ersten 9.000 Messbilder summieren sich auf 37,60 Minuten. Aufwaermen,
Bildpruefungen, Pruefsummen und Dateizugriffe kommen dazu. Deshalb verwendet
Version 1.1.0 standardmaessig einen kurzen direkten Alt/Neu-Vergleich.
