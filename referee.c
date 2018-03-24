/*--------------------------------------------------------------------*/
/* referee.c                                                          */
/* Author: Ally Dalman                                                */
/*--------------------------------------------------------------------*/
#define _POSIX_SOURCE 1 /* for fdopen */
#include "board.h"

#ifndef S_SPLINT_S
#include <sys/resource.h>
#endif
/*--------------------------------------------------------------------*/
/* The number of seconds that will be used as a time limit for the
   player files. */
enum {TIME_LIMIT = 60};

/* Size of the board is 8 by 8. */
enum {SIZE = 8};

/* Size of the "./" that is appended to player names (including null
   character. */
enum {SIZE_OF_DOTSLASH = 3};

/* Size of the "_vs_" that is appended to player names (including null
   character. */
enum {SIZE_OF_VS = 5};
/*--------------------------------------------------------------------*/
/* Close all files given as arguments, i.e. file1, file2, file3, file4.
 */
static void closeFiles(FILE *file1, FILE *file2, FILE *file3,
                       FILE *file4) {

   assert(file1 != NULL);
   assert(file2 != NULL);
   assert(file3 != NULL);
   assert(file4 != NULL);
   fclose(file1);
   fclose(file2);
   fclose(file3);
   fclose(file4);

}
/*--------------------------------------------------------------------*/
/* Close remaining pipes given pipe1[2], pipe2[2], pipe3[2], pipe4[2],
   argv[]. */
static void closePipes(int pipe1[], int pipe2[], int pipe3[],
                       int pipe4[], char* argv[]) {
   int iRet;

   assert(pipe1 != NULL);
   assert(pipe2 != NULL);
   assert(pipe3 != NULL);
   assert(pipe4 != NULL);
   
   iRet = close(pipe1[0]);
   if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
   iRet = close(pipe2[0]);
   if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
   iRet = close(pipe3[1]);
   if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
   iRet = close(pipe4[1]);
   if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }

}
/*--------------------------------------------------------------------*/
/* Free remaining memory allocated for exec1, exec2 and kill child
   processes iPid1 and iPid2. If tracking is on (i.e. equal to 1) then
   the filename is also freed.*/
static void cleanUp(char* exec1, char* exec2, pid_t iPid1, pid_t iPid2,
                    int tracking, char *filename) 
{

   assert(exec1 != NULL);
   assert(exec2 != NULL);
   assert(filename != NULL);
   if (tracking == 1) free(filename);
   
   free(exec1);
   free(exec2);
   kill(iPid1, SIGKILL);
   kill(iPid2, SIGKILL);

}

/*--------------------------------------------------------------------*/
/* Checks that the give player files player1, player2 exist. Return 0
   if the files do not exist or are not executable. */
static int playerCheck(char* player1, char* player2) {

   assert(player1 != NULL);
   assert(player2 != NULL);

   if (access(player1, X_OK) == -1) {
      fprintf(stderr, "File %s does not exist or is not executable\n",
              player1);
      return 0;
   }
   if (access(player2, X_OK) == -1) {
      fprintf(stderr, "File %s does not exist or is not executable\n",
              player2);
      return 0;
   }
   return 1;
}
/*--------------------------------------------------------------------*/
/* Starts the game of othello by drawing the initializing the board
   and drawing the initial state to the give psFile if tracking is 1.
   Returns the oBoard. */

static void startGame(Board_T oBoard, int tracking, FILE *psFile) {

   int row;
   int column;


   if (tracking == 1) {
      assert(psFile != NULL);
      fprintf(psFile, "\nInitial game state:\n");
      fprintf(psFile, "FIRST = x, SECOND = o\n\n");
      fprintf(psFile, "   A B C D E F G H\n");

      for (column = 0; column < SIZE; column++) {
         fprintf(psFile, "%d ", column);
         for (row = 0; row < SIZE; row++) {
            fprintf(psFile, " %c", Board_getSymbol(oBoard, row,
                                                   column));
         }
         fprintf(psFile, "\n");
      }
      fprintf(psFile, "\n");
      
   }
}
/*--------------------------------------------------------------------*/
/* Draws the game board after the move is made given the corresponding
   oBoard, row, column,and  move count. Draws the board to psFile if 
   tracking is "on" (i.e. equal to 1). Returns the number of the player
   that just went. */

static void writeMove(Board_T oBoard, int row, char column, int count,
                    FILE *psFile, int tracking) {

   if (tracking == 1) {
      assert(psFile != NULL);
      if (Board_getPlayer(oBoard) == 1) {
         fprintf(psFile, "Move #%d (by FIRST player): %c%d\n", count,
                 column, row);
      }
      else {
         fprintf(psFile, "Move #%d (by SECOND player): %c%d\n", count,
                 column, row);
      }
   }
}
/*--------------------------------------------------------------------*/
/* Draws the current game state of the oBoard to the given psFile if
   tracking is on (i.e. equal to 1.) Returns the player that just 
   went. */

