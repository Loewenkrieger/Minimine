# Minimine 0.5.0 - Anleitung

## Start und Spielstaende

Nur [CasioSandbox.hh3](../releases/0.5.0/CasioSandbox.hh3) auf den ClassPad II
fx-CP400 kopieren und mit Hollyhock 3 starten. Das Startmenue bietet neues Spiel,
Laden, Hilfe, Benchmark und Beenden. Aeltere Versionen bleiben unter releases.

CLEAR pausiert das Spiel. Dort gibt es Fortsetzen, Speichern, Laden, Hilfe und
Beenden. Alle Eintraege und Schaltflaechen koennen direkt angetippt werden.
Alternativ waehlen Pfeile, EXE bestaetigt und CLEAR kehrt zurueck.

Beenden schliesst sofort, auch bei ungespeicherten Aenderungen. Es gibt keine
Rueckfrage und kein automatisches Speichern. Zum schnellen Beenden CLEAR druecken
und Beenden antippen.

Speichern und Laden bieten jeweils drei Slots. Vor Ueberschreiben eines Slots
oder Verwerfen des laufenden Spiels beim Laden erscheint weiterhin eine
Bestaetigung. Nein ist vorausgewaehlt; Ja/Nein koennen angetippt werden.
Spielstaende von 0.4.0 bis 0.4.2 bleiben ladbar. Ihre Welt wird nicht umgebaut:
Neue Ruinen, Basaltschicht und Bosskammer erfordern ein neues Spiel.
In alten Welten bleiben die bisherigen Lichtverhaeltnisse erhalten; neue
herstellbare Bauteile und Ausruestung sind dort ebenfalls benutzbar.

0.5.0 schreibt Speicherformat 2. Nach erneutem Speichern kann eine alte
Spielversion die neue Datei nicht lesen; vorhandene Format-1-Dateien werden
zunaechst als die bisherige A/B-Kopie erhalten.

**Manuell speichern, bevor das Spiel beendet oder der Rechner ausgeschaltet wird.**
Beim ersten Speichern entsteht `/Minimine`. Je Slot werden zwei wechselnde
Dateien angelegt: `slot1a.sav` / `slot1b.sav` bis Slot 3. Beide Dateien eines Slots
zusammen sichern. Beim Laden wird die neueste gueltige Kopie verwendet; eine
beschaedigte Kopie kann durch die andere aufgefangen werden. Meldet das Spiel
einen Fehler, bleibt das laufende Spiel erhalten. Unbekannte Dateiversionen
werden abgewiesen.

Gespeichert werden Welt, Bewegung, Inventar, Verschleiss, Ruestung, Leben,
Hunger, Mobs, liegende Beute, Ofeninhalte und -fortschritt sowie Spielzeit und
Zufallszustand. Abbaufortschritt und Menueauswahl starten nach Laden neu.
Inventar, Ofenansicht und alle Menues pausieren die gesamte Spielzeit; auch
waehrend das Spiel ausgeschaltet ist, arbeiten Oefen nicht weiter.

## Steuerung

| Taste | Funktion |
| --- | --- |
| Links / rechts | Laufen; Ziel auf die entsprechende Seite setzen |
| Hoch / SHIFT | Ein Sprung pro Tastendruck |
| Runter | Sneak einmal ein- oder ausschalten |
| X / Y / Z | Hotbarplatz 1 / 2 / 3 aktivieren |
| Keyboard | Inventar mit 2x2-Craftingfeld oeffnen / schliessen |
| DEL halten | Anvisierten Block abbauen |
| EXE druecken / halten | Schlagen, essen oder bauen |
| EXE neu druecken | Werkbank / Ofen / Kiste oeffnen, Tuer / Bett / Siegel benutzen |
| 0 | Benchmark |
| CLEAR | Pausenmenue; im Inventar schliessen |

Das Ziel liegt in der unmittelbar an den Koerper angrenzenden Blockspalte.
Automatisch wird auf Koerpermitte gezielt, auch beim Laufen gegen eine Wand.
Gehaltene Zahlentasten haben Vorrang. Nach Loslassen zeigt das Ziel beim Laufen
wieder nach vorn; im Stand bleibt die letzte Richtung erhalten.

| Links | Rechts | Hoehe |
| --- | --- | --- |
| 1 | 3 | Fuesse |
| 4 | 6 | Augen |
| 7 | 9 | Eine Blockreihe ueber dem Kopf |

2 zielt auf die erste vollstaendige Blockreihe unter den Fuessen, auch waehrend
eines Sprungs. Zum Hochbauen einen Block in der Hotbar auswaehlen, mit 2 nach
unten zielen, mit Hoch oder SHIFT springen und mit EXE unter sich platzieren.
EXE kann dabei gehalten werden. Die Figur darf den neuen Block nicht ueberlappen.

8 zielt ueber den Kopf. Beim Ducken koennen Augen und Fuesse in dieselbe
Blockreihe fallen. Nach einer Menuepause EXE und DEL zuerst
loslassen; erst danach werden diese Aktionen wieder freigegeben.

## Inventar, Ruestung und Crafting

### Touchbedienung

- Einen belegten Slot beruehren und zum Ziel ziehen, dort loslassen. Erst beim
  Loslassen wird verschoben; der Zielslot wird beim Ziehen hervorgehoben.
- Alternativ zuerst die Quelle und danach das Ziel antippen. Ein erneutes
  Antippen derselben Quelle hebt die Markierung auf.
- Fuer einen halben Stapel die Taste **DIV (geteilt durch)** beim Touchvorgang
  gedrueckt halten. Beim Tippen reicht dies beim Quellen- oder Zieltippen.
  Ungerade Mengen werden aufgerundet: 33 gibt 17 ab, 16 bleiben zurueck.
  Ein einzelnes Item wird vollstaendig bewegt, Haltbarkeit bleibt erhalten.
- Ganze Stapel duerfen leere Ziele belegen, gleiche Stapel bis 64 ergaenzen oder
  andere Stapel tauschen. Halbe Stapel gehen nur in leere oder passende Ziele.
  Reicht deren Platz nicht, wird nur der passende Anteil verschoben.
- Loslassen ausserhalb eines Slots bricht eine Ziehbewegung ohne Transfer ab.
  Unpassende Ruestung, ungueltiger Brennstoff und eingehende Transfers auf
  Ergebnisplaetze werden abgewiesen.
- Im Craftingfeld das Ergebnis antippen, um einmal herzustellen. Eine gezogene
  Rezeptvorschau erzeugt keine Items. Der Ofenausgang laesst sich normal entnehmen.
- Die Schaltflaechen unten wechseln zur Ruestung/zurueck oder schliessen das
  Inventar. SHIFT bleibt unveraendert fuer den Ruestungswechsel.

### Tasten und Stapel

Drei Hotbarplaetze und zwoelf Inventarplaetze stehen zur Verfuegung. Die Hotbar
zeigt Namen und Anzahl. Start: je 32 Erde, Stein und Holzbretter. Materialien
passen bis 64 in einen Slot; Spitzhacken, Schwerter und Ruestung einzeln.

- Pfeile bewegen die Auswahl, ohne die Figur zu bewegen.
- EXE markiert eine Quelle. EXE am Ziel verschiebt den ganzen Stapel. Derselbe
  Quellplatz hebt die Markierung auf.
- 5 verteilt ein Item aus der markierten Quelle; die Quelle bleibt markiert.
- Leere Ziele uebernehmen, gleiche Materialien werden bis 64 zusammengefuehrt,
  andere Stapel getauscht. Rest bleibt in der Quelle. 5 tauscht nicht.
- X/Y/Z holen den ganzen Stapel des jeweiligen Hotbarplatzes **zum Cursor**.
  Cursor und aktiver Hotbarplatz bleiben unveraendert; die Quellmarkierung wird
  aufgehoben. Das funktioniert auch fuer passende Crafting-, Ofen- und
  Ruestungsplaetze. Ergebnisplaetze erlauben keine eingehenden Transfers.
- SHIFT zeigt Helm, Brustpanzer, Hose, Stiefel und zwei Zubehoerplaetze; erneut SHIFT kehrt zurueck.
  Zutaten bleiben liegen, die Quellmarkierung wird aufgehoben.
- EXE auf dem Craftingergebnis stellt das Rezept einmal her. Vorschau verbraucht
  nichts. Keyboard/CLEAR schliesst und gibt uebrige Zutaten zurueck.

Fehlt Platz fuer Ergebnis oder Zutatenrueckgabe, bleibt alles erhalten. Die
Rueckgabekapazitaet wird auch bei Transfers in Ruestung oder Ofen geprueft.
Haltbarkeit bleibt beim Verschieben, Tauschen und Speichern erhalten.

