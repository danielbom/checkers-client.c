#include <stdio.h>    // printf, scanf, ...
#include <unistd.h>   // sleep(int)
#include <stdarg.h>   // va_list
#include <string.h>   // strncmp

#include "colorize/colorize.c"
#include "./server/client.c"
#include "./async-console-reader/async-console-reader.c"

// ------------------- //
//       Debugger      //
// ------------------- //
int debug(char* fmt, ...) {
  int res = 0;
#if DEBUG
  // https://stackoverflow.com/questions/150543/forward-an-invocation-of-a-variadic-function-in-c

  /* Declare a va_list type variable */
  va_list myargs;

  /* Initialise the va_list variable with the ... after fmt */

  va_start(myargs, fmt);

  /* Forward the '...' to vprintf */
  res = vprintf(fmt, myargs);

  /* Clean up the va_list */
  va_end(myargs);

#endif
  return res;
}

// ------------------- //
//        Assets       //
// ------------------- //
char blackPiece[3][7] = {
  " OOOO ",
  "OOOOOO",
  " OOOO "
};
char whitePiece[3][7] = {
  " XXXX ",
  "XXXXXX",
  " XXXX "
};
char blackLady[3][7] = {
  " \\__/ ",
  "OOOOOO",
  " OOOO "
};
char whiteLady[3][7] = {
  " \\__/ ",
  "XXXXXX",
  " XXXX "
};
char emptyOdd[3][7] = {
  "      ",
  "      ",
  "      ",
};
char emptyEven[3][7] = {
  "      ",
  "      ",
  "      ",
};

// ------------------- //
//        Utils        //
// ------------------- //
int isDigit(char ch) {
  return '0' <= ch && ch <= '9';
}

void unimplemented(char* function) {
  printf("[Unimplemented] %s\n", function);
  printf("...\n");
}

// ------------------- //
//      Controll       //
// ------------------- //
typedef struct piece {
  char player;  // ["[B]lack", "[W]hite"]
  char lady;    // ["[T]rue",  "[F]alse"]
} Piece;

char board[8][8];
Piece pieces[24];
char deadBlacks, deadWhites;  // Dead pieces counters

char currentPlayer = 'B'; // ["[B]lack", "[W]hite"]

char *MOVE_COLOR = BG_YELLOW_COLOR;
int SLEEP_TIME = 1; // Seconds

int countMoves = 0;
int LIMIT_MOVES = 80;

char error;

int px, py, mx, my;
char buffer[256] = {0};
int shift;

int isLocalGame = 1;
int isConsoleEnable = 1;
char selfPlayer;

// ------------------- //
//       Console       //
// ------------------- //
void GameConsumerHandle(char* input) {
  // printf(" Consuming %s\n", input);
  if (isLocalGame) {
    strcpy(buffer, input);
  } else {
    ClientSendMessage(input);
  }
}

void GameConfigConsole() {
  pthread_t thread;
  ConsoleSetConsumer(GameConsumerHandle);
  ConsoleStart(&thread);
}

// ------------------- //
//       Network       //
// ------------------- //
void GameCreateGameMenu() {
  pthread_t thread;
  
  ClientSetUsername("daniel");
  ClientSetPassword("pass");
  ClientSetRoomName("room");
  ClientInit();
  int error = ClientCreateRoom(2);

  isLocalGame = 0;
  selfPlayer = 'B';
  if (!error) {
    ClientSenderRun(&thread);
    ClientReceiverRun(&thread);
  }
}

void GameAccessGameMenu() {
  pthread_t thread;
  
  ClientSetUsername("mara");
  ClientSetPassword("pass");
  ClientSetRoomName("room");
  ClientInit();
  int error = ClientAccessRoom();
  
  isLocalGame = 0;
  selfPlayer = 'W';
  if (!error) {
    ClientSenderRun(&thread);
    ClientReceiverRun(&thread);
    while (ClientProps.isRunning) {}
  }
}

// ------------------- //
//     Persistence     //
// ------------------- //
void GameSaveCurrentMove() {
  char filename[] = "moves.txt";
  FILE* file = fopen(filename, countMoves == 1 ? "w+" : "a+");
  if (file == NULL) {
    debug("Não foi possível abrir o arquivo %s\n", filename);
  }

  fprintf(file, "%s\n", buffer);

  fclose(file);
}

