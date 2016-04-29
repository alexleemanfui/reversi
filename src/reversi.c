#define IDC_MAIN_PASS 101
#define IDC_MAIN_NEW_GAME 102
#define IDC_MAIN_HISTORY 103
#define IDC_MAIN_STATIC 104
#define IDC_MAIN_STATUS 105
#define IDC_MAIN_PROGRESS 106

#include <windows.h>
#include <commctrl.h>
#include <stdlib.h>
#include "reversi.h"
#include "resource.h"
#include "draw_chess_board.h"
#include "board.h"
#include "game.h"
#include "stack.h"
#include "list.h"

const char g_ClassName[] = "WindowClass";

// Global variables:
stackADT HistoryStack;
BoardADT Board;
ColourT Human, CurrentPlayer;
InitialPosT InitialPos = WhiteBlack;
LevelT Level = Hard;

// Global variables: Background Colour
HBRUSH g_EditBackground = NULL;
HBRUSH g_Background = NULL;


// "About Dialog Box" procedure
BOOL CALLBACK AboutDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  switch (Message)
  {
    case WM_INITDIALOG:
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          EndDialog(hWnd, IDOK);
          break;

        case IDCANCEL:
          EndDialog(hWnd, IDCANCEL);
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}


// "Preferences Dialog Box" procedure
BOOL CALLBACK PreferencesDlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  HWND hRadio[3];

  hRadio[0] = GetDlgItem(hWnd, IDC_WHITEBLACK);
  hRadio[1] = GetDlgItem(hWnd, IDC_BLACKWHITE);
  hRadio[2] = GetDlgItem(hWnd, IDC_WHITEWHITE);
  switch (Message)
  {
    case WM_INITDIALOG:
      switch (InitialPos)
      {
        case WhiteBlack:
          SendMessage(hRadio[0], BM_CLICK, 0, 0);
          break;
        case BlackWhite:
          SendMessage(hRadio[1], BM_CLICK, 0, 0);
          break;
        case WhiteWhite:
          SendMessage(hRadio[2], BM_CLICK, 0, 0);
      }
      return TRUE;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDOK:
          if (SendMessage(hRadio[0], BM_GETCHECK, 0, 0))
            InitialPos = WhiteBlack;
          else if (SendMessage(hRadio[1], BM_GETCHECK, 0, 0))
            InitialPos = BlackWhite;
          else
            InitialPos = WhiteWhite;
          EndDialog(hWnd, IDOK);
          break;

        case IDCANCEL:
          EndDialog(hWnd, IDCANCEL);
      }
      break;

    default:
      return FALSE;
  }
  return TRUE;
}


