# Casio Sandbox 0.2.0

Ein eigenständiges 2D-Blockspiel für den **Casio ClassPad II fx-CP400 mit Hollyhock 3**.
Version 0.2.0 ergänzt Inventar, Abbauzeiten und Sneak und beschleunigt den
Zeichenweg durch vorbereitete Blockgrafiken.

**Nur [CasioSandbox.hh3](../releases/0.2.0/CasioSandbox.hh3) auf den CP400 kopieren**
und die bisherige Datei ersetzen. Weitere Dateien werden auf dem Gerät nicht benötigt.
Die gesicherte Version 0.1.0 liegt in [releases/0.1.0](../releases/0.1.0/).

Die nativen Tests und der vollständige ClassPad-Bau bestehen; Einzelheiten
stehen in [VERIFICATION.md](VERIFICATION.md). Die neue Version muss noch
auf echter Hardware getestet werden. Dein Benchmark aus 0.1.0 mit
**14,5 / 11,9 / 11,0 FPS** steht in [BENCHMARK.md](BENCHMARK.md).
Ein Performance-Gewinn von 0.2.0 ist noch nicht gemessen.

## Bedienung

| Taste | Aktion |
| --- | --- |
| Links / rechts | Laufen; Ziel auf die entsprechende Seite setzen |
| Hoch oder SHIFT | Ein Sprung je Tastendruck |
| Runter | Sneak ein-/ausschalten, ohne Wiederholung beim Halten |
| 1 / 2 / 3 / 4 / 6 / 7 / 8 / 9 | Acht Zielrichtungen wie auf dem Zahlenblock |
| X / Y / Z | Hotbarplatz 1 / 2 / 3 direkt auswählen |
| Keyboard | Inventar öffnen/schließen, Spiel pausieren |
| DEL halten | Zielblock mit Fortschrittsleiste abbauen |
| EXE | Sofort platzieren; Halten wiederholt im Abstand von neun Physikschritten |
| 0 | Benchmark starten |
| CLEAR | Hilfe/Pause; im Inventar schließen; Benchmark abbrechen |

Gehaltene Zahlentasten haben Vorrang vor automatischem Zielen. Nach Loslassen
zeigt das Ziel beim Laufen nach vorn; im Stand bleibt die letzte Richtung.
Der Materialwechsel über 5 entfällt.

Auf der Hilfe setzt EXE das Spiel fort; CLEAR kehrt zum Launcher zurück.
Nach Menüpausen müssen gehaltenes DEL und EXE erst losgelassen werden.
Die Pfeile bedienen im Inventar ausschließlich die Auswahl.

## Sneak

Die Figur ist geduckt 14 statt 22 Pixel hoch und läuft halb so schnell.
Füße und Kamera bleiben beim Umschalten an derselben Position. Am Boden
verhindert eine Kantenbremse unbeabsichtigtes Herunterlaufen. Springen bleibt
mit unveränderter Sprungkraft möglich und lässt Sneak eingeschaltet.
In der Luft greift die Kantenbremse nicht. Fehlt der Boden, fällt die Figur.

Unter einer niedrigen Decke kann die Figur nicht aufstehen: „Kein Platz“
erscheint. Nach Verlassen des engen Bereichs erneut Runter drücken.
Inventar und Hilfe erhalten den Sneak-Zustand.

## Inventar

Drei Hotbarplätze oben, darunter zwölf weitere Plätze in drei Spalten.
Anfangs enthält die Hotbar je 32 Erde, Stein und Holz. Maximal 999 Blöcke
je Stapel.

1. Mit Pfeilen einen Platz wählen und mit EXE als Quelle markieren.
2. Ziel wählen und mit EXE bestätigen.
3. Derselbe Platz hebt die Markierung auf.

Ein leeres Ziel übernimmt den ganzen Stapel. Gleiche Materialien werden bis
999 zusammengeführt, der Rest bleibt in der Quelle. Unterschiedliche Stapel
werden getauscht. Schließen mit einer markierten Quelle verliert keine Blöcke.
X/Y/Z wählen den entsprechenden Hotbarplatz und aktivieren ihn auch im Inventar.

