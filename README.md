## HEXAMIX - A Color Merge Puzzle

![HEXAMIX - A Color Merge Puzzle](screenshots/hero.png "HEXAMIX - A Color Merge Puzzle")

### Description

HEXAMIX is a strategic hexagon placement puzzle built around color mixing.

You are dealt primary colored hexagons — red, yellow and blue — one at a time, and you choose where to drop them on a hexagonal board. The moment a piece lands, it merges with **every** compatible neighbor at once, and the cell settles on the color that took part in the most merges. Secondary and tertiary colors are never handed to you: they can only be *earned* by mixing.

The reward grows exponentially with the size of the merge, so the game is a constant tension between playing it safe and setting up a big combo. Cross a score threshold and the board grows into a larger one, scattering whatever you left behind onto it — so today's mess becomes tomorrow's obstacle. Tertiary colors are dead ends that merge with nothing, slowly clogging the board. The game ends when there is nowhere left to place a piece.

Every so often a wildcard hexagon appears in the queue, pulsing and cycling through every hue. It blends with any color, and it is the only way to clear the dead-end tertiaries.

### Features

 - Star merge: a placed piece merges with all of its compatible neighbors simultaneously
 - A 12-color RYB wheel — primaries mix into secondaries, secondaries into tertiary dead ends
 - Exponential scoring: every extra merge doubles the reward, up to 64 points in a single move
 - Eight progressively larger boards, from 7 cells up to 721
 - Leftover pieces carry over into the next board, scattered to new positions
 - A rainbow wildcard hexagon that blends with any color, including tertiary dead ends
 - Built-in illustrated help pages explaining the merge rules

### Controls

Mouse:
 - Left Button: start the game, and place the current piece on an empty cell

Keyboard:
 - H: open and close the help screen
 - Left / Right Arrow: navigate between help pages
 - S: toggle the background music
 - R: restart after a game over
 - Ctrl + R: restart at any time

Editor (debug):
 - E: toggle editor mode
 - Right Button: paint the selected color onto a cell
 - Middle Button: clear a cell
 - Mouse Wheel: cycle through the available colors

### Screenshots

Pieces merge with every compatible neighbor at once, and crossing a score threshold grows the board, scattering the leftover pieces onto it:

![Merging and advancing to the next board](screenshots/merge-and-transition.gif "Merging and advancing to the next board")

The rainbow wildcard hexagon blends with any color, clearing the dead-end tertiaries that clog the board:

![The special hexagon in action](screenshots/special-and-transition.gif "The special hexagon in action")

### Developers

 - David Buzatto - Programming, arts, sounds, game design... All :D

### Links

 - YouTube Gameplay: https://youtu.be/AnvAnZ4pfSw
 - itch.io Release: https://davidbuzatto.itch.io/hexamix

### License

This project sources are licensed under an unmodified zlib/libpng license, which is an OSI-certified, BSD-like license that allows static linking with closed source software. Check [LICENSE](LICENSE) for further details.

Additional licenses:

 - Fredoka font by Milena Brandão and Hafontia, licensed under the SIL Open Font License. See [OFL.txt](resources/fonts/Fredoka/OFL.txt).
 - Music and sound effects by [Kenney](https://kenney.nl), released under the CC0 license.

*Copyright (c) 2026 @davidbuzatto*
