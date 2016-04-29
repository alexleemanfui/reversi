#include <windows.h>
#include <string.h>
#include <stdlib.h>
#include "draw_chess_board.h"
#include "list.h"
#include "Game.h"


// Return the coordinates of the cell when the mouse is clicked
// Return a negative value if the mouse is not on the chess board
int GetMousePosition(HWND hWnd, LPARAM lParam)
{
  int x, y;
  Dimension BoardDimension;

  x = LOWORD(lParam);                                            // x - coordinate
  y = HIWORD(lParam);                                            // y - coordinate
  InitBoardDimension(hWnd, &BoardDimension);

  // Not on the chess board
  if (x < BoardDimension.GreenBoardRect.left 
      || x > BoardDimension.GreenBoardRect.right
      || y < BoardDimension.GreenBoardRect.top
      || y > BoardDimension.GreenBoardRect.bottom)
    return -1;

  // Minus 1 because when the mouse is at the margin,
  // x or y is divisible by the cell width
  x = (x - 1 - BoardDimension.GreenBoardRect.left)/BoardDimension.CellWidth;
  y = (y - 1 - BoardDimension.GreenBoardRect.top)/BoardDimension.CellWidth;

  return (y*8 + x);
}


void DrawChessBoard(HWND hWnd, HDC hdc, BoardADT Board, stackADT HistoryStack, ColourT Human, ColourT CurrentPlayer)
{
  Dimension BoardDimension;

  InitBoardDimension(hWnd, &BoardDimension);
  DrawBoard(hdc, BoardDimension);
  DrawGrid(hdc, BoardDimension);
  DrawCoordinates(hdc, BoardDimension);
  DrawDisc(hdc, Board, BoardDimension);
  DrawLegalMove(hdc, Board, BoardDimension, CurrentPlayer);
  DrawLastMove(hdc, Board, BoardDimension, HistoryStack);
  DrawScore(hWnd, hdc, Board, Human);
}


// Initialize the Board Dimension
void InitBoardDimension(HWND hWnd, Dimension* BoardDimension)
{
  RECT ClientRect;
  int width, height, BrownEdge;

  GetClientRect(hWnd, &ClientRect);

  // Divide the Client Area into 70% to 30%
  ClientRect.right = (ClientRect.right - ClientRect.left)*7/10;
  width = ClientRect.right - ClientRect.left;
  height = ClientRect.bottom - ClientRect.top;

  // Margin is at least 25 units
  if (width<=height)
  {
    BoardDimension->BrownBoardRect.left = ClientRect.left + 25;
    BoardDimension->BrownBoardRect.right = ClientRect.right - 25;
    BoardDimension->BrownBoardRect.top = ClientRect.top + height/2 - width/2 + 25;
    BoardDimension->BrownBoardRect.bottom = BoardDimension->BrownBoardRect.top + width - 50;
  }
  else
  {
    BoardDimension->BrownBoardRect.left = ClientRect.left + width/2 - height/2 + 25;
    BoardDimension->BrownBoardRect.right = BoardDimension->BrownBoardRect.left + height - 50;
    BoardDimension->BrownBoardRect.top = ClientRect.top + 25;
    BoardDimension->BrownBoardRect.bottom = ClientRect.bottom - 25;
  }

  // Assuming Cell Width : Brown Board Edge = 2 : 1
  // Board Width = Cell Width * 9
  width = BoardDimension->BrownBoardRect.right - BoardDimension->BrownBoardRect.left;
  BoardDimension->CellWidth = width/9;
  BrownEdge = (width - width/9*8) / 2;

  BoardDimension->GreenBoardRect.left = BoardDimension->BrownBoardRect.left + BrownEdge;
  BoardDimension->GreenBoardRect.right = BoardDimension->GreenBoardRect.left + BoardDimension->CellWidth * 8;
  BoardDimension->GreenBoardRect.top = BoardDimension->BrownBoardRect.top + BrownEdge;
  BoardDimension->GreenBoardRect.bottom = BoardDimension->GreenBoardRect.top + BoardDimension->CellWidth * 8;
}