// ------------------- //
//     Game Utils      //
// ------------------- //
char BLACK[] = "Black";
char WHITE[] = "White";
char* GameGetCurrentPlayer() {
  return currentPlayer == 'B' ? BLACK : WHITE;
}

void GameFillBuffer() {
  // setbuf(stdin , NULL);
  scanf(" %[^\n]", buffer);
  // ConsoleWaitConsume();
}

void GameCheckLady(int mx, int my) {
  debug("Check Lady Move(%d,%d)\n", mx, my);
  if (mx == 0 || mx == 7) {
    pieces[board[mx][my] - 1].lady = 1;
  }
}

char GameIsBounded() {
  return px >= 0 && py >= 0 && mx >= 0 && my >= 0 && px < 8 && py < 8 && mx < 8 && my < 8;
}

void GameSwapPlayer() {
  currentPlayer = currentPlayer == 'B' ? 'W' : 'B';
}

int GameGetCoordinate() {
  for (; buffer[shift] && !isDigit(buffer[shift]); shift++);  // skip non digit
  int x = isDigit(buffer[shift]) ? buffer[shift++] - '0' : -1;   // consume 1 digit
  for (; buffer[shift] && !isDigit(buffer[shift]); shift++);  // skip non digit
  int y = isDigit(buffer[shift]) ? buffer[shift++] - '0' : -1;   // consume 1 digit
  debug("LOG: Coordinate(%d,%d)\n", x, y);

  if (x != -1 && y != -1) {
    px = mx;
    py = my;
    mx = x;
    my = y;
    return 0;
  }
  return 1;
}

// ------------------- //
//      Printers       //
// ------------------- //
void GamePrintErrors() {
  if (error >= 1)
    debug("ERROR: Piece(%d,%d) Move(%d,%d) Player(%s) CurrentMove(%d)\n\t", px, py, mx, my, GameGetCurrentPlayer(), countMoves);

  switch (error) {
    case 1:
      debug("Message: Move out of the bound\n");
      break;
    case 2:
      debug("Message: Illegal simple move\n");
      break;
    case 3:
      debug("Message: Illegal capture of own piece\n");
      break;
    case 4:
      debug("Message: Illegal move\n");
      break;
    case 5:
      debug("Message: Picked piece of another player\n");
      break;
    case 6:
      debug("Message: Picked empty piece\n");
      break;
    case 7:
      debug("Message: Illegal lady capture\n");
      break;
    case 8:
      debug("Message: Malformad input\n");
  }
  error = 0;
}

void GamePrintState() {
  int i, j;
  debug("==> Pieces: ");
  for (i = 0; i < 24; i++) debug("%c ", pieces[i].player);

  debug("\n==>   Lady: ");
  for (i = 0; i < 24; i++) debug("%d ", pieces[i].lady);

  debug("\n==>  Board:\n");
  debug("   ");
  for (i = 0; i < 8; i++) {
    debug("% 3d ", i);
  }
  debug("\n");
  for (i = 0; i < 8; i++) {
    debug("%d  ", i);
    for (j = 0; j < 8; j++) {
      if (board[i][j]) {
        debug("% 3d ", board[i][j]);
      } else {
        debug("    ");
      }
    }
    debug("\n");
  }
  debug("\n");

  debug("==> Counters\n");
  debug("Black deads %d\n", deadBlacks);
  debug("White deads %d\n", deadWhites);
}

void GamePrintDiv() {
  printf(" ---");
  for (int i = 0; i < 8; i++) printf("-------");
  printf("\n");
}

void GameDrawBoardOnlyColors() {
  int i, j, k, id, player;
  for (i = 0; i < 8; i++) {
    for (k = 0; k < 2; k++) {
      if (k == 1) {
        printf(" %d  ", i);
      } else {
        printf("    ");
      }
      for (j = 0; j < 8; j++) {
        if ((i % 2 == 0 && j % 2 == 1) || (i % 2 == 1 && j % 2 == 0)) {
          printf("%s", BG_BLACK_COLOR);
        } else {
          printf("%s", BG_WHITE_COLOR);
        }

        if ((px == i && py == j) || (mx == i && my == j))
          printf("%s", MOVE_COLOR);

        if (board[i][j] == 0) {
          printf("     ");
        } else {
          id = board[i][j] - 1;
          player = pieces[id].player;
          printf("%s", player == 'B' ? BOLD_BLUE_COLOR : BOLD_GREEN_COLOR);

          if (pieces[id].lady) {
            if (k == 0)      printf(" █ █ ");
            else if (k == 1) printf(" ███ ");
            else             printf("     ");
          } else {
            if (k == 0)      printf(" ███ ");
            else if (k == 1) printf(" ███ ");
            else             printf("     ");
          }
        }
        printf("%s", RESET_COLOR);
      }
      printf("\n");
    }
  }
  printf("\n    ");
  for (i = 0; i < 8; i++) {
    printf("  %c  ", '0' + i);
  }
  printf("\n");
}

