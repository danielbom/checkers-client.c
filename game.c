#include <stdio.h>    // printf, scanf, ...
#include <unistd.h>   // sleep(int)
#include <stdarg.h>   // va_list
#include "colorize.c"

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

typedef struct piece {
  char player;  // ["[B]lack", "[W]hite"]
  char lady;    // ["[T]rue",  "[F]alse"]
} Piece;

// ------------------- //
//      Controll       //
// ------------------- //
char board[8][8];
Piece pieces[24];
char deadBlacks, deadWhites;  // Dead pieces counters

char currentPlayer = 'B'; // ["[B]lack", "[W]hite"]

char *MOVE_COLOR = REVERSE_COLOR;
int SLEEP_TIME = 1; // Seconds

int countMoves = 0;
int LIMIT_MOVES = 80;

char error;

int px, py, mx, my;
char buffer[256] = {0};
int shift;

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
  "X    X",
  " XXXX "
};
char blackLady[3][7] = {
  " \\__/ ",
  "OOOOOO",
  " OOOO "
};
char whiteLady[3][7] = {
  " \\__/ ",
  "X    X",
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
char BLACK[] = "Black";
char WHITE[] = "White";
char* getCurrentPlayer() {
  return currentPlayer == 'B' ? BLACK : WHITE;
}

int indexOfTh(char* str, char ch, int th) {
  while (*str == ' ') str++; // lstrip
  
  int count = 0;
  for (int i = 0; str[i]; i++) {
    if (str[i] == ch) {
      count++;
      if (count == th) return i;
      while (str[i] == ch) i++;
      if (str[i] == 0) i--;
    }
  }
  return -1;
}

void checkLady(int mx, int my) {
  if (mx == 0 || mx == 7) {
    pieces[board[mx][my] - 1].lady = 1;
  }
}

char isBounded() {
  return px >= 0 && py >= 0 && mx >= 0 && my >= 0 && px < 8 && py < 8 && mx < 8 && my < 8;
}

void swapPlayer() {
  currentPlayer = currentPlayer == 'B' ? 'W' : 'B';
}

int getCoordinate(char *input) {
  px = mx;
  py = my;
  return sscanf(input, "%d,%d", &mx, &my);
}

// ------------------- //
//     Persistence     //
// ------------------- //
void saveCurrentMove() {
  char filename[] = "moves.txt";
  FILE* file = fopen(filename, countMoves == 1 ? "w+" : "a+");
  if (file == NULL) {
    debug("Não foi possível abrir o arquivo %s\n", filename);
  }

  fprintf(file, "%s\n", buffer);

  fclose(file);
}

// ------------------- //
//      Printers       //
// ------------------- //
void printState() {
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

void printDiv() {
  printf(" ---");
  for (int i = 0; i < 8; i++) {
    printf("-------");
  }
  printf("\n");
}

void drawBoard() {
  int i, j, k, id, player;
  printf("   |");
  for (i = 0; i < 8; i++) {
    printf("   %c  |", '0' + i);
  }
  printf("\n");
  printDiv();
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
          if (pieces[id].lady) {
            printf("%s", player == 'B' ? blackLady[k] : whiteLady[k]);
          } else {
            printf("%s", player == 'B' ? blackPiece[k] : whitePiece[k]);
          }
          printf("%s", RESET_COLOR);
        }

        if ((px == i && py == j) || (mx == i && my == j))
          printf("%s", RESET_COLOR);
        printf("|");
      }
      printf("\n");
    }
    printDiv();
  }

  if (px != -1) {
    printf("\n");
    printf("Last move (%d,%d) -> (%d,%d) [%s]\n", px, py, mx, my, getCurrentPlayer());
  }

  printState();
}

void drawResult() {
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

void printErrors() {
  if (error >= 1)
    debug("ERROR: Piece(%d,%d) Move(%d,%d) Player(%s) CurrentMove(%d)\n\t", px, py, mx, my, getCurrentPlayer(), countMoves);

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

// ------------------- //
//         Game        //
// ------------------- //
void init() {
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

char trySimpleMove(char up) {
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

char tryCapture(char up) {
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

char tryMultipleCapture(char up) {
  if (tryCapture(up)) return 1;
  debug("LOG: Capture [%s]\n", getCurrentPlayer());

  if (error) return 1;

  int n = indexOfTh(buffer + shift, ' ', 1);
  while (n != -1) {
    shift += n + 1;
    countMoves++;
    debug("LOG: Input [%s]\n", buffer + shift);
    if(getCoordinate(buffer + shift) == 2) {
      if (isBounded()) {
        if (!tryCapture(1) || !tryCapture(0)) {
          debug("LOG: Capture [%s]\n", getCurrentPlayer());
          error = 0;
        } else {
          error = 4;
          return 1;
        }
      } else {
        error = 1;
        return 1;
      }
      drawBoard();
      printf("\nPlayer [ %s ]\n", getCurrentPlayer());
      sleep(SLEEP_TIME);
      debug("LOG: Piece [%d, %d] -> Move [%d, %d]\n", px, py, mx, my);
    } else {
      error = 8;
      return 1;
    }
    
    n = indexOfTh(buffer + shift, ' ', 1);
  }
  return 0;
}

char tryMove(char up) {
  if (board[mx][my] != 0) {
    error = 4;
    return 1;
  }
  
  if (!trySimpleMove(up)) {
    debug("LOG: Simple Move\n");
    return 0;
  }

  if (!tryMultipleCapture(up)) {
    debug("LOG: Capture [%s]\n", getCurrentPlayer());
    return 0;
  }
  
  error = 4;
  return 1;
}

char move() {
  if (error) return 1;
  debug("LOG: Piece [%d, %d]\n", px, py);
  debug("LOG: Move  [%d, %d]\n", mx, my);
  if (isBounded()) {
    char id = board[px][py];
    if (id == 0) {
      error = 6;
      return 1;
    } else {
      if (currentPlayer == pieces[id - 1].player) {
        if (pieces[id - 1].lady) {
          if (px < mx && !tryMove(0)) return 0;
          if (px > mx && !tryMove(1)) return 0;
          error = 7;
          return 1;
        } else {
          if (currentPlayer == 'B') {
            if (px < mx && !tryMove(0)) return 0;
          } else {
            if (px > mx && !tryMove(1)) return 0;
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

void getInput() {
  setbuf(stdin , NULL);
  printf("[%d] Player [ %s ]\n", countMoves, getCurrentPlayer());
  printf("Move px,py mx,my ...: ");
  scanf("%[^\n]", buffer);
  shift = indexOfTh(buffer, ' ', 1) + 1;
  if (shift == -1 || getCoordinate(buffer) != 2 || getCoordinate(buffer + shift) != 2)
    error = 8;
  printf("\n");
}

void update() {
  if (error != 0) return;

  countMoves++;
  saveCurrentMove();
  swapPlayer();
  checkLady(mx, my);
  drawBoard();
}

void play() {
  getInput();
  move();
  update();
  printErrors();
  sleep(SLEEP_TIME);
}

int main(int argc, char const *argv[]) {
  init();
  drawBoard();
  
  while (deadBlacks != 12 && deadWhites != 12 && countMoves <= LIMIT_MOVES)
    play();
  
  drawResult();
  return 0;
}
