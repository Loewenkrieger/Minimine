# Geraetetest fuer Minimine 0.5.3

Die PC-Tests und der Geraetebau stehen in [VERIFICATION.md](VERIFICATION.md).
Diese Liste ist fuer den echten ClassPad II fx-CP400 mit Hollyhock 3 vorgesehen.
Noch keine Pruefung dieser Version am echten Geraet wurde durchgefuehrt.

| Angabe | Wert |
| --- | --- |
| Hollyhock-/OS-Version | |
| Spielversion | 0.5.3; SHA-256 siehe releases/0.5.3/build-info.json |
| Datum, Takt, Einstellungen | |
| Werkzeugkette | SuperH GCC 14.1.0, fxSDK 2.11.0, gint 2.11.0-91 |

## Schwerpunkt 0.5.3

- [ ] Bestehenden 0.5.0-/0.5.1-/0.5.2-Spielstand laden; Welt und Mengen unveraendert.
- [ ] Mobs nach links/rechts, beide Laufbilder und Trefferblinken vergleichen.
- [ ] Teilweise sichtbare Mobs an allen vier Bildschirmraendern pruefen.
- [ ] Helle, halbdunkle und dunkle Bereiche bei Kamerabewegung vergleichen.
- [ ] Zeilenlicht zeigt dieselben Lichtstufen ohne Streifen oder Randfehler.
- [ ] Bekannten Spielstand an derselben Position pruefen, FPS mit 0.5.2 vergleichen.
- [ ] Oberflaeche, Hoehle mit Fackeln und Szene mit mehreren Mobs vergleichen.
- [ ] Inventar/Pause oeffnen und schliessen; Licht und Anzeigen werden korrekt erneuert.

Bench 1.2.0 wurde bereits vollstaendig auf dem CP400 ausgefuehrt und ausgewertet:
[900 Messbilder und Ergebnisse](../benchmarks/1.2.0/DEVICE_RESULTS.md).
Die rund 18 Prozent gelten fuer die beiden beleuchteten Tests. Fuer 0.5.2 hat
der Benutzer im normalen Spiel rund 9 FPS gemeldet; ein direkter Vergleich des
gleichen Spielstands mit 0.5.3 steht aus. Der Kurzbenchmark muss dafuer nicht
erneut ausgefuehrt werden.

| Position im selben Spielstand | FPS 0.5.2 | FPS 0.5.3 |
| --- | ---: | ---: |
| Oberflaeche | | |
| Hoehle mit Fackeln | | |
| Mobs und Beleuchtung | | |

## Start, Bewegung und Zielen

- [ ] `.hh3` startet im Launcher; Startmenue bietet neues Spiel und Laden.
- [ ] Startinventar: je 32 Erde, Stein und Bretter; Hotbar zeigt Namen.
- [ ] Links/rechts waehlt unmittelbar die benachbarte Spalte, auch an einer Wand.
- [ ] 1/3 Fuss-, 4/6 Augenhoehe, 7/9 eine Reihe ueber dem Kopf; 2 unter den Fuessen,
      8 ueber dem Kopf. Auch an Blockgrenzen, im Lauf und geduckt pruefen.
- [ ] Hochbauen: Block auswaehlen, 2 halten, Hoch/SHIFT und EXE. Mindestens drei
      Spruenge nacheinander auf den neu gesetzten Bloecken landen; mit gehaltenem
      und kurz gedruecktem EXE sowie Sneak wiederholen. Kein Block im Koerper.
- [ ] Gehaltenes manuelles Zielen hat Vorrang. Nach Loslassen: im Lauf nach vorn,
      im Stand zuletzt gewaehlte Richtung.
- [ ] Hoch und SHIFT: ein Sprung je Druck, keiner durch Halten nach der Landung.
- [ ] Runter: Sneak einmal je Druck; kleiner, halbes Tempo, Fuesse/Kamera fest.
- [ ] Sneak stoppt an beiden Kanten, erlaubt Spruenge, faellt bei entferntem Boden.
- [ ] Niedriger Gang passierbar; versuchtes Aufrichten zeigt Kein Platz und bleibt
      geduckt. Nach dem Gang erneut Runter druecken.

## Touchscreen

