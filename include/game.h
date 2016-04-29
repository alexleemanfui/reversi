#ifndef _GAME_H
#define _GAME_H

#include <windows.h>
#include "Board.h"
#include "list.h"
#include "stack.h"

typedef enum {Easy, Normal, Hard} LevelT;

listADT GenerateMove(BoardADT, ColourT);
int MoveIsLegal(BoardADT, int, int, ColourT);

BoardADT MakeMove(BoardADT, int, ColourT);
void SaveMove(stackADT, BoardADT, int);
BoardADT UndoMove(stackADT, ColourT);

int GameIsOver(BoardADT);
ColourT GetWinner(BoardADT);

int ChooseMove(HWND, HWND, BoardADT, stackADT, ColourT, LevelT);
static int Evaluate(BoardADT, ColourT);
static int AlphaBeta(HWND, HWND, BoardADT, int, int, int, int, ColourT);
static int CornerPieceDifference(BoardADT, ColourT);
static int NearCornerPieceDifference(BoardADT, ColourT);
static int TotalPieceDifference(BoardADT, ColourT);
static int MobilityDifference(BoardADT, ColourT);
static int StabilityDifference(BoardADT, ColourT);
static int FrontierDifference(BoardADT, ColourT);

#endif