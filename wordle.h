#define _POSIX_C_SOURCE 200809L // UTILISER LES FONCTIONS POSIX REVISEES 2008-2009
#define _DEFAULT_SOURCE         // IMPORTER LES FONCTIONS USUELLES CELLES DE LINUX/BSD PROVIDED
#define WORD_LEN 5              // Longueur du mot à deviner
#define MAX_GUESSES 5           // Nombre maximum d'essais

#include <stdio.h>   // printf, fgets, fopen...
#include <stdlib.h>  // malloc, free, rand, srand, atexit
#include <stdbool.h> // type bool (true/false)
#include <string.h>  // strlen, strcpy, strcmp, strcspn
#include <time.h>    // time (pour le seed) et nanosleep (pour les pauses)
#include <ctype.h>   // isalpha, toupper, tolower

/*033=ESC en code ASCII c'est le caractére de début d'une commande dans le terminal*/
#define RESET "\033[0m" // Reset(ANSI ESCAPE CODES)==>TEXT STYLES
#define BOLD "\033[1m"  // Texte en gras
#define DIM "\033[2m"   // Texte atténué (non utilisé pour l'instant)

#define GREEN_BG "\033[48;2;83;141;78m" // 48=SET BACKGROUND ET 2=RGB(5=256 couleurs palette) ET 83;141;78 C'EST LE COULEUR
#define YELLOW_BG "\033[48;2;181;159;59m"
#define GRAY_BG "\033[48;2;58;58;60m"
#define EMPTY_BG "\033[48;2;22;22;24m"  // Fond d'une case vide / pas encore évaluée
#define FLASH_BG "\033[48;2;255;255;255m" // Fond blanc: effet de "flash" pendant l'animation
#define KEY_BG "\033[48;2;129;131;132m"   // Fond d'une touche du clavier pas encore utilisée
#define WHITE_FG "\033[38;2;255;255;255m" // 48=SET FOREGROUND ET 2=RGB(5=256 couleurs palette) ET 83;141;78 C'EST LE COULEUR
#define BLACK_FG "\033[38;2;20;20;20m"

// Couleurs des textes (titre, aide, invite, erreur, succès)
#define TITLE_COLOR "\033[1;38;2;255;255;255m"
#define HUD_COLOR "\033[38;2;170;172;178m"
#define PROMPT_COLOR "\033[1;38;2;185;159;59m"
#define ERROR_COLOR "\033[1;38;2;230;90;90m"
#define SUCCESS_COLOR "\033[1;38;2;120;200;120m"

// Commandes de contrôle du curseur / effacement
#define CURSOR_HOME "\033[H"     // Curseur en haut à gauche (sans effacer)
#define CLEAR_TO_EOL "\033[K"    // Efface jusqu'à la fin de la ligne
#define CLEAR_TO_END "\033[J"    // Efface jusqu'à la fin de l'écran
#define HIDE_CURSOR "\033[?25l"  // Cache le curseur
#define SHOW_CURSOR "\033[?25h"  // Affiche le curseur
// déclaration d'un nouveau type pour les différentes état qu'une lettre peut etre
// (l'ordre compte: UNKNOWN < ABSENT < PRESENT < EXACT, utilisé pour comparer les états)
typedef enum
{
  UNKNOWN,
  ABSENT,
  PRESENT,
  EXACT
} LetterState;
void printInstructions(void);
void animateGuess(char guesses[][WORD_LEN + 1], LetterState results[][WORD_LEN], int row,LetterState keyboard[26]);
void renderGame(const char guesses[][WORD_LEN + 1], const LetterState results[][WORD_LEN],int filledrows, int animrow, int revealCount, int flashIndex,const LetterState keyboard[26], const char *status);
void printHeader(void);
void printFancyTitle(void);
void clearScreen(void);
void printLegend(void);
void drawKey(LetterState state, char letter);
void drawStateTile(LetterState state, char letter);
void drawTile(const char *bg, const char *fg, char letter, bool filled);
void initKeyboard(LetterState keyboard[26]);
void updateKeyboard(const char *guess, const LetterState result[WORD_LEN], LetterState keyboard[26]);
void evaluateGuess(const char *word, const char *guess, LetterState result[WORD_LEN]);
bool validGuess(const char *guess);
void removeNewLine(char *s);
bool pick_random_word(const char *nomfichier, char *mot);
void sleepMs(long ms);
void cleanup(void);
void showCursor(void);
void hideCursor(void);