- [ ] Start-/Pausenmenue: jeden Eintrag direkt antippen, ohne Pfeil-/EXE-Auswahl.
- [ ] Speichern/Laden: Slots, Zurueck sowie Ja/Nein in Rueckfragen antippen.
- [ ] Hilfe/Ergebnisse: Spielen, Benchmark und Menue per Touch erreichbar.
- [ ] Inventar: Quelle/Ziel antippen und alternativ Stapel per Ziehen absetzen.
- [ ] DIV halten beim Ziehen bzw. Quellen-/Zieltippen: 32 gibt 16 ab, 33 gibt 17
      ab, ein einzelnes Item bleibt nicht haengen. SHIFT wechselt nur Ruestung.
- [ ] Gleiche Stapel bis 64 zusammenfuehren; Rest bleibt liegen. Ganze Stapel
      tauschen, halbe Stapel bei anderem Material abweisen.
- [ ] Ganze/halbe Stapel zwischen Hotbar, Rucksack, Crafting und Ofen bewegen.
      Einzelne verschlissene Ausruestung in passende Slots ziehen und zurueck.
- [ ] Loslassen ausserhalb, auf einer Vorschau oder einem ungueltigen Slot
      verliert nichts. Gezogenes Ergebnis stellt nichts her, Antippen einmal.
- [ ] Touchschaltflaechen fuer Ruestung und Schliessen funktionieren.
- [ ] Getrennte Beruehrungen: Halten/Loslassen fuehrt keine doppelte Aktion aus;
      Oeffnen eines Untermenues aktiviert dort nicht versehentlich einen Eintrag.
- [ ] CLEAR, dann Beenden antippen: sofort zurueck zum Launcher, auch ungespeichert.
      Kein automatisches Speichern. Dasselbe per Pfeile/EXE pruefen.

## Inventar, Rezepte und Haltbarkeit

- [ ] Keyboard/CLEAR schliesst; Pfeile bedienen ausschliesslich das Menue.
- [ ] EXE Quelle/Ziel, 5 einzelnes Item; gleiche Quelle mit EXE abwaehlen.
- [ ] X/Y/Z holt Hotbarstapel zum Cursor, aendert Cursor und aktiven Platz nicht.
      Leeres Ziel, Tausch, 64er-Grenze mit Rest und ungueltiges Ergebnisziel pruefen.
- [ ] SHIFT zeigt vier Ruestungsslots und wieder zurueck. Zutaten bleiben erhalten.
      SHIFT springt dort nicht; Hoch/Runter aendern Bewegung/Sneak nicht.
- [ ] Quickmove zu Crafting, passendem Ruestungsslot und Ofen funktioniert;
      falscher Ruestungstyp, falscher Brennstoff und eingehender Ausgangstransfer scheitern.
- [ ] Haltbarkeit bleibt beim Verschieben/Tauschen zwischen allen Bereichen erhalten.
- [ ] Alle Rezepte aus [ANLEITUNG.md](ANLEITUNG.md) herstellen; verschobene Muster,
      falsche Materialien, Zusatzfelder, fehlender Platz und Zutatenrueckgabe pruefen.
- [ ] Werkbank platzieren, EXE loslassen, neu druecken: 3x3-Feld oeffnet.
- [ ] Menue lange offen lassen, dann EXE/DEL gehalten schliessen: kein Nachlauf,
      keine versehentliche Aktion; erst nach Loslassen wieder aktiv.

## Abbauen, Erz und Oefen

- [ ] DEL-Zeiten laut Anleitung; Fortschrittsleiste auch an Bildschirmraendern sichtbar.
- [ ] Loslassen, Ziel-/Werkzeugwechsel und Menues loeschen den Fortschritt.
- [ ] Ohne Spitzhacke kein Stein/Kohle-Drop; Eisenerz nur mit Steinspitzhacke
      zu rohem Eisen. Holzspitzhacke reicht fuer Kohle.
- [ ] Kohle und Eisenerz sind optisch unterscheidbar und in Stein vorhanden.
- [ ] Letzter Werkzeuggebrauch liefert Drop, danach Bruch. Abbruch und volles
      Inventar kosten keine Nutzung. Grundgestein bleibt bestehen.
- [ ] Ofen platzieren und oeffnen: Eingang/Brennstoff/Ausgang und beide Leisten.
- [ ] Eisen und alle drei Fleischsorten in zehn Spielsekunden verarbeiten.
- [ ] Kohle 80 s, Stamm/Bretter 15 s, Stock 5 s. Zwei Stoecke fuer einen Vorgang pruefen.
- [ ] Mehrere Oefen arbeiten bei geschlossenem Menue weiter, in Menues nicht.
- [ ] Voller Ausgang haelt Fortschritt an; Restfeuer laeuft weiter. Anderer Eingang
      setzt Fortschritt zurueck. Neuer Brennstoff startet nur bei gueltigem Vorgang.
- [ ] Ofenabbau ohne Spitzhacke scheitert. Bei fehlendem Platz bleibt Ofen samt
      Inhalt bestehen; bei Platz wird alles einmalig eingesammelt.
- [ ] 32 Oefen moeglich; weitere Platzierung verbraucht kein Item.

## Mobs, Kampf und Ueberleben

- [ ] Schwein, Kuh und Huhn oberirdisch; Zombies oberirdisch und in Hoehlen.
- [ ] Keine Zombiespawns innerhalb zwoelf Bloecken um den Startpunkt.
- [ ] Tiere wandern und fliehen nach Treffern; Zombies verfolgen und springen
      ein Block hohes Hindernis. Entfernte Mobs verschwinden.
- [ ] Leere Hand und alle drei Schwerter schlagen mit EXE, gehalten wiederholt.
- [ ] Nur naechster erreichbarer Mob getroffen; Waende verhindern Treffer.
- [ ] Mob hat Vorrang vor Werkbank/Ofen. Nach Tod oeffnet gehaltenes EXE kein Menue;
      neu druecken erlaubt Interaktion. Schwert verliert nur bei Treffern Haltbarkeit.
- [ ] Lebensleisten nur bei verletzten Mobs, auch am Rand innerhalb des Ausschnitts.
- [ ] Tiere hinterlassen Fleisch, bei vollem Inventar bleibt Beute liegen.
- [ ] Zehn Herz-/Hungersymbole sichtbar; Hunger nach Minute und je 20 Aktionen pruefen.
- [ ] Regeneration ab 18 Hunger, kein Essenverbrauch bei vollem Hunger, Hunger-Schaden
      bei null. Alle rohen/gebratenen Sorten fuellen die richtigen Punkte.
- [ ] Zombie-Hit: zwei Schaden ohne Ruestung; Schutzzeit verhindert schnelle Mehrfachtreffer.
- [ ] Eisenruestung wirkt nur im passenden Slot, kompletter Satz 60 Prozent Schutz;
      Teile verschleissen nur bei erlittenem Mobtreffer. Letzte Nutzung schuetzt noch.
- [ ] Tod: freie Startposition, volles Leben/Hunger, fuenf Sekunden Schutz; Inventar
      und Ausruestung mit unveraenderter Resthaltbarkeit erhalten.

## Speichern und Laden auf dem CP400

- [ ] Vorhandene Spielstaende von 0.4.0 und 0.4.1 lassen sich unveraendert laden.

- [ ] Erstes Speichern erzeugt `/Minimine` ohne vorbereitete Zusatzdateien.
- [ ] Alle drei Slots schreiben und nach Beenden/Neustart zurueckladen.
- [ ] Welt, Position/Bewegung, ausgewahlte Hotbar, Ruestung/Verschleiss, Leben/Hunger,
      Mobs/Beute sowie mehrere laufende Oefen stimmen nach Laden.
- [ ] Spielzeit und Ofenfortschritt holen die ausgeschaltete Zeit nicht nach.
- [ ] Ueberschreiben und Verwerfen ungespeicherter Aenderungen: Nein vorausgewaehlt,
      Nein/CLEAR behaelt das Spiel, Ja fuehrt die Aktion aus.
- [ ] Je Slot entstehen wechselnde A/B-Dateien; nach wiederholtem Speichern wird
      der neueste gueltige Stand geladen.
- [ ] Mit vorheriger Sicherung der sechs Dateien eine Kopie am PC beschaedigen:
      andere Kopie laedt mit Sicherungshinweis. Beide defekt: Fehler, laufendes Spiel bleibt.
- [ ] Fehlender Speicherplatz meldet Fehler, bisherige gueltige Kopie bleibt ladbar.
      Speicherunterbrechungen nur mit vorheriger Sicherung kontrolliert testen.

## Darstellung, Last und Vergleich

- [ ] Keine Grafikreste nach Menues, saubere Transparenz, neue Itemnamen gut lesbar.
- [ ] Mehrere Mobs, Beute und Oefen gleichzeitig: Bedienung, FPS und Simulationsverlust notieren.
- [ ] 0 startet getrennten Benchmark, Spielzustand bleibt erhalten; EXE setzt fort.
- [ ] Weiterhin drei Szenen, 30 Aufwaerm- und 150 Messbilder je Szene, Sneak aus.

