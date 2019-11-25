#ifndef ASYNC_CONSOLE_READER
#define ASYNC_CONSOLE_READER
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <pthread.h>

// https://stackoverflow.com/questions/1798511/how-to-avoid-pressing-enter-with-getchar
#include <termios.h>
#include <unistd.h>

// https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c
#include <signal.h>

#define KEY_UP 65
#define KEY_DOWN 66
#define KEY_LEFT 67
#define KEY_RIGHT 68

#define KEY_BACKSPACE 127
#define INPUT_LENGTH 16

static struct {
  struct termios oldt, newt;
  char input[INPUT_LENGTH];

  int filled, cursor;
  int special_1, special_2, special;
  int wait_key, wait_consume;

  void (*callback_arrow_key_listener)(char);
  void (*callback_keyboard_listener)(char);
  void (*callback_consumer)(char*);
  void (*callback_listener)(char*);
  void (*callback_exit)();
} ConsoleProps = {0};

void ConsoleSignalCallback(int sig);

void ConsoleSetArrowKeyListener(void (*callback)(char)) {
  ConsoleProps.callback_arrow_key_listener = callback;
}
void ConsoleSetKeyboardListener(void (*callback)(char)) {
  ConsoleProps.callback_keyboard_listener = callback;
}
void ConsoleSetConsumer(void (*callback)(char*)) {
  ConsoleProps.callback_consumer = callback;
}
void ConsoleSetListener(void (*callback)(char*)) {
  ConsoleProps.callback_listener = callback;
}

void ConsoleWaitKey() {
  while (!ConsoleProps.wait_key);
  ConsoleProps.wait_key = 0;
}
void ConsoleWaitConsume() {
  while (!ConsoleProps.wait_consume);
  ConsoleProps.wait_consume = 0;
}

void ConsolePutInput(char ch) {
  if (ch == KEY_BACKSPACE) {
    if (ConsoleProps.cursor > 0) {
      if (ConsoleProps.cursor == ConsoleProps.filled)
        ConsoleProps.filled--;
      ConsoleProps.cursor--;
    }
  } else {
    if (ConsoleProps.cursor < INPUT_LENGTH) {
      if (ConsoleProps.cursor == ConsoleProps.filled)
        ConsoleProps.filled++;
      ConsoleProps.input[ConsoleProps.cursor++] = ch;
      ConsoleProps.input[ConsoleProps.filled] = 0;
    }
  }
}
void ConsoleFillInput(char ch) {
  if (ConsoleProps.special_1 == 0) {
    if (ch == 27) {
      ConsoleProps.special_1 = 1;
      ConsoleProps.special = 1;
    } else {
      ConsoleProps.special = 0;
      ConsolePutInput(ch);
    }
  } else {
    if (ConsoleProps.special_2 == 0) {
      if (ch == 91) {
        ConsoleProps.special_2 = 1;
        ConsoleProps.special = 1;
      } else {
        ConsoleProps.special_1 = 0;
        ConsoleProps.special = 0;
        ConsolePutInput('^');
        ConsolePutInput(ch);
      }
    } else {
      // arrow key is pressed
      if (ConsoleProps.callback_arrow_key_listener)
        ConsoleProps.callback_arrow_key_listener(ch);

      ConsoleProps.special_1 = ConsoleProps.special_2 = 0;
      ConsoleProps.special = 1;
    }
  }
}

static void *_ConsoleRun(void *args) {
  char ch;
  
  while (1) {
    ch = getchar();

    if (ch == '\n') {
      if (ConsoleProps.callback_consumer)
        ConsoleProps.callback_consumer(ConsoleProps.input);

      ConsoleProps.wait_consume = 1;
      ConsoleProps.filled = ConsoleProps.cursor = 0;
      ConsoleProps.input[0] = 0;
    } else {
      ConsoleFillInput(ch);

      if (!ConsoleProps.special) {
        if (ConsoleProps.callback_keyboard_listener)
          ConsoleProps.callback_keyboard_listener(ch);

        if (ConsoleProps.callback_listener)
          ConsoleProps.callback_listener(ConsoleProps.input);
        
        ConsoleProps.wait_key = 1;
      }
    }
  }
}
void ConsoleRun() {
  _ConsoleRun(NULL);
}

void ConsoleStart(pthread_t* thread) {
  pthread_create(thread, NULL, _ConsoleRun, NULL);
}

void ConsoleInit() {
  tcgetattr(STDIN_FILENO, &ConsoleProps.oldt);
  ConsoleProps.newt = ConsoleProps.oldt;
  ConsoleProps.newt.c_lflag &= ~(ICANON);
  tcsetattr(STDIN_FILENO, TCSANOW, &ConsoleProps.newt);

  signal(SIGINT, ConsoleSignalCallback);
}
void ConsoleClose() {
  tcsetattr(STDIN_FILENO, TCSANOW, &ConsoleProps.oldt);
}

void ConsoleSignalCallback(int sig) {
  ConsoleClose();
  exit(0);
}

#endif