static int drawGame(Board_T oBoard, FILE *psFile, int tracking) {

   if (tracking == 1) {
      assert(psFile != NULL);
      fprintf(psFile, "\nCurrent game state:\n");
      fprintf(psFile, "FIRST = x, SECOND = o\n\n");
      fprintf(psFile, "   A B C D E F G H\n");
   }
   return Board_draw(oBoard);
}
/*--------------------------------------------------------------------*/
/* Runs a game of othello between two players. argc is the command line
   argument count and argv contains the command line arguments. Return
   0. */

int main(int argc, char *argv[]) {

   pid_t iPid1, iPid2;
   int iRet;

   char *player1,  *player2;
   char *exec1, *exec2;
   char *dotSlash1, *dotSlash2;
   char *filename;

   int Child1ToParent[2];
   int ParentToChild1[2];
   int Child2ToParent[2];
   int ParentToChild2[2];

   FILE *psFileChild1ToParent;
   FILE *psFileChild2ToParent;
   FILE *psFileParentToChild1;
   FILE *psFileParentToChild2;
   FILE *psFile;

   Board_T oBoard;
   char columnChar;
   int column, row;
   int count, score, prevPlay,  tracking;

   if (argc < 3) return 0;
   dotSlash1 = "./";
   dotSlash2 = "./";
   tracking = 0;
   
   /* Checks to see if tracking is on. If it is, assign the command line
      arguments to the players appropriately. */
   if (strcmp(argv[1], "-tracking") == 0) {
      tracking = 1;
      player1 = argv[2];
      player2 = argv[3];
      if (playerCheck(player1, player2) == 0) return 0;

      /* Create the name of the file name. */
      filename = calloc((size_t)(strlen(argv[2]) + strlen(argv[3])
                                 + SIZE_OF_VS), 1);
      assert(filename != NULL);
      strcpy(filename, argv[2]);
      strcat(filename, "_vs_");
      strcat(filename, argv[3]);
      psFile = fopen(filename, "w");
   }
   /* If tracking is not on, assign the command line arguments to the
      players appropriately. */
   else {
      player1 = argv[1];
      player2 = argv[2];
      if (playerCheck(player1, player2) == 0) return 0;
      filename = "";
      psFile = NULL;
   }
   
   /* Append a "./" to the player names to run the files later. */
   exec1 = calloc((size_t)(strlen(player1) + SIZE_OF_DOTSLASH), 1);
   assert(exec1 != NULL);
   strcpy(exec1, dotSlash1);
   strcat(exec1, player1);

   exec2 = calloc((size_t)(strlen(player2) + SIZE_OF_DOTSLASH), 1);
   assert(exec2 != NULL);
   strcpy(exec2, dotSlash2);
   strcat(exec2, player2);

   /* Set up pipes. */
   if (pipe(Child1ToParent)) {perror(argv[0]); exit(EXIT_FAILURE);}
   if (pipe(ParentToChild1)) {perror(argv[0]); exit(EXIT_FAILURE);}
   if (pipe(Child2ToParent)) {perror(argv[0]); exit(EXIT_FAILURE);}
   if (pipe(ParentToChild2)) {perror(argv[0]); exit(EXIT_FAILURE);}

   /* Create child1 process. */
   iPid1 = fork();
   if (iPid1 == -1) {perror(argv[0]); exit(EXIT_FAILURE);}

   /* Code executed by child1. */
   if (iPid1 == 0) {

      #ifndef S_SPLINT_S
      struct rlimit sRlimit1;
      #endif

      char *apcArgv[3];
      int iRet;

      /* Redirect stdin to the read part of ParentToChild1 pipe. */
      iRet = close(0);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      iRet = dup(ParentToChild1[0]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      iRet = close(ParentToChild1[0]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }

      /* Redirect stdout to the write part of Child1ToParent pipe.*/
      iRet = close(1);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      iRet = dup(Child1ToParent[1]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      iRet = close(Child1ToParent[1]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }

      /* Close unneeded pipes. */
      iRet = close(Child1ToParent[0]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      closePipes(Child2ToParent, ParentToChild2, Child2ToParent,
                 ParentToChild2, argv);

      /* Set the arguments for execvp. */
      apcArgv[0] = exec1;
      apcArgv[1] = "FIRST";
      apcArgv[2] = NULL;

      /* Set the timing limits for the player file. */
      #ifndef S_SPLINT_S
      sRlimit1.rlim_cur = TIME_LIMIT;
      sRlimit1.rlim_max = TIME_LIMIT;
      setrlimit(RLIMIT_CPU, &sRlimit1);
      #endif
      execvp(exec1, apcArgv);
      perror(argv[0]);
      exit(EXIT_FAILURE);
   }
   /* Create child2 process. */
   iPid2 = fork();
   if (iPid2 == -1) {perror(argv[0]); exit(EXIT_FAILURE);}
   
   /* Code executed by child1. */
   if (iPid2 == 0)
   {
      #ifndef S_SPLINT_S
      struct rlimit sRlimit2;
      #endif
      char *apcArgv[3];
      int iRet;

      /* Redirect stdin to the read part of ParentToChild2 pipe. */
      iRet = close(0);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      iRet = dup(ParentToChild2[0]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      iRet = close(ParentToChild2[0]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }

      /* Redirect stdout to the write part of Child2ToParent pipe.*/
      iRet = close(1);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      iRet = dup(Child2ToParent[1]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }
      iRet = close(Child2ToParent[1]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }

      /* Close unneeded pipes. */
      closePipes(Child1ToParent, ParentToChild1,  Child1ToParent,
                 ParentToChild1, argv);
      iRet = close(Child2ToParent[0]);
      if (iRet == -1) {perror(argv[0]); exit(EXIT_FAILURE); }

      /* Set the arguments for execvp. */
      apcArgv[0] = exec2;
      apcArgv[1] = "SECOND";
      apcArgv[2] = NULL;
      /* Set the timing limits for the player file. */
      #ifndef S_SPLINT_S
      sRlimit2.rlim_cur = TIME_LIMIT;
      sRlimit2.rlim_max = TIME_LIMIT;
      setrlimit(RLIMIT_CPU, &sRlimit2);
      #endif
      execvp(exec2, apcArgv);
      perror(argv[0]);
      exit(EXIT_FAILURE);
   }
   /* Close unneeded pipes for the parent process.*/
   closePipes(ParentToChild1, ParentToChild2, Child1ToParent,
              Child2ToParent, argv);

   psFileChild1ToParent = fdopen(Child1ToParent[0], "r");
   psFileParentToChild2 = fdopen(ParentToChild2[1], "w");
   psFileChild2ToParent = fdopen(Child2ToParent[0], "r");
   psFileParentToChild1 = fdopen(ParentToChild1[1], "w");

   /* Start the game, set move count equal to 0. */
   count = 0;
   oBoard = Board_init(tracking, psFile);
   startGame(oBoard, tracking, psFile);
   prevPlay = 0;
   for(;;)
   {
      if (Board_getPlayer(oBoard) == 1) {
         /* The first player's move. */
         if (fscanf(psFileChild1ToParent,
                    " %c%d", &columnChar, &row) == -1) {
            
            score = Board_endGameBad(oBoard, player1, player2, 1);
            printf("%d\n", score);
            
            closePipes(Child1ToParent, Child2ToParent, ParentToChild1,
                       ParentToChild2, argv);
            closeFiles(psFileChild1ToParent, psFileChild2ToParent,
                       psFileParentToChild1, psFileParentToChild2);
            cleanUp(exec1, exec2, iPid1, iPid2, tracking, filename);
            return score;
         }
      }
      else {
         /* The second player's move. */
         if (fscanf(psFileChild2ToParent,
                    " %c%d", &columnChar, &row) == -1) {

            score = Board_endGameBad(oBoard, player1, player2, 1);
            printf("%d\n", score);

            closePipes(Child1ToParent, Child2ToParent, ParentToChild1,
                       ParentToChild2, argv);
            closeFiles(psFileChild1ToParent, psFileChild2ToParent,
                       psFileParentToChild1, psFileParentToChild2);
            cleanUp(exec1, exec2, iPid1, iPid2, tracking, filename);
            return score;
         }
      }
      /* Convert the column to an int. */
      column = (int)(columnChar - 'A');
      writeMove(oBoard, row, columnChar, count, psFile, tracking);

      /* End the game is the move is not valid. */
      if (Board_moveIsValid(oBoard, row, column) == 0) {
         score = Board_endGameBad(oBoard, player1, player2, 0);
         printf("%d\n", score);

         /* Close pipes, files, free memory, kill children */
         closePipes(Child1ToParent, Child2ToParent, ParentToChild1,
                    ParentToChild2, argv);
         closeFiles(psFileChild1ToParent, psFileChild2ToParent,
                    psFileParentToChild1, psFileParentToChild2);
         cleanUp(exec1, exec2, iPid1, iPid2, tracking, filename); 
         return score;
      }
      /* Print the move to the other player. */
      if (Board_getPlayer(oBoard) == 1) {
         fprintf(psFileParentToChild2, "%c%d\n", columnChar, row);
         iRet = fflush(NULL);
         if (iRet == EOF) {perror(argv[0]); exit(EXIT_FAILURE); }
      }
      else {
         fprintf(psFileParentToChild1, "%c%d\n", columnChar, row);
         iRet = fflush(NULL);
         if (iRet == EOF) {perror(argv[0]); exit(EXIT_FAILURE); }
      }
      Board_makeMove(oBoard, row, column); /* Make the move. */
      
      /* Draw the board after the move is made. */
      prevPlay = drawGame(oBoard, psFile, tracking);
      count++; /* Increment move count. */
      
         /* If there are no more valid moves, end the game.*/
         if (prevPlay == 0) {
               score = Board_endGame(oBoard, player1, player2);
               printf("%d\n", score);

               /* Close pipes, files, free memory, kill children */
               closePipes(Child1ToParent, Child2ToParent,
                          ParentToChild1, ParentToChild2, argv);
               closeFiles(psFileChild1ToParent, psFileChild2ToParent,
                          psFileParentToChild1, psFileParentToChild2);
               cleanUp(exec1, exec2, iPid1, iPid2, tracking, filename);
               return score;
         }
   }
}
