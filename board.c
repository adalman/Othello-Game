/*--------------------------------------------------------------------*/
/* board.c                                                            */
/* Author: Ally Dalman                                                */
/*--------------------------------------------------------------------*/

#include "board.h"

/* Size of the array. */
enum {SIZE = 8};

/* Maximum number of points. */
enum {MAX_SCORE = 64};

/* Minimum number of points. */
enum {MIN_SCORE = -64};

/* The location of the initial tiles.*/
enum {INITIAL_TILE1 = 3};

/* The location of the initial tiles.*/
enum {INITIAL_TILE2 = 4};

/*--------------------------------------------------------------------*/

struct Board {
   /* The 8 by 8 array that represents the board. */
   int board[SIZE][SIZE];

   /* The current player. */
   int player;

   /* Variable that stores whether or not tracking is on.*/
   int track;

   /* Name of the file that the program should write to if tracking is
      on. */
   FILE *file;

};

/*--------------------------------------------------------------------*/

/* Returns the number of the other player, i.e. the player that is not
   the current player in oBoard. */
static int Board_getOtherPlayer(Board_T oBoard) {
   if (oBoard->player == 1) return 2;
   if (oBoard->player == 2) return 1;
   return 0;
}
/*--------------------------------------------------------------------*/
/* Checks that the tile corresponding to a given row and column on the 
   oBoard is a legal move. dy and dx correspond to the direction from 
   the tile that is being checked. If the tile in that direction belongs
   to the other player is and is eventually bordered by a tile of the 
   current player, return 1 for success. Otherwise return 0. */
static int Board_legalMove(Board_T oBoard, int row, int column, int dy
                           , int dx) {

   int rTemp;
   int cTemp;

   rTemp = row + dy;
   cTemp = column + dx;

   /* Check that move is adjacent to a tile in use and there is a tile
      of the other player in between the move and a tile of the current
      player. */
   while (oBoard->board[rTemp][cTemp] == Board_getOtherPlayer(oBoard)) {
      rTemp = rTemp + dy;
      cTemp = cTemp + dx;
      if ((cTemp < SIZE) && (cTemp >= 0) && (rTemp < SIZE)
          && (rTemp >= 0)) {
         if (oBoard->board[rTemp][cTemp] == oBoard->player) return 1;
      }
   }
   return 0;
}

/*--------------------------------------------------------------------*/
/* Flips tiles on an oBoard that result from a certain move 
   corresponding to the given row and column. dy and dx correspond to
   the current direction that is being checked. */
static void Board_flipTiles(Board_T oBoard, int row, int column,
                            int dy, int dx) {

   int rTemp, cTemp;

   rTemp = row + dy;
   cTemp = column + dx;

   /* Increment move as long as tile belongs to the other player    
      until a tile belonging to the current player is reached. */
   while (oBoard->board[rTemp][cTemp] == Board_getOtherPlayer(oBoard))
   {
      rTemp = rTemp + dy;
      cTemp = cTemp + dx;
      if ((cTemp < SIZE) && (cTemp >= 0) && (rTemp < SIZE)
          && (rTemp >= 0)) {
         
         if (oBoard->board[rTemp][cTemp] == oBoard->player) {
            for (;;) {
               /* Go back to original tile, flipping all tiles
                  in between. */
               oBoard->board[rTemp][cTemp] = oBoard->player;
               rTemp = rTemp - dy;
               cTemp = cTemp - dx;
               if ((rTemp == row) && (cTemp == column)) return;
            }
         }
      }
   }
}

/*--------------------------------------------------------------------*/
/* Checks that the next player can make a valid move on oBoard. Returns
   1 if a valid move exists and 0 if not. */

static int Board_nextMove(Board_T oBoard) {

   int row, column;

   for (row = 0; row < SIZE; row++) {
      for (column = 0; column < SIZE; column++) {
         if (Board_moveIsValid(oBoard, row, column) == 1) return 1;
      }
   }
   return 0;
}

/*--------------------------------------------------------------------*/
/* Counts how many tiles on the oBoard belong to the given player. 
   Returns the number of tiles.*/