Aufgesammelte Items ergaenzen zuerst passende Hotbarstapel, danach passende
Inventarstapel, danach leere Inventarplaetze und zuletzt leere Hotbarplaetze.
Verbrauchte Hotbarplaetze werden nicht automatisch nachgefuellt.

### Rezepte

Muster duerfen innerhalb des Feldes verschoben sein, soweit sie hineinpassen.
Zusaetzliche Zutaten verhindern das Rezept. Punkt bedeutet leer.

| Ergebnis | Muster / Zutaten | Feld |
| --- | --- | --- |
| 4 Bretter | 1 Baumstamm | 2x2 / 3x3 |
| 4 Stoecke | 2 Bretter uebereinander | 2x2 / 3x3 |
| Werkbank | 4 Bretter im 2x2-Quadrat | 2x2 / 3x3 |
| Holz- / Steinspitzhacke | `MMM / .S. / .S.`; M = Bretter / Stein | Werkbank |
| Holz- / Stein- / Eisenschwert | `M / M / S`; M = Bretter / Stein / Eisenbarren | Werkbank |
| Ofen | `MMM / M.M / MMM`; M = Stein | Werkbank |
| Eisenhelm | `III / I.I` | Werkbank |
| Eisenbrustpanzer | `I.I / III / III` | Werkbank |
| Eisenhose | `III / I.I / I.I` | Werkbank |
| Eisenstiefel | `I.I / I.I` | Werkbank |

S = Stock, I = Eisenbarren. Werkbank oder Ofen in der Hotbar auswaehlen,
platzieren, EXE loslassen und erneut auf den Block druecken.

## Abbau und Oefen

Zum Benutzen eine Spitzhacke in der Hotbar aktivieren. Im Rucksack wirkt sie nicht.

| Block | Hand | Holzspitzhacke | Steinspitzhacke |
| --- | ---: | ---: | ---: |
| Erde / Gras | 0,35 s | 0,35 s | 0,35 s |
| Bretter / Stamm / Werkbank | 0,60 s | 0,60 s | 0,60 s |
| Stein / Kohle | 1,00 s | 1,00 s | 0,50 s |
| Eisenerz | 2,00 s | 2,00 s | 1,00 s |
| Ofen | Spitzhacke erforderlich | 1,00 s | 0,50 s |
| Blaetter | 0,15 s | 0,15 s | 0,15 s |
| Grundgestein | unabbaubar | unabbaubar | unabbaubar |

Stein und Kohle liefern erst mit Spitzhacke Items. Eisenerz liefert erst mit
Steinspitzhacke rohes Eisen. Falsches Werkzeug entfernt Stein/Erz ohne Drop.
Blaetter liefern nichts. Staemme liefern Baumstaemme, Bretterbloecke Bretter.

Holzspitzhacken halten 64, Steinspitzhacken 128 erfolgreiche Blockabbaue. Der
letzte Abbau wird noch gutgeschrieben. Abbruch, Grundgestein und fehlender Platz
kosten keine Nutzung. Loslassen, Ziel-/Werkzeugwechsel oder Menue setzen den
Fortschritt zurueck. Die kleine Leiste bleibt innerhalb des Spielausschnitts.
Ein Ofen laesst sich nur mit Platz fuer **Ofen und gesamten Inhalt** abbauen.
Mehr als 32 platzierte Oefen sind nicht moeglich; das Item bleibt dann erhalten.

Im Ofen gibt es Eingang, Brennstoff und Ausgang. Die zwei Leisten zeigen
Schmelzfortschritt und Restbrennzeit. Das geoeffnete Menue pausiert; nach dem
Schliessen arbeitet jeder Ofen an seiner Position weiter.

| Eingang | Ausgang | Dauer |
| --- | --- | ---: |
| Rohes Eisen | Eisenbarren | 10 Spielsekunden |
| Rohes Schweine- / Rind- / Huehnerfleisch | Passende gebratene Sorte | 10 Spielsekunden |

Kohle brennt 80 Sekunden, Baumstamm und Bretter jeweils 15, Stock 5 Sekunden.
Neuer Brennstoff wird nur bei gueltigem Eingang und freiem Ausgang gezuendet.
Bereits vorhandene Restbrennzeit laeuft im Spiel weiter. Ein voller Ausgang
haelt das Schmelzen an; ein anderes Eingangsmaterial setzt den Fortschritt zurueck.

