# Wordle (C)

A terminal version of Wordle written in C. Guess the secret 5-letter word in 5 tries, with colored tiles, an on-screen AZERTY keyboard that tracks the letters you've used, and a tile-by-tile reveal animation.

The in-game text is in French.

## How to play

1. Start the game and press **Enter** on the instructions screen.
2. Type a 5-letter word and press **Enter**.
3. Each letter is colored to give you a hint:

| Color  | Meaning                                       |
| ------ | --------------------------------------------- |
| Green  | The letter is in the word and in the right spot |
| Yellow | The letter is in the word but in the wrong spot |
| Gray   | The letter is not in the word                 |

4. You have **5 attempts** to find the word.
5. Type `q` at any prompt to quit.
6. After each game, answer `O` to play again or anything else to stop.

Invalid guesses (not exactly 5 letters, or non-letters) show an error and don't use up an attempt.

## Features

- Colored tiles using ANSI true-color escape codes
- Animated reveal, one tile at a time
- AZERTY on-screen keyboard that never "downgrades" a key (green > yellow > gray)
- Correct handling of repeated letters (e.g. guessing `speed` against `abide`)
- Random word picked from `source.txt` each game
- Replay loop, with a tailored congratulation message depending on how many tries you needed

## Project structure

```
.
├── main.c        # Game loop: input, win/lose logic, replay
├── wordle.c      # Rendering, animation, word picking, guess evaluation
├── wordle.h      # Constants, color macros, LetterState enum, prototypes
├── source.txt    # Word list (one 5-letter word per line)
└── .vscode/      # VS Code build task (MSYS gcc on Windows)
```

## Requirements

- A C compiler (e.g. `gcc`)
- A terminal that supports ANSI escape codes and 24-bit color (most modern terminals do: Windows Terminal, iTerm2, GNOME Terminal, VS Code's terminal, etc.)

## Build and run

From the project folder:

```bash
gcc -Wall -o wordle main.c wordle.c
./wordle
```

On Windows (MSYS / UCRT64 shell):

```bash
gcc -Wall -o wordle.exe main.c wordle.c
./wordle.exe
```

> `source.txt` is read from the current directory, so run the game from the project folder.

### Building in VS Code

The included task builds only the active file (`${file}`). To build the whole project, either use the command above in the terminal, or change the task's `args` to compile both source files:

```json
"args": ["-g", "${fileDirname}\\main.c", "${fileDirname}\\wordle.c", "-o", "${fileDirname}\\wordle.exe"]
```

## Customizing

Settings live at the top of `wordle.h`:

| Constant      | Default | Description                   |
| ------------- | ------- | ----------------------------- |
| `WORD_LEN`    | `5`     | Length of the secret word     |
| `MAX_GUESSES` | `6`     | Number of attempts per game   |

To change the vocabulary, edit `source.txt` (one lowercase word per line; lines that aren't exactly `WORD_LEN` letters are ignored).

## How guess evaluation works

`evaluateGuess` in `wordle.c` works in two passes:

1. Mark every letter in the correct position as green and count the remaining unmatched letters of the secret word.
2. For the other letters, mark yellow if that letter is still available in the count (and consume it), otherwise gray.

This is what makes duplicate letters behave like the real Wordle.
