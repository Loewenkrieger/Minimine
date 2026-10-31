# Gerätetest für Version 0.2.0

Die Prüfungen am PC und der Gerätebau sind dokumentiert in
[VERIFICATION.md](VERIFICATION.md). Dieses Protokoll erfasst die echte Hardware.

## Gerät

| Angabe | Wert |
| --- | --- |
| Modell | Casio ClassPad II fx-CP400 |
| Hollyhock-Version | |
| OS-Version | |
| Spielversion | 0.2.0; SHA-256 siehe VERIFICATION.md |
| Datum | |
| Geräteeinstellungen / Takt | |
| Werkzeugkette | SuperH GCC 14.1.0, fxSDK 2.11.0, gint 2.11.0-91 |

## Bedienung und Bewegung

- [ ] Hilfe erscheint, EXE startet, CLEAR in der Hilfe kehrt zum Launcher zurück.
- [ ] Links/rechts laufen und das Ziel folgt, auch an einer Wand.
- [ ] Gehaltene Zahlentasten haben beim Laufen Vorrang. Nach Loslassen zeigt
      das Ziel beim Laufen nach vorn; im Stand bleibt die letzte Richtung.
- [ ] Hoch und SHIFT lösen jeweils einen Sprung aus. Gehaltene Tasten lösen
      nach der Landung keinen weiteren Sprung aus.
- [ ] Runter schaltet Sneak einmal ein, erneutes Drücken aus. Halten schaltet
      nicht wiederholt um.
- [ ] Sneak: Figur kleiner, halbes Tempo, Füße unverändert, kein Kamerasprung.
- [ ] An linker und rechter Kante stoppt Sneak; Sprung über die Kante bleibt möglich.
- [ ] Ein ein Block hoher Gang ist geduckt passierbar. Aufrichten unter der
      Decke zeigt „Kein Platz“. Nach Verlassen erneut Runter drücken.
- [ ] Entfernen des Bodens lässt die Figur trotz Sneak fallen.

## Inventar

- [ ] Keyboard öffnet und schließt; CLEAR schließt ebenfalls.
- [ ] Drei Hotbarplätze plus zwölf Inventarplätze in drei Spalten sichtbar.
- [ ] Pfeile bewegen nur den Cursor; Hoch springt nicht, Runter schaltet Sneak nicht.
- [ ] EXE: Quelle, dann Ziel. Gleicher Platz hebt die Markierung auf.
- [ ] Leeres Ziel übernimmt, gleiche Materialien werden bis 999 zusammengeführt
      (Rest bleibt in der Quelle), unterschiedliche Materialien werden getauscht.
- [ ] Mit markierter Quelle schließen: kein Gegenstandsverlust.
- [ ] X/Y/Z wählen im Spiel direkt Platz 1/2/3; im Inventar zusätzlich den Cursor.
- [ ] Leerer aktiver Hotbarplatz wird nicht aus dem Inventar nachgefüllt.
- [ ] Spiel und Sneak bleiben beim Öffnen/Schließen von Inventar und Hilfe erhalten.
- [ ] Nach langer Pause kein Physiksprung; gehaltenes EXE/DEL baut nichts
      versehentlich. Erst nach Loslassen werden diese Aktionen wieder freigegeben.

## Abbau und Darstellung

- [ ] DEL halten: Erde/Gras 0,35 s, Holz/Stamm 0,6 s, Stein/Erz 1,0 s,
      Blätter 0,15 s. Grundgestein bleibt bestehen.
- [ ] Fortschrittsleiste über dem Ziel, auch am Rand innerhalb des Spielausschnitts.
- [ ] Loslassen, anderes Ziel und Öffnen von Inventar/Hilfe setzen Fortschritt zurück.
- [ ] Abgebautes Material erscheint im passenden Stapel. Blätter liefern kein Material.
- [ ] Volles Inventar: Block bleibt stehen, Hinweis „Inventar voll“.
- [ ] EXE platziert sofort; keine Platzierung im Körper, auch beim Ducken.
- [ ] Keine Grafikreste nach Menüwechseln; Bäume transparent, Bildschirmränder sauber.
- [ ] Unter Belastung auf Verzögerungen und „Simulationsverlust“ achten.

## Benchmark

**0** startet den Vergleichstest. Die bearbeitete Spielwelt und Sneak müssen
danach beim Fortsetzen erhalten sein. Je Szene 30 Aufwärm- und 150 Messbilder.
Drei vollständige Läufe bei denselben Einstellungen wie 0.1.0 ausführen.
Das alte Foto steht ausgewertet in [BENCHMARK.md](BENCHMARK.md).

| Lauf | Szene | FPS | Mittel ms | P95 ms | Max ms | L us | Z us | LCD us |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Oberfläche | | | | | | | |
| 1 | Höhlen | | | | | | | |
| 1 | Tiefengestein | | | | | | | |
| 2 | Oberfläche | | | | | | | |
| 2 | Höhlen | | | | | | | |
| 2 | Tiefengestein | | | | | | | |
| 3 | Oberfläche | | | | | | | |
| 3 | Höhlen | | | | | | | |
| 3 | Tiefengestein | | | | | | | |

Beobachtungen:

