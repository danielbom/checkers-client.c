#include "colorize.c"
#include <stdio.h>

int main() {
  printf("%s ██████╗██╗  ██╗███████╗ ██████╗██╗  ██╗███████╗██████╗ ███████╗\n", RED_COLOR);
  printf("%s██╔════╝██║  ██║██╔════╝██╔════╝██║ ██╔╝██╔════╝██╔══██╗██╔════╝\n", GREEN_COLOR);
  printf("%s██║     ███████║█████╗  ██║     █████╔╝ █████╗  ██████╔╝███████╗\n", YELLOW_COLOR);
  printf("%s██║     ██╔══██║██╔══╝  ██║     ██╔═██╗ ██╔══╝  ██╔══██╗╚════██║\n", BLUE_COLOR);
  printf("%s╚██████╗██║  ██║███████╗╚██████╗██║  ██╗███████╗██║  ██║███████║\n", CYAN_COLOR);
  printf("%s ╚═════╝╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚══════╝\n", RESET_COLOR);
  printf("                                                                \n");

  printf("%s ██████╗██╗  ██╗███████╗ ██████╗██╗  ██╗███████╗██████╗ ███████╗\n", BOLD_RED_COLOR);
  printf("%s██╔════╝██║  ██║██╔════╝██╔════╝██║ ██╔╝██╔════╝██╔══██╗██╔════╝\n", BOLD_GREEN_COLOR);
  printf("%s██║     ███████║█████╗  ██║     █████╔╝ █████╗  ██████╔╝███████╗\n", BOLD_YELLOW_COLOR);
  printf("%s██║     ██╔══██║██╔══╝  ██║     ██╔═██╗ ██╔══╝  ██╔══██╗╚════██║\n", BOLD_BLUE_COLOR);
  printf("%s╚██████╗██║  ██║███████╗╚██████╗██║  ██╗███████╗██║  ██║███████║\n", BOLD_CYAN_COLOR);
  printf("%s ╚═════╝╚═╝  ╚═╝╚══════╝ ╚═════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝╚══════╝\n", REVERSE_COLOR);
  printf("%s                                                                \n", RESET_COLOR);

  char phrase[] = "Lorem ipsum dolor sit amet.";

  printf("%s%s\n", RED_COLOR, phrase);
  printf("%s%s\n", GREEN_COLOR, phrase);
  printf("%s%s\n", YELLOW_COLOR, phrase);
  printf("%s%s\n", BLUE_COLOR, phrase);
  printf("%s%s\n", CYAN_COLOR, phrase);
  printf("%s%s\n", RESET_COLOR, phrase);
  printf("\n");
  printf("%s%s\n", BOLD_RED_COLOR, phrase);
  printf("%s%s\n", BOLD_GREEN_COLOR, phrase);
  printf("%s%s\n", BOLD_YELLOW_COLOR, phrase);
  printf("%s%s\n", BOLD_BLUE_COLOR, phrase);
  printf("%s%s\n", BOLD_CYAN_COLOR, phrase);
  printf("%s%s\n", REVERSE_COLOR, phrase);
  printf("%s%s\n", RESET_COLOR, phrase);
  printf("\n");
  printf("%s%s\n", BG_BLACK_COLOR, phrase);
  printf("%s%s\n", BG_RED_COLOR, phrase);
  printf("%s%s\n", BG_GREEN_COLOR, phrase);
  printf("%s%s\n", BG_YELLOW_COLOR, phrase);
  printf("%s%s\n", BG_BLUE_COLOR, phrase);
  printf("%s%s\n", BG_CYAN_COLOR, phrase);
  printf("%s%s\n", BG_WHITE_COLOR, phrase);
  printf("%s%s\n", RESET_COLOR, phrase);


  return 0;
}
