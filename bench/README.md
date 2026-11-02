# Minimine Bench 1.2.0

Separates Testprogramm fuer den CP400 mit Hollyhock 3. Minimine und dessen
Spielstaende werden nicht geoeffnet oder veraendert.

Fertige Datei: [MinimineBench.hh3](../benchmarks/1.2.0/MinimineBench.hh3).

1. `MinimineBench.hh3` auf den Rechner kopieren und im Launcher starten.
2. **EXE** startet den kurzen Vergleich: drei Varianten, fuenf Szenen,
   eine Wiederholung. Die Anzeige zaehlt **Test 1/15 bis 15/15**.
3. Nach "Fertig" das Programm verlassen und den Ordner **`MinimineBench`**
   vom Rechner kopieren. Alle vier Dateien mit derselben Laufnummer senden,
   zum Beispiel `run0001-*`. Eine ZIP mit dem ganzen Ordner geht ebenfalls.

Der Kurzvergleich prueft den Originalrenderer 0.5.0, die Block-/Moboptimierungen
mit alter Beleuchtung (Spiel 0.5.2) und dieselben Optimierungen mit neuem
Zeilenlicht. Alle drei verwenden normale Aufloesung und gint-Ausgabe. Der
Lichtansatz war beim Bau von 1.2.0 nur im Benchmark enthalten und wird ab
Spielversion 0.5.3 auch im Spiel verwendet. Aus den bisherigen Messzeiten
ergeben sich grob **2 bis 4 Minuten** inklusive Aufwaermen und Pruefungen;
die komplette Laufzeit ist in den uebergebenen Logs nicht erfasst.

Der erste vollstaendige 1.2.0-Geraetelauf ist inzwischen ausgewertet:
[Ergebnisse und Original-Logs](../benchmarks/1.2.0/DEVICE_RESULTS.md).
Die beiden beleuchteten Szenen gewinnen rund 18 Prozent gegenueber 0.5.2.
Die fertige 1.2.0-Datei bleibt unveraendert; die Vergleichspfade sind in den
Quellen weiterhin getrennt vorhanden.

**Links/rechts:** Kurzvergleich, langer Gesamtlauf oder einzelne Variante.
**Hoch/runter:** eine oder drei Wiederholungen. **CLEAR:** waehrend eines Laufs abbrechen, im Hauptmenue
beenden. Bisher abgeschlossene Messbloecke und eine begonnene Teilmessung
bleiben gespeichert. Der lange Gesamtlauf enthaelt weiterhin die alten
Experimente und kann deutlich ueber eine Stunde dauern. Fuer die neue Messung
bitte den voreingestellten Kurzvergleich verwenden.

Die Szenen sollen waehrend der Messung sichtbar sein. Falls eine Variante ein
schwarzes oder falsches Bild zeigt, bitte die Variante nennen und die Logs
mitschicken. Nach einem unerwarteten Neustart ist die zuletzt gestartete
Variante in der Statusdatei erkennbar.

**LOG** ist nur die Dateinummer, **TEST** der Gesamtfortschritt. Die Zahl hinter
dem Variantennamen beschreibt die Wiederholung, im Kurzvergleich also **1/1**.

Die Menues erscheinen in normaler Aufloesung. Waehrend der beiden 2x-Tests im langen Gesamtlauf wird
das gesamte Testbild grober dargestellt. Fuer Vergleichslaeufe dieselben
Geraeteeinstellungen verwenden; das Programm aendert den Takt nicht.

Logs je Lauf:

- `meta.txt`: Programmstand, Taktangaben, Einstellungen und Speicherbelegung.
- `summary.csv`: FPS, Mittel/P95/Maximum, Teilzeiten und Bildpruefung je Test.
- `frames.csv`: Einzelmessungen mit Spielzustandspruefsummen.
- `status.csv`: gestartete/abgeschlossene Tests, Abbrueche und Fehler.

Bei einem Schreibfehler stoppt der Lauf. Vorhandene Laufnummern werden nicht
ueberschrieben. Nach dem Kopieren koennen alte Logs auf dem Rechner geloescht
werden, um Platz fuer neue Laeufe freizugeben.

Bauen: `tools/build-bench.ps1`. Tests: unter WSL `bash tools/test-bench.sh`.
Details zu Varianten und Messverfahren: [BENCHMARK_SUITE.md](../docs/BENCHMARK_SUITE.md).
