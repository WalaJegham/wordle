#define _POSIX_C_SOURCE 200809L // UTILISER LES FONCTIONS POSIX REVISEES 2008-2009
#define _DEFAULT_SOURCE         // IMPORTER LES FONCTIONS USUELLES CELLES DE LINUX/BSD PROVIDED
#define WORD_LEN 5              // Longueur du mot à deviner
#define MAX_GUESSES 6          // Nombre maximum d'essais

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

static bool cursorHidden = false; // Mémorise si le curseur est actuellement caché
// Cache le curseur (seulement s'il ne l'est pas déjà)
void hideCursor(void)
{
  if (!cursorHidden)
  {
    printf(HIDE_CURSOR);
    fflush(stdout);
    cursorHidden = true;
  }
}
// Réaffiche le curseur (seulement s'il était caché)
void showCursor(void)
{
  if (cursorHidden)
  {
    printf(SHOW_CURSOR);
    fflush(stdout);
    cursorHidden = false;
  }
}
// Appelée automatiquement à la fin du programme (atexit) pour rendre le curseur
void cleanup(void)
{
  showCursor();
}
// Pause en millisecondes (convertit ms en secondes + nanosecondes pour nanosleep)
void sleepMs(long ms)
{
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000L;
  nanosleep(&ts, NULL);
}
// Choisit un mot aléatoire de 5 lettres dans le fichier et le copie dans 'mot'
bool pick_random_word(const char *nomfichier, char *mot)
{
  FILE *f = fopen(nomfichier, "r");
  if (f==NULL)
  {
    printf("Impossible d'ouvrir le fichier%s\n",nomfichier);
    return false;
  }
  char ligne[64];
  int compt = 0;
  // compter les mots de 5 caracteres
  while (fgets(ligne, sizeof(ligne), f) != NULL)
  {
    ligne[strcspn(ligne, "\r\n")] = '\0'; // strip nouveau ligne
    if (strlen(ligne) == WORD_LEN)
    {
      compt++;
    }
  }
  if (compt == 0) // Aucun mot valide dans le fichier
  {
    fclose(f);
    return 0;
  }
  int alea = rand() % compt; // indexe aléatoire dans [0, compt-1]
  // Reparcours de fichier et arret dans cette indexe
  rewind(f); // Retour au début du fichier
  int i = 0;
  while (fgets(ligne, sizeof(ligne), f) != NULL)
  {
    ligne[strcspn(ligne, "\r\n")] = '\0';
    if (strlen(ligne) != WORD_LEN) // On ignore les mots qui n'ont pas 5 lettres
    {
      continue;
    }
    if (i == alea) // C'est le mot tiré au sort: on le copie et on termine
    {
      strcpy(mot, ligne);
      fclose(f);
      return true;
    }
    i++;
  }
  fclose(f);
  return false;
}
/*const char *words[] = {
  "eagle","crane","house","plant","water","light","chair",
  "tiger","bread","stone","knife","lemon"
};*/

