# Bernas Career

**Your game. Your story.** A football player career simulator for the **web, Wii and GameCube**.

Create your player, train your attributes, simulate matches, build your reputation and earn a move to a new club. The console editions are native C++ homebrew applications built with **devkitPPC, libogc and libfat**.

This is the **English edition**. The web interface, console menus, notifications and documentation are in English. Save files remain compatible with the Portuguese edition.

## Platforms

| Platform | Start here | Technology |
|---|---|---|
| Web | Open [`web/index.html`](web/index.html) in a browser | HTML, CSS and JavaScript; no dependencies |
| Wii | [`releases/wii/apps/bernas-career/boot.dol`](releases/wii/apps/bernas-career/boot.dol) | devkitPPC + libogc + libfat |
| GameCube | [`releases/gamecube/bernas-career.dol`](releases/gamecube/bernas-career.dol) | devkitPPC + libogc + libfat |

Matches are simulated through statistics. This project does not include playable 3D football matches, online multiplayer or live football data.

## Features

- Custom player name, starting club and position: striker, attacking midfielder or left winger.
- Eight clubs in a fictional league with a complete 14-matchday, home-and-away schedule.
- Position-specific overall rating, weekly training, energy recovery, morale and XP progression.
- Match ratings, goals, assists, league standings, weekly wages and career earnings.
- Transfer offers after matchdays 4, 8 and 12 when your level matches an interested club.
- Transfers that take effect at the beginning of the following season.
- Career totals, league titles and a history of the last eight completed seasons.
- A shared text save format for the web and both consoles.

Club names identify teams in a fictional simulation. The league, strengths, wages and results are invented. This project is not affiliated with the clubs and does not include official club crests.

## Play in a browser

Download and extract the project, then open **`web/index.html`** in Chrome, Edge or Firefox. No package installation or server is required.

Select **New career** to choose your name, position and starting club. Train or recover once per matchday, then simulate the next fixture. After all 14 matchdays, start a new season to record the result in your history and apply any agreed transfer.

Progress is saved in this browser when local storage is available. Use **Export career** to keep a separate save before clearing browser data or switching devices. Player names are converted to plain Latin characters for compatibility with the console font.

The `web/` directory also works on a static web host. The root `index.html` redirects to it. A ready-made console download is included at `web/bernas-career-wii-gamecube.zip`.

## Play on Wii

1. Copy the **`apps`** directory from `releases/wii/` to the root of a FAT32 SD card.
2. Check that the game is at **`SD:/apps/bernas-career/boot.dol`**.
3. Open **Bernas Career** through the Homebrew Channel already installed on your Wii.

The package includes an English `meta.xml` description and an icon. Use a Wii Remote held vertically, or a GameCube controller on a Wii model with compatible controller ports.

## Play on GameCube

Open **`releases/gamecube/bernas-career.dol`** through Swiss or another homebrew loader that accepts DOL files. Connect a GameCube controller to port 1.

Saving needs a FAT device supported by the libfat/libogc configuration, such as a compatible SD Gecko setup. Support depends on the adapter and loader. **Ordinary Memory Cards are not supported. SD2SP2 has not been validated.** You can play a session without saving; the footer reports when saving is unavailable.

## Controls

| Action | Wii Remote | GameCube controller |
|---|---|---|
| Switch tab | D-pad left/right | D-pad or stick left/right |
| Select an option | D-pad up/down | D-pad or stick up/down |
| Confirm, train or simulate | A | A |
| Return to the career dashboard | B | B |
| Save manually | + | START |
| Load a saved career | − | X |
| Exit | HOME | Z |

In the native player setup, select **Name** and press A. Left/right moves the cursor; up/down changes the character; A confirms and B cancels. Starting a new career replaces the existing save.

## Build the console versions

Install the official [devkitPro toolchain](https://devkitpro.org/) with the `wii-dev` and `gamecube-dev` package groups. On Windows, run the commands in the **devkitPro MSYS2 terminal**.

```sh
pacman -S --needed wii-dev gamecube-dev
```

From this repository, with `DEVKITPRO` and `DEVKITPPC` configured:

```sh
make -C native PLATFORM=wii
make -C native PLATFORM=gamecube
```

Outputs:

```text
native/bernas-career-wii.dol
native/bernas-career-gamecube.dol
```

For example, on Windows, a project extracted to `C:\Projects\BernasCareer` is reached from MSYS2 with:

```sh
cd /c/Projects/BernasCareer
make -C native PLATFORM=wii
make -C native PLATFORM=gamecube
```

Each target uses the official rules and libraries for that platform. Do not substitute the GameCube DOL for the Wii DOL. The native UI uses GX and a built-in bitmap font; GRRLIB, SDL and external game assets are not required.

## Test the simulation

With Python 3, Node.js and a host C++17 compiler installed:

```sh
python3 tools/test.py
```

The runner builds a temporary host test binary and checks the native engine, all 56 directed league fixtures, standings, the weekly activity limit, save validation and six complete seasons. It then requires the C++ and JavaScript engines to produce exactly the same save after 84 matches and transfer decisions.

## Share saves between platforms

1. Export a `.bcareer` file from the web version.
2. Back up any existing console save.
3. Rename the exported file to **`bernas-career.sav`** and put it at the root of the FAT device used by the console game. On Wii, prefer the SD root.
4. Choose **Continue career**, or use the load button during play.

To return to the web version, copy `bernas-career.sav` from the console to your computer and use **Import career**.

The native game tries `sd:`, `usb:`, `carda:`, `cardb:` and the default device root. Do not remove the storage device during a write. The save header is `BERNASCAREER 1`; translated text is not part of the serialized simulation state.

## Repository layout

```text
native/include/career.hpp     Portable C++ simulation and save format
native/include/renderer.hpp   GX renderer and bitmap font
native/source/main.cpp       Console UI, input and FAT storage
native/Makefile               Separate Wii and GameCube targets
web/                         English browser game
tests/                       Engine and C++/JavaScript parity tests
tools/test.py                Host test runner
tools/package_release.py     Package existing native builds
assets/icon.png              Homebrew Channel icon
releases/                    Prebuilt English console binaries
```

After building both consoles, run `python3 tools/package_release.py` to refresh the prebuilt files, the web console download and the release archives in `dist/`. Packaging uses only the Python standard library.

To upload this project to GitHub, use the contents of this repository directory so **`README.md`**, **`native/`** and **`web/`** appear at the repository root. No account credentials, hosting project identifiers or compiler installation are included.

## Validation and limitations

Both English DOL files were compiled with **devkitPPC r51 / GCC 16.2.0**, **libogc 3.2.1** and **libfat**. Their DOL section boundaries and entry points were checked. Host engine tests and exact C++/JavaScript save parity pass. The English interface was checked through a browser.

**The DOL files have not been tested on real consoles or in Dolphin.** The game is a statistical career simulator with a deliberately small fictional competition.

## License

Original project code is distributed under the [MIT License](LICENSE). Third-party names belong to their respective owners. The build tools and linked libraries retain their own licenses and are not bundled as a compiler installation.

Official references: [devkitPro](https://devkitpro.org/), [Wii examples](https://github.com/devkitPro/wii-examples), [GameCube examples](https://github.com/devkitPro/gamecube-examples), [devkitPPC rules](https://github.com/devkitPro/devkitppc-rules).
