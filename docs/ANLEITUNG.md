# Minimine 0.3.0 - Anleitung

## Start

Nur [CasioSandbox.hh3](../releases/0.3.0/CasioSandbox.hh3) auf den
ClassPad II fx-CP400 kopieren und mit Hollyhock 3 starten.
EXE startet das Spiel. Die Welt wird noch nicht gespeichert.
Versionen 0.1.0 und 0.2.0 bleiben unter releases erhalten.

## Tasten

| Taste | Funktion |
| --- | --- |
| Links / rechts | Laufen, Ziel auf die entsprechende Seite setzen |
| Hoch / SHIFT | Ein Sprung pro Tastendruck |
| Runter | Sneak umschalten |
| X / Y / Z | Hotbarplatz 1 / 2 / 3 aktivieren |
| Keyboard | Inventar mit 2x2-Craftingfeld oeffnen |
| DEL halten | Anvisierten Block abbauen |
| EXE | Block platzieren oder anvisierte Werkbank oeffnen |
| 1-9 ohne 5 | Acht manuelle Zielrichtungen |
| 0 | Benchmark |
| CLEAR | Hilfe; im Inventar zurueck zum Spiel |

Gehaltene Zahlentasten haben Vorrang vor automatischem Zielen. Nach Loslassen
zeigt das Ziel beim Laufen nach vorn; im Stand bleibt die letzte Richtung.

In der Hilfe setzt EXE fort; CLEAR kehrt zum Launcher zurueck. Nach einer
Menuepause gehaltenes EXE und DEL erst loslassen, bevor sie wieder wirken.

## Inventar und Crafting bedienen

Drei Hotbarplaetze und zwoelf Inventarplaetze bleiben erhalten. Die Hotbar zeigt
Itemnamen und Anzahl. Materialien passen bis 64 in einen Slot, Werkzeuge einzeln.
Am Anfang gibt es je 32 Erde, Stein und Holzbretter.

- Pfeile bewegen die Auswahl zwischen Inventar, Craftingfeldern und Ergebnis.
- EXE markiert eine Quelle. EXE auf einem Ziel verschiebt den ganzen Stapel.
- 5 legt **ein Item** aus der markierten Quelle ins gewaehlte Feld.
  Die Quelle bleibt markiert, solange darin noch Items liegen.
- EXE auf dem Quellplatz hebt die Markierung auf.
- Leere Ziele uebernehmen; gleiche Materialien werden bis 64 zusammengefuehrt,
  unterschiedliche Stapel getauscht. 5 tauscht keine unterschiedlichen Items.
- Auf dem Ergebnisplatz stellt EXE das angezeigte Rezept einmal her. Erst dabei
  wird je ein Item aus den belegten Rezeptfeldern verbraucht.
- X/Y/Z springen zum jeweiligen Hotbarplatz und aktivieren ihn.
- Keyboard oder CLEAR schliesst. Uebrige Zutaten kommen ins Inventar zurueck.

Die Vorschau ist kein gespeicherter Gegenstand und kann nichts duplizieren.
Ergebnisse und uebrige Zutaten brauchen Platz. Fehlt dieser, bleibt alles
unveraendert und ein Hinweis erscheint. Vor dem Crafting wird auch der Platz
fuer die spaetere Zutatenrueckgabe geprueft, damit das Menue schliessbar bleibt.

Neue Materialien ergaenzen zuerst passende Hotbarstapel, dann passende
Inventarstapel; danach leere Inventarplaetze und zuletzt leere Hotbarplaetze.
Verbrauchte Hotbarstapel werden nicht automatisch nachgefuellt.

## Rezepte

| Ergebnis | Anordnung | Benutzt |
| --- | --- | --- |
| 4 Holzbretter | 1 Baumstamm in einem beliebigen Feld | Inventar oder Werkbank |
| 4 Stoecke | 2 Holzbretter uebereinander | Inventar oder Werkbank |
| 1 Werkbank | 4 Holzbretter als 2x2-Quadrat | Inventar oder Werkbank |
| 1 Holzspitzhacke | 3 Holzbretter oben, darunter 2 Stoecke mittig | Werkbank |
| 1 Steinspitzhacke | 3 Steine oben, darunter 2 Stoecke mittig | Werkbank |

Die Muster duerfen innerhalb des Feldes verschoben sein, soweit sie hineinpassen.
Zusaetzliche Zutaten oder gemischte Materialien ergeben kein gueltiges Rezept.