// Window procedure
LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
  switch (Message)
  {
    // This message is sent automatically when the control is about to be drawn
    // WM_CTLCOLORSTATIC is sent for Edit Control for which is read-only
    case WM_CTLCOLORSTATIC:  
      if (GetDlgCtrlID((HWND) lParam) == IDC_MAIN_HISTORY)
      {
        HDC hdc = (HDC) wParam;

        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, OPAQUE);

        // Set the background colour of the Game Move History to white
        g_EditBackground = CreateSolidBrush(RGB(255, 255, 255));
        return (LONG) g_EditBackground;
      }
      else if (GetDlgCtrlID((HWND) lParam) == IDC_MAIN_STATIC)
      {
        HDC hdc = (HDC) wParam;

        SetTextColor(hdc, RGB(0, 0, 0));
        SetBkMode(hdc, TRANSPARENT);

        // Set the background colour of the static control to the orginal background
        return (LONG) g_Background;
      }
      break;

    case WM_CREATE:
    {
      HFONT hFont;
      HWND hEdit, hStatus;
      RECT ClientRect, StatusRect;
      int width, height, StatusWidth[] = {250, 500, -1};

      GetClientRect(hWnd, &ClientRect);

      // Create the status bar
      hStatus = CreateWindowEx(0, STATUSCLASSNAME, NULL,
                               WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                               0, 0, 0, 0, hWnd, (HMENU) IDC_MAIN_STATUS,
                               GetModuleHandle(NULL), NULL);
      SendMessage(hStatus, SB_SETPARTS, 3, (LPARAM) StatusWidth);
      SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) "Your Move.");
      UpdateLevel(hWnd, Level);

      // Create the progress bar in the status bar
      SendMessage(hStatus, SB_GETRECT, 1, (LPARAM) &StatusRect);
      CreateWindowEx(0, PROGRESS_CLASS, NULL,
                     WS_CHILD | WS_VISIBLE,
                     StatusRect.left, StatusRect.top,
                     StatusRect.right - StatusRect.left, StatusRect.bottom - StatusRect.top,
                     hStatus, (HMENU) IDC_MAIN_PROGRESS,
                     GetModuleHandle(NULL), NULL);

      // Divide the Client Area into 70% to 30% horizontally
      ClientRect.left = (ClientRect.right - ClientRect.left)*7/10;
      ClientRect.top += 25;
      ClientRect.bottom -= 25;
      ClientRect.right -= 25;
      width = (ClientRect.right - ClientRect.left - 25)/2;
      height = 30;

      // Create the "Pass" Button
      CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "Pass",
                     WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                     ClientRect.left, (ClientRect.bottom - ClientRect.top)*4/10 - 15,
                     width, height, hWnd, (HMENU) IDC_MAIN_PASS,
                     GetModuleHandle(NULL), NULL);

      // Create the "New Game" Button
      CreateWindowEx(WS_EX_WINDOWEDGE, "BUTTON", "New Game",
                     WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                     ClientRect.left + width + 25, (ClientRect.bottom - ClientRect.top)*4/10 - 15,
                     width, height, hWnd, (HMENU) IDC_MAIN_NEW_GAME,
                     GetModuleHandle(NULL), NULL);

      // Create the Game Move History
      width = ClientRect.right - ClientRect.left;
      height = (ClientRect.bottom - ClientRect.top)/2;
      hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "Reversi",
                             WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_READONLY,
                             ClientRect.left, (ClientRect.bottom + ClientRect.top)/2,
                             width, height, hWnd, (HMENU) IDC_MAIN_HISTORY,
                             GetModuleHandle(NULL), NULL);

      // Create the static text - "Game Move History"
      CreateWindowEx(WS_EX_WINDOWEDGE, "STATIC", "Game Move History",
                     WS_CHILD | WS_VISIBLE | SS_SIMPLE,
                     ClientRect.left, (ClientRect.bottom + ClientRect.top)/2 - 20,
                     width, 20, hWnd, (HMENU) IDC_MAIN_STATIC,
                     GetModuleHandle(NULL), NULL);

      // Set the font to fixed-pitch font
      hFont = GetStockObject(ANSI_FIXED_FONT);
      SendMessage(hEdit, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(FALSE, 0));
      SendMessage(hStatus, WM_SETFONT, (WPARAM) hFont, MAKELPARAM(FALSE, 0));
    }
    break;

    case WM_LBUTTONUP:
    {
      if (GameIsOver(Board))
        break;

      // Move made by Human
      if (CurrentPlayer == Human)
      {
        int Move;

        // Move is negative when Human did not click on the board
        Move = GetMousePosition(hWnd, lParam);
        if (Move >= 0)
        {
          if (MoveIsLegal(Board, Move/8, Move%8, Human))
          {
            // Do not destroy board, it is saved in the stack
            Board = MakeMove(Board, Move, Human);
            SaveMove(HistoryStack, Board, Move);

            // Switch player
            CurrentPlayer = Opponent(CurrentPlayer);
            DrawGame(hWnd, Board, HistoryStack, Human, CurrentPlayer);
          }
          else if (UpdatePassButton(hWnd, Board, CurrentPlayer) != 0)
            MessageBox(hWnd, "You have no possible moves so you have to pass your move.",
                       "Information", MB_OK | MB_ICONINFORMATION);
          else
          {
            HWND hStatus;
            char str[25];

            hStatus = GetDlgItem(hWnd, IDC_MAIN_STATUS);
            str[0] = (char) (Move%8 + 65);
            str[1] = (char) (Move/8 + 49);
            str[2] = '\0';
            strcat(str, " - Illegal Move...");
            SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) str);
          }
        }

      }

      // Move made by Computer
      if (CurrentPlayer != Human)
      {
        listADT MoveList;

        // Pass if the computer has no legal moves
        MoveList = GenerateMove(Board, CurrentPlayer);
        if (!ListEqual(MoveList, EmptyList()))
        {
          HWND hProgress, hStatus;
          int BestMove;

          // Set the cursor to hourglass
          SetCursor(LoadCursor(NULL, IDC_WAIT));
          // Get the handle of the progress bar
          hStatus = GetDlgItem(hWnd, IDC_MAIN_STATUS);
          hProgress = GetDlgItem(hStatus, IDC_MAIN_PROGRESS);

          BestMove = ChooseMove(hStatus, hProgress, Board, HistoryStack, CurrentPlayer, Level);

          // BestMove < 0 if CPU has no possible moves
          if (BestMove >= 0)
          {
            // Do not destroy board, it is saved in the stack
            Board = MakeMove(Board, BestMove, CurrentPlayer);
            SaveMove(HistoryStack, Board, BestMove);
          }

          // Restore the cursor
          SetCursor(LoadCursor(NULL, IDC_ARROW));
        }
        DestroyList(MoveList);

        // Switch the player
        CurrentPlayer = Opponent(CurrentPlayer);
        DrawGame(hWnd, Board, HistoryStack, Human, CurrentPlayer);
      }

      if (GameIsOver(Board))
      {
        ColourT Winner;
        HWND hStatus;

        hStatus = GetDlgItem(hWnd, IDC_MAIN_STATUS);
        SendMessage(hStatus, SB_SETTEXT, 0, (LPARAM) "Game is over.");

        Winner = GetWinner(Board);
        if (Winner == Human)
          MessageBox(hWnd, "Game is Over. You have won. You are a good player!", "Reversi", MB_OK | MB_ICONINFORMATION);
        else if (Winner == Opponent(Human))
          MessageBox(hWnd, "Game is Over. You have lost. Try once more.", "Reversi", MB_OK | MB_ICONINFORMATION);
        else
          MessageBox(hWnd, "Game is Over. Draw game. Try once more.", "Reversi", MB_OK | MB_ICONINFORMATION);
      }
    }
    break;

    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(hWnd, &ps);

      DrawChessBoard(hWnd, hdc, Board, HistoryStack, Human, CurrentPlayer);
      EndPaint(hWnd, &ps);
      UpdateHistory(hWnd, HistoryStack);
    }
    break;

    case WM_SIZING:
    {
      RECT *rect;

      // lParam stores the screen coordinates of drag rectangle
      rect = (RECT*) lParam;

      // Mininmum dimension is 700 * 500
      if (rect->right - rect->left < 700)
        switch (wParam)
        {
          case WMSZ_BOTTOMLEFT: case WMSZ_LEFT: case WMSZ_TOPLEFT:
            rect->left = rect->right - 700;
            break;

          case WMSZ_BOTTOMRIGHT: case WMSZ_RIGHT: case WMSZ_TOPRIGHT:
            rect->right = rect->left + 700;
        }

      if (rect->bottom - rect->top < 500)
        switch (wParam)
        {
          case WMSZ_BOTTOM: case WMSZ_BOTTOMLEFT: case WMSZ_BOTTOMRIGHT:
            rect->bottom = rect->top + 500;
            break;

          case WMSZ_TOP: case WMSZ_TOPLEFT: case WMSZ_TOPRIGHT:
            rect->top = rect->bottom - 500;
        }
    }
    break;

    case WM_SIZE:
    {
      HWND hButton, hEdit, hStatus;
      RECT ClientRect;
      int width, height;

      // Reset the position of the status bar
      hStatus = GetDlgItem(hWnd, IDC_MAIN_STATUS);
      SendMessage(hStatus, WM_SIZE, 0, 0);

      GetClientRect(hWnd, &ClientRect);

      // Divide the Client Area into 70% to 30% horizontally
      ClientRect.left = (ClientRect.right - ClientRect.left)*7/10;
      ClientRect.top += 25;
      ClientRect.bottom -= 25;
      ClientRect.right -= 25;
      width = (ClientRect.right - ClientRect.left - 25)/2;
      height = 30;

      // Reset the position of the "Pass" Button
      hButton = GetDlgItem(hWnd, IDC_MAIN_PASS);
      SetWindowPos(hButton, HWND_TOPMOST,
                   ClientRect.left, (ClientRect.bottom - ClientRect.top)*4/10 - 15,
                   width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

      // Reset the position of the "New Game" Button
      hButton = GetDlgItem(hWnd, IDC_MAIN_NEW_GAME);
      SetWindowPos(hButton, HWND_TOPMOST,
                   ClientRect.left + width + 25, (ClientRect.bottom - ClientRect.top)*4/10 - 15,
                   width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

      // Reset the position of the History
      width = ClientRect.right - ClientRect.left;
      height = (ClientRect.bottom - ClientRect.top)/2;
      hEdit = GetDlgItem(hWnd, IDC_MAIN_HISTORY);
      SetWindowPos(hEdit, HWND_TOPMOST,
                   ClientRect.left, (ClientRect.bottom + ClientRect.top)/2,
                   width, height, SWP_NOZORDER | SWP_SHOWWINDOW);

      // Reset the position of the static text
      hEdit = GetDlgItem(hWnd, IDC_MAIN_STATIC);
      SetWindowPos(hEdit, HWND_TOPMOST,
                   ClientRect.left, (ClientRect.bottom + ClientRect.top)/2 - 20,
                   width, 20, SWP_NOZORDER | SWP_SHOWWINDOW);
    }
    break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case ID_GAME_NEW_GAME: case IDC_MAIN_NEW_GAME:
          // No need to re-initalize the game
          // if the game is at the start
          // and the initial position is the same as the preference.
          if (StackDepth(HistoryStack) == 1)
          {
            stackElementT stackElement;
            BoardADT InitialBoard = EmptyBoard(InitialPos);

            stackElement = Pop(HistoryStack);
            if (BoardEqual(stackElement.Board, InitialBoard))
            {
              DestroyBoard(InitialBoard);
              Push(HistoryStack, stackElement);
              break;
            }
            DestroyBoard(InitialBoard);
            DestroyBoard(stackElement.Board);
          }

          DestroyStack(HistoryStack);
          InitializeGame(hWnd);
          DrawGame(hWnd, Board, HistoryStack, Human, CurrentPlayer);
          break;

        case ID_GAME_CPU_MOVE_FIRST:
          // No need to call UpdateCPUMoveFirstMenu 
          // DrawGame will automatically update the menu

          Human = White;
          // Let the Computer moves immediately
          SendMessage(hWnd, WM_LBUTTONUP, wParam, lParam);
          break;

        case ID_GAME_PASS: case IDC_MAIN_PASS:
          CurrentPlayer = Opponent(CurrentPlayer);
          DrawGame(hWnd, Board, HistoryStack, Human, CurrentPlayer);

          // Let the Computer moves immediately
          SendMessage(hWnd, WM_LBUTTONUP, wParam, lParam);
          break;

        case ID_GAME_UNDO_MOVE:
          if (StackDepth(HistoryStack) > 1)
          {
            Board = UndoMove(HistoryStack, Human);
            if (StackDepth(HistoryStack) == 1)
            {
              // Black always moves first
              Human = Black;
              CurrentPlayer = Black;
            }
            DrawGame(hWnd, Board, HistoryStack, Human, CurrentPlayer);
          }
          break;

        case ID_OPTIONS_LEVEL_EASY:
          Level = Easy;
          UpdateLevel(hWnd, Easy);
          break;

        case ID_OPTIONS_LEVEL_NORMAL:
          Level = Normal;
          UpdateLevel(hWnd, Normal);
          break;

        case ID_OPTIONS_LEVEL_HARD:
          Level = Hard;
          UpdateLevel(hWnd, Hard);
          break;

        case ID_OPTIONS_PREFERENCES:
					DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PREFERENCES), hWnd, PreferencesDlgProc);
          // Update the board if the game is at the beginning
          if (StackDepth(HistoryStack) == 1)
          {
            HWND hButton;

            // Simulate the user clicking a "New Game" Button
            hButton = GetDlgItem(hWnd, IDC_MAIN_NEW_GAME);
            SendMessage(hButton, BM_CLICK, 0, 0);
          }
          break;

        case ID_HELP_ABOUT:
          DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_ABOUT), hWnd, AboutDlgProc);
          break;

        case ID_GAME_EXIT:
          PostMessage(hWnd, WM_CLOSE, 0, 0);
      }
      break;

    case WM_CLOSE:
      DestroyWindow(hWnd);
      break;

    case WM_DESTROY:
      DeleteObject(g_Background);
      DeleteObject(g_EditBackground);
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hWnd, Message, wParam, lParam);
  }
  return 0;
}


