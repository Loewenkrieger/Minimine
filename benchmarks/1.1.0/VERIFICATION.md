# Pruefbericht: Minimine Bench 1.1.0

- HH3: ELF32 Big Endian, SuperH EXEC, Metadatenversion 1.1.0.
- Datei: **237.892 Bytes**.
- SHA-256: `c7318c9954b172bbd26792062cf11ae28bd7685daa330a698750311dfa34ef7c`.
- Statischer RAM-Ladebereich: **236.336 / 337.920 Bytes**.
- ILRAM: **592 / 4.096 Bytes**. Azur-Queue einmalig 65.536 Heap-Bytes.
- Referenzrenderer: unveraenderte 0.5.0-Quelle mit SHA-256
  `3a103f4d480964582a63e9cebcc987d4d927c1880e29ca00a888925c7dc0cc10`.
- Beide Renderer im selben Benchmark; die zusaetzliche alte Grafikkopie ist
  nur im Vergleichsprogramm enthalten. Im Spiel bleibt der Grafikcache bei
  **20.370 Bytes** und es gibt keinen zweiten Vollbildpuffer.

Erfolgreiche ASan-/UBSan-Pruefungen am PC:

- Alle 15 Varianten in fuenf Szenen und drei Momentaufnahmen gegen die
  eingefrorene Referenz; native/2x Bildpunkte und vollstaendige Ausgabe geprueft.
- Zusaetzlich 1.028 pixelgleiche Alt/Neu-Bilder mit allen Mobtypen,
  Blickrichtungen, Laufbildern, Trefferzustaenden, Licht-/Kameraoffsets und Raendern.
- Echte Menue-Steuerung: Kurzvergleich ist Standard, eine Wiederholung,
  20 vollstaendige Bloecke und 1.200 Messbilder. Die protokollierten
  Simulationsschritte stimmen mit den 60 verteilten Stichproben ueberein.
- Abbruch vor und waehrend der Messung, gesicherte Teilmessungen und Beenden.
- CSV-Werte, FPS/P95, kurze Schreibvorgaenge, Platzmangel und Schliessfehler.
- Vollstaendige Spieltests einschliesslich Speicherformat, Inventar,
  Touchbedienung, Langzeitpruefungen und bisheriger Benchmarkroute bestanden.
- Menuevorschau visuell kontrolliert: vier Varianten, eine Wiederholung,
  Kurzvergleich als sichtbare Voreinstellung.

Die echte CPU-Ausgabe und die Geschwindigkeit von 0.5.1 muessen auf dem CP400
gemessen werden. Die Hostpruefung simuliert die Hardwareausgabe. Es wird kein
FPS-Gewinn behauptet, bevor die neuen Geraetelogs vorliegen.
