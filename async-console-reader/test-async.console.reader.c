#include "./async-console-reader.c"

void keyboardHandle(char ch) {
}
void listenerHandle(char* input) {
  printf(" Listening %s\n", input);
}
void consumerHandle(char* input) {
  printf(" Consuming %s\n", input);
}
void arrowsHandle(char arrow) {
  printf(" Arrow press ");
  switch (arrow) {
    case KEY_UP:
      printf("UP\n");
      break;
    case KEY_DOWN:
      printf("DOWN\n");
      break;
    case KEY_LEFT:
      printf("LEFT\n");
      break;
    case KEY_RIGHT:
      printf("RIGHT\n");
      break;
    default:
      printf("Arrows console reader error!\n");
      break;
  }
}

int main() {
  ConsoleInit();
  ConsoleSetConsumer(consumerHandle);
  ConsoleSetListener(listenerHandle);
  ConsoleSetArrowKeyListener(arrowsHandle);
  ConsoleSetKeyboardListener(keyboardHandle);
  ConsoleRun();
  return 0;
}