// Draw the Brown and Green Board
static void DrawBoard(HDC hdc, Dimension BoardDimension)
{
  HBRUSH OldBrush, NewBrush;
  HPEN OldPen, NewPen;

  // Draw Brown Board
  NewBrush = CreateSolidBrush(RGB(114, 79, 52));
  NewPen = CreatePen(PS_SOLID, 0, RGB(114, 79, 52));

  OldBrush = SelectObject(hdc, NewBrush);                // Save the Old Brush
  OldPen = SelectObject(hdc, NewPen);                    // Save the Old Pen
  RoundRect(hdc, BoardDimension.BrownBoardRect.left, BoardDimension.BrownBoardRect.top,
            BoardDimension.BrownBoardRect.right, BoardDimension.BrownBoardRect.bottom,15,15);
  DeleteObject(NewBrush);
  DeleteObject(NewPen);
  
  // Draw Green Board
  NewBrush = CreateSolidBrush(RGB(55, 132, 101));
  NewPen = CreatePen(PS_SOLID, 0, RGB(55, 132, 101));

  SelectObject(hdc, NewBrush);
  SelectObject(hdc, NewPen);
  RoundRect(hdc, BoardDimension.GreenBoardRect.left, BoardDimension.GreenBoardRect.top,
            BoardDimension.GreenBoardRect.right, BoardDimension.GreenBoardRect.bottom,0,0);
  DeleteObject(NewBrush);
  DeleteObject(NewPen);

  SelectObject(hdc, OldBrush);                           // Restore Old Brush
  SelectObject(hdc, OldPen);                             // Restore Old Pen
}


// Draw the grid lines
static void DrawGrid(HDC hdc,Dimension BoardDimension)
{
  int i;
  HPEN OldPen, NewPen;

  // Draw Light Green Lines
  NewPen = CreatePen(PS_SOLID, 0, RGB(89, 166, 135));
  OldPen = SelectObject(hdc, NewPen);                  // Save the Old Pen

  // Horizontal Lines
  for (i = BoardDimension.GreenBoardRect.top + 1;
       i < BoardDimension.GreenBoardRect.bottom;
       i += BoardDimension.CellWidth)
  {
    MoveToEx(hdc, BoardDimension.GreenBoardRect.left + 1, i, NULL);
    LineTo(hdc, BoardDimension.GreenBoardRect.right, i);
  }

  // Vertical Lines
  for (i = BoardDimension.GreenBoardRect.left + 1;
       i < BoardDimension.GreenBoardRect.right;
       i += BoardDimension.CellWidth)
  {
    MoveToEx(hdc, i, BoardDimension.GreenBoardRect.top + 1, NULL);
    LineTo(hdc, i, BoardDimension.GreenBoardRect.bottom);
  }
  DeleteObject(NewPen);


  // Draw Dark Green Lines
  NewPen = CreatePen(PS_SOLID, 0, RGB(0, 76, 45));
  SelectObject(hdc, NewPen);

  // Horizontal Lines
  for (i = BoardDimension.GreenBoardRect.top + BoardDimension.CellWidth;
       i <= BoardDimension.GreenBoardRect.bottom;
       i += BoardDimension.CellWidth)
  {
    MoveToEx(hdc, BoardDimension.GreenBoardRect.left, i, NULL);
    LineTo(hdc, BoardDimension.GreenBoardRect.right + 1, i);
  }

  // Vertical Lines
  for (i = BoardDimension.GreenBoardRect.left + BoardDimension.CellWidth;
       i <= BoardDimension.GreenBoardRect.right;
       i += BoardDimension.CellWidth)
  {
    MoveToEx(hdc, i, BoardDimension.GreenBoardRect.top, NULL);
    LineTo(hdc, i, BoardDimension.GreenBoardRect.bottom);
  }
  DeleteObject(NewPen);


  // Draw upper, left, Dark Brown Lines
  NewPen = CreatePen(PS_SOLID, 0, RGB(62, 27, 0));
  SelectObject(hdc, NewPen);

  MoveToEx(hdc, BoardDimension.GreenBoardRect.left, BoardDimension.GreenBoardRect.top, NULL);
  LineTo(hdc, BoardDimension.GreenBoardRect.right + 1, BoardDimension.GreenBoardRect.top);

  MoveToEx(hdc, BoardDimension.GreenBoardRect.left, BoardDimension.GreenBoardRect.top, NULL);
  LineTo(hdc, BoardDimension.GreenBoardRect.left, BoardDimension.GreenBoardRect.bottom + 1);

  DeleteObject(NewPen);


  // Draw lower, right, Light Brown Lines
  NewPen = CreatePen(PS_SOLID, 0, RGB(152, 117, 90));
  SelectObject(hdc, NewPen);

  MoveToEx(hdc, BoardDimension.GreenBoardRect.left, BoardDimension.GreenBoardRect.bottom + 1, NULL);
  LineTo(hdc, BoardDimension.GreenBoardRect.right + 2, BoardDimension.GreenBoardRect.bottom + 1);

  MoveToEx(hdc, BoardDimension.GreenBoardRect.right+1, BoardDimension.GreenBoardRect.top, NULL);
  LineTo(hdc, BoardDimension.GreenBoardRect.right+1, BoardDimension.GreenBoardRect.bottom + 1);

  DeleteObject(NewPen);
  SelectObject(hdc, OldPen);                              // Restore Old Pen
}


