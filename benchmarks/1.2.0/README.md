# Minimine Bench 1.2.0

1. `MinimineBench.hh3` auf den CP400 kopieren und im Launcher starten.
2. **EXE** startet den voreingestellten Kurzvergleich.
3. **Test 1/15 bis 15/15** durchlaufen lassen. Eine Wiederholung ist eingestellt.
4. Nach "Fertig" die vier neuen Log-Dateien aus **MinimineBench** zurueckkopieren.

Drei Varianten mit gleicher gint-Ausgabe und normaler Aufloesung:

- Originalrenderer 0.5.0.
- Block-/Moboptimierungen mit bisheriger Beleuchtung (Spiel 0.5.2).
- Dieselben Optimierungen mit neuem Zeilenlicht, nur hier im Benchmark.

Geschaetzte Dauer **2 bis 4 Minuten**, abgeleitet aus den bisherigen Messzeiten.
Eine echte Laufzeitmessung von 1.2.0 steht noch aus. Keine drei Wiederholungen
oder "Alle Varianten (lang)" fuer diesen Vergleich einstellen.

**LOG** ist die Dateinummer, **TEST** der Gesamtfortschritt und **1/1** die
Wiederholung. Vorhandene Logs werden nicht ueberschrieben. **CLEAR** bricht ab
und sichert die Teilmessung. Nach der Ergebnisansicht mit EXE ins Menue,
dort mit CLEAR beenden. Das Programm greift nicht auf Spielstaende zu.

Bitte `meta.txt`, `summary.csv`, `frames.csv` und `status.csv` mit derselben neuen
Laufnummer senden. Eine ZIP des Ordners geht ebenfalls.

[Messverfahren](../../docs/BENCHMARK_SUITE.md) · [Pruefbericht](VERIFICATION.md)