## Kampf und Ueberleben

Mit leerer Hand oder Schwert schlaegt EXE. Halten wiederholt Schlaege. Es trifft
hoechstens der naechste Mob in Zielrichtung, bis 24 Pixel vor der Koerperkante.
Feste Bloecke verhindern Treffer hindurch. Ein erreichbarer Mob hat Vorrang vor
Werkbank und Ofen. Gehaltenes EXE oeffnet nach dem Kampf kein Menue automatisch.
Mit Nahrung wird gegessen, mit einem Block gebaut.

| Angriff | Schaden | Abstand | Erfolgreiche Treffer bis Bruch |
| --- | ---: | ---: | ---: |
| Hand | 1 | 0,5 s | unbegrenzt |
| Holzschwert | 4 | 0,4 s | 64 |
| Steinschwert | 5 | 0,4 s | 128 |
| Eisenschwert | 6 | 0,4 s | 256 |

Schwerter verlieren nur bei Treffern Haltbarkeit; der letzte Treffer wirkt noch voll.

Oberirdisch leben Schweine (6 Leben), Kuehe (10), Huehner (4) und Zombies (12).
In Hoehlen entstehen Zombies. Tiere fliehen nach Treffern zwei Sekunden lang.
Zombies verfolgen die Figur und koennen einzelne Blockhindernisse ueberspringen.
Es gibt maximal acht Tiere und acht feindliche Mobs, alle fuenf Spielsekunden hoechstens
einen Spawn ausserhalb des Bildes. Zwoelf Bloecke um den Startpunkt sind vor
Zombiespawns geschuetzt. Mobs weiter als 32 Bloecke entfernt verschwinden.

Tiere hinterlassen je ein rohes Fleisch. Bei Annaeherung wird es eingesammelt;
bei vollem Inventar bleibt es liegen. Nahe gleiche Beute wird zusammengefuehrt.
Es gibt hoechstens 64 Beutestapel, Verfall nach fuenf Spielminuten. Bei vollem
Beutepool wird der aelteste ersetzt. Verletzte Mobs zeigen eine Lebensleiste.

Leben und Hunger starten jeweils bei 20 Punkten, als zehn Symbole im Kopfbereich.
Pro Spielminute sinkt Hunger um eins; je 20 Spruenge oder ausgefuehrte Schlaege
verbrauchen einen weiteren Punkt. Ab 18 Hunger regeneriert alle vier Sekunden
ein Lebenspunkt; vier regenerierte Punkte verbrauchen einen Hungerpunkt.
Bei Hunger null schadet Hunger alle vier Sekunden um einen Lebenspunkt.

Rohes Schwein/Rind fuellt drei, gebratenes acht Hungerpunkte. Huhn fuellt roh
zwei, gebraten sechs. Bei vollem Hunger wird nichts verbraucht. Essen mit
gehaltenem EXE erfolgt hoechstens einmal pro Sekunde.

Zombies verursachen zwei Schaden, hoechstens einmal je Sekunde. Nach einem
Mobtreffer gilt 0,75 Sekunden Schutz vor weiteren Treffern. Beim Tod erscheint
die Figur beim Startpunkt an einer freien Position, mit vollem Leben/Hunger
und fuenf Sekunden Schutz. **Inventar und Ausruestung bleiben mitsamt Verschleiss.**

| Eisenruestung | Schutzpunkte | Haltbarkeit in Treffern |
| --- | ---: | ---: |
| Helm | 2 | 128 |
| Brustpanzer | 6 | 256 |
| Hose | 5 | 192 |
| Stiefel | 2 | 128 |

Nur im passenden Ruestungsslot wirkt Schutz: vier Prozent weniger Mobschaden je
Punkt, kompletter Satz 60 Prozent. Jeder erlittene Mobtreffer kostet jedes
getragene Teil eine Nutzung. Ein zerbrechendes Teil schuetzt bei diesem Treffer
noch. Ruestung schuetzt nicht gegen Hunger.

## Sneak, Benchmark und Entwicklung

Sneak duckt die Figur von 22 auf 14 Pixel und halbiert ihr Tempo. Fuesse und
Kamera bleiben beim Umschalten gleich. Am Boden verhindert Sneak das Laufen
ueber eine Kante. Springen behaelt Kraft und Sneak; in der Luft greift die
Kantenbremse nicht. Wird der Boden entfernt, wirkt die Schwerkraft.
Unter niedrigen Decken bleibt Sneak an; spaeter erneut Runter druecken.