static int Board_countTiles(Board_T oBoard, int player) {

   int row, column, count;
   count = 0;

   /* Count the number of tiles belonging to given player. */
   for (row = 0; row < SIZE; row++) {
      for (column = 0; column < SIZE; column++) {
         if (oBoard->board[row][column] == player) count++;
      }
   }
   return count;
}
/*--------------------------------------------------------------------*/
Board_T Board_init(int tracking, FILE *psFile) {

   Board_T oBoard;

   if (tracking == 1) assert(psFile != NULL);

   /* Initialize the oBoard.*/
   oBoard = (Board_T)calloc(sizeof(struct Board), 1);
   assert(oBoard != NULL);
   oBoard->player = 1;
   oBoard->track = tracking;
   oBoard->file = psFile;

   /* Set up the center four tiles. */
   oBoard->board[INITIAL_TILE1][INITIAL_TILE1] = 2;
   oBoard->board[INITIAL_TILE2][INITIAL_TILE2] = 2;
   oBoard->board[INITIAL_TILE1][INITIAL_TILE2] = 1;
   oBoard->board[INITIAL_TILE2][INITIAL_TILE1] = 1;

   return oBoard;
}
/*--------------------------------------------------------------------*/
int Board_draw(Board_T oBoard)
{
   int row;
   int column;

   /* Draw the board with the correct tiles.*/ 
   if (oBoard->track == 1) {
      for (row = 0; row < SIZE; row++) {
         fprintf(oBoard->file, "%d ", row);
         for (column = 0; column < SIZE; column++) {
            fprintf(oBoard->file, " %c", Board_getSymbol(oBoard, row, column));
         }
         fprintf(oBoard->file, "\n");
      }
      fprintf(oBoard->file, "\n");
   }
   
   /* Set the next player to be the other player. */
   oBoard->player = Board_getOtherPlayer(oBoard);

   /* If the other player doesn't have any valid moves, set the player 
    back to the player that just went.*/
   if (Board_nextMove(oBoard) == 0) {
      oBoard->player = Board_getOtherPlayer(oBoard);
      /* If neither player has a valid move, return 0. */
      if (Board_nextMove(oBoard) == 0) return 0;
      return oBoard->player;
   }
   /* Return the number of the player that just went. */
   return Board_getOtherPlayer(oBoard);
}

/*--------------------------------------------------------------------*/
/* Returns the character symbol for any tile on oBoard where the row 
   and column are given. */
char Board_getSymbol(Board_T oBoard, int row, int column) {
   if (oBoard->board[row][column] == 0)  return '.';
   else if (oBoard->board[row][column] == 1)  return 'x';
   else if (oBoard->board[row][column] == 2)  return 'o';
   return '\0';
}
/*--------------------------------------------------------------------*/
int Board_moveIsValid(Board_T oBoard, int row, int column) {

   int i;
   int rChange;
   int cChange;

   /* Make sure the move is within bounds and available. */
   if ((row < SIZE) && (row >= 0) && (column < SIZE)
       && (column >= 0)) {
      
      if (oBoard->board[row][column] == 0) {

         /* North. */
         rChange = -1;
         cChange = 0;
         i = Board_legalMove(oBoard, row, column, rChange, cChange);
         if (i == 1) return i;

         /* Northeast. */
         rChange = -1;
         cChange = 1;
         i = Board_legalMove(oBoard, row, column, rChange, cChange);
         if (i == 1) return i;

         /* East. */
         rChange = 0;
         cChange = 1;
         i = Board_legalMove(oBoard, row, column, rChange, cChange);
         if (i == 1) return i;

         /* Southeast. */
         rChange = 1;
         cChange = 1;
         i = Board_legalMove(oBoard, row, column, rChange, cChange);
         if (i == 1) return i;

         /* South. */
         rChange = 1;
         cChange = 0;
         i = Board_legalMove(oBoard, row, column, rChange, cChange);
         if (i == 1) return i;
         
         /* Southwest. */
         rChange = 1;
         cChange = -1;
         i = Board_legalMove(oBoard, row, column, rChange, cChange);
         if (i == 1) return i;

         /* West. */
         rChange = 0;
         cChange = -1;
         i = Board_legalMove(oBoard, row, column, rChange, cChange);
         if (i == 1) return i;

         /* Northwest. */
         rChange = -1;
         cChange = -1;
         i = Board_legalMove(oBoard, row, column, rChange, cChange);
         if (i == 1) return i;
      }
   }
   return 0;
}
         
