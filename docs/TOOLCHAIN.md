# Eingerichtete ClassPad-Bauumgebung

Nach Freigabe am 31. Oktober 2026 in der vorhandenen WSL-Distribution `kali-linux`
eingerichtet und mit dem Projekt geprüft.

| Komponente | Version |
| --- | --- |
| Linux-Containerlaufzeit | Docker Engine / CLI 28.5.2+dfsg4 |
| SuperH-Compiler | sh-elf-gcc 14.1.0 |
| fxSDK | 2.11.0 |
| gint | 2.11.0-91 |
| CMake | 3.31.6 |
| Bauplattform | Linux x86_64 |

Docker Engine und CLI wurden mit ihren benötigten Paketabhängigkeiten in Kali
installiert. Die Paketlisten wurden vorher aktualisiert. Die eigentliche
ClassPad-Werkzeugkette liegt im heruntergeladenen Community-Containerimage.

## Festgehaltenes Image

Quelle: [ClassPad-Community: Anwendungen bauen](https://classpad.dev/wiki/gint/getting-started/building/)
und [gint-docker-Quellprojekt](https://github.com/TheRainbowPhoenix/gint-docker).

```text
ghcr.io/therainbowphoenix/gint-docker@sha256:9f358bfc47ee95cfd8ee973aabdd641d995ca4bdd1324c92dc1bd87c78d8661e
```

`tools/toolchain-image.txt` ist die maßgebliche Einstellung. Die Bauhelfer nutzen
das bereits installierte Image mit `--pull never`; eine spätere Änderung des
Tags `main` verändert den Projektbuild deshalb nicht automatisch.

## Bauablauf

1. Unter Windows `tools/build.ps1` aufrufen. Der Helfer startet bei Bedarf die
   vorhandene Docker Engine in `kali-linux` und ruft den Linux-Bauhelfer auf.
2. Der Container erhält den Projektordner lesend und `build-cp` als Ausgabeordner.
   Netzwerkzugriff und zusätzliche Container-Capabilities sind abgeschaltet.
3. `tools/container-build.sh` kopiert die benötigten Quellen in das temporäre
   Linux-Dateisystem. Das vermeidet die unterschiedlichen Dateieigentümer und
   die eingeschränkten Berechtigungsoperationen eines Windows-/DrvFS-Ordners.
4. CMake und fxSDK bauen das Programm. Der HH3-Ausgabeordner wird ausdrücklich
   auf das Bauverzeichnis gesetzt, weil fxSDK relative HH3-Ausgabepfade sonst
   gegen den Quellordner auflöst.
5. `tools/verify_hh3.py` prüft Dateiformat, Metadaten und Ladebereiche. Zusätzlich
   werden `sh-elf-readelf`- und Abschnittsgrößenberichte erzeugt.
6. Die geprüften Ergebnisse werden in `build-cp` exportiert. Der temporäre
   Container wird anschließend entfernt. Das Werkzeugkettenimage bleibt für
   weitere Builds vorhanden.

## Ergebnis

Zwei frische Builds, davon einer über den Windows-Helfer, erzeugten die identische
37.732 Byte große `.hh3`. Hauptladebereich: 92.400 von 337.920 Byte belegt.
Prüfsumme und weitere Angaben stehen in `docs/VERIFICATION.md` und im erzeugten
`build-cp/build-info.json`.

Ein Start auf dem echten CP400 und echte Performance-Werte stehen noch aus.