// #define WORD_COUNT (sizeof(words)/sizeof(words[0]))
// Enlève le '\n' (et le '\r' sous Windows) à la fin d'une chaîne lue avec fgets
void removeNewLine(char *s)
{
  // en windows une nouvelle ligne se fait par \r\n
  int len = strlen(s);
  if (len > 0 && s[len - 1] == '\n')
  {
    s[len - 1] = '\0';
  }
  len = strlen(s);
  if (len > 0 && s[len - 1] == '\r')
  {
    s[len - 1] = '\0';
  }
}
// Vérifie que la proposition fait exactement 5 caractères et que ce sont tous des lettres
bool validGuess(const char *guess)
{
  if (strlen(guess) != WORD_LEN)
    return false;
  for (int i = 0; i < WORD_LEN; i++)
  {
    if (!isalpha((unsigned char)guess[i]))
      return false;
  }
  return true;
}
// Compare la proposition au mot secret et remplit result[] (EXACT / PRESENT / ABSENT)
void evaluateGuess(const char *word, const char *guess, LetterState result[WORD_LEN])
{
  int remaining[26] = {0}; // Compte les lettres du mot secret pas encore "consommées"
  // 1ère passe: repérer les lettres bien placées (vertes)
  for (int i = 0; i < WORD_LEN; i++)
  {
    if (guess[i] == word[i])
    {
      result[i] = EXACT;
    }
    else
    {
      result[i] = UNKNOWN;
      remaining[word[i] - 'a']++; // Cette lettre du mot reste disponible pour un "jaune"
    }
  }
  // 2ème passe: pour les autres, jaune si la lettre reste disponible, sinon gris
  for (int i = 0; i < WORD_LEN; i++)
  {
    if (result[i] == EXACT)
    {
      continue;
    }
    int index = guess[i] - 'a';
    if (remaining[index] > 0)
    {
      result[i] = PRESENT;
      remaining[index]--; // On consomme la lettre (gère les lettres doublées)
    }
    else
    {
      result[i] = ABSENT;
    }
  }
}
// Met à jour les couleurs du clavier sans jamais "rétrograder" une touche (vert > jaune > gris)
void updateKeyboard(const char *guess, const LetterState result[WORD_LEN], LetterState keyboard[26])
{
  for (int i = 0; i < WORD_LEN; i++)
  {
    int index = guess[i] - 'a';
    if (result[i] > keyboard[index]) // On ne garde que le meilleur état trouvé
    {
      keyboard[index] = result[i];
    }
  }
}
// Remet toutes les touches du clavier à l'état inconnu (début de partie)
void initKeyboard(LetterState keyboard[26])
{
  for (int i = 0; i < 26; i++)
  {
    keyboard[i] = UNKNOWN;
  }
}
// Dessine une case: fond 'bg', couleur de texte 'fg'. Si filled=false, case vide sans lettre
void drawTile(const char *bg, const char *fg, char letter, bool filled)
{
  if (filled)
  {
    printf("%s%s%s %c %s", bg, fg, BOLD, (char)toupper((unsigned char)letter), RESET);
  }
  else
  {
    printf("%s  %s ", bg, RESET);
  }
}
// Dessine une case du plateau avec la couleur correspondant à son état
void drawStateTile(LetterState state, char letter)
{
  switch (state)
  {
  case EXACT: // Vert
    drawTile(GREEN_BG, WHITE_FG, letter, true);
    break;
  case ABSENT: // Gris
    drawTile(GRAY_BG, WHITE_FG, letter, true);
    break;
  case PRESENT: // Jaune
    drawTile(YELLOW_BG, WHITE_FG, letter, true);
    break;
  default: // Pas encore évaluée
    drawTile(EMPTY_BG, WHITE_FG, letter, true);
  }
}
// Dessine une touche du clavier selon l'état de la lettre (gris clair si jamais utilisée)
void drawKey(LetterState state, char letter)
{
  switch (state)
  {
  case EXACT:
    drawTile(GREEN_BG, WHITE_FG, letter, true);
    break;
  case PRESENT:
    drawTile(YELLOW_BG, WHITE_FG, letter, true);
    break;
  case ABSENT:
    drawTile(GRAY_BG, WHITE_FG, letter, true);
    break;
  default:
    drawTile(KEY_BG, WHITE_FG, letter, true);
  }
}
/*Légende compacte pour que le joueur puisse savoir les coleurs*/
void printLegend(void)
{
  printf("            "); // Marge à gauche
  drawTile(GREEN_BG, WHITE_FG, 'A', true);
  printf(HUD_COLOR " Bien Placee   " RESET);
  drawTile(YELLOW_BG, WHITE_FG, 'B', true);
  printf(HUD_COLOR " Mauvaise Place  " RESET);
  drawTile(GRAY_BG, WHITE_FG, 'C', true);
  printf(HUD_COLOR " Inexistant  " RESET);
}
void clearScreen(void)
{
  printf("\033[2J\033[H"); //\033[2J=Blank screen et \033[H=déplacer le cureur à gauche en haut
  fflush(stdout);          // Pour ne pas avoir un delai et forcer le texte immédiatement
}

