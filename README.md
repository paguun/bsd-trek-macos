# BSD Trek for macOS

[日本語の導入・操作ガイド](README-ja.md)

Play the classic BSD **Trek** in the macOS Terminal. This project adapts the
original C sources distributed by NetBSD, with a small compatibility header,
a standalone Makefile, and zsh launchers.

The original navigation, combat, events, difficulty and scoring routines are
retained. The save implementation has been adapted so games can be restored
after restarting the process on systems that randomise memory addresses (ASLR).

## Requirements

- A Mac with Intel or Apple Silicon and Apple Command Line Tools.
- Terminal and zsh, as supplied with macOS.

Homebrew and Python are not required to play. The game is compiled on your Mac;
this repository does not include a prebuilt executable.

Install the Command Line Tools if necessary:

```zsh
xcode-select --install
```

## Build and play

```zsh
git clone https://github.com/paguun/bsd-trek-macos.git
cd bsd-trek-macos
zsh ./run-trek.zsh
```

Alternatively, download **Code → Download ZIP**, extract it, open Terminal in
the extracted directory and run `zsh ./run-trek.zsh`.

The launcher builds the game when required. It runs from the project directory
so that the save file has a consistent location. You can also build and run
manually with `make` followed by `./bin/trek`.

At startup, press Return and try:

```text
What length game: short
What skill game: novice
Enter a password: captain
```

The password is only for the game's self-destruct command. Use an arbitrary
game password. The interface and game commands are in English.

## Essential commands

| Command | Action |
| --- | --- |
| `?` | List valid commands or choices at the current prompt |
| `srscan` | Scan the current quadrant |
| `status` | Display ship status |
| `lrscan` | Scan neighbouring quadrants |
| `computer chart` | Display the known galaxy |
| `computer trajectory` | Calculate courses to local Klingon ships |
| `warp 5` | Set the warp factor |
| `move 90 1` | Travel one quadrant to the right |
| `shields down` / `shields up` | Lower or raise shields |
| `phasers automatic 1000` | Fire phasers using 1,000 energy units |
| `torpedo 90 no` | Fire one photon torpedo to the right |
| `damages` | Display damage and repair information |
| `dock` / `undock` | Dock at an adjacent starbase or leave it |
| `dump` | Save the game |
| `terminate` | End the current game |

Courses are in degrees: **0 up, 90 right, 180 down, 270 left**. Distances are in
quadrants; `0.1` is one sector. Phasers require shields to be down.
`help` requests emergency starbase rescue; use `?` for a command list.

See the [Japanese guide](README-ja.md), the original man page (`man ./trek.6`),
and Eric Allman's original manual source in `docs/trek.me` for more detail.

## Save and restore

Type `dump` at the game's command prompt. The launcher stores `trek.dump` in
the project directory. Saving again replaces this single save slot.

To exit after saving, type `terminate` and answer `no` to `Another game:`.
To restore, launch the game, press Return, and enter `restart` at
`What length game:`. Keep the save file when moving the project directory.

Save files are intended for the same build and architecture. Saves from other
versions of BSD Trek are not compatible. The original random-number generator
state was not included in the upstream save format and is not included here.

## Scope of the port

- `src/compat.h`: NetBSD compatibility macros, attributes and constants.
- `Makefile`: a standalone build using the system C compiler and `make`.
- `src/dumpgame.c`: saves event references as indices, restores runtime
  pointers (including the time-warp snapshot), and writes saves through a
  temporary file before replacing the previous save.
- `run-trek.zsh` and `Trek.command`: build and launch helpers.

The other 47 upstream C files are unchanged. Existing upstream behaviour and
bugs are not comprehensively addressed by this port.

## Validation and bug reports

The game has been reported to launch successfully on a user's Mac. The macOS
version and processor architecture of that report have not yet been recorded;
compatibility across macOS versions and both architectures has not been verified.

Linux/GCC checks cover startup, scans, navigation, shields, impulse movement,
rest, torpedo firing, save/restore across processes, snapshot pointer rebinding,
and rejection of incomplete or incompatible save files.

Optional regression checks require Python 3:

```sh
python3 tests/check.py
```

Please [open an issue](https://github.com/paguun/bsd-trek-macos/issues) with your
macOS version, processor type, compiler output if relevant, and steps to
reproduce a problem.

## Origin and licence

Trek was written by **Eric Allman**, with contributions from Jeff Poskanzer,
Pete Rubinstein, Steve Bourne and later BSD maintainers.

This port uses a snapshot downloaded on **12 October 2026** from the
[NetBSD Trek source directory](https://cdn.netbsd.org/pub/NetBSD/NetBSD-current/src/games/trek/).
It belongs to the BSD Trek family; individual BSD and Linux `bsdgames`
distributions may differ.

Upstream copyright notices and BSD 3-Clause licence terms are retained in the
source files. Local port additions use the same terms. See [LICENSE.txt](LICENSE.txt).
Original-file checksums, the upstream Makefile and the save-code patch are in
`docs/`.
