#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include <stdlib.h>
#include "game.h"


// Generate possible moves
listADT GenerateMove(BoardADT Board, ColourT CurrentPlayer)
{
  int r = 2, c = 2, ini_r = 2, ini_c = 2;
  listADT MoveList = EmptyList();

  while (r >= 0 && c >= 0)
  {
    if (MoveIsLegal(Board, r, c, CurrentPlayer))
      MoveList = ListCons(r*8 + c, MoveList);

    // Move around the square
    if (c == ini_c && r < 7 - ini_r)
      r++;
    else if (r == 7 - ini_r && c < 7 - ini_c)
      c++;
    else if (c == 7 - ini_c && r > ini_r)
      r--;
    else if (r == ini_r && c > ini_c)
      c--;

    // Reach the initial position
    if (r == ini_r && c == ini_c)
    {
      r = --ini_r;
      c = --ini_c;
    }
  }
  return MoveList;
}


// Check if the move is legal
int MoveIsLegal(BoardADT Board, int r, int c, ColourT CurrentPlayer)
{
  int dir_r, dir_c, new_r, new_c;

  if (CellColour(Board, r, c) == Empty)
    // To check the 8 squares around the central cell
    for (dir_r = -1; dir_r <= 1; dir_r++)
      for (dir_c = -1; dir_c <= 1; dir_c++)
      {
        new_r = r + dir_r;
        new_c = c + dir_c;

        // No need to check the central cell
        if ((dir_r || dir_c) && (0 <= new_r && new_r < 8) && (0 <= new_c && new_c < 8))
        {
          // Find the pattern: Empty, Black, White    (White Legal Move)
          //                   Empty, White, Black    (Black Legal Move)
          if (CellColour(Board, new_r, new_c) == Opponent(CurrentPlayer))
            for (new_r += dir_r, new_c += dir_c;
                 (0 <= new_r && new_r < 8) && (0 <= new_c && new_c < 8);
                 new_r += dir_r, new_c += dir_c)
            {
              ColourT colour = CellColour(Board, new_r, new_c);

              if (colour == Empty)
                break;
              else if (colour == CurrentPlayer)
                return 1;
            }
        }
      }
  return 0;
}


// Make a move and return a new board
BoardADT MakeMove(BoardADT Board, int Move, ColourT CurrentPlayer)
{
  int r = Move/8, c = Move%8;
  int dir_r, dir_c, new_r, new_c;
  BoardADT NewBoard;

  NewBoard = DuplicateBoard(Board);

  // Put the player's disc onto the empty cell
  SetBit(NewBoard, r, c, CurrentPlayer);

  // Flip other discs
  for (dir_r = -1; dir_r <= 1; dir_r++)
    for (dir_c = -1; dir_c <= 1; dir_c++)
    {
      new_r = r + dir_r;
      new_c = c + dir_c;

      // No need to check the central cell
      if ((dir_r || dir_c) && (0 <= new_r && new_r < 8) && (0 <= new_c && new_c < 8))
      {
        // Find the pattern: Empty, Black, White    (White Legal Move)
        //                   Empty, White, Black    (Black Legal Move)
        if (CellColour(NewBoard, new_r, new_c) == Opponent(CurrentPlayer))
          for (new_r += dir_r, new_c += dir_c;
              (0 <= new_r && new_r < 8) && (0 <= new_c && new_c < 8);
              new_r += dir_r, new_c += dir_c)
          {
            ColourT colour = CellColour(NewBoard, new_r, new_c);

            if (colour == Empty)
              break;
            else if (colour == CurrentPlayer)
            {
              // Flip the discs in between
              for (new_r -= dir_r, new_c -= dir_c;
                   new_r != r || new_c != c;
                   new_r -= dir_r, new_c -= dir_c)
                SetBit(NewBoard, new_r, new_c, CurrentPlayer);
              break;
            }
          }
      }
    }
  return NewBoard;
}


// Save the Board and the Move in the stack
void SaveMove(stackADT HistoryStack, BoardADT Board, int Move)
{
  stackElementT stackElement;

  stackElement.Board = Board;
  stackElement.Move = Move;
  Push(HistoryStack, stackElement);
}


// Undo the move so that the current player is human thereafter
BoardADT UndoMove(stackADT HistoryStack, ColourT Human)
{
  stackElementT stackElement;

  // Check if the game is at the initial state
  if (StackDepth(HistoryStack) == 1)
  {
    stackElement = Pop(HistoryStack);
    Push(HistoryStack, stackElement);
    return stackElement.Board;
  }

  // Undo last move by CPU
  stackElement = Pop(HistoryStack);
  DestroyBoard(stackElement.Board);

  // Check if the game is at initial state
  if (StackDepth(HistoryStack) == 1)
  {
    stackElement = Pop(HistoryStack);
    Push(HistoryStack, stackElement);
    return stackElement.Board;
  }

  // Undo second move by human
  stackElement = Pop(HistoryStack);
  // If human had passed, undo one move only
  if (CellColour(stackElement.Board, stackElement.Move/8, stackElement.Move%8) == Opponent(Human))
  {
    Push(HistoryStack, stackElement);
    return stackElement.Board;
  }
  DestroyBoard(stackElement.Board);

  stackElement = Pop(HistoryStack);
  Push(HistoryStack, stackElement);

  return stackElement.Board;
}


