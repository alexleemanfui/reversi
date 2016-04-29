#include <windows.h>
#include "board.h"

struct BoardCDT {
  char WhiteBitBoard[8];
  char BlackBitBoard[8];
  int WhiteCount, BlackCount;
};


// The Board is not Empty but with four initial discs.
BoardADT EmptyBoard(InitialPosT InitialPos)
{
  BoardADT Board;
  int i;

  Board = (BoardADT) GlobalAlloc(GPTR, sizeof(*Board));
  for (i = 0; i < 8; i++)
  {
    Board->BlackBitBoard[i] = 0;
    Board->WhiteBitBoard[i] = 0;
  }
  Board->BlackCount = 0;
  Board->WhiteCount = 0;

  // Initialize the Chess Board
  switch (InitialPos)
  {
    case WhiteBlack:
      SetBit(Board, 3, 3, White);
      SetBit(Board, 4, 4, White);
      SetBit(Board, 3, 4, Black);
      SetBit(Board, 4, 3, Black);
      break;

    case BlackWhite:
      SetBit(Board, 3, 3, Black);
      SetBit(Board, 4, 4, Black);
      SetBit(Board, 3, 4, White);
      SetBit(Board, 4, 3, White);
      break;

    case WhiteWhite:
      SetBit(Board, 3, 3, White);
      SetBit(Board, 4, 4, Black);
      SetBit(Board, 3, 4, White);
      SetBit(Board, 4, 3, Black);
      break;
  }
  return Board;
}


// Set the bit to one according to the colour
void SetBit(BoardADT Board, int r, int c, ColourT colour)
{
  ColourT col;

  col = CellColour(Board, r, c);
  if (col == colour) return;                           // The bit is already set
  switch (colour)
  {
    case White:
      Board->WhiteBitBoard[r] |= (0x80 >> c);          // Set the bit to one
      Board->WhiteCount++;
      if (col == Black)
      {
        Board->BlackBitBoard[r] &= ~(0x80 >> c);       // Set the bit to zero
        Board->BlackCount--;
      }
      break;

    case Black:
      Board->BlackBitBoard[r] |= (0x80 >> c);          // Set the bit to one
      Board->BlackCount++;
      if (col == White)
      {
        Board->WhiteBitBoard[r] &= ~(0x80 >> c);       // Set the bit to zero
        Board->WhiteCount--;
      }
  }
}


// Return the colour of the cell.
// Return Empty if the cell is empty.
ColourT CellColour(BoardADT Board, int r, int c)
{
  // Check if the bit is one
  if (Board->WhiteBitBoard[r] == (Board->WhiteBitBoard[r] | (0x80 >> c)))
    return White;
  if (Board->BlackBitBoard[r] == (Board->BlackBitBoard[r] | (0x80 >> c)))
    return Black;
  return Empty;
}


// Return the number of discs
int GetDiscCount(BoardADT Board, ColourT colour)
{
  if (colour == White)
    return Board->WhiteCount;
  return Board->BlackCount;
}


// Return the opponent of the player
ColourT Opponent(ColourT CurrentPlayer)
{
  if (CurrentPlayer == White)
    return Black;
  return White;
}


// Check if the two board equal
int BoardEqual(BoardADT Board1, BoardADT Board2)
{
  int i;
  
  for (i = 0; i < 8; i++)
    if (Board1->BlackBitBoard[i] != Board2->BlackBitBoard[i] ||
        Board1->WhiteBitBoard[i] != Board2->WhiteBitBoard[i])
      return 0;
  return 1;
}


// Make a copy of a board
BoardADT DuplicateBoard(BoardADT Board)
{
  BoardADT NewBoard;
  int i;

  NewBoard = (BoardADT) GlobalAlloc(GPTR, sizeof(*NewBoard));
  for (i = 0; i < 8; i++)
  {
    NewBoard->BlackBitBoard[i] = Board->BlackBitBoard[i];
    NewBoard->WhiteBitBoard[i] = Board->WhiteBitBoard[i];
    NewBoard->BlackCount = Board->BlackCount;
    NewBoard->WhiteCount = Board->WhiteCount;
  }
  return NewBoard;
}


// Free the memory of the board
void DestroyBoard(BoardADT Board)
{
  GlobalFree((HANDLE) Board);
}