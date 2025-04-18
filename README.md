<p align="center"><img src="res/img/titlescreen_big.png" height="300px" alt=""/></p>
<h1 align="center">Tome of the Vastlands: Scourge of Greed</h1>
<p align="center"><strong><code>Vastlands-GB</code></strong></p>
<p align="center">Ein natives Game Boy Color Spiel</p>
<p align="center">
  <img src="https://img.shields.io/maintenance/yes/2025"  alt=""/>
</p>

## Einleitung

*TODO*

<p align="center"><img src="/static/imgs/logo.png" height="100px" alt=""/></p>

## Mitwirkende

| Mitwirkende          | GitHub                                                                                                                            |
|----------------------|-----------------------------------------------------------------------------------------------------------------------------------|
| Luke Grasser (Zetsu) | <a href="https://github.com/zetsuboushii"><img src="https://avatars.githubusercontent.com/u/65507051?v=4" width="100px;" alt=""/> |

## Installation

Zu Beginn das Projekt klonen. Es werden zudem folgende Abhängigkeiten benötigt:

* [GBDK 2020](https://github.com/gbdk-2020/gbdk-2020)

## Bauen der ROM

Das Bauen der Game Boy Color ROM erfolgt über CMake.

Im `build`-Verzeichnis folgendes aufrufen:

```shell
cmake ..
make
```

Damit wird eine `.gbc`-Binary erstellt.

Für aktive Entwicklung und insbesondere Debugging empfiehlt sich das Shell-Skript `hotreload.sh` auszuführen:

```shell
bash hotreload.sh
```

## Ausführen der ROM

Die ROM kann über jeden Game-Boy-Emulator geladen werden, sowie (unter Umständen) auf echter Hardware mittels einer
Flash-Cartridge (keine Garantie).

Zur Entwicklung empfehlen sich folgende Emulatoren:

**Windows**

* [bgb](https://bgb.bircd.org/) (für Genauigkeit)
* [Emulicious](https://emulicious.net/) (für Debugging)

**Linux**

* [Emulicious](https://emulicious.net/)

Zum Ausprobieren und Spielen; Emulatoren für alle gängigen Plattformen:

* [RetroArch](https://www.retroarch.com/) (Emulatoren-Frontend für Convenience)
* [Visual Boy Advance](https://visualboyadvance.org/)
* [mGBA](https://mgba.io/)

**Original-Hardware**

Zum authentischen Spielen auf echter Hardware können Flashcards verwendet werden. Die ROM sollte auf allen Geräten
funktionieren, die den Game Boy Color unterstützen, spricht primär der Game Boy Color selbst und der Game Boy Advance