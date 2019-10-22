#include <stdio.h>

char board[8][8];

char pieces[24];      // ["[B]lack", "[W]hite"]
char alive[24];       // ["[T]rue",  "[F]alse"]
char lady[24];        // ["[T]rue",  "[F]alse"]
char deadBlacks, deadWhites;  // Dead pieces counters

char currentPlayer = 'B'; // ["[B]lack", "[W]hite"]
char error = 0;

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

void checkLady(int mx, int my) {
  if (my == 0 || my == 8) {
    lady[board[mx][my]] = 1;
  }
}

void printDiv() {
  printf(" ---");
  for (int i = 0; i < 8; i++) {
    printf("-------");
  }
  printf("\n");
}

void drawBoard() {
  char player;
  int i, j, k;
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
          player = pieces[board[i][j] - 1];
          if (player == 'B') {
            printf("%s|", blackPiece[k]);
          } else {
            printf("%s|", whitePiece[k]);
          }
        }
      }
      printf("\n");
    }
    printDiv();
  }
}

void printState() {
  int i, j;
  printf("==> Pieces\n");
  for (i = 0; i < 24; i++) {
    printf("%c ", pieces[i]);
  }
  printf("\n");

  printf("==> Alive\n");
  for (i = 0; i < 24; i++) {
    printf("%d ", alive[i]);
  }
  printf("\n");

  printf("==> Lady\n");
  for (i = 0; i < 24; i++) {
    printf("%d ", lady[i]);
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
    pieces[i] = 'B';
    pieces[i + 12] = 'W';
    alive[i] = alive[i + 12] = 1;
    lady[i] = lady[i + 12] = 0;
  }

  deadBlacks = deadWhites = 0;

}

void swapPlayer() {
  currentPlayer = currentPlayer == 'B' ? 'W' : 'B';
}

void doSimpleMove(char px, char py, char mx, char my) {
  char id = board[px][py];
  if (py + 1 == my || py - 1 == my) {
    if (board[mx][my] == 0) { // Move
      printf("LOG: Piece moved\n");
      board[px][py] = 0;
      board[mx][my] = id;
      swapPlayer();
    } else { // Illegal
      error = 2;
    }
  } else { // Illegal
    error = 2;
  }
}

void play() {
  printf("Player [ %s ]\n", currentPlayer == 'B' ? "Black" : "White");
  int px, py, mx, my, id, id2;
  printf("Piece [x,y]: ");
  scanf("%d,%d", &px, &py);
  printf("Move [x,y]: ");
  scanf("%d,%d", &mx, &my);
  printf("\n");

  printf("LOG: Piece [%d, %d]\n", px, py);
  printf("LOG: Move  [%d, %d]\n", mx, my);
  printf("\n");

  if (px >= 0 && py >= 0 && mx >= 0 && my >= 0) {
    if (px < 8 && py < 8 && mx < 8 && my < 8) {
      printf("LOG: Valid ranges\n");
      id = board[px][py];
      if (id != 0 && currentPlayer == pieces[id - 1]) {
        printf("LOG: Piece picked is from current player [%s]\n", currentPlayer == 'B' ? "Black" : "White");
        if (currentPlayer == 'B') {
          if (px < mx) {  // Foraward
            if (px + 1 == mx) { // Simple move
              doSimpleMove(px, py, mx, my);
            } else if (px + 2 == mx) {  // Capture
              if (py + 2 == my) {
                id2 = board[px + 1][py + 1] - 1;
                if (pieces[id2] != currentPlayer) {
                  printf("LOG: Capture\n");
                  board[px + 0][py + 0] = 0;
                  board[px + 1][py + 1] = 0;
                  board[px + 2][py + 2] = id;
                  deadBlacks++;
                  swapPlayer();
                } else { // Illegal
                  error = 3;
                }
              } else if (py - 2 == my) {
                id2 = board[px + 1][py - 1] - 1;
                if (pieces[id2] != currentPlayer) {
                  printf("LOG: Capture\n");
                  board[px + 0][py - 0] = 0;
                  board[px + 1][py - 1] = 0;
                  board[px + 2][py - 2] = id;
                  deadBlacks++;
                  swapPlayer();
                } else { // Illegal
                  error = 3;
                }
              } else {  // Illegal
                error = 4;
              }
            } else {  // Illegal
              error = 4;
            }
          } else {  // Illegal
            error = 4;
          }
        } else {
          if (px > mx) { // Backward
            if (px - 1 == mx) { // Simple move
              doSimpleMove(px, py, mx, my);
            } else if (px - 2 == mx) {  // Capture
              if (py + 2 == my) {
                id2 = board[px - 1][py + 1] - 1;
                if (pieces[id2] != currentPlayer) {
                  printf("LOG: Capture\n");
                  board[px - 0][py + 0] = 0;
                  board[px - 1][py + 1] = 0;
                  board[px - 2][py + 2] = id;
                  deadWhites++;
                  swapPlayer();
                } else { // Illegal
                  error = 3;
                }
              } else if (py - 2 == my) {
                id2 = board[px - 1][py - 1] - 1;
                if (pieces[id2] != currentPlayer) {
                  printf("LOG: Capture\n");
                  board[px - 0][py - 0] = 0;
                  board[px - 1][py - 1] = 0;
                  board[px - 2][py - 2] = id;
                  deadWhites++;
                  swapPlayer();
                } else { // Illegal
                  error = 3;
                }
              } else {
                error = 4;
              }
            } else {  // Illegal
              error = 4;
            }
          } else {
            error = 4;
          }
        }
      } else {
        error = 5;
      }
      printf("\n");
    } else {
      error = 1;
    }
  } else {
    error = 1;
  }

  switch (error) {
    case 0:
      checkLady(mx, my);
      drawBoard();
      // printState();
      break;
    case 1:
      printf("ERROR: Move out of the bound: piece(%d,%d), move(%d,%d)\n", px, py, mx, my);
      break;
    case 2:
      printf("ERROR: Illegal simple move: piece(%d,%d), move(%d,%d)\n", px, py, mx, my);
      break;
    case 3:
      printf("ERROR: Illegal capture of own piece: piece(%d,%d) move(%d,%d) %s\n", px, py, mx, my, pieces[board[px][py] - 1] == 'B' ? "Black" : "White");
      break;
    case 4:
      printf("ERROR: Illegal move: piece(%d,%d), move(%d,%d)\n", px, py, mx, my);
      break;
    case 5:
      printf("ERROR: Picked piece of another player [%s]\n", pieces[board[px][py] - 1] == 'B' ? "Black" : "White");
    default:
      break;
  }
  error = 0;
}

int main(int argc, char const *argv[]) {
  init();
  // printState();
  drawBoard();
  while (deadBlacks != 12 || deadWhites != 12) {
    play();
  }
  if (deadBlacks == 12) {
    printf("White win!!!");
  }
  if (deadWhites == 12) {
    printf("Black win!!!");
  }
  return 0;
}
