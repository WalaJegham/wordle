#include "wordle.h"
#include <stdio.h>
#include <stdlib.h>  
#include <stdbool.h> 
#include <string.h> 
#include <time.h>    
#include <ctype.h>
int main(void)
{
  atexit(cleanup); // Garantit que le curseur est réaffiché quand le programme se termine
  char input[64];  // Buffer pour ce que tape le joueur
  bool play=true;  // Reste vrai tant que le joueur veut rejouer
  srand((unsigned)time(NULL)); // manipuler le 'seed' pour ne pas avoir le meme mot chaque fois
  printInstructions();
  if (fgets(input, sizeof(input), stdin) == NULL) // Attend Entrée pour commencer
  {
    return 0;
  }
  // Boucle principale: une itération = une partie
  while(play){
    clearScreen();
    char *mot;
    mot = malloc(sizeof(char) * (WORD_LEN + 1)); //+1 pour le caractére '\0'
    if (!pick_random_word("source.txt", mot)) // Tire un nouveau mot à chaque partie
    {
      printf("Erreur d'imporation du mot\n");
      return 1;
    }
    char guesses[MAX_GUESSES][WORD_LEN+1];     // Propositions du joueur
    LetterState results[MAX_GUESSES][WORD_LEN]; // Résultat (couleurs) de chaque proposition
    LetterState keyboard[26];                   // État de chaque lettre de l'alphabet
    initKeyboard(keyboard);
    int guessCount=0; // Nombre d'essais déjà utilisés
    bool won=false;
    // Boucle des essais
    while(guessCount<MAX_GUESSES){
      char prompt[128];
      snprintf(prompt,sizeof(prompt),PROMPT_COLOR "     Proposition %d/%d" RESET,guessCount+1,MAX_GUESSES);
      renderGame(guesses,results,guessCount,-1,0,-1,keyboard,prompt); // Affiche le jeu + l'invite
      showCursor();
      if (fgets(input,sizeof(input),stdin)==NULL) // Fin de l'entrée (Ctrl+D / Ctrl+Z)
      {
        cleanup();
        return 0;
      }
      removeNewLine(input);
      // Le joueur veut quitter
      if(strcmp(input,"q")==0 || strcmp(input,"Q")==0){
        clearScreen();
        showCursor();
        printf(TITLE_COLOR "\n    Merci pour jouer!\n\n" RESET);
        return 0;
      }
      // Proposition invalide: on affiche l'erreur 1,3 s puis on redemande (sans compter l'essai)
      if(!validGuess(input)){
        char err[128];
        snprintf(err,sizeof(err),ERROR_COLOR "        Entrer exactement 5 lettres(A-Z)." RESET);
        renderGame(guesses,results,guessCount,-1,0,-1,keyboard,err);
        sleepMs(1300);
        continue;
      }
      // Conversion en minuscules (le mot secret et evaluateGuess travaillent en minuscules)
      char guess[WORD_LEN+1];
      for(int i=0;i<WORD_LEN;i++){
        guess[i]=(char)tolower((unsigned char) input[i]);
      }
      guess[WORD_LEN]='\0';
      strcpy(guesses[guessCount],guess);               // Sauvegarde la proposition
      evaluateGuess(mot,guess,results[guessCount]);    // Calcule les couleurs
      animateGuess(guesses,results,guessCount,keyboard); // Affiche l'animation de révélation
      // Vérifie si toutes les lettres sont vertes (mot trouvé)
      bool allExact=true;
      for(int i=0;i<WORD_LEN;i++){
        if (results[guessCount][i]!=EXACT)
        {
          allExact=false;break;
        }
      }
      guessCount++;
      if(allExact){
        won=true;
        break;
      }
    }
      // Fin de partie: prépare le message final (victoire ou défaite)
      char endMsg[128];
      if(won){
        const char *m; // Félicitation selon le nombre d'essais utilisés
        switch(guessCount){
          case 1: m="Genie!";break;
          case 2: m="Magnefique!";break;
          case 3: m="Impressionnant!";break;
          case 4: m="Splendide!";break;
          case 5: m="Tres Bien!";break;
          default:m="Oops!";break;
        }
        snprintf(endMsg,sizeof(endMsg),SUCCESS_COLOR "      %s Résolu dans %d/%d" RESET,m,guessCount,MAX_GUESSES);
      }
      else{
        // Défaite: on révèle le mot en majuscules
        char upper[WORD_LEN+1];
        for (int i = 0; i < WORD_LEN; i++)
        {
          upper[i]=(char)toupper((unsigned char)mot[i]);
        }
        upper[WORD_LEN]='\0';
        snprintf(endMsg,sizeof(endMsg),ERROR_COLOR "      Le mot est: %s"RESET,upper);}
      renderGame(guesses,results,guessCount,-1,0,-1,keyboard,endMsg); // Affiche le plateau final + message
      printf("\n");
      printf(SUCCESS_COLOR "      Jouer une autre fois? (O/N)" RESET);
      fflush(stdout);
      showCursor();
      if(fgets(input,sizeof(input),stdin)==NULL){
        break;}
      if(input[0]!='O' && input[0]!='o'){ // Toute réponse autre que O/o arrête la boucle
        play=false;
      }
      free(mot); // Libère la mémoire du mot de cette partie
    }
    clearScreen();
    showCursor();
    printf(TITLE_COLOR "\n Merci pour jouer!\n\n" RESET);
    return 0;
}