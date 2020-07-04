// main driver of minwm
// trent wilson
// 03 jul 2020

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

//for now input max is width of screen. should be more than enough but
//perhaps the future will be more dynamic and nice
// return codes 0 = success; -1 = no more room; -2 = unrecognized input 
int get_input(char* user_string, int max_element, int cur_y, int cur_x){
  int next_position = strnlen(user_string, max_element + 1);
  int user_key = mvwgetch(stdscr, cur_y, cur_x);

  if(next_position > max_element)
    return -1;

  if(user_key == KEY_BACKSPACE || user_key == KEY_DC){
    user_string[next_position - 1] = '\0'; 
    return 0;
  }
  if(user_key >= 32 || user_key <= 126){
    user_string[next_position] = (char) user_key;
    return 0;
  }  

  return -2;
}

int run(){
  int cur_y = 0, cur_x = 0, max_y = 0, max_x = 0;
  int ret_code = 0;
  char* user_string = NULL;

  getmaxyx(stdscr, max_y, max_x);
  user_string = (char*) malloc(sizeof(char) * (max_x + 1));
  for(int i = 0; i <= max_x; i++){
    user_string[i] = 0;
  }
  while(true){
    ret_code = get_input(user_string, max_x - 1, cur_y, cur_x);
    wmove(stdscr, 1, 0);
    clrtoeol();
    printw("%s", user_string);
    //wmove(stdscr, 0, strnlen(user_string, max_x));
    wmove(stdscr, 1, 1);

    wrefresh(stdscr);
  }

}

int main() {
  //init the terminal in curses mode
  initscr();
  keypad(stdscr, TRUE);
  //allows user input text to be immediately available
  //as opposed to waiting until newline is reached
  cbreak(); 
  noecho();
  //i actually do want the blinking cursor, but can't seem to move it from (0,0)
  //even when using wmove(...) so turning it off for now.
  curs_set(FALSE);

  run();

  //end cleanly
  endwin();

  return 0;
}