void GameDrawBoardWithCharacteres() {
  int i, j, k, id, player;
  GamePrintDiv();
  for (i = 0; i < 8; i++) {
    for (k = 0; k < 3; k++) {
      if (k == 1) {
        printf(" %d |", i);
      } else {
        printf("   |");
      }
      for (j = 0; j < 8; j++) {
        if ((px == i && py == j) || (mx == i && my == j))
          printf("%s", MOVE_COLOR);
        
        if (board[i][j] == 0) {
          printf("%s", (j + i) % 2 == 0 ? emptyEven[k] : emptyOdd[k]);
        } else {
          id = board[i][j] - 1;
          player = pieces[id].player;
          printf("%s", player == 'B' ? BOLD_BLUE_COLOR : BOLD_GREEN_COLOR);
          char *piece = pieces[id].lady
            ? player == 'B' ? blackLady[k] : whiteLady[k]
            : player == 'B' ? blackPiece[k] : whitePiece[k];
          printf("%s%s", piece, RESET_COLOR);
        }

        if ((px == i && py == j) || (mx == i && my == j))
          printf("%s", RESET_COLOR);
        printf("|");
      }
      printf("\n");
    }
    GamePrintDiv();
  }
  printf("   |");
  for (i = 0; i < 8; i++) {
    printf("   %c  |", '0' + i);
  }
  printf("\n");
}

void GameDrawBoard() {
  GameDrawBoardOnlyColors();
  if (px != -1) {
    printf("\n");
    printf("[%d] Last move (%d,%d) -> (%d,%d) [%s]\n", countMoves - 1, px, py, mx, my, GameGetCurrentPlayer());
  }

  GamePrintState();
}

void GameDrawResult() {
  printf("\n\n");
  if (deadWhites < deadBlacks) {
    printf("%s", BOLD_GREEN_COLOR);
    printf(" __    __ __  __ __ ______  ____    __    __ __ __  __  __  __ __ __ \n");
    printf(" ||    || ||  || || | || | ||       ||    || || ||\\ || (( \\ || || || \n");
    printf(" \\\\ /\\ // ||==|| ||   ||   ||==     \\\\ /\\ // || ||\\\\||  \\\\  || || || \n");
    printf("  \\V/\\V/  ||  || ||   ||   ||___     \\V/\\V/  || || \\|| \\_)) .. .. .. \n");
    printf("                                                                     \n");
  } else if (deadBlacks < deadWhites) {
    printf("%s", BOLD_BLUE_COLOR);
    printf(" ____  __     ___    ___ __ __    __    __ __ __  __  __  __ __ __ \n");
    printf(" || )) ||    // \\\\  //   || //    ||    || || ||\\ || (( \\ || || || \n");
    printf(" ||=)  ||    ||=|| ((    ||<<     \\\\ /\\ // || ||\\\\||  \\\\  || || || \n");
    printf(" ||_)) ||__| || ||  \\\\__ || \\\\     \\V/\\V/  || || \\|| \\_)) .. .. .. \n");
    printf("                                                                   \n");
  } else {
    printf(" ____   ____   ___  __    __      ___   ___  ___  ___  ____ \n");
    printf(" || \\\\  || \\\\ // \\\\ ||    ||     // \\\\ // \\\\ ||\\\\//|| ||    \n");
    printf(" ||  )) ||_// ||=|| \\\\ /\\ //    (( ___ ||=|| || \\/ || ||==  \n");
    printf(" ||_//  || \\\\ || ||  \\V/\\V/      \\\\_|| || || ||    || ||___ \n");
    printf("                                                            \n");
  }
  printf("%s", RESET_COLOR);
}