Aufgesammelte Materialien ergänzen zuerst passende Hotbarstapel, dann passende
Inventarstapel. Danach werden leere Inventarplätze und zuletzt leere
Hotbarplätze verwendet. Leere Hotbarstapel werden nicht automatisch aus dem
Inventar nachgefüllt.

## Abbau

| Material | Dauer | Physikschritte |
| --- | ---: | ---: |
| Erde / Gras | 0,35 s | 21 |
| Holz / Baumstamm | 0,60 s | 36 |
| Stein / Erz | 1,00 s | 60 |
| Blätter | 0,15 s | 9 |
| Grundgestein | unabbaubar | — |

DEL muss gehalten werden. Loslassen, Zielwechsel und Öffnen eines Menüs
setzen den Fortschritt zurück. Erst der abgeschlossene Abbau entfernt den Block
und sammelt das Material ein. Bei Platzmangel bleibt der Block bestehen und
„Inventar voll“ erscheint.

Gras liefert Erde, Erz liefert Stein, Baumstämme liefern Holz. Blätter liefern
kein Material. Natürliche Stämme und Blätter sind durchgehbar; platzierte
Holzblöcke sind fest. Im Körper der Figur lassen sich keine Blöcke platzieren.

## Welt und Benchmark

Die Welt hat 256 × 96 Blöcke mit je 16 × 16 Pixeln. Oberfläche, Tunnel,
Gestein, Erz und Bäume werden mit festem Startwert erzeugt. Das Spiel hat noch
keine Speicherung, Gegner, Herstellung oder Flüssigkeiten. Beim Beenden geht
die Spielwelt verloren.

**0** startet drei feste Kamerafahrten durch eine getrennte Testwelt.
Jede Szene hat 30 Aufwärmbilder und 150 Messbilder. Pro Benchmark-Bild wird
genau ein Physikschritt ausgeführt; Kamera und Eingaben hängen von der
Bildnummer ab. Sneak ist aus, das Inventar geschlossen. Die Folge stimmt mit
Version 0.1.0 überein. EXE kehrt nach dem Test zur erhaltenen Spielwelt zurück.

Die normale Physik arbeitet mit 60 Schritten je Sekunde. Bei extremen
Verzögerungen werden höchstens acht Schritte je Bild nachgeholt; verlorene
Schritte zeigt „Simulationsverlust“ an.

| Anzeige | Bedeutung |
| --- | --- |
| FPS | Gezeigte Bilder geteilt durch gemessene Gesamtzeit |
| Bild / Mittel | Mittlere Zeit zwischen abgeschlossenen LCD-Übertragungen |
| L | Eingabe und Spiellogik |
| Z | Zeichnen einschließlich Bedienoberfläche |
| LCD | Vollständige Übertragung zum Bildschirm |
| P95 | 95 % der gemessenen Bilder sind so schnell oder schneller |
| Max | Langsamstes Bild der Szene |

Die Bildzeit umfasst zusätzlich die Messwertverwaltung; Teilzeiten müssen sich
nicht exakt zur Bildzeit addieren. Kennzahlen werden ungefähr alle 0,5 s aus
höchstens 150 Bildern aktualisiert. Anfangs steht dort 0,0.

Zum Vergleich auf dem CP400 **0** drücken und die Ergebnisansicht fotografieren,
möglichst für drei Läufe bei gleichen Geräteeinstellungen.
Das [Gerätetestprotokoll](DEVICE_TEST.md) enthält die Funktionsprüfung.
PC-Tests und Vorschauen liefern keine CP400-FPS-Werte. Messdateien werden auf
dem Taschenrechner nicht geschrieben.


## `.hh3` bauen

Die ClassPad-Bauumgebung ist nach Freigabe in der vorhandenen WSL-Distribution
`kali-linux` eingerichtet worden. Sie verwendet die dokumentierte
Community-Containerumgebung, festgehalten über den unveränderlichen Image-Digest
in `tools/toolchain-image.txt`.

Unter Windows im Projektordner:

```powershell
.\tools\build.ps1
```

Der Helfer startet die vorhandene Docker Engine in WSL und baut das Projekt.
Er installiert nichts und lädt kein neues Image. Der Container hat keinen
Netzwerkzugriff; der Projektquellcode ist darin nur lesbar. Der eigentliche Bau
findet temporär im Linux-Dateisystem statt. Nur die geprüften Ergebnisse werden
in den Windows-Projektordner zurückkopiert.