/*--------------------------------------------------------------------*/
int Board_makeMove(Board_T oBoard, int row, int column) {

   int rChange;
   int cChange;

      oBoard->board[row][column] = oBoard->player;

      /* North. */
      rChange = -1;
      cChange = 0;
      Board_flipTiles(oBoard, row, column, rChange, cChange);

      /* Northeast. */
      rChange = -1;
      cChange = 1;
      Board_flipTiles(oBoard, row, column, rChange, cChange);

      /* East. */
      rChange = 0;
      cChange = 1;
      Board_flipTiles(oBoard, row, column, rChange, cChange);
      
      /* Southeast. */
      rChange = 1;
      cChange = 1;
      Board_flipTiles(oBoard, row, column, rChange, cChange);
      
      /* South. */
      rChange = 1;
      cChange = 0;
      Board_flipTiles(oBoard, row, column, rChange, cChange);
      
      /* Southwest. */
      rChange = 1;
      cChange = -1;
      Board_flipTiles(oBoard, row, column, rChange, cChange);
      
      /* West. */
      rChange = 0;
      cChange = -1;
      Board_flipTiles(oBoard, row, column, rChange, cChange);
      
      /* Northwest. */
      rChange = -1;
      cChange = -1;
      Board_flipTiles(oBoard, row, column, rChange, cChange);
   
   return 1;
}

/*--------------------------------------------------------------------*/
int Board_getPlayer(Board_T oBoard) {
   return oBoard->player;
}

/*--------------------------------------------------------------------*/
int Board_endGame(Board_T oBoard, char *player1, char *player2) {

   int count1;
   int count2;
   int score;

   assert(player1 != NULL);
   assert(player2 != NULL);
   if (oBoard->player == 1) {
      count1 = Board_countTiles(oBoard, oBoard->player);
      count2 = Board_countTiles(oBoard, Board_getOtherPlayer(oBoard));
   }
   else {
      count2 = Board_countTiles(oBoard, oBoard->player);
      count1 = Board_countTiles(oBoard, Board_getOtherPlayer(oBoard));
   }
   /* The score is equal to player 1's tiles minus player 2's. */
   score = count1 - count2;

   if (oBoard->track == 1) {
      fprintf(oBoard->file, "FIRST (%s) vs SECOND (%s)\n", player1, player2);
      if (count1 > count2) {
         fprintf(oBoard->file, "Winner FIRST %s\n", player1);
         fprintf(oBoard->file, "Score %d\n", score);
      }
      else if (count2 > count1) {
         fprintf(oBoard->file, "Winner SECOND %s\n", player2);
         fprintf(oBoard->file, "Score %d\n", score);
         
      }
      else if (count2 == count1) {
         fprintf(oBoard->file, "Winner (draw)\n");
         fprintf(oBoard->file, "Score %d\n", score);

      }
      fclose(oBoard->file); /* Close file if tracking is on. */
   }
   
   free(oBoard); /* Free memory. */
   return score;
}

/*--------------------------------------------------------------------*/
int Board_endGameBad(Board_T oBoard, char *player1, char *player2,
   int crash) {

   int score;

   assert(player1 != NULL);
   assert(player2 != NULL);
   /* Assign the other player the maximum score. */
   if (oBoard->player == 1) score = MIN_SCORE;
   else /*if (oBoard->player == 2)*/ score = MAX_SCORE;
   if (oBoard->track == 1) {
      fprintf(oBoard->file, "FIRST (%s) vs SECOND (%s)\n", player1, player2);
      /* The other player wins. */
      if (oBoard->player == 1) {
         fprintf(oBoard->file, "Winner SECOND %s\n", player2);
         fprintf(oBoard->file, "Score %d\n", score);
         if (crash == 1) fprintf(oBoard->file, "Player crashed\n");
         else fprintf(oBoard->file, "Bad move\n");
      }
      if (oBoard->player == 2) {
         fprintf(oBoard->file, "Winner FIRST %s\n", player1);
         fprintf(oBoard->file, "Score %d\n", score);
         if (crash == 1) fprintf(oBoard->file, "Player crashed\n");
         else fprintf(oBoard->file, "Bad move\n");
      }
      fclose(oBoard->file); /* Close file if tracking is on. */
   }
   
   free(oBoard); /*Free memory. */
   return score;
}
/*--------------------------------------------------------------------*/
