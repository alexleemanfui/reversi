#ifndef _DRAW_CHESS_BOARD_H
#define _DRAW_CHESS_BOARD_H

#include "Board.h"
#include "stack.h"

typedef struct 
{
  RECT BrownBoardRect, GreenBoardRect;
  int CellWidth;
} Dimension;

int GetMousePosition(HWND, LPARAM);
void DrawChessBoard(HWND, HDC, BoardADT, stackADT, ColourT, ColourT);
void InitBoardDimension(HWND, Dimension*);
static void DrawBoard(HDC, Dimension);
static void DrawGrid(HDC, Dimension);
static void DrawCoordinates(HDC, Dimension);
static void DrawDisc(HDC, BoardADT, Dimension);
static void DrawScore(HWND, HDC, BoardADT, ColourT);
static void DrawLegalMove(HDC, BoardADT, Dimension, ColourT);
static void DrawLastMove(HDC, BoardADT, Dimension, stackADT);

#endif