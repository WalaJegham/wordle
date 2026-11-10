#define _POSIX_C_SOURCE 200809L //UTILISER LES FONCTIONS POSIX REVISEES 2008-2009
#define _DEFAULT_SOURCE //IMPORTER LES FONCTIONS USUELLES CELLES DE LINUX/BSD PROVIDED
#define WORD_LEN 5
#define MAX_GUESSES 5


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

/*033=ESC en code ASCII c'est le caractére de début d'une commande dans le terminal*/
#define RESET     "\033[0m" //Reset(ANSI ESCAPE CODES)==>TEXT STYLES
#define BOLD      "\033[1m"
#define DIM       "\033[2m"


#define GREEN_BG  "\033[48;2;83;141;78m"//48=SET BACKGROUND ET 2=RGB(5=256 couleurs palette) ET 83;141;78 C'EST LE COULEUR
#define YELLOW_BG "\033[48;2;181;159;59m"
#define GRAY_BG   "\033[48;2;58;58;60m"
#define EMPTY_BG  "\033[48;2;22;22;24m"
#define FLASH_BG  "\033[48;2;255;255;255m"
#define KEY_BG    "\033[48;2;129;131;132m"
#define WHITE_FG  "\033[38;2;255;255;255m"//48=SET FOREGROUND ET 2=RGB(5=256 couleurs palette) ET 83;141;78 C'EST LE COULEUR
#define BLACK_FG  "\033[38;2;20;20;20m"


#define TITLE_COLOR   "\033[1;38;2;255;255;255m"
#define HUD_COLOR     "\033[38;2;170;172;178m"
#define PROMPT_COLOR  "\033[1;38;2;185;159;59m"
#define ERROR_COLOR   "\033[1;38;2;230;90;90m"
#define SUCCESS_COLOR "\033[1;38;2;120;200;120m"


#define CURSOR_HOME   "\033[H"
#define CLEAR_TO_EOL  "\033[K"
#define CLEAR_TO_END  "\033[J"
#define HIDE_CURSOR   "\033[?25l"
#define SHOW_CURSOR   "\033[?25h"

typedef enum{UNKNOWN, ABSENT, PRESENT, EXACT } LetterState;
bool pick_random_word(const char *nomfichier,char *mot){
  FILE *f=fopen(nomfichier,"r");
  char ligne[64];
  int compt=0;
  //compter les mots de 5 caracteres
  while(fgets(ligne,sizeof(ligne),f)!=""){
    ligne[strcspn(ligne,"\r\n")]='\0';//strip nouveau ligne
    if(strlen(ligne)==WORD_LEN){compt++;}
  }
  if(compt==0){fclose(f);return 0;}
  int alea=rand() % compt;//indexe aléatoire dans [0, compt-1]
  //Reparcours de fichier et arret dans cette indexe
  rewind(f);
  int i=0;
  while(fgets(ligne,sizof(ligne),f)!=""){
    ligne[strcspn(ligne,"\r\n")]='\0';
    if(strlen(ligne)!=WORD_LEN){continue;}
    if(i==alea){
      strcpy(mot,ligne);
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

//#define WORD_COUNT (sizeof(words)/sizeof(words[0]))

void clearScreen(void){
  printf("\033[2J\033[H");//\033[2J=Blank screen et \033[H=déplacer le cureur à gauche en haut
  fflush(stdout);//Pour ne pas avoir un delai et forcer le texte immédiatement
}

//Titre degradé
void printFancyTitle(void){
  static const int c[6][3]={
    {88,178,132},
    {122,184,96},
    {176,188,84},
    {214,178,86},
    {226,148,92},
    {214,116,122}
  };//Matrice de 3 triplets RGB(static pour le créer une seule fois)
  const char *t="WORDLE";
  printf("          ");
  for(int i=0;i<6;i++){
    printf("\033[48;2;%d;%d;%dm\033[38;2;255;255;255m\033[1m %c \033[0m",
      c[i][0],c[i][1],c[i][2],t[i]);
  }//chaque lettre tiendra un couleur spécifique
}
void printHeader(void){
  printf(CLEAR_TO_EOL "\n");//supprimer les "Leftovers" des dessins précedents
  printFancyTitle();
  printf(RESET CLEAR_TO_EOL "\n");
  printf(CLEAR_TO_EOL "\n");
}
int main(void){
  char input[64];
  srand((unsigned)time(NULL));//manipuler le 'seed' pour ne pas avoir le meme mot chaque fois
  char mot[WORD_LEN+1];//+1 pour le caractére '\0'
  if (!pick_random_word("source.txt",&mot))
  {
    printf(stderr,"Erreur d'imporation du mot\n");
    return 1;
  }
  clearScreen();
  printFancyTitle();
  printf(RESET "\n");
  printf(CLEAR_TO_EOL "\n");
  printf(HUD_COLOR "            APPUYER Entrer pour continuer..." RESET);
  if(fgets(input,sizeof(input),stdin)==NULL){
    return 0;
  }
  clearScreen();
  printf(TITLE_COLOR "\n              Merci pour jouer!!\n\n" RESET);
  return 0;
}