// Check if the game is over
int GameIsOver(BoardADT Board)
{
  listADT MoveList;
  int value = 1;

  MoveList = GenerateMove(Board, White);
  if (!ListEqual(MoveList, EmptyList()))
    value = 0;
  else
  {
    DestroyList(MoveList);
    MoveList = GenerateMove(Board, Black);
    if (!ListEqual(MoveList, EmptyList()))
      value = 0;
  }
  DestroyList(MoveList);
  return value;
}


// Return the winner of the game
// Return Empty for draw game
ColourT GetWinner(BoardADT Board)
{
  int WhiteCount, BlackCount;

  WhiteCount = GetDiscCount(Board, White);
  BlackCount = GetDiscCount(Board, Black);
  if (WhiteCount > BlackCount)
    return White;
  if (WhiteCount < BlackCount)
    return Black;
  return Empty;
}


// Divide the game into stages and call AlphaBeta procedure for different depth
int ChooseMove(HWND hStatus, HWND hProgress, BoardADT Board, stackADT HistoryStack, ColourT CurrentPlayer, LevelT Level)
{
  int BestMove, Depth[] = {2, 4, 6};

  switch (Level)
  {
    case Easy: case Normal:
      BestMove = AlphaBeta(hStatus, hProgress, Board, Depth[Level], Depth[Level], -50000, 50000, CurrentPlayer);
      break;

    case Hard:
      if (StackDepth(HistoryStack) < 49)
        BestMove = AlphaBeta(hStatus, hProgress, Board, Depth[Level], Depth[Level], -50000, 50000, CurrentPlayer);
      else
        BestMove = AlphaBeta(hStatus, hProgress, Board, 12, 12, -50000, 50000, CurrentPlayer);
  }
  return BestMove;
}


// Evaluate the board with respect to current player
static int Evaluate(BoardADT Board, ColourT CurrentPlayer)
{
  int TotalMove, Stage, value = 0;
  // Weight:
  // Corner pieces, Near-Corner pieces, Total pieces, Mobility, Stability, Frontier
  static int Weight[][6] = {
    {0, -116, -17, 34, 0, -17},
    {608, -651, -27, 70, 657, -60},
    {910, -548, -5, 83, 358, -92},
    {907, -433, 3, 96, 242, -100},
    {865, -322, 5, 105, 154, -99},
    {803, -219, 7, 120, 86, -96},
    {716, -120, 13, 136, 43, -89},
    {598, -50, 21, 154, 16, -85},
    {444, -1, 26, 182, 4, -81},
    {236, 23, 41, 240, -0, -71}};

  // Divide the game into 10 stages according to the number of moves
  TotalMove = GetDiscCount(Board, CurrentPlayer) + GetDiscCount(Board, Opponent(CurrentPlayer)) - 4;
  Stage = TotalMove/6;

  if (Stage < 10)
  {
    value = CornerPieceDifference(Board, CurrentPlayer)*Weight[Stage][0]
          + NearCornerPieceDifference(Board, CurrentPlayer)*Weight[Stage][1]
          + TotalPieceDifference(Board, CurrentPlayer)*Weight[Stage][2]
          + MobilityDifference(Board, CurrentPlayer)*Weight[Stage][3]
          + StabilityDifference(Board, CurrentPlayer)*Weight[Stage][4]
          + FrontierDifference(Board, CurrentPlayer)*Weight[Stage][5];
    if (GameIsOver(Board) && GetWinner(Board) == Opponent(CurrentPlayer))
      value -= 10000;
  }
  else
    value = TotalPieceDifference(Board, CurrentPlayer);
  return value;
}


