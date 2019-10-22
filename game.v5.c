#include <stdio.h>
#include <unistd.h>

typedef struct piece {
  char player;  // ["[B]lack", "[W]hite"]
  char alive;   // ["[T]rue",  "[F]alse"]
  char lady;    // ["[T]rue",  "[F]alse"]
} Piece;

char board[8][8];

Piece pieces[24];
char deadBlacks, deadWhites;  // Dead pieces counters

char currentPlayer = 'B'; // ["[B]lack", "[W]hite"]
char error = 0;
int sleepTime = 1; // Seconds

// Controll
int px, py, mx, my;
char buffer[256] = {0};

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

void printState() {
  int i, j;
  printf("==> Pieces\n");
  for (i = 0; i < 24; i++) {
    printf("%c ", pieces[i].player);
  }
  printf("\n");

  printf("==> Alive\n");
  for (i = 0; i < 24; i++) {
    printf("%d ", pieces[i].alive);
  }
  printf("\n");

  printf("==> Lady\n");
  for (i = 0; i < 24; i++) {
    printf("%d ", pieces[i].lady);
  }
  printf("\n");

  printf("==> Board\n");
  printf("   ");
  for (i = 0; i < 8; i++) {
    printf("% 3d ", i);
  }
  printf("\n");
  for (i = 0; i < 8; i++) {
    printf("%d  ", i);
    for (j = 0; j < 8; j++) {
      if (board[i][j]) {
        printf("% 3d ", board[i][j]);
      } else {
        printf("    ");
      }
    }
    printf("\n");
  }
  printf("\n");
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

  // printState();
}

void printErrors(char px, char py, char mx, char my) {
  if (error >= 1)
    printf("ERROR: Piece(%d,%d) Move(%d,%d) Player(%s)\n\t", px, py, mx, my, getCurrentPlayer());

  switch (error) {
    case 1:
      printf("Message: Move out of the bound\n");
      break;
    case 2:
      printf("Message: Illegal simple move\n");
      break;
    case 3:
      printf("Message: Illegal capture of own piece\n");
      break;
    case 4:
      printf("Message: Illegal move\n");
      break;
    case 5:
      printf("Message: Picked piece of another player\n");
      break;
    case 6:
      printf("Message: Picked empty piece\n");
      break;
    case 7:
      printf("Message: Illegal lady capture\n");
      break;
    default:
      break;
  }
  error = 0;
}

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
    pieces[i].alive = pieces[i + 12].alive = 1;
    pieces[i].lady = pieces[i + 12].lady = 0;
  }

  deadBlacks = deadWhites = 0;
}

void swapPlayer() {
  currentPlayer = currentPlayer == 'B' ? 'W' : 'B';
}

char trySimpleMove(char px, char py, char mx, char my, char up) {
  char pn = up ? px - 1 : px + 1;
  if (pn == mx) {
    char id = board[px][py];
    if ((py + 1 == my || py - 1 == my) && (board[mx][my] == 0)) { // Move
      board[px][py] = 0;
      board[mx][my] = id;
      return 0;
    }
    error = 2;
    return 0;
  }
  return 1;
}

char tryCapture(char px, char py, char mx, char my, char up) {
  char id = board[px][py];
  char p1 = up ? px - 1 : px + 1;
  char p2 = up ? px - 2 : px + 2;
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

char tryMove(char px, char py, char mx, char my, char up) {
  if (!trySimpleMove(px, py, mx, my, up)) {
    printf("LOG: Simple Move\n");
    return 0;
  }

  if (!tryCapture(px, py, mx, my, up)) {
    printf("LOG: Capture [%s]\n", getCurrentPlayer());
    return 0;
  }
  
  error = 4;
  return 1;
}

char isBounded(char px, char py, char mx, char my) {
  return px >= 0 && py >= 0 && mx >= 0 && my >= 0 && px < 8 && py < 8 && mx < 8 && my < 8;
}

char move(char px, char py, char mx, char my) {
  printf("LOG: Piece [%d, %d]\n", px, py);
  printf("LOG: Move  [%d, %d]\n", mx, my);
  if (isBounded(px, py, mx, my)) {
    char id = board[px][py];
    if (id == 0) {
      error = 6;
      return 1;
    } else {
      if (currentPlayer == pieces[id - 1].player) {
        if (pieces[id - 1].lady) {
          if (px < mx && !tryMove(px, py, mx, my, 0))
            return 0;
          if (px > mx && !tryMove(px, py, mx, my, 1))
            return 0;
          error = 7;
          return 1;
        } else {
          if (currentPlayer == 'B') {
            if (px < mx && !tryMove(px, py, mx, my, 0))
              return 0;
          } else {
            if (px > mx && !tryMove(px, py, mx, my, 1))
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
  sscanf(buffer, "%d,%d %d,%d", &px, &py, &mx, &my);
  printf("\n");
}

void multipleCapture() {
  int n = indexOfTh(buffer, ' ', 2);
  int shift = n;
  while (error == 0 && n != -1) {
    drawBoard();
    sleep(sleepTime);
    printf("\nPlayer [ %s ]\n\n", getCurrentPlayer());
    px = mx; 
    py = my;
    if(sscanf(buffer + shift, "%d,%d", &mx, &my) == 2) {
      if (isBounded(px, py, mx, my)) {
        tryCapture(px, py, mx, my, 1);
        tryCapture(px, py, mx, my, 0);
        error = 0;
      } else {
        error = 1;
      }
    }

    n = indexOfTh(buffer + shift + 1, ' ', 1);
    shift += n;
  }
}

void play() {
  getInput();
  move(px, py, mx, my);
  multipleCapture();

  if (error == 0) {
    swapPlayer();
    checkLady(mx, my);
    drawBoard();
  }

  printErrors(px, py, mx, my);
  sleep(sleepTime);
}

int main(int argc, char const *argv[]) {
  init();
  drawBoard();
  while (deadBlacks != 12 || deadWhites != 12) play();
  if (deadBlacks == 12) printf("White win!!!");
  if (deadWhites == 12) printf("Black win!!!");
  return 0;
}
