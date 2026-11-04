# Minimine

Ein kleines 2D-Blockspiel fuer den Casio ClassPad II fx-CP400 mit Hollyhock 3.
Mit Crafting, Basisbau, dunklen Hoehlen, Schatztruhen und einem Steinwaechter.
Baue ein Lager, fertige Diamantausruestung und finde drei Relikte fuer seine Kammer.

[Version 0.6.0 herunterladen](releases/0.6.0/CasioSandbox.hh3).
Nur die `.hh3` auf den CP400 kopieren und im Launcher starten.
Beim Speichern legt das Spiel den Ordner `/Minimine` selbst an.

<p>
  <img src="docs/screenshots/base.png" alt="Basis mit Holzwaenden, Tuer und Einrichtung" width="210">
  <img src="docs/screenshots/diamonds.png" alt="Diamanterz und Fackellicht in einer tiefen Hoehle" width="210">
  <img src="docs/screenshots/crafting.png" alt="Inventar mit Werkbank und Spitzhackenrezept" width="210">
</p>

Basisbau, tiefe Hoehlen und Crafting – Spielgrafik aus Version 0.6.0.

| Taste | Aktion |
| --- | --- |
| Links / rechts | Laufen und zielen |
| Hoch / SHIFT | Springen |
| Runter | Sneak umschalten |
| X / Y / Z / ^ | Hotbar 1 / 2 / 3 / 4; im Inventar zum Cursor uebertragen |
| Touch auf Hotbar | Slot auswaehlen |
| Keyboard | Inventar oeffnen / schliessen |
| DEL halten | Abbauen |
| EXE | Schlagen, essen, bauen oder anvisierten Block benutzen |
| 1 / 4 / 7 und 3 / 6 / 9 | Links / rechts auf Fuss-, Augen- oder Hoehe ueber dem Kopf zielen |
| 2 / 8 | Unter die Fuesse / ueber den Kopf zielen |
| SHIFT im Inventar | Ruestung und zwei Zubehoerplaetze |
| Touch im Inventar | Stapel ziehen oder Quelle und Ziel antippen |
| DIV halten + Touch | Halben Stapel uebertragen |
| 5 im Inventar | Einzelnes Item aus markierter Quelle verschieben |
| CLEAR | Pause, Speichern, Laden, Schwierigkeit, Hilfe, Beenden |
| 0 | Benchmark |

Hochbauen: mit **2** nach unten zielen, springen und mit **EXE** unter sich bauen.

Normale Mobs lassen sich schieben und bleiben im Nahkampf erreichbar.
Diamanten entstehen einmalig in tiefem Basalt, auch in vorhandenen Welten.
Die Schwierigkeit Leicht/Mittel/Schwer ist beim Start und im Pausenmenue waehlbar.

Materialien: maximal 64 je Slot. Werkzeuge und Ruestung: einzeln, mit Haltbarkeit.
Speichern erfolgt manuell. Beenden schliesst sofort ohne Rueckfrage und ohne
Speichern. Beim Tod bleiben Inventar und Ausruestung erhalten.

Bauen: `tools/build.ps1`. Tests: `tools/test.ps1`.

Renderer-Vergleiche mit CSV-Logs: [Minimine Bench](bench/README.md).
0.6.0 behaelt die zeilenweise Beleuchtung. Alte Spielstaende bleiben ladbar;
beim Speichern wird Format 3 verwendet.

[Anleitung](docs/ANLEITUNG.md) · [Bauumgebung](docs/TOOLCHAIN.md) ·
[Benchmark](docs/BENCHMARK.md) · [Pruefstand](docs/VERIFICATION.md)