// Perform Alpha-Beta pruning
static int AlphaBeta(HWND hStatus, HWND hProgress, BoardADT Board, int Depth, int MaxDepth, int alpha, int beta, ColourT CurrentPlayer)
{
  int BestMove = -1, value = 0, Mobility, MoveLeft;
  listADT MoveList, TempList;
  BoardADT BoardAfterMove;

  if (Depth == 0)
    return Evaluate(Board, CurrentPlayer);

  MoveList = GenerateMove(Board, CurrentPlayer);
  Mobility = ListLength(MoveList);
  MoveLeft = Mobility;
  // Save the list in order to free the list later
  TempList = MoveList;

  // Quiescence Search
  // If there is only one legal move, increase the search depth by 1
  if (Mobility == 1)
  {
    Depth++;
    MaxDepth++;
  }

  if (Depth == MaxDepth)
  {
    SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, Mobility));
    SendMessage(hProgress, PBM_SETSTEP, (WPARAM) 1, 0);
  }

  if (MoveLeft)
  {
    // Initialize BestMove to ensure there is at least one move available
    BestMove = ListHead(MoveList);

    do
    {
      // Update the progress
      if (Depth == MaxDepth)
      {
        char str[30] = "Computer is thinking... ";
        _itoa((Mobility - ListLength(MoveList) + 1)*100/Mobility, &(str[24]), 10);
        strcat(str, "%\0");

        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) str);
        SendMessage(hProgress, PBM_STEPIT, 0, 0);
      }

      BoardAfterMove = MakeMove(Board, ListHead(MoveList), CurrentPlayer);
      value = -AlphaBeta(hStatus, hProgress, BoardAfterMove, Depth - 1, MaxDepth, -beta, -alpha, Opponent(CurrentPlayer));
      DestroyBoard(BoardAfterMove);

      if (value >= beta)
      {
        DestroyList(TempList);
        return beta;
      }

      // Do not add equal sign
      // Because when there is only one move, value may be equal -infinity
      if (value > alpha)
      {
        alpha = value;
        if (Depth == MaxDepth)
          BestMove = ListHead(MoveList);
      }

      MoveList = ListTail(MoveList);
      MoveLeft--;
    } while (MoveLeft);
  }
  else if (Depth != MaxDepth)
  {
    // The player needs to pass
    value = -AlphaBeta(hStatus, hProgress, Board, Depth - 1, MaxDepth, -beta, -alpha, Opponent(CurrentPlayer));
    if (value >= beta)
      return beta;
    if (value > alpha)
      return value;
    return alpha;
  }

  DestroyList(TempList);

  if (Depth == MaxDepth)
  {
    // Reset the progress bar
    SendMessage(hProgress, PBM_SETPOS, (WPARAM) 0, 0);
    SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) "Your Move.");

    return BestMove;
  }

  return alpha;
}


// Return corner piece difference
static int CornerPieceDifference(BoardADT Board, ColourT CurrentPlayer)
{
  int r, c, count = 0;

  for (r = 0; r < 8; r += 7)
    for (c = 0; c < 8; c += 7)
    {
      ColourT colour = CellColour(Board, r, c);

      if (colour == CurrentPlayer)
        count++;
      else if (colour == Opponent(CurrentPlayer))
        count--;
    }
  return count;
}


// Return corner-adjacent piece difference
static int NearCornerPieceDifference(BoardADT Board, ColourT CurrentPlayer)
{
  int r, c, count = 0;

  for (r = 0; r < 8; r += 7)
    for (c = 1; c < 7; c += 5)
    {
      ColourT colour = CellColour(Board, r, c);

      if (colour == CurrentPlayer)
        count++;
      else if (colour == Opponent(CurrentPlayer))
        count--;

      colour = CellColour(Board, c, r);

      if (colour == CurrentPlayer)
        count++;
      else if (colour == Opponent(CurrentPlayer))
        count--;
    }

  for (r = 1; r < 7; r += 5)
    for (c = 1; c < 7; c += 5)
    {
      ColourT colour = CellColour(Board, r, c);

      if (colour == CurrentPlayer)
        count++;
      else if (colour == Opponent(CurrentPlayer))
        count--;
    }
  return count;
}


// Return total piece difference
static int TotalPieceDifference(BoardADT Board, ColourT CurrentPlayer)
{
  return (GetDiscCount(Board, CurrentPlayer) - GetDiscCount(Board, Opponent(CurrentPlayer)));
}


// Return mobility difference
static int MobilityDifference(BoardADT Board, ColourT CurrentPlayer)
{
  int CurrentMobility, OpponentMobility;
  listADT MoveList;

  MoveList = GenerateMove(Board, CurrentPlayer);
  CurrentMobility = ListLength(MoveList);
  DestroyList(MoveList);
  MoveList = GenerateMove(Board, Opponent(CurrentPlayer));
  OpponentMobility = ListLength(MoveList);
  DestroyList(MoveList);

  return (CurrentMobility - OpponentMobility);
}


