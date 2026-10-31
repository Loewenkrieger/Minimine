# Minimine

Ein kleines 2D-Blockspiel fuer den Casio ClassPad II fx-CP400 mit Hollyhock 3.
Laufen, springen, abbauen und bauen - mit Crafting, Werkzeugen, Inventar und Sneak.

## Spielen

[Version 0.3.0 herunterladen](releases/0.3.0/CasioSandbox.hh3), die `.hh3` auf
den CP400 kopieren und im Hollyhock-3-Launcher starten. Weitere Dateien werden
auf dem Taschenrechner nicht gebraucht.

| Taste | Aktion |
| --- | --- |
| Links / rechts | Laufen und zielen |
| Hoch / SHIFT | Springen |
| Runter | Sneak umschalten |
| X / Y / Z | Hotbarplatz 1 / 2 / 3 |
| Keyboard | Inventar oeffnen / schliessen |
| DEL halten | Block abbauen |
| EXE | Bauen / Werkbank oeffnen; im Inventar Stapel / Ergebnis |
| 5 | Im Inventar ein Item aus der markierten Quelle verschieben |
| 1-9 ohne 5 | Manuell zielen |
| 0 | Benchmark |
| CLEAR | Hilfe; im Inventar schliessen |

Materialien: maximal 64 je Slot. Spitzhacken: einzeln, mit Haltbarkeit.
Im Inventar gibt es ein 2x2-Craftingfeld, an der Werkbank ein 3x3-Feld.
Stein liefert nur mit ausgewaehlter Spitzhacke ein Item.

Der Spielstand wird noch nicht gespeichert.

## Entwickeln

Mit der eingerichteten ClassPad-Werkzeugkette: `tools/build.ps1`.
Tests: `tools/test.ps1`.

[Anleitung](docs/ANLEITUNG.md) · [Bauumgebung](docs/TOOLCHAIN.md) ·
[Benchmark](docs/BENCHMARK.md) · [Pruefstand](docs/VERIFICATION.md)
