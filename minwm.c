// main driver of minwm
// trent wilson
// 03 jul 2020

#include <ncurses.h>

int main() {
  //init the terminal in curses mode
  initscr();
  printw("test");
  //dumps updated virt screen onto real screen
  refresh();
  getch();
  //end cleanly
  endwin();

  return 0;
}

