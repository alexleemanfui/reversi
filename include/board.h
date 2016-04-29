#ifndef _BOARD_H
#define _BOARD_H

typedef struct BoardCDT *BoardADT;
typedef enum {White, Black, Empty} ColourT;
typedef enum {WhiteBlack, BlackWhite, WhiteWhite} InitialPosT;

BoardADT EmptyBoard(InitialPosT);
void SetBit(BoardADT, int, int, ColourT);
ColourT CellColour(BoardADT, int, int);
int GetDiscCount(BoardADT, ColourT);
ColourT Opponent(ColourT);
int BoardEqual(BoardADT, BoardADT);
BoardADT DuplicateBoard(BoardADT);
void DestroyBoard(BoardADT Board);

#endif