Der Vergleichsbenchmark nutzt eine getrennte urspruengliche Welt ohne Kohle,
Mobs, Hunger oder Oefen. Drei alte Kamerafahrten, je 30 Aufwaermbilder und
150 Messbilder bleiben erhalten. Die laufende Spielwelt bleibt unveraendert.
60 Physikschritte pro Sekunde, maximal acht Nachholschritte pro Bild; ausgelassene
Schritte erscheinen als Simulationsverlust. Auf dem PC werden keine CP400-FPS gemessen.

Bauen: `tools/build.ps1`. Tests: `tools/test.ps1` oder `bash tools/test.sh`.
Einzelne Tests: z.B. `bash tools/test.sh survival storage`.
Siehe [TOOLCHAIN.md](TOOLCHAIN.md), [VERIFICATION.md](VERIFICATION.md),
[DEVICE_TEST.md](DEVICE_TEST.md) und [BENCHMARK.md](BENCHMARK.md).


## Expedition und Steinwaechter

Ein neuer Durchlauf fuehrt vom Lager ueber Eisenwerkzeuge und drei Relikte zur
versiegelten Kammer. Im CLEAR-Menue stehen der Reliktzaehler und das naechste Ziel.
Die bekannte Startausruestung mit je 32 Erde, Stein und Brettern bleibt erhalten.

### Licht und Minenraeume

Ein Stueck Kohle ueber einem Stock ergibt vier Fackeln, auch im 2x2-Feld.
In neuen Welten ist die direkte Umgebung ohne Fackel zwei Bloecke weit hell.
Eine ausgewaehlte Fackel erweitert den sichtbaren Bereich auf vier Bloecke.
Platzierte Fackeln reichen sechs Bloecke weit, davon drei hell und drei gedaemmt.
Sie brennen dauerhaft, sind nicht massiv und lassen sich mit DEL wieder aufnehmen.
Die Beleuchtung verwendet drei Stufen und einfache blockweise Entfernungen.

In der Reichweite platzierter Fackeln entstehen keine neuen feindlichen Mobs.
Bereits vorhandene Gegner koennen hineinlaufen. Die urspruengliche Schutzzone
um den Start bleibt unabhaengig vom Bett bestehen.

Versorgungskammern enthalten Nahrung, Kohle und Baumaterial. In der verlassenen
Werkstatt stehen Werkbank und Ofen; ihre Truhe enthaelt auch eine beschaedigte
Spitzhacke. Drei groessere Ruinen enthalten jeweils ein einzigartiges Relikt:
Mine, Tiefe und Basalt. Der Bergmannsanhaenger liegt in der ersten Reliktruhe.
Alle Inhalte werden bei der Welterstellung festgelegt und mitgespeichert.

### Basis und weitere Rezepte

| Ergebnis | Rezept an der Werkbank |
| --- | --- |
| Kiste | Acht Bretter als Ring, Mitte leer |
| Tuer | Sechs Bretter als 2x3-Rechteck |
| Bett | Drei Stoecke oben, darunter zwei Reihen mit je drei Brettern |
| Eisenspitzhacke | Drei Eisenbarren oben, darunter zwei Stoecke mittig |
| Stiefel der Eile | `I.I / I.I / C.C` |
| Sprungamulett | `.C. / CIC / .C.`; erst nach dem Sieg ueber den Steinwaechter |

I = Eisenbarren, C = Leuchtkristall, Punkt = leer.

Eine Kiste hat zwoelf Plaetze. EXE oeffnet sie; Tasten, Touch, Quickselect und
halbe Stapel mit DIV funktionieren wie im Inventar. Zum Abbau muessen Kiste
und der gesamte Inhalt ins Spielerinventar passen; sonst bleibt alles bestehen.

Eine Tuer belegt zwei Bloecke uebereinander und benoetigt Boden darunter.
EXE oeffnet oder schliesst sie. Figuren im Durchgang verhindern das Schliessen.
Zombies koennen geschlossene Tueren nicht oeffnen.

Ein Bett belegt zwei Bloecke nebeneinander auf festem Boden. EXE setzt es als
persoenlichen Rueckkehrpunkt. Ist dort beim Tod kein Platz oder wurde das Bett
abgebaut, wird eine freie Position am urspruenglichen Start gesucht.
Das Bett heilt nicht und aendert die Spielzeit nicht.

