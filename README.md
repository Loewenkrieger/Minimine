# Minimine

Ein kleines 2D-Blockspiel fuer den Casio ClassPad II fx-CP400 mit Hollyhock 3.
Mit Crafting, Oefen, Tieren, Zombies, Kampf und drei Spielstaenden.

[Version 0.4.0 herunterladen](releases/0.4.0/CasioSandbox.hh3).
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
| EXE | Schlagen, essen, bauen oder Werkbank / Ofen oeffnen |
| 1 / 4 / 7 und 3 / 6 / 9 | Links / rechts auf Fuss-, Augen- oder Hoehe ueber dem Kopf zielen |
| 2 / 8 | Unter die Fuesse / ueber den Kopf zielen |
| SHIFT im Inventar | Ruestung anzeigen |
| 5 im Inventar | Einzelnes Item aus markierter Quelle verschieben |
| CLEAR | Pause, Speichern, Laden, Hilfe, Beenden |
| 0 | Benchmark |

Materialien: maximal 64 je Slot. Werkzeuge und Ruestung: einzeln, mit Haltbarkeit.
Speichern erfolgt manuell. Beim Tod bleiben Inventar und Ausruestung erhalten.

Bauen: `tools/build.ps1`. Tests: `tools/test.ps1`.

[Anleitung](docs/ANLEITUNG.md) · [Bauumgebung](docs/TOOLCHAIN.md) ·
[Benchmark](docs/BENCHMARK.md) · [Pruefstand](docs/VERIFICATION.md)