// ------------------- //
//         Game        //
// ------------------- //
void GameInit() {
  int i, j;
  for (i = 0; i < 8; i++) {
    for (j = 0; j < 8; j++) {
      board[i][j] = 0;
    }
  }

  int black = 1;
  int white = 24;
  for (i = 0; i < 3; i++) {
    for (j = i % 2 == 0; j < 8; j += 2) {
      board[i][j] = black++;
      board[7 - i][7 - j] = white--;
    }
  }

  for (i = 0; i < 12; i++) {
    pieces[i].player = 'B';
    pieces[i + 12].player = 'W';
    pieces[i].lady = pieces[i + 12].lady = 0;
  }

  shift = error = countMoves = deadBlacks = deadWhites = 0;
  px = py = mx = my = -1;
}

char GameTrySimpleMove(char up) {
  char pn = up ? px - 1 : px + 1;
  if (pn == mx) {
    char id = board[px][py];
    if (py + 1 == my || py - 1 == my) { // Move
      board[px][py] = 0;
      board[mx][my] = id;
      return 0;
    }
    error = 2;
    return 0;
  }
  return 1;
}

char GameTryCapture(char up) {
  char id = board[px][py];
  char p1 = up ? px - 1 : px + 1;
  char p2 = up ? px - 2 : px + 2;
  debug("ID: [%d] P1 [%d] P2 [%d]\n", id, p1, p2);
  if (p2 == mx) {
    if (py + 2 == my) {
      if (pieces[board[p1][py + 1] - 1].player != currentPlayer) {
        board[px][py + 0] = 0;
        board[p1][py + 1] = 0;
        board[p2][py + 2] = id;
        deadWhites += currentPlayer == 'B';
        deadBlacks += currentPlayer == 'W';
        return 0;
      }
      error = 3;
      return 1;
    } else if (py - 2 == my) {
      if (pieces[board[p1][py - 1] - 1].player != currentPlayer) {
        board[px][py - 0] = 0;
        board[p1][py - 1] = 0;
        board[p2][py - 2] = id;
        deadWhites += currentPlayer == 'B';
        deadBlacks += currentPlayer == 'W';
        return 0;
      }
      error = 3;
      return 1;
    }
  }
  error = 4;
  return 1;
}

char GameTryMultipleCapture(char up) {
  if (GameTryCapture(up)) return 1;
  debug("LOG: Capture [%s]\n", GameGetCurrentPlayer());

  while (!GameGetCoordinate()) {
    GameDrawBoard();
    sleep(SLEEP_TIME);
    printf("\n\n");
    debug("LOG: Capture [%s]\n", GameGetCurrentPlayer());
    if (GameIsBounded()) {
      if (!GameTryCapture(1) || !GameTryCapture(0)) {
        countMoves++;
        error = 0;
      } else {
        error = 4;
        return 1;
      }
    } else {
      error = 1;
      return 1;
    }
  }
  return 0;
}

char GameTryMove(char up) {
  if (board[mx][my] != 0) {
    error = 4;
    return 1;
  }
  
  if (!GameTrySimpleMove(up)) {
      debug("LOG: Simple Move\n");
      return 0;
  }

  if (!GameTryMultipleCapture(up)) {
    debug("LOG: Capture [%s]\n", GameGetCurrentPlayer());
    return 0;
  }
  
  error = 4;
  return 1;
}

// ------------------- //
//         Loop        //
// ------------------- //
char GameMove() {
  if (error) return 1;
  debug("LOG: Piece [%d, %d]\n", px, py);
  debug("LOG: Move  [%d, %d]\n", mx, my);
  if (GameIsBounded()) {
    char id = board[px][py];
    if (id == 0) {
      error = 6;
      return 1;
    } else {
      if (currentPlayer == pieces[id - 1].player) {
        if (pieces[id - 1].lady) {
          if (px < mx && !GameTryMove(0)) return 0;
          if (px > mx && !GameTryMove(1)) return 0;
          error = 7;
          return 1;
        } else {
          if (currentPlayer == 'B') {
            if (px < mx && !GameTryMove(0)) return 0;
          } else {
            if (px > mx && !GameTryMove(1)) return 0;
          }
        }
        error = 4;
        return 1;
      }
      error = 5;
      return 1;
    }
  } else {
    error = 1;
    return 1;
  }
  return 0;
}