Drei Laeufe unter denselben Einstellungen wie 0.1.0 fotografieren.
Ausgangswerte stehen in [BENCHMARK.md](BENCHMARK.md).

| Lauf | Szene | FPS | Mittel ms | P95 ms | Max ms | L us | Z us | LCD us |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 1 | Oberflaeche | | | | | | | |
| 1 | Hoehlen | | | | | | | |
| 1 | Tiefengestein | | | | | | | |
| 2 | Oberflaeche | | | | | | | |
| 2 | Hoehlen | | | | | | | |
| 2 | Tiefengestein | | | | | | | |
| 3 | Oberflaeche | | | | | | | |
| 3 | Hoehlen | | | | | | | |
| 3 | Tiefengestein | | | | | | | |

Beobachtungen:


## Expedition, Basisbau und Boss (0.5.0)

- [ ] Neues Spiel: Ruinen, Basalt und Kristallerz sind vorhanden; alte Welten
      laden unveraendert und behalten ihre bisherige Darstellung.
- [ ] Vier Fackeln aus Kohle und Stock herstellen. Handlicht, platzierte Fackel,
      drei Helligkeitsstufen und Lichtgrenzen bei Kamerabewegung pruefen.
- [ ] Platzierte Fackeln verhindern neue feindliche Spawns, vorhandene Mobs
      bleiben aktiv. Fackeln verbrennen nicht und lassen sich wieder aufnehmen.
- [ ] Versorgungskammer, Werkstatt und drei Reliktruinen erkunden. Truheninhalte
      bleiben nach Entnahme und nach Speichern/Laden erhalten.
- [ ] Kiste herstellen, platzieren, per EXE oeffnen. Zwoelf Plaetze, ganze/halbe
      Touchtransfers, 5, Quickselect und Schliessen pruefen.
- [ ] Volle Kiste abbauen: nur bei Platz fuer Kiste und gesamten Inhalt, keine Verluste.
- [ ] Tuer belegt zwei Felder; oeffnen/schliessen per EXE. Spieler oder Mob im
      Durchgang verhindert Schliessen; beide Haelften lassen sich gemeinsam abbauen.
- [ ] Bett belegt zwei Felder. EXE setzt Rueckkehrpunkt; Tod, blockiertes Bett und
      Bettabbau pruefen. Inventar, Ruestung und Zubehoer bleiben erhalten.
- [ ] Eisenspitzhacke herstellen; Basalt/Kristallerz nur damit abbauen.
- [ ] Ruestungsansicht mit sechs Slots: nur passendes Zubehoer, kein doppeltes
      Anlegen desselben Stuecks; Touch, DIV und Quickselect erhalten Mengen.
- [ ] Bergmannsanhaenger: jeder fuenfte Hackenabbau ohne Verschleiss. Stiefel:
      schneller laufen und Sneak. Amulett: genau ein weiterer Luftsprung.
- [ ] Zombies: erkennbare Angriffsvorbereitung, Trefferblitz, Rueckstoss.
      Fledermaus: Warten, Warnung, gerader Angriff und Erholung.
- [ ] Alle drei Relikte sammeln; Zaehler und Hinweis im CLEAR-Menue stimmen.
      Siegel verlangt Relikte im Inventar und oeffnet nach EXE dauerhaft.
- [ ] Waechter beim Betreten sichtbar und beleuchtet. Warnung, Ansturm,
      Bodenschlag, Sprung zum Ausweichen und Erholung pruefen.
- [ ] Verlassen/Tod setzt nur den Kampf zurueck; Zugang bleibt offen.
- [ ] Sieg gibt einmalig Amulett in der Belohnungstruhe und schaltet Rezept frei.
      Volle/entfernte Truhe: Belohnung bleibt vorgemerkt.
- [ ] Alle neuen Zustaende speichern/laden: Lichtquellen, Kisten, Tuerhaelften,
      Bett, Zubehoer, Relikte, laufender Bosskampf, Sieg und ausstehende Belohnung.
- [ ] Vorhandene 0.4.2-Spielstaende laden und anschliessend als Format 2 speichern.
- [ ] Unter vielen Lichtquellen, Mobs und Oefen FPS und Simulationsverlust notieren.
      Alten Vergleichsbenchmark separat ausfuehren; er verwendet keine neuen Systeme.