Direkt in der eingerichteten WSL-Umgebung:

```sh
sudo service docker start
sudo bash tools/build-container.sh
```

Bei erfolgreichem Bau entstehen:

```text
build-cp/CasioSandbox.hh3   Programm für Hollyhock 3
build-cp/build-info.json   Prüfsumme, Launcher-Metadaten, Speicher und Quellstände
build-cp/CasioSandbox.map  Linker-Speicherbelegung
build-cp/elf-info.txt      Unabhängige ELF-/Ladebereichsprüfung
build-cp/section-sizes.txt Größen der einzelnen Programmabschnitte
build-cp/casio_sandbox     Ungestrippte ELF-Datei für spätere Fehlersuche
```

**Nur `CasioSandbox.hh3` auf den CP400 kopieren**, ins Hauptverzeichnis des
Gerätespeichers. Danach im vorhandenen Hollyhock-3-Launcher starten.

Bei einer anderen, bereits eingerichteten ClassPad-fähigen fxSDK-Werkzeugkette
kann auch `bash tools/build.sh` verwendet werden. Benötigt werden Python 3,
CMake, `sh-elf-gcc` und gint ab 2.11 mit fxCP-/HH3-Unterstützung. Details und
installierte Versionen stehen in `docs/TOOLCHAIN.md`.

## Prüfungen ohne ClassPad

In der vorhandenen Linux-/WSL-Umgebung mit einem C-Compiler:

```sh
bash tools/test.sh
```

Unter Windows kann das Hilfsskript die vorhandene WSL-Distribution verwenden:

```powershell
.\tools\test.ps1
# Bei einer anderen Distribution:
.\tools\test.ps1 -Distribution Ubuntu
```

Die Tests laufen mit AddressSanitizer und UndefinedBehaviorSanitizer. Sie prüfen
unter anderem Weltgrenzen, Kollisionen, Inventaränderungen, 20.000 gemischte
Spielschritte, FPS-/P95-Berechnung sowie den echten Programmeinstieg mit einem
simulierten Timer und simulierten Tasteneingaben. Der vollständige Benchmark
wird dabei durchlaufen und die unveränderte Spielwelt geprüft.

Ein kleiner Testadapter zeichnet zusätzlich Bildschirmvorschauen nach
`build/native/*.png`, sofern Python 3 verfügbar ist. Er nutzt eine andere
Testschrift. Er bildet weder die CP400-Hardware noch deren Geschwindigkeit nach.

## Aufbau

```text
src/game.*       Welt, Physik, Inventar und feste Benchmark-Kamerafahrt
src/perf.*       Begrenzter Messwertpuffer, FPS und Perzentile
src/render.*     Welt und Bedienoberfläche über gint
src/main.c       Tasten, Spielablauf und Hardware-Zeitmessung
tools/           Bau- und Prüfskripte, PNG-Vorschauen
tests/           Native Tests und kleiner Darstellungsadapter
docs/            Prüfstand und Protokollvorlage für das echte Gerät
```

Die beiden Spielwelten brauchen zusammen **50.080 Byte**. Der einmal vorbereitete
Grafikcache ben�tigt **15.968 Byte** (Atlas, Transparenzmasken und H�gelkontur),
also weniger als 20 KiB. Kein zus�tzlicher Vollbildpuffer und keine
Speicheranforderungen in der Spielschleife. Speicherbelegung des fertigen
Ger�tebaus: siehe [VERIFICATION.md](VERIFICATION.md).

## Technische Referenzen

- [ClassPad: gint-Einführung](https://classpad.dev/wiki/gint/getting-started/introduction/)
- [ClassPad: Anwendungen bauen](https://classpad.dev/wiki/gint/getting-started/building/)
- [ClassPad: Hardware, Darstellung, Eingaben und Profiling](https://classpad.dev/wiki/gint/getting-started/understanding-gint/)
- [gint-Quellstand der Headerprüfung](https://github.com/ClasspadDev/gint/tree/120cffd9b8093c96047f10858c75a2adae62347b)

