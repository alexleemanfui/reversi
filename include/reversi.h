#ifndef _REVERSI_H
#define _REVERSI_H

#include <windows.h>
#include "Board.h"
#include "Game.h"
#include "stack.h"

ATOM RegisterWindowClass(HINSTANCE);
void InitializeGame(HWND);
void DrawGame(HWND, BoardADT, stackADT, ColourT, ColourT);
void UpdateHistory(HWND, stackADT);
int UpdatePassButton(HWND, BoardADT, ColourT);
void UpdateCPUMoveFirstMenu(HWND, stackADT);
void UpdateUndoMoveMenu(HWND, stackADT);
void UpdateLevel(HWND, LevelT);

#endif