// Initialize the game
void InitializeGame(HWND hWnd)
{
  HMENU hMenu, hGameMenu;
  HWND hButton;

  hMenu = GetMenu(hWnd);
  hGameMenu = GetSubMenu(hMenu, 0);

  // Enable the Menu item - "Let the Computer Move First"
  EnableMenuItem(hGameMenu, ID_GAME_CPU_MOVE_FIRST, MF_BYCOMMAND | MF_ENABLED);
  // Disable the Menu item - "Pass on a Move"
  EnableMenuItem(hGameMenu, ID_GAME_PASS, MF_BYCOMMAND | MF_GRAYED);
  // Disable the Menu item - "Undo Move"
  EnableMenuItem(hGameMenu, ID_GAME_UNDO_MOVE, MF_BYCOMMAND | MF_GRAYED);
  // Disable the Pass Button
  hButton = GetDlgItem(hWnd, IDC_MAIN_PASS);
  EnableWindow(hButton, FALSE);

  HistoryStack = EmptyStack();
  Board = EmptyBoard(InitialPos);
  // Save the board so that it can be freed later
  SaveMove(HistoryStack, Board, -1);
  // Black always moves first
  Human = Black;
  CurrentPlayer = Black;
}


// Draw the game interface
void DrawGame(HWND hWnd, BoardADT Board, stackADT HistoryStack, ColourT Human, ColourT CurrentPlayer)
{
  HDC hdc = GetDC(hWnd);

  DrawChessBoard(hWnd, hdc, Board, HistoryStack, Human, CurrentPlayer);
  ReleaseDC(hWnd, hdc);

  UpdateCPUMoveFirstMenu(hWnd, HistoryStack);
  UpdateUndoMoveMenu(hWnd, HistoryStack);
  UpdatePassButton(hWnd, Board, CurrentPlayer);
  UpdateHistory(hWnd, HistoryStack);
}