// Return stability difference (approximate)
// No stable pieces if no corners
static int StabilityDifference(BoardADT Board, ColourT CurrentPlayer)
{
  int r, c, count = 0;
  ColourT TopRight, TopLeft, BottomLeft, BottomRight;

  TopLeft = CellColour(Board, 0, 0);
  TopRight = CellColour(Board, 0, 7);
  BottomLeft = CellColour(Board, 7, 0);
  BottomRight = CellColour(Board, 7, 7);

  if (TopLeft != Empty || TopRight != Empty)
  {
    if ((TopLeft == CurrentPlayer && TopRight == Opponent(CurrentPlayer)) ||
        (TopLeft == Opponent(CurrentPlayer) && TopRight == CurrentPlayer))
    {
      for (c = 1; c < 7; c++)
      {
        ColourT colour = CellColour(Board, 0, c);

        if (colour == CurrentPlayer)
          count++;
        else if (colour == Opponent(CurrentPlayer))
          count--;
      }
    }
    else if (TopLeft == CurrentPlayer || TopRight == CurrentPlayer)
    {
      for (c = 1; c < 7; c++)
        if (CellColour(Board, 0, c) == CurrentPlayer)
          count++;
    }
    else
    {
      for (c = 1; c < 7; c++)
        if (CellColour(Board, 0, c) == Opponent(CurrentPlayer))
          count--;
    }
  }

  if (TopLeft != Empty || BottomLeft != Empty)
  {
    if ((TopLeft == CurrentPlayer && BottomLeft == Opponent(CurrentPlayer)) ||
        (TopLeft == Opponent(CurrentPlayer) && BottomLeft == CurrentPlayer))
    {
      for (r = 1; r < 7; r++)
      {
        ColourT colour = CellColour(Board, r, 0);

        if (colour == CurrentPlayer)
          count++;
        else if (colour == Opponent(CurrentPlayer))
          count--;
      }
    }
    else if (TopLeft == CurrentPlayer || BottomLeft == CurrentPlayer)
    {
      for (r = 1; r < 7; r++)
        if (CellColour(Board, r, 0) == CurrentPlayer)
          count++;
    }
    else
    {
      for (r = 1; r < 7; r++)
        if (CellColour(Board, r, 0) == Opponent(CurrentPlayer))
          count--;
    }
  }

  if (BottomLeft != Empty || BottomRight != Empty)
  {
    if ((BottomLeft == CurrentPlayer && BottomRight == Opponent(CurrentPlayer)) ||
        (BottomLeft == Opponent(CurrentPlayer) && BottomRight == CurrentPlayer))
    {
      for (c = 1; c < 7; c++)
      {
        ColourT colour = CellColour(Board, 7, c);

        if (colour == CurrentPlayer)
          count++;
        else if (colour == Opponent(CurrentPlayer))
          count--;
      }
    }
    else if (BottomLeft == CurrentPlayer || BottomRight == CurrentPlayer)
    {
      for (c = 1; c < 7; c++)
        if (CellColour(Board, 7, c) == CurrentPlayer)
          count++;
    }
    else
    {
      for (c = 1; c < 7; c++)
        if (CellColour(Board, 7, c) == Opponent(CurrentPlayer))
          count--;
    }
  }

  if (TopRight != Empty || BottomRight != Empty)
  {
    if ((TopRight == CurrentPlayer && BottomRight == Opponent(CurrentPlayer)) ||
        (TopRight == Opponent(CurrentPlayer) && BottomRight == CurrentPlayer))
    {
      for (r = 1; r < 7; r++)
      {
        ColourT colour = CellColour(Board, r, 7);

        if (colour == CurrentPlayer)
          count++;
        else if (colour == Opponent(CurrentPlayer))
          count--;
      }
    }
    else if (TopRight == CurrentPlayer || BottomRight == CurrentPlayer)
    {
      for (r = 1; r < 7; r++)
        if (CellColour(Board, r, 7) == CurrentPlayer)
          count++;
    }
    else
    {
      for (r = 1; r < 7; r++)
        if (CellColour(Board, r, 7) == Opponent(CurrentPlayer))
          count--;
    }
  }
  return count;
}


// Return frontier difference
// Frontier pieces are pieces which are adjacent to at least one empty cell
static int FrontierDifference(BoardADT Board, ColourT CurrentPlayer)
{
  int r, c, dir_r, dir_c, new_r, new_c, count = 0;

  for (r = 0; r < 8; r++)
    for (c = 0; c < 8; c++)
    {
      ColourT colour = CellColour(Board, r, c);
      
      if (colour != Empty)
      {
        for (dir_r = -1; dir_r <= 1; dir_r++)
          for (dir_c = -1; dir_c <= 1; dir_c++)
          {
            new_r = r + dir_r;
            new_c = c + dir_c;

            // No need to check the central cell
            if ((dir_r || dir_c) && (0 <= new_r && new_r < 8) && (0 <= new_c && new_c < 8))
            {
              if (CellColour(Board, new_r, new_c) == Empty)
              {
                if (colour == CurrentPlayer)
                  count++;
                else
                  count--;
                break;
              }
            }
          }
      }
    }
  return count;
}