int GameGetCommand() {
  if (strncmp(buffer, "give up", 7) == 0) {
    printf("\nPlayer '%s' give up...", GameGetCurrentPlayer());
    if (currentPlayer == 'B') deadBlacks = 12;
    else deadWhites = 12;
    GameSaveCurrentMove();
    return 0;
  }
  if (strncmp(buffer, "show", 4) == 0) {
    printf("\n");
    GameDrawBoard();
    return 0;
  }
  if (strncmp(buffer, "help", 4) == 0) {
    printf("\n>>> Commands available:\n");
    printf("\n    give up: Leave the match (+20 rounds)");
    printf("\n    show: Show the board");
    printf("\n    help: Show this message\n");
    return 0;
  }

  return 1;
}

void GameGetInput() {
  printf("\n[%d] Player [ %s ]\n", countMoves, GameGetCurrentPlayer());
  printf("Move pxpy mxmy ...: ");
  GameFillBuffer();
  if (buffer[0] != '/') {
    if (GameGetCoordinate() || GameGetCoordinate()) {
      if (GameGetCommand())
        error = 8;
    }
  } else {
    printf("%s\n", buffer);
  }
  printf("\n");
}

void GameUpdate() {
  shift = 0;    // Flush reader
  if (error) return;

  countMoves++;
  GameSaveCurrentMove();
  GameSwapPlayer();
  GameCheckLady(mx, my);
  GameDrawBoard();
}

void GamePlay() {
  GameGetInput();
  GameMove();
  GameUpdate();
  GamePrintErrors();
  sleep(SLEEP_TIME);
}

void GamePlayLocal() {
  GameInit();
  GameDrawBoard();
  
  while (deadBlacks != 12 && deadWhites != 12 && countMoves <= LIMIT_MOVES)
    GamePlay();
  
  GameDrawResult();
}

// ------------------- //
//         Menu        //
// ------------------- //
void GameMenuOptionAbout() {
  unimplemented("About");
}
void GameMenuOptionSettings() {
  unimplemented("Settings");
}
void GameMenuOptionInstructions() {
  unimplemented("Instructions");
}
void GameMenuOptionAccessGame() {
  GameAccessGameMenu();
}
void GameMenuOptionCreateGame() {
  GameCreateGameMenu();
}
void GameMenuOptionPlayLocal() {
  GamePlayLocal();
}
void GameMenuOptionExit() {
  unimplemented("Exit");
}

void GameLogo() {
  char* color1 = BOLD_BLUE_COLOR;
  char* color2 = BOLD_GREEN_COLOR;
  printf("%s ██████╗██╗  ██╗███████╗ ██████╗██╗  ██╗███████╗██████╗ ███████╗\n", color1);
  printf("%s██╔════╝██║  ██║██╔════╝██╔════╝██║ ██╔╝██╔════╝██╔══██╗██╔════╝\n", color2);
  printf("%s██║     ███████║█████╗  ██║     █████╔╝ █████╗  ██████╔╝███████╗\n", color1);
  printf("%s██║     ██╔══██║██╔══╝  ██║     ██╔═██╗ ██╔══╝  ██╔══██╗╚════██║\n", color2);
  printf("%s╚██████╗██║  ██║███████╗╚██████╗██║  ██╗███████╗██║  ██║███████║\n", color1);
  printf("%s ╚═════╝╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚══════╝\n", color2);
  printf("%s                                                                \n", RESET_COLOR);
}

void GameMenu() {
  printf("1 -       Play local      - 1\n");
  printf("2 -  Create game (online) - 2\n");
  printf("3 -  Access game (online) - 3\n");
  printf("4 -      Instructions     - 4\n");
  printf("5 -        Settings       - 5\n");
  printf("6 -         About         - 6\n");
  printf("0 -          Exit         - 0\n");

  int option = -1;

  while (1) {
    printf(">>> ");
    GameFillBuffer();
    sscanf(buffer, "%d", &option);
    if (option >= 0 && option <= 6) break;
    printf("Invalid option! Try again...\n");
  }

  switch(option) {
    case 0: GameMenuOptionExit(); break;
    case 1: GameMenuOptionPlayLocal(); break;
    case 2: GameMenuOptionCreateGame(); break;
    case 3: GameMenuOptionAccessGame(); break;
    case 4: GameMenuOptionInstructions(); break;
    case 5: GameMenuOptionSettings(); break;
    case 6: GameMenuOptionAbout(); break;
  }
}

int main(int argc, char const *argv[]) {
  // GameConfigConsole();
  GameLogo();
  GameMenu();
  return 0;
}