// Update the Game History
void UpdateHistory(HWND hWnd, stackADT HistoryStack)
{
  HWND hEdit;
  stackElementT stackElement;
  int TextLength, LineNo, Depth = StackDepth(HistoryStack) - 1;
  char *buffer;

  // Get the handle of Edit Control
  hEdit = GetDlgItem(hWnd, IDC_MAIN_HISTORY);

  TextLength = GetWindowTextLength(hEdit);
  if (TextLength > 0)
    buffer = GlobalAlloc(GPTR, TextLength + 8);
  
  GetWindowText(hEdit, buffer, TextLength + 1);
  buffer[TextLength] = '\0';
  LineNo = (TextLength - 7)/8;

  if (Depth > LineNo)
  {
    strcat(buffer, "\r\n ");

    // Show the nth number
    if (Depth < 10)
      _itoa(Depth, &(buffer[TextLength + 3]), 10);
    else
      _itoa(Depth, &(buffer[TextLength + 2]), 10);

    strcat(buffer, ". ");
    
    // Show the nth move
    stackElement = Pop(HistoryStack);
    buffer[TextLength + 6] = (char) (stackElement.Move%8 + 65);
    buffer[TextLength + 7] = (char) (stackElement.Move/8 + 49);
    buffer[TextLength + 8] = '\0';
    Push(HistoryStack, stackElement);
  }
  else if (Depth < LineNo)
    buffer[Depth*8 + 7] = '\0';

  SetWindowText(hEdit, buffer);

  // Scroll to bottom
  SendMessage(hEdit, WM_VSCROLL, SB_BOTTOM, 0);

  GlobalFree(buffer);
}


