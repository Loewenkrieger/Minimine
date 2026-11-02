# Minimine

Ein kleines 2D-Blockspiel fuer den Casio ClassPad II fx-CP400 mit Hollyhock 3.
Mit Crafting, Basisbau, dunklen Hoehlen, Schatztruhen und einem Steinwaechter.
Baue ein Lager, fertige Eisenwerkzeuge und finde drei Relikte fuer seine Kammer.

[Version 0.5.0 herunterladen](https://github.com/Loewenkrieger/Minimine/releases/tag/v0.5.0).
Nur die `.hh3` auf den CP400 kopieren und im Launcher starten.
Beim Speichern legt das Spiel den Ordner `/Minimine` selbst an.

| Taste | Aktion |
| --- | --- |
| Links / rechts | Laufen und zielen |
| Hoch / SHIFT | Springen |
| Runter | Sneak umschalten |
| X / Y / Z | Hotbar 1 / 2 / 3; im Inventar zum Cursor uebertragen |
| Keyboard | Inventar oeffnen / schliessen |
| DEL halten | Abbauen |
| EXE | Schlagen, essen, bauen oder anvisierten Block benutzen |
| 1 / 4 / 7 und 3 / 6 / 9 | Links / rechts auf Fuss-, Augen- oder Hoehe ueber dem Kopf zielen |
| 2 / 8 | Unter die Fuesse / ueber den Kopf zielen |
| SHIFT im Inventar | Ruestung und zwei Zubehoerplaetze |
| Touch im Inventar | Stapel ziehen oder Quelle und Ziel antippen |
| DIV halten + Touch | Halben Stapel uebertragen |
| 5 im Inventar | Einzelnes Item aus markierter Quelle verschieben |
| CLEAR | Pause, Speichern, Laden, Hilfe, Beenden |
| 0 | Benchmark |

Hochbauen: mit **2** nach unten zielen, springen und mit **EXE** unter sich bauen.

Materialien: maximal 64 je Slot. Werkzeuge und Ruestung: einzeln, mit Haltbarkeit.
Speichern erfolgt manuell. Beenden schliesst sofort ohne Rueckfrage und ohne
Speichern. Beim Tod bleiben Inventar und Ausruestung erhalten.

Bauen: `tools/build.ps1`. Tests: `tools/test.ps1`.

[Anleitung](docs/ANLEITUNG.md) Â· [Bauumgebung](docs/TOOLCHAIN.md) Â·
[Benchmark](docs/BENCHMARK.md) Â· [Pruefstand](docs/VERIFICATION.md)
