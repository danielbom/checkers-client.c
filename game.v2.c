#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

char board[8][8];

char pieces[24];      // ["[B]lack", "[W]hite"]
char alive[24];       // ["[T]rue",  "[F]alse"]
char lady[24];        // ["[T]rue",  "[F]alse"]
char deadBlacks, deadWhites;  // Dead pieces counters

char currentPlayer = 'B'; // ["[B]lack", "[W]hite"]
char error = 0;
int sleepTime = 1; // Seconds

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
  int i, j, k, player;
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

void printErrors(char px, char py, char mx, char my) {
  switch (error) {
    case 1:
      printf("ERROR: Move out of the bound: piece(%d,%d), move(%d,%d)\n", px, py, mx, my);
      break;
    case 2:
      printf("ERROR: Illegal simple move: piece(%d,%d), move(%d,%d)\n", px, py, mx, my);
      break;
    case 3:
      printf("ERROR: Illegal capture of own piece: piece(%d,%d) move(%d,%d) %s\n", px, py, mx, my, getCurrentPlayer());
      break;
    case 4:
      printf("ERROR: Illegal move: piece(%d,%d), move(%d,%d)\n", px, py, mx, my);
      break;
    case 5:
      printf("ERROR: Picked piece of another player [%s]\n", getCurrentPlayer());
      break;
    case 6:
      printf("ERROR: Picked empty piece\n");
      break;
    default:
      break;
  }
  if (error) {
    exit(0);
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
  }
  return 1;
}

char tryCapture(char px, char py, char mx, char my, char up) {
  char id = board[px][py];
  char p1 = up ? px - 1 : px + 1;
  char p2 = up ? px - 2 : px + 2;
  if (p2 == mx) {
    if (py + 2 == my) {
      if (pieces[board[p1][py + 1] - 1] != currentPlayer) {
        board[px][py + 0] = 0;
        board[p1][py + 1] = 0;
        board[p2][py + 2] = id;
        return 0;
        if (currentPlayer == 'B') {
          deadWhites++;
        } else {
          deadBlacks++;
        }
      } else { // Illegal
        error = 3;
      }
    } else if (py - 2 == my) {
      if (pieces[board[p1][py - 1] - 1] != currentPlayer) {
        board[px][py - 0] = 0;
        board[p1][py - 1] = 0;
        board[p2][py - 2] = id;
        if (currentPlayer == 'B') {
          deadWhites++;
        } else {
          deadBlacks++;
        }
        return 0;
      } else { // Illegal
        error = 3;
      }
    } else {
      error = 4;
    }
  }
  return 1;
}

char tryBackward(char px, char py, char mx, char my) {
  char id2, id = board[px][py];
  if (trySimpleMove(px, py, mx, my, 1)) {
    if (tryCapture(px, py, mx, my, 1)) {
      error = 4;
      return 1;
    }
  }
  return 0;
}

char tryForaward(char px, char py, char mx, char my) {
  char id2, id = board[px][py];
  if (trySimpleMove(px, py, mx, my, 0)) {
    if (tryCapture(px, py, mx, my, 0)) {
      error = 4;
      return 1;
    }
  }
  return 0;
}

char isBounded(char px, char py, char mx, char my) {
  return px >= 0 && py >= 0 && mx >= 0 && my >= 0 && px < 8 && py < 8 && mx < 8 && my < 8;
}

char move(char px, char py, char mx, char my) {
  if (isBounded(px, py, mx, my)) {
    char id = board[px][py];
    if (id == 0) {
      error = 6;
      return 1;
    } else if (currentPlayer == pieces[id - 1]) {
      if (currentPlayer == 'B') {
        if (px < mx && tryForaward(px, py, mx, my)) {
          error = 4;
        }
      } else {
        if (px > mx && tryBackward(px, py, mx, my)) {
          error = 4;
        }
      }
    } else {
      error = 5;
    }
  } else {
    error = 1;
  }
}

void play() {
  int px, py, mx, my, id, id2;
  char buffer[256] = {0};
  setbuf(stdin , NULL);

  printf("Player [ %s ]\n", getCurrentPlayer());
  printf("Move px,py mx,my ...: ");
  scanf("%[^\n]", buffer);
  sscanf(buffer, "%d,%d %d,%d", &px, &py, &mx, &my);
  printf("\n");

  move(px, py, mx, my);

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

  if (error == 0) {
    swapPlayer();
    checkLady(mx, my);
    drawBoard();
    // printState();
  }

  printErrors(px, py, mx, my);
  sleep(sleepTime);
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
