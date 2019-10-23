#include <stdio.h>    // printf, scanf, ...
#include <unistd.h>   // sleep(int)
#include <stdarg.h>   // va_list

int debug(char* fmt, ...) {
#if DEBUG
  // https://stackoverflow.com/questions/150543/forward-an-invocation-of-a-variadic-function-in-c
  int res;

  /* Declare a va_list type variable */
  va_list myargs;

  /* Initialise the va_list variable with the ... after fmt */

  va_start(myargs, fmt);

  /* Forward the '...' to vprintf */
  res = vprintf(fmt, myargs);

  /* Clean up the va_list */
  va_end(myargs);

  return res;
#endif
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
char error = 0;
int sleepTime = 1; // Seconds
int px, py, mx, my;
char buffer[256] = {0};

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
  "......",
  "......",
  "......",
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
  if (mx == 0 || mx == 8) {
    pieces[board[mx][my] - 1].lady = 1;
  }
}

char isBounded() {
  return px >= 0 && py >= 0 && mx >= 0 && my >= 0 && px < 8 && py < 8 && mx < 8 && my < 8;
}

void swapPlayer() {
  currentPlayer = currentPlayer == 'B' ? 'W' : 'B';
}

// ------------------- //
//      Printers       //
// ------------------- //
void printState() {
  int i, j;
  debug("==> Pieces\n");
  for (i = 0; i < 24; i++) {
    debug("%c ", pieces[i].player);
  }
  debug("\n");

  debug("==> Lady\n");
  for (i = 0; i < 24; i++) {
    debug("%d ", pieces[i].lady);
  }
  debug("\n");

  debug("==> Board\n");
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
        if (board[i][j] == 0) {
          printf("%s|", (j + i) % 2 == 0 ? emptyEven[k] : emptyOdd[k]);
        } else {
          id = board[i][j] - 1;
          player = pieces[id].player;
          if (pieces[id].lady) {
            if (player == 'B') {
              printf("%s|", blackLady[k]);
            } else {
              printf("%s|", whiteLady[k]);
            }
          } else {
            if (player == 'B') {
              printf("%s|", blackPiece[k]);
            } else {
              printf("%s|", whitePiece[k]);
            }
          }
        }
      }
      printf("\n");
    }
    printDiv();
  }

  printState();
}

void printErrors() {
  if (error >= 1)
    debug("ERROR: Piece(%d,%d) Move(%d,%d) Player(%s)\n\t", px, py, mx, my, getCurrentPlayer());

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

  deadBlacks = deadWhites = 0;
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

char tryMove(char up) {
  if (board[mx][my] != 0) {
    error = 4;
    return 1;
  }
  
  if (!trySimpleMove(up)) {
    debug("LOG: Simple Move\n");
    return 0;
  }

  if (!tryCapture(up)) {
    debug("LOG: Capture [%s]\n", getCurrentPlayer());
    return 0;
  }
  
  error = 4;
  return 1;
}

char multipleCapture() {
  if (error) return 1;
  int n = indexOfTh(buffer, ' ', 2);
  int shift = 0;
  while (n != -1) {
    shift += n + 1;
    drawBoard();
    sleep(sleepTime);
    printf("\nPlayer [ %s ]\n\n", getCurrentPlayer());
    px = mx; 
    py = my;
    debug("LOG: Input [%s]\n", buffer + shift);
    if(sscanf(buffer + shift, "%d,%d", &mx, &my) == 2) {
      debug("LOG: Piece [%d, %d]\n", px, py);
      debug("LOG: Move  [%d, %d]\n", mx, my);
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
    } else {
      error = 8;
      return 1;
    }
    
    n = indexOfTh(buffer + shift, ' ', 1);
  }
  return 0;
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
          if (px < mx && !tryMove(0))
            return 0;
          if (px > mx && !tryMove(1))
            return 0;
          error = 7;
          return 1;
        } else {
          if (currentPlayer == 'B') {
            if (px < mx && !tryMove(0))
              return 0;
          } else {
            if (px > mx && !tryMove(1))
              return 0;
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
  printf("Player [ %s ]\n", getCurrentPlayer());
  printf("Move px,py mx,my ...: ");
  scanf("%[^\n]", buffer);
  if (sscanf(buffer, "%d,%d %d,%d", &px, &py, &mx, &my) != 4) {
    error = 8;
  }
  printf("\n");
}

void play() {
  getInput();
  move();
  multipleCapture();

  if (error == 0) {
    swapPlayer();
    checkLady(mx, my);
    drawBoard();
  }

  printErrors();
  sleep(sleepTime);
}

int main(int argc, char const *argv[]) {
  init();
  drawBoard();
  while (deadBlacks != 12 && deadWhites != 12) play();
  if (deadBlacks == 12) printf("White win!!!\n");
  if (deadWhites == 12) printf("Black win!!!\n");
  return 0;
}