// Return 1 if the pass button is enabled, otherwise return 0
int UpdatePassButton(HWND hWnd, BoardADT Board, ColourT CurrentPlayer)
{
  HMENU hMenu, hGameMenu;
  HWND hButton;
  listADT MoveList;
  UINT Flag;
  int value;

  MoveList = GenerateMove(Board, CurrentPlayer);
  if (ListEqual(MoveList, EmptyList()) && !GameIsOver(Board))
  {
    value = 1;
    Flag = MF_ENABLED;
  }
  else
  {
    value = 0;
    Flag = MF_GRAYED;
  }
  
  // Enable or disable the Menu item - "Pass on a Move"
  hMenu = GetMenu(hWnd);
  hGameMenu = GetSubMenu(hMenu, 0);
  EnableMenuItem(hGameMenu, ID_GAME_PASS, MF_BYCOMMAND | Flag);

  // Enable or disable the Pass Button
  hButton = GetDlgItem(hWnd, IDC_MAIN_PASS);
  EnableWindow(hButton, (BOOL) value);

  DestroyList(MoveList);
  return value;
}


// Update the Menu item - "Let the Computer Move First"
void UpdateCPUMoveFirstMenu(HWND hWnd, stackADT HistoryStack)
{
  HMENU hMenu, hGameMenu;
  UINT Flag;

  if (StackDepth(HistoryStack) <= 1)
    Flag = MF_ENABLED;
  else
    Flag = MF_GRAYED;
  
  // Enable or disable the Menu item - "Let the Computer Move First"
  hMenu = GetMenu(hWnd);
  hGameMenu = GetSubMenu(hMenu, 0);
  EnableMenuItem(hGameMenu, ID_GAME_CPU_MOVE_FIRST, MF_BYCOMMAND | Flag);
}


