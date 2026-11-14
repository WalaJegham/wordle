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
//déclaration d'un nouveau type pour les différentes état qu'une lettre peut etre
typedef enum{UNKNOWN, ABSENT, PRESENT, EXACT } LetterState;

static bool cursorHidden=false;
void hideCursor(void){
 if(!cursorHidden){
	printf(HIDE_CURSOR);
	fflush(stdout);
	cursorHidden=true;
 }
}
void showCursor(void){
if(cursorHidden){
	printf(SHOW_CURSOR);
	fflush(stdout);
	cursorHidden=true;
	}
}
void cleanup(void){
showCursor();
}
void sleepMs(long ms){
struct timespec ts;
ts.tv_sec=ms/1000;
ts.tv_nsec=(ms%1000)*1000000L;
nanosleep(&ts,NULL);
}
bool pick_random_word(const char *nomfichier,char *mot){
  FILE *f=fopen(nomfichier,"r");
  char ligne[64];
  int compt=0;
  //compter les mots de 5 caracteres
  while(fgets(ligne,sizeof(ligne),f)!=NULL){
    ligne[strcspn(ligne,"\r\n")]='\0';//strip nouveau ligne
    if(strlen(ligne)==WORD_LEN){compt++;}
  }
  if(compt==0){fclose(f);return 0;}
  int alea=rand() % compt;//indexe aléatoire dans [0, compt-1]
  //Reparcours de fichier et arret dans cette indexe
  rewind(f);
  int i=0;
  while(fgets(ligne,sizeof(ligne),f)!=NULL){
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
void removeNewLine(char *s){
  //en windows une nouvelle ligne se fait par \r\n
  size_t len = strlen(s);
  if(len >0  && s[len-1]=='\n'){
    s[len-1]='\0';
  }
  len = strlen(s);
  if(len >0 && s[len-1]=='\r'){
    s[len-1]='\0';
  }
}
bool validGuess(const char *guess){
  if (strlen(guess)!=WORD_LEN)return false;
  for(int i=0;i<WORD_LEN;i++){
    if(!isalpha((unsigned char)guess[i])) return false;
  }
  return true;
}
void evaluateGuess(const char *word,const char *guess,LetterState result[WORD_LEN]){
  int remaining[26]={0};
  for (int i = 0; i < WORD_LEN; i++)
  {
    if(guess[i]==word[i]){
      result[i]=EXACT;
    }
    else{
      result[i]=UNKNOWN;
      remaining[word[i]-'a']++;
    }
  }
  for (int i = 0; i < WORD_LEN; i++)
  {
    if (result[i]==EXACT)
    {
      continue;
    }
    int index=guess[i]-'a';
    if(remaining[index]>0){
      result[i]=PRESENT;
      remaining[index]--;
    }
    else
    {
      result[i]=ABSENT;
    }
  }
}
void updateKeyboard(const char *guess,const LetterState result[WORD_LEN],LetterState keyboard[26]){
  for(int i=0;i<WORD_LEN;i++){
    int index=guess[i]-'a';
    if(result[i]==EXACT){keyboard[index]=EXACT;}
    else if(result[i]==PRESENT){keyboard[index]=PRESENT;}
    else{keyboard[index]=ABSENT;}
  }
}
void initKeyboard(LetterState keyboard[26]){
  for (int i = 0; i < 26; i++)
  {
    keyboard[i]=UNKNOWN;
  }
}
void drawTile(const char *bg,const char *fg,char letter,bool filled){
  if(filled){
    printf("%s%s%s %c %s",bg,fg,BOLD,(char)toupper((unsigned char)letter),RESET);
  }
  else{
    printf("%s %s ",bg,RESET);
  }
}
void drawStateTile(LetterState state,char letter){
  switch(state){
    case EXACT:drawTile(GREEN_BG,WHITE_FG,letter,true);break;
    case ABSENT:drawTile(GRAY_BG,WHITE_FG,letter,true);break;
    case PRESENT:drawTile(YELLOW_BG,WHITE_FG,letter,true);break;
    default:drawTile(EMPTY_BG,WHITE_FG,letter,true);
  }
}
void drawKey(LetterState state,char letter){
  switch (state)
  {
    case EXACT:drawTile(GREEN_BG,WHITE_FG,letter,true);break;
    case PRESENT:drawTile(YELLOW_BG,WHITE_FG,letter,true);break;
    case ABSENT:drawTile(GRAY_BG,WHITE_FG,letter,true);break;
    default:drawTile(KEY_BG,WHITE_FG,letter,true);
  }
}
/*Légende compacte pour que le joueur puisse savoir les coleurs*/
void printLegend(void){
printf ("            ");
drawTile(GREEN_BG,WHITE_FG, 'A',true);
printf(HUD_COLOR " Bien Placé   " RESET);
drawTile(YELLOW_BG,WHITE_FG, 'B',true); 
printf(HUD_COLOR " Mauvaise Place  " RESET);
drawTile(GRAY_BG,WHITE_FG, 'C',true);
printf(HUD_COLOR " Inexistant  " RESET);
}
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
void renderGame(const char guesses[][WORD_LEN+1],const LetterState results[][WORD_LEN],
		int filledrows,int animow,int revealCount,int flashIndex
		const LetterState keyboard[26],const char *status){
  printf(CURSOR_HOME);
  printHeader();
  for(int r=0;r<MAX_GUESSES;r++){
	printf("           ");
	for (int c=0;c<filledRows;c++){
		if(r<filledRows){
			drawStateTile(reults[r][c],guesses[r][c]);}
		else if(r==animRow){
			if(c<revealCount){
				drawStateTile(results[r][c],guesses[r][c]);}
			else if(c==flashIndex){
				drawTile(FLASH_BG,BLACK_FG,guesses[r][c],true);}
			else{drawTile(EMPTY_BG,WHITE_FG,guesses[r][c],true);}
		}
		else{drawTile(EMPTY_BG,WHITE_FG,' ',false);}
		printf(RESET CLEAR_TO_EOL);
	}
  }
 printf(CLEAR_TO_EOL "\n");
 printLegend();
 printf(RESET CLEAR_TO_EOL "\n");
 printf(CLEAR_TO_EOL "\n");
 const char *rows[3]={"AZERTYUIOP","QSDFGHJKLM","WXCVBN"};
 int indents[3]={1,3,7};
 for(int r=0;r<3;r++){
	for(int s=0;s<indents[r];s++){
        printf("   ");	
	} 
	for(int i=0;rows[r][i]!='\0';i++){
		char ch=rows[r][i];
		drawKey(keyboard[ch-'A'],ch);
	}
	printf(RESET CLEAR_TO_EOL "\n");
 }
 printf(CLEAR_TO_EOL "\n");
 printf("%s",status);
 printf(CLEAR_TO_EOL CLEAR_TO_END);
 fflush(stdout);
}

void animateGuess(char guesses[][WORD_LEN+1],LetterState results[][WORD_LEN],int row,
			LetterState keyboard[26]){
renderGame(guesses,results,row,row,0,-1,keyboard,"");
sleepMs(250);
for(int c=0;c<WORD_LEN;c++){
	renderGame(guesses,results,row,row,c,c,keyboard,"");
	sleepMs(125);
	renderGame(guesses,results,row,row,c+1,-1,keyboard,"");
	sleepMs(200);
	}
updateKeyboard(guesses[row],results[row],keyboard);
renderGame(guesses,results,row+1,-1,0,-1,keyboard,"");
sleepMs(100);
showCursor();
}
void printInstructions(void){
clearScreen();
printf(CLEAR_TO_EOL "\n");
printFancyTitle();
printf(RESET CLEAR_TO_EOL "\n");
printf(CLEAR_TO_EOL "\n");
printf(TITLE_COLOR "           COMMENT JOUER??" RESET CLEAR_TO_EOL "\n");
printf(CLEAR_TO_EOL "\n");
printf(HUD_COLOR "             Deviner le mot de 5 Lettres dans 5 essais." RESET CLEAR_TO_EOL "\n");
printf(HUD_COLOR "             Aprés chaque essaie, les couleurs des cadres vous disent si vous votre proposition est vraie ou pas:" RESET CLEAR_TO_EOL "\n");
printf(CLEAR_TO_EOL "\n");
printf("               ");
drawTile(GREEN_BG,WHITE_FG,'A',true);
printf(HUD_COLOR " Vert - la lettre existe dans le mot et bien placée" RESET CLEAR_TO_EOL "\n");
printf("               ");
drawTile(YELLOW_BG,WHITE_FG,'B',true);
printf(HUD_COLOR " Jaune - la lettre existe dans le mot mais mal placée" RESET CLEAR_TO_EOL "\n");
printf("               ");
drawTile(GRAY_BG,WHITE_FG,'C',true);
printf(HUD_COLOR " Gris - la lettre n'existe pas dans le mot" RESET CLEAR_TO_EOL "\n");
printf(CLEAR_TO_EOL "\n");
printf(HUD_COLOR "          Veuillez donner votre proposition et appuyez Entrer. Ecrire q pour quitter."RESET CLEAR_TO_EOL "\n");
printf(CLEAR_TO_EOL "\n");
printf(PROMPT_COLOR "        Appuyez Entrer pour commencer..." RESET CLEAR_TO_EOL "\n");
fflush(stdout);
}
int main(void){
  char input[64];
  srand((unsigned)time(NULL));//manipuler le 'seed' pour ne pas avoir le meme mot chaque fois
  char *mot;
  mot=malloc(sizeof(char)*(WORD_LEN+1));//+1 pour le caractére '\0'
  if (!pick_random_word("source.txt",mot))
  {
    printf("Erreur d'imporation du mot\n");
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