// Draw the coordinates on the board
static void DrawCoordinates(HDC hdc, Dimension BoardDimension)
{
  HFONT OldFont, NewFont;
  char i;
  int FontHeight;
  char str[2]="A\0";
  RECT rect;

  FontHeight = BoardDimension.GreenBoardRect.top - BoardDimension.BrownBoardRect.top - 2;
  NewFont = CreateFont(FontHeight, 0, 0, 0, FW_MEDIUM, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
  OldFont = SelectObject(hdc, NewFont);                   // Save Old Font

  // Set Text Color to Light Brown
  SetTextColor(hdc, RGB(178, 143, 116));
  SetBkMode(hdc, TRANSPARENT);

  // Print out Horizontal Coordinates
  rect.left = BoardDimension.GreenBoardRect.left;
  rect.right = rect.left + BoardDimension.CellWidth;
  rect.top = BoardDimension.BrownBoardRect.top;
  rect.bottom = BoardDimension.GreenBoardRect.top;
  for (i = 'A'; i <= 'H'; i++)
  {
    str[0] = i;                                           // Convert the char to a string
    DrawText(hdc, str, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    rect.left = rect.right;
    rect.right += BoardDimension.CellWidth;
  }

  // Print out Vertical Coordinates
  rect.left = BoardDimension.BrownBoardRect.left+1;
  rect.right = BoardDimension.GreenBoardRect.left;
  rect.top = BoardDimension.GreenBoardRect.top;
  rect.bottom = rect.top+BoardDimension.CellWidth;
  for (i = '1'; i <= '8'; i++)
  {
    str[0] = i;                                           // Convert the char to a string
    DrawText(hdc, str, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    rect.top = rect.bottom;
    rect.bottom += BoardDimension.CellWidth;
  }

  DeleteObject(NewFont);
  SelectObject(hdc, OldFont);                             // Restore Old Font
}


// Output the score
static void DrawScore(HWND hWnd, HDC hdc, BoardADT Board, ColourT Human)
{
  HFONT OldFont, NewFont;
  int FontHeight;
  RECT rect;
  RECT ClientRect;
  char str_DiscNo[10] = " \0";

  GetClientRect(hWnd, &ClientRect);

  // Divide the Client Area into 70% to 30% horizontally
  ClientRect.left = (ClientRect.right - ClientRect.left)*7/10;
  ClientRect.right -= 25;
  // Divide the Client Area into 40% to 60% vertically
  ClientRect.top += 25;
  ClientRect.bottom = (ClientRect.bottom - 25 - ClientRect.top)*6/10;

  // The Score part occupies about 40%
  FontHeight = (ClientRect.bottom - ClientRect.top)/6;
  NewFont = CreateFont(FontHeight, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
  OldFont = SelectObject(hdc, NewFont);                   // Save Old Font

  // Set Text Color to Black and Background to Gray
  SetTextColor(hdc, RGB(0, 0, 0));
  SetBkColor(hdc, GetSysColor(COLOR_MENU));
  SetBkMode(hdc, OPAQUE);

  // Draw the text - "Score"
  rect.left = ClientRect.left;
  rect.right = ClientRect.right;
  rect.top = ClientRect.top;
  rect.bottom = rect.top + FontHeight;
  DrawText(hdc, "Score", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Draw the text - ":"
  rect.top = rect.bottom;
  rect.bottom += FontHeight;
  DrawText(hdc, ":", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Output the number of discs of Computer
  rect.right = rect.left + (rect.right - rect.left)/2;
  _itoa(GetDiscCount(Board, Opponent(Human)), &(str_DiscNo[1]), 10);
  // The space is used to cover the previous number
  // Note that str_DiscNo[0] = ' '
  strcat(str_DiscNo, " \0");
  DrawText(hdc, str_DiscNo, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Output the number of discs of Human
  rect.left = rect.right;
  rect.right = ClientRect.right;
  _itoa(GetDiscCount(Board, Human), &(str_DiscNo[1]), 10);
  // The space is used to cover the previous number
  // Note that str_DiscNo[0] = ' '
  strcat(str_DiscNo, " \0");
  DrawText(hdc, str_DiscNo, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  DeleteObject(NewFont);

  FontHeight /= 2;
  NewFont = CreateFont(FontHeight, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 0, 0, "Arial");
  SelectObject(hdc, NewFont);

  // Draw the text - "YOU"
  rect.top = rect.bottom;
  rect.bottom += FontHeight;
  DrawText(hdc, "You", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Draw the text - "CPU"
  rect.right = rect.left;
  rect.left = ClientRect.left;
  DrawText(hdc, "CPU", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Draw the text - Colour of Computer
  rect.top = rect.bottom;
  rect.bottom += FontHeight;
  if (Human == White)
    DrawText(hdc, "Black", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  else
    DrawText(hdc, "White", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  // Draw the text - Colour of Human
  rect.left = rect.right;
  rect.right = ClientRect.right;
  if (Human == White)
    DrawText(hdc, "White", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  else
    DrawText(hdc, "Black", -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  
  DeleteObject(NewFont);
  SelectObject(hdc, OldFont);                             // Restore Old Font
}


// Draw the disc of players
static void DrawDisc(HDC hdc, BoardADT Board, Dimension BoardDimension)
{
  int r, c;
  ColourT colour;
  HBRUSH OldBrush, NewBrush;
  HPEN OldPen, NewPen;
  
  OldBrush = SelectObject(hdc, (HBRUSH) NULL);                  // Save the Old Brush

  NewPen = CreatePen(PS_SOLID, 1, RGB(89,166,135));
  OldPen = SelectObject(hdc, NewPen);                           // Save the Old Pen

  for (r = 0; r < 8; r++)
    for (c = 0; c < 8; c++)
    {
      colour = CellColour(Board, r, c);
      if (colour == Empty)
        continue;
      else if (colour == White)
        NewBrush = CreateSolidBrush(RGB(202, 202, 202));        // White Colour
      else
        NewBrush = CreateSolidBrush(RGB(58, 58, 58));           // Black Colour

      SelectObject(hdc, NewBrush);
      Ellipse(hdc, BoardDimension.GreenBoardRect.left + c*BoardDimension.CellWidth + 3,
                   BoardDimension.GreenBoardRect.top + r*BoardDimension.CellWidth + 3,
                   BoardDimension.GreenBoardRect.left + (c+1)*BoardDimension.CellWidth - 3,
                   BoardDimension.GreenBoardRect.top + (r+1)*BoardDimension.CellWidth - 3);
      DeleteObject(NewBrush);
    }
  DeleteObject(NewPen);

  SelectObject(hdc, OldBrush);                                  // Restore the Old Brush
  SelectObject(hdc, OldPen);                                    // Restore the Old Pen
}


// Draw the possible move of the current player
static void DrawLegalMove(HDC hdc, BoardADT Board, Dimension BoardDimension, ColourT CurrentPlayer)
{
  int r, c, margin;
  listADT MoveList;
  HBRUSH OldBrush, NewBrush;
  HPEN OldPen, NewPen;
  
  NewPen = CreatePen(PS_SOLID, 1, RGB(89, 166, 135));
  OldPen = SelectObject(hdc, NewPen);                           // Save the Old Pen

  if (CurrentPlayer == White)
    NewBrush = CreateSolidBrush(RGB(202, 202, 202));            // White Colour
  else
    NewBrush = CreateSolidBrush(RGB(58, 58, 58));               // Black Colour

  OldBrush = SelectObject(hdc, NewBrush);                       // Save the Old Brush

  MoveList = GenerateMove(Board, CurrentPlayer);
  margin = BoardDimension.CellWidth*43/100;                   // The dot is small, 14% of cell width
  while (!ListEqual(MoveList, EmptyList()))
  {
    r = ListHead(MoveList)/8;
    c = ListHead(MoveList)%8;

    Ellipse(hdc, BoardDimension.GreenBoardRect.left + c*BoardDimension.CellWidth + margin,
                 BoardDimension.GreenBoardRect.top + r*BoardDimension.CellWidth + margin,
                 BoardDimension.GreenBoardRect.left + (c+1)*BoardDimension.CellWidth - margin,
                 BoardDimension.GreenBoardRect.top + (r+1)*BoardDimension.CellWidth - margin);
    MoveList = ListTail(MoveList);
  }
  DeleteObject(NewBrush);
  DeleteObject(NewPen);

  SelectObject(hdc, OldBrush);                                  // Restore the Old Brush
  SelectObject(hdc, OldPen);                                    // Restore the Old Pen
}


// Show Last Move
static void DrawLastMove(HDC hdc, BoardADT Board, Dimension BoardDimension, stackADT HistoryStack)
{
  int r, c, margin;
  stackElementT stackElement;
  HBRUSH OldBrush, NewBrush;
  HPEN OldPen, NewPen;

  // The game is at the initial state
  if (StackDepth(HistoryStack) == 1)
    return;

  NewBrush = CreateSolidBrush(RGB(255, 0, 0));                // Red Colour
  OldBrush = SelectObject(hdc, NewBrush);                     // Save the Old Brush

  stackElement = Pop(HistoryStack);                           // Get last move
  r = stackElement.Move/8;
  c = stackElement.Move%8;
  Push(HistoryStack, stackElement);                           // Restore the stack

  if (CellColour(Board, r, c) == White)
    NewPen = CreatePen(PS_SOLID, 1, RGB(202, 202, 202));      // White Pen
  else
    NewPen = CreatePen(PS_SOLID, 1, RGB(58, 58, 58));         // Black Pen
  OldPen = SelectObject(hdc, NewPen);                         // Save the Old Pen

  margin = BoardDimension.CellWidth*43/100;                   // The dot is small, 14% of cell width

  Ellipse(hdc, BoardDimension.GreenBoardRect.left + c*BoardDimension.CellWidth + margin,
               BoardDimension.GreenBoardRect.top + r*BoardDimension.CellWidth + margin,
               BoardDimension.GreenBoardRect.left + (c+1)*BoardDimension.CellWidth - margin,
               BoardDimension.GreenBoardRect.top + (r+1)*BoardDimension.CellWidth - margin);

  DeleteObject(NewBrush);
  DeleteObject(NewPen);
  SelectObject(hdc, OldBrush);                                // Restore the Old Brush
  SelectObject(hdc, OldPen);                                  // Restore the Old Pen
}