Die Welt bietet insgesamt Platz fuer 16 Kisten, 64 Fackeln und 32 Oefen,
einschliesslich vorhandener Minenobjekte. Eine weitere Platzierung verbraucht
kein Item. Gefundene Kisten, Fackeln und Oefen lassen sich wiederverwenden.

### Basalt, Kristalle und Zubehoer

Ab der tiefen Basaltschicht findet man Basalt und Kristallerz. Beide benoetigen
eine Eisenspitzhacke und bleiben bei schwaecherem Werkzeug bestehen.
Die Eisenspitzhacke haelt 256 Nutzungen. Abbauzeiten: Stein/Kohle/Ofen 0,25 s,
Eisenerz 0,5 s, Basalt 1,25 s, Kristallerz 1 s. Kristallerz liefert einen Leuchtkristall.
Die geschuetzte aeussere Wand der Bosskammer bleibt unabbaubar.

Die Ausruestungsansicht hat vier Ruestungs- und zwei Zubehoerplaetze.
Zubehoer wird einzeln getragen; dasselbe Stueck kann nicht doppelt ausgeruestet werden.

| Zubehoer | Wirkung |
| --- | --- |
| Bergmannsanhaenger | Jeder fuenfte erfolgreiche Abbau mit Spitzhacke kostet keine Nutzung |
| Stiefel der Eile | 15 Prozent hoeheres Lauftempo, auch bei weiterhin halbiertem Sneaktempo |
| Sprungamulett | Ein weiterer Sprung pro Luftphase; Landen gibt ihn wieder frei |

Alle Relikte sind ohne Sprungamulett erreichbar. Eine versteckte Truhe in der
Tiefe enthaelt weitere Kristalle und Stiefel der Eile.

### Fledermaus und Kampf

Treffer lassen Gegner kurz aufleuchten und stossen normale Gegner zurueck.
Zombies bereiten einen Nahkampfangriff kurz sichtbar mit einem Ausrufezeichen vor.
Ausweichen kann den Treffer verhindern; zwischen Angriffen bleibt ihre Pause bestehen.

Hoehlenfledermaeuse haben vier Lebenspunkte. Sie warten unter einer Decke,
kuendigen den Angriff an und fliegen geradlinig auf die erfasste Spielerposition.
Danach erholen sie sich und suchen wieder eine Position weiter oben.
Ein Treffer verursacht einen Lebenspunkt Schaden. Fackeln helfen, sie rechtzeitig zu sehen.

### Relikte und Bosskammer

Alle drei Relikte ins Spielerinventar nehmen und das Siegel an der Kammer im
Osten der Basaltschicht mit EXE benutzen. Die Relikte werden einmalig eingesetzt;
der Zugang bleibt anschliessend dauerhaft offen. Drei Relikte in einer Kiste
reichen zum Oeffnen nicht aus, der Zaehler zeigt aber weiterhin die gefundenen.

Beim Betreten beginnt der Kampf gegen den Steinwaechter mit 100 Lebenspunkten.
In der Kammer entstehen waehrend des Kampfes keine weiteren Gegner.
Er kuendigt Ansturm und Bodenschlag sichtbar an; nach jedem Angriff folgt eine
Erholungspause. Ueber den Ansturm und die Bodenwelle kann gesprungen werden.
Unter halbem Leben werden Warnung und Erholung kuerzer. Treffer im Ansturm und
Bodenwellen verursachen drei Schaden vor Ruestung, sonstiger Kontakt zwei.

Verlassen der Kammer oder Tod beendet den Versuch. Der naechste beginnt wieder
mit einem vollstaendig geheilten Waechter; der Zugang bleibt offen. Inventar und
Ausruestung bleiben beim Tod wie bisher erhalten.

Ein Sieg wird dauerhaft gespeichert. Die Belohnungstruhe enthaelt ein
Sprungamulett; zusaetzlich wird dessen Rezept freigeschaltet. Ist die Truhe voll
oder entfernt, bleibt die Belohnung vorgemerkt, bis am Belohnungsplatz eine
Kiste mit freiem Slot bereitsteht oder wieder entstehen kann.
Laufende Kampfphasen und eine noch ausstehende Belohnung werden mitgespeichert.
