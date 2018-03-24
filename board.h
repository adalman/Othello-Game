/*--------------------------------------------------------------------*/
/* board.h                                                            */
/* Author: Ally Dalman                                                */
/*--------------------------------------------------------------------*/
#ifndef BOARD_INCLUDED
#define BOARD_INCLUDED

#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <assert.h>

/* The Board object is a 2d integer array that represents the othello
   board during a game. */

typedef struct Board *Board_T;

/* Creates and initializes a new oBoard object, given the tracking 
   number and the psFile to print to if tracking is on. Returns the 
   oBoard.*/
Board_T Board_init(int tracking, FILE *psFile);

/* Draw the oBoard after the current move, give the corresponding 
   moveRow number and moveColumn character and the move count. Return 
   the number of the player that just went  or 0 if there are no valid 
   moves left for either player. */
int Board_draw(Board_T oBoard);

/* Checks that the move corresponding to the given row and column on 
   oBoard is valid by calling Board_isLegal() in all 8 directions around
   the tile. Returns 1 if it is move and 0 if not.*/
int Board_moveIsValid(Board_T oBoard, int row, int column);
   
/* Make the move given by the row and column on oBoard. Return 1 if
   successful and 0 if not. */
int Board_makeMove(Board_T oBoard, int row, int column);

/* Returns the current player in oBoard. */
int Board_getPlayer(Board_T oBoard);

/* Ends the game on oBoard given the player1 and player2 names. Returns
   the score.*/
int Board_endGame(Board_T oBoard, char *player1, char *player2);

/* Ends the game badly (i.e. an invalid move is played) on oBoard given 
   the player1 and player2 names and int crash, which is 1 if one of the
   player crashes. Returns the score. */
int Board_endGameBad(Board_T oBoard, char *player1, char *player2, int crash);

/* Returns the character symbol for any tile on oBoard where the row 
   and column are given. */
char Board_getSymbol(Board_T oBoard, int row, int column);

#endif