// Titre degradé
void printFancyTitle(void)
{
  static const int c[6][3] = {
      {88, 178, 132},
      {122, 184, 96},
      {176, 188, 84},
      {214, 178, 86},
      {226, 148, 92},
      {214, 116, 122}}; // Matrice de 3 triplets RGB(static pour le créer une seule fois)
  const char *t = "WORDLE";
  printf("          \t\t"); // Marge à gauche pour centrer le titre
  for (int i = 0; i < 6; i++)
  {
    printf("\033[48;2;%d;%d;%dm\033[38;2;255;255;255m\033[1m %c \033[0m",
           c[i][0], c[i][1], c[i][2], t[i]);
  } // chaque lettre tiendra un couleur spécifique
}
// En-tête du jeu: ligne vide + titre + ligne vide
void printHeader(void)
{
  printf(CLEAR_TO_EOL "\n"); // supprimer les "Leftovers" des dessins précedents
  printFancyTitle();
  printf(RESET CLEAR_TO_EOL "\n");
  printf(CLEAR_TO_EOL "\n");
}
// Redessine tout l'écran de jeu (plateau + légende + clavier + message 'status')
// filledrows = nb de lignes déjà évaluées | animrow = ligne en cours d'animation (-1 si aucune)
// revealCount = nb de cases déjà révélées pendant l'animation | flashIndex = case qui clignote (-1 si aucune)
void renderGame(const char guesses[][WORD_LEN + 1], const LetterState results[][WORD_LEN],
                int filledrows, int animrow, int revealCount, int flashIndex,
                const LetterState keyboard[26], const char *status)
{
  printf(CURSOR_HOME); // On repart du haut de l'écran et on réécrit par-dessus (évite le scintillement)
  printHeader();

  // Dessin du plateau: une ligne par essai
  for (int r = 0; r < MAX_GUESSES; r++)
  {
    printf("           "); // Marge à gauche
    for (int c = 0; c < WORD_LEN; c++)
    {
      if (r < filledrows) // Ligne déjà jouée: couleurs finales
      {
        drawStateTile(results[r][c], guesses[r][c]);
      }
      else if (r == animrow) // Ligne en cours d'animation
      {
        if (c < revealCount) // Cases déjà révélées
          drawStateTile(results[r][c], guesses[r][c]);
        else if (c == flashIndex) // Case qui clignote en blanc
          drawTile(FLASH_BG, BLACK_FG, guesses[r][c], true);
        else // Cases pas encore révélées
          drawTile(EMPTY_BG, WHITE_FG, guesses[r][c], true);
      }
      else // Ligne pas encore jouée: cases vides
      {
        drawTile(EMPTY_BG, WHITE_FG, ' ', false);
      }
    }
    printf(RESET CLEAR_TO_EOL "\n");   // outside the tile loop
  }

  printf(CLEAR_TO_EOL "\n");
  printLegend();
  printf(RESET CLEAR_TO_EOL "\n");
  printf(CLEAR_TO_EOL "\n");

  // Dessin du clavier AZERTY (3 rangées)
  const char *rows[3] = {"AZERTYUIOP", "QSDFGHJKLM", "WXCVBN"};
  int indents[3] = {4, 4, 12};          // in characters, each key is 4 wide
  for (int r = 0; r < 3; r++)
  {
    printf("%*s", indents[r], ""); // Décalage de la rangée (espaces)
    for (int i = 0; rows[r][i] != '\0'; i++)
    {
      char ch = rows[r][i];
      drawKey(keyboard[ch - 'A'], ch); // 'A'->0, 'B'->1 ... pour retrouver l'état de la lettre
    }
    printf(RESET CLEAR_TO_EOL "\n");
  }

  printf(CLEAR_TO_EOL "\n");
  printf("%s", status);                  // Message en bas (invite, erreur, résultat...)
  printf(CLEAR_TO_EOL CLEAR_TO_END);     // Efface tout ce qui reste en dessous
  fflush(stdout);                        // Force l'affichage immédiat
}
// Animation de révélation d'une proposition, case par case (flash puis couleur)
void animateGuess(char guesses[][WORD_LEN + 1], LetterState results[][WORD_LEN], int row,
                  LetterState keyboard[26])
{
  renderGame(guesses, results, row, row, 0, -1, keyboard, ""); // Affiche la ligne sans révélation
  sleepMs(250);
  for (int c = 0; c < WORD_LEN; c++)
  {
    renderGame(guesses, results, row, row, c, c, keyboard, "");     // Flash blanc sur la case c
    sleepMs(125);
    renderGame(guesses, results, row, row, c + 1, -1, keyboard, ""); // La case c prend sa couleur
    sleepMs(200);
  }
  updateKeyboard(guesses[row], results[row], keyboard); // Le clavier ne change qu'après l'animation
  renderGame(guesses, results, row + 1, -1, 0, -1, keyboard, ""); // Affichage final de la ligne
  sleepMs(100);
  showCursor();
}
// Écran d'accueil avec les règles du jeu
void printInstructions(void)
{
  clearScreen();
  printf(CLEAR_TO_EOL "\n");
  printFancyTitle();
  printf(RESET CLEAR_TO_EOL "\n");
  printf(CLEAR_TO_EOL "\n");
  printf(TITLE_COLOR "           COMMENT JOUER??" RESET CLEAR_TO_EOL "\n");
  printf(CLEAR_TO_EOL "\n");
  printf(HUD_COLOR "             Deviner le mot de 5 Lettres dans 5 essais." RESET CLEAR_TO_EOL "\n");
  printf(HUD_COLOR "             Apres chaque essaie, les couleurs des cadres vous disent si vous votre proposition est vraie ou pas:" RESET CLEAR_TO_EOL "\n");
  printf(CLEAR_TO_EOL "\n");
  printf("               ");
  drawTile(GREEN_BG, WHITE_FG, 'A', true);
  printf(HUD_COLOR " Vert - la lettre existe dans le mot et bien placee" RESET CLEAR_TO_EOL "\n");
  printf("               ");
  drawTile(YELLOW_BG, WHITE_FG, 'B', true);
  printf(HUD_COLOR " Jaune - la lettre existe dans le mot mais mal placee" RESET CLEAR_TO_EOL "\n");
  printf("               ");
  drawTile(GRAY_BG, WHITE_FG, 'C', true);
  printf(HUD_COLOR " Gris - la lettre n'existe pas dans le mot" RESET CLEAR_TO_EOL "\n");
  printf(CLEAR_TO_EOL "\n");
  printf(HUD_COLOR "          Veuillez donner votre proposition et appuyez Entrer. Ecrire q pour quitter." RESET CLEAR_TO_EOL "\n");
  printf(CLEAR_TO_EOL "\n");
  printf(PROMPT_COLOR "        Appuyez Entrer pour commencer..." RESET CLEAR_TO_EOL "\n");
  fflush(stdout);
}