// Update the Menu item - "Undo Move"
void UpdateUndoMoveMenu(HWND hWnd, stackADT HistoryStack)
{
  HMENU hMenu, hGameMenu;
  UINT Flag;

  if (StackDepth(HistoryStack) <= 1)
    Flag = MF_GRAYED;
  else
    Flag = MF_ENABLED;
  
  // Enable or disable the Menu item - "Undo Move"
  hMenu = GetMenu(hWnd);
  hGameMenu = GetSubMenu(hMenu, 0);
  EnableMenuItem(hGameMenu, ID_GAME_UNDO_MOVE, MF_BYCOMMAND | Flag);
}


// Sets the level item to checked or unchecked
void UpdateLevel(HWND hWnd, LevelT Level)
{
  HWND hStatus;
  HMENU hMenu, hOptionsMenu, hLevelMenu;
  int i;
  char str[3][20] = {"Level: Easy", "Level: Normal", "Level: Hard"};

  hMenu = GetMenu(hWnd);
  hOptionsMenu = GetSubMenu(hMenu, 1);
  hLevelMenu = GetSubMenu(hOptionsMenu, 0);

  // Unchecked all items
  for (i = 0; i < 3; i++)
    CheckMenuItem(hLevelMenu, i, MF_BYPOSITION | MF_UNCHECKED);

  CheckMenuItem(hLevelMenu, Level, MF_BYPOSITION | MF_CHECKED);

  // Update the status
  hStatus = GetDlgItem(hWnd, IDC_MAIN_STATUS);
  SendMessage(hStatus, SB_SETTEXT, 2, (LPARAM) str[Level]);
}


// Register window class
ATOM RegisterWindowClass(HINSTANCE hInstance)
{
  WNDCLASSEX wc;

  // Create a Brush with Gray Colour
  g_Background = CreateSolidBrush(GetSysColor(COLOR_MENU));

  wc.cbSize  = sizeof(WNDCLASSEX);
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON));
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = g_Background;
  wc.lpszMenuName = MAKEINTRESOURCE(IDR_MENU);
  wc.lpszClassName = g_ClassName;
  wc.hIconSm = (HICON) LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, 16, 16, 0);

  return RegisterClassEx(&wc);
}


// Main program
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
  HWND hWnd;
  MSG Msg;
  HACCEL hAccelTable;

  // Register the window class and check for error
  if (!RegisterWindowClass(hInstance))
  {
    MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  // Load accelerators
  hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDR_MENU);
  hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, g_ClassName, "Reversi",
                        WS_OVERLAPPEDWINDOW  | WS_MAXIMIZE | WS_VISIBLE,
                        0, 0, 800, 600,
                        NULL, NULL, hInstance, NULL);

  // Error checking
  if (hWnd == NULL)
  {
    MessageBox(NULL, "Window Creation Failed!", "Error!",  MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }

  InitCommonControls();
  InitializeGame(hWnd);

  ShowWindow(hWnd, SW_MAXIMIZE);
  UpdateWindow(hWnd);

  // Main message loop
  while (GetMessage(&Msg, NULL, 0, 0) > 0)
  {
    if (!TranslateAccelerator(hWnd, hAccelTable, &Msg)) 
    {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
    }
  }
  return Msg.wParam;
}