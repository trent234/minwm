//minwm - a Window Manager Made in NCURSES for mobile Linux
//Copyright (c) 2020 Trent Wilson 
//See README for LICENSE

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

//got ncurses fundamentals from 
//tldp.org/HOWTO/NCURSES-Programming-HOWTO/

//got some fundamentals on how a window manager works from evilwm
//basically the spawn idea (not implemented yet)
//https://github.com/kek/evilwm/blob/master/misc.c

//this will be handy in a few different places:
//exec_paths, list, maybe current procs, probably more things too.
struct str_list {
  char** p;
  int len;
  int size;
} str_list;


//will return a list of processes and programs to choose from
//caller is responsible to free memory. is that a bad idea?
char* list(){
  return NULL;
}

//this function returns a pointer to an array of directory paths
//these paths are given by bash's $PATH and will be used in list() to get
//a list of available programs. 
//caller is responsible to free memory (2D). is that a bad idea?
//add return fail conditions.
int get_exec_paths(struct str_list* list){
  FILE* stream;
  char buf[1024];
  int len;
  
  stream = popen("echo $PATH", "r");
  if (stream == NULL) 
    exit(1); //log it once that exists
  fgets(buf, sizeof(buf), stream);
  pclose(stream);

  list->p = (char**) malloc(sizeof(char*) * 25);
  list->size = 25;
  for (char *p = strtok(buf,":"); p != NULL || 
    list->len == list->size; p = strtok(NULL, ":")){
      len = strlen(p) + 1;
      list->p[list->len] = (char*) malloc(sizeof(char) * len);
      strncpy(list->p[list->len++], p, len);
  }
  return 0;
}

//for now input max is width of screen. should be more than enough but
//perhaps the future will be more dynamic and nice
//return codes 0 = success; -1 = no more room; -2 = unrecognized input 
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
  for(int i = 0; i <= max_x; i++)
    user_string[i] = 0;
  while(true){
    ret_code = get_input(user_string, max_x - 1, cur_y, cur_x);
    if (!ret_code){
      wmove(stdscr, 1, 0);
      clrtoeol();
      printw("%s", user_string);
      //this isn't working at the moment. want cursor to blink after text.
      wmove(stdscr, 1, strnlen(user_string, max_x));
      wrefresh(stdscr);
    }
  }
  free(user_string); //mem mgmt
}

int main() {
  initscr();
  //allows exotic keys. using so we can use delete as well. maybe unnecessary.
  keypad(stdscr, TRUE);
  //allows input to be immediately available instead of waiting for newline 
  cbreak(); 
  //hide auto output input at cursor location so we have more control 
  noecho();
  //i want the blinking cursor but can't move it from (0,0) even using wmove() 
  //curs_set(FALSE);

  //printing it here is for testing only
  struct str_list paths = {NULL,0,0};
  get_exec_paths(&paths);
  for(int i = 0; i < paths.len; i++){
    mvwprintw(stdscr, i + 5, 0,"%s", paths.p[i]); 
  } 
  //char* x = list();

  run();
  endwin();

  //mem mgmt
  for(int i = 0; i < paths.len; i++)
    free(paths.p[i]);
  free(paths.p);

  return 0;
}