Beispiel Holzspitzhacke:

```text
B B B
. S .
. S .
```

B = Holzbretter, S = Stock, Punkt = leer.
Fuer die Steinspitzhacke Steine statt Brettern verwenden.

Die Werkbank ueber die Hotbar auswaehlen, mit EXE platzieren, dann loslassen und
den Block erneut mit EXE anwaehlen. Das oeffnet das pausierte 3x3-Craftingmenue.
Mit DEL laesst sie sich wieder abbauen und als Werkbank einsammeln.

## Werkzeuge und Abbau

Zum Benutzen eine Spitzhacke in die Hotbar verschieben und mit X/Y/Z auswaehlen.
Eine Spitzhacke im zusaetzlichen Inventar gilt nicht als ausgeruestet.

| Material | Hand / Holzspitzhacke | Steinspitzhacke |
| --- | ---: | ---: |
| Erde / Gras | 0,35 s | 0,35 s |
| Bretter / Baumstamm / Werkbank | 0,60 s | 0,60 s |
| Stein / Erz | 1,00 s | 0,50 s |
| Blaetter | 0,15 s | 0,15 s |
| Grundgestein | unabbaubar | unabbaubar |

Stein und Erz liefern **ohne ausgewaehlte Spitzhacke kein Item**, verschwinden
nach vollstaendigem Abbau aber trotzdem. Mit Spitzhacke liefern beide Stein.
Natuerliche Baumstaemme und platzierte Stammblocks liefern Baumstaemme;
Bretterbloecke liefern Holzbretter. Blaetter liefern nichts.

Holzspitzhacken halten 64, Steinspitzhacken 128 erfolgreich abgebaute Bloecke.
Jeder vollstaendige Blockabbau mit dem Werkzeug kostet eine Nutzung. Abbruch,
Grundgestein und ein wegen vollem Inventar verhinderter Abbau kosten nichts.
Nach der letzten Nutzung wird der Block noch eingesammelt, dann zerbricht
die Spitzhacke. Resthaltbarkeit bleibt beim Verschieben und Tauschen erhalten.
Die Hotbar zeigt eine Haltbarkeitsleiste, das Inventar den genauen Wert.

Loslassen, Zielwechsel, Werkzeugwechsel und Menues setzen den Abbaufortschritt
zurueck. Die Leiste bleibt im Spielausschnitt. Ohne Platz fuer einen vorgesehenen
Drop bleibt der Block stehen. Im Koerper der Figur kann nichts platziert werden.

## Sneak

Sneak duckt die Figur von 22 auf 14 Pixel und halbiert die Laufgeschwindigkeit.
Fuesse und Kamera bleiben beim Umschalten gleich. Am Boden verhindert die
Kantenbremse unbeabsichtigtes Herunterlaufen. Springen behaelt seine Kraft und
laesst Sneak eingeschaltet; in der Luft greift die Kantenbremse nicht.
Fehlender Boden laesst die Figur fallen.

Aufrichten braucht Platz. Unter einer niedrigen Decke bleibt Sneak an und
„Kein Platz“ erscheint. Spaeter erneut Runter druecken.
Inventar und Werkbank pausieren die Physik; Pfeile bedienen dort nur die Auswahl.

## Benchmark und Entwicklung

Der Benchmark bleibt unveraendert: getrennte Welt, fester Seed, drei Kamerafahrten,
je 30 Aufwaermbilder plus 150 Messbilder. Sneak ist aus, Inventar geschlossen.
Im Spiel gelten 60 Physikschritte pro Sekunde und hoechstens acht Nachholschritte
pro Bild. Verlorene Schritte werden als Simulationsverlust angezeigt.

FPS, Mittel/P95/Max und L/Z/LCD sind echte Laufzeitmesswerte. PC-Vorschauen
liefern keine CP400-FPS. Geraetewerte stehen in [BENCHMARK.md](BENCHMARK.md).

Bauen mit der eingerichteten Werkzeugkette: `tools/build.ps1`.
Tests: `tools/test.ps1` beziehungsweise `bash tools/test.sh`.
Siehe [TOOLCHAIN.md](TOOLCHAIN.md), [VERIFICATION.md](VERIFICATION.md)
und [DEVICE_TEST.md](DEVICE_TEST.md).

