//minwm - a Window Manager Made in NCURSES for mobile Linux
//Copyright (c) 2020 Trent Wilson 
//See README for LICENSE

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

//got some fundamentals on how a window manager works from evilwm
//basically the spawn idea (not implemented yet)
//https://github.com/kek/evilwm/blob/master/misc.c

//this will be handy in a few different places:
//exec_paths, list, maybe current procs, probably more things too.
//p is ptr to arr of char* where user can alloc space to store str
//len is how many currently alloc'd; size is max size of arr
//next allows the lists to be concatenated.. truly list-like
typedef struct str_list {
  char** p;
  int len;
  int size;
  struct str_list* next;
} str_list;

int free_str_list(str_list* in);
int print_str_list(str_list* in, int cur_y, int max_y, int cur_x);
int get_prog_list(str_list** in_list, char* substr);
int get_input(char* user_string, int max_element, int cur_y, int cur_x);
int run();

//frees all str_lists that may be connected as well as their str contents
int free_str_list(str_list* in){
  str_list* temp;
  //while there are lists and we haven't filled available y axis space already
  while(in){
    for(int i = 0; i < in->len; i++)
      free(in->p[i]);
    temp = in;
    in = in->next;
    free(temp);
  }
  return 0;
}

//in = str_list to print off. cur_y / max_y to constrain list len
//x dimension should be input constrained elsewhere (not implemented yet)
int print_str_list(str_list* in, int cur_y, int max_y, int cur_x){
  //while there are lists and we haven't filled available y axis space already
  while(in){
    for(int i = 0; i < in->len && cur_y < max_y; i++){
      wmove(stdscr, cur_y++, 1);
      clrtoeol();
      printw("%s", in->p[i]); 
    }
    in = in->next;
  }
  return 0;
}

//in_list is ref to caller's ptr where results will be stored
//substr is the user input used to narrow the results
//caller is responsible to free memory. be sure to call free_str_list()
//return codes: 0 = success; -1 = stream failed to open; 2 = stream close fail
int get_prog_list(str_list** in_list, char* substr){
  int len, ret_code;
  char* count_cmd;
  char* prog_cmd;
  char buf[100];
  FILE* stream;
  str_list* list = (str_list*) malloc(sizeof(str_list));
  //initializing struct data members that need it
  list->next = NULL;
  list->len = 0;

  if(substr){
    //create count_cmd string with grep
    len = strlen("compgen -c | sort | uniq | grep ") + 
      strlen(substr) + strlen(" | wc -l | tr -d \'\n\'") + 1;
    count_cmd = (char*) malloc(sizeof(char) * len);
    strncpy(count_cmd, "compgen -c | sort | uniq | grep ", len);
    strncat(count_cmd, substr, len);
    strncat(count_cmd, " | wc -l | tr -d \'\n\'", len);
    //create count_cmd string with grep
    len = strlen("compgen -c | sort | uniq | grep ") + strlen(substr) + 1;
    prog_cmd = (char*) malloc(sizeof(char) * len);
    strncpy(prog_cmd, "compgen -c | sort | uniq | grep ", len);
    strncat(prog_cmd, substr, len);
  }
  else{
    //create count_cmd string without grep
    len = strlen("compgen -c | sort | uniq | wc -l | tr -d \'\n\'") + 1;
    count_cmd = (char*) malloc(sizeof(char) * len);
    strncpy(count_cmd, "compgen -c | sort | uniq | wc -l | tr -d \'\n\'", len);
    //create count_cmd string without grep
    len = strlen("compgen -c | sort | uniq") + 1;
    prog_cmd = (char*) malloc(sizeof(char) * len);
    strncpy(prog_cmd, "compgen -c | sort | uniq", len);
  }
  //open stream to get count and malloc space for that many strings
  stream = popen(count_cmd, "r");
  if (stream == NULL) 
    return -1; //log it once that exists
  fgets(buf, sizeof(buf), stream);
  list->size = atoi(buf);
  list->p = (char**) malloc(sizeof(char*) * list->size);

  //open stream to get prog names 
  stream = popen(prog_cmd, "r");
  if (stream == NULL) 
    return -1; //log it once that exists
  while(fgets(buf, sizeof(buf), stream)){
    //clever strcspn use from:
    //https://stackoverflow.com/questions/2693776/
    buf[strcspn(buf, "\n")] = 0;
    list->p[list->len] = (char*) malloc(sizeof(char) * strlen(buf) + 1);
    strncpy(list->p[list->len++], buf, strlen(buf)); 
  }
  ret_code = pclose(stream);
  if(ret_code == -1)
    return -2;

  //assigning to our param so passed in ptr now has it
  *in_list = list;
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
  str_list* prog_list;

  getmaxyx(stdscr, max_y, max_x);
  user_string = (char*) malloc(sizeof(char) * (max_x + 1));
  for(int i = 0; i <= max_x; i++)
    user_string[i] = 0;
  while(true){
    ret_code = get_input(user_string, max_x - 1, cur_y, cur_x);
    if (!ret_code){
      //horribly inefficient, so refactor this to update only whats needed
      erase();
      wmove(stdscr, 1, 0);
      //clrtoeol();
      printw("%s", user_string);
      //this isn't working at the moment. want cursor to blink after text.
      wmove(stdscr, 0, strnlen(user_string, max_x));
      //get prog list & print it
      //this needs refactor. store results in arr where index = user_input pos
      //so user delete goes back to previously calc'd result
      //change grep so substr has to be at beginning
      //bug where we delete back to 0 has grep output errors into our list
      //bug where artifacts exist at end of prog name e.g. "^?"
      ret_code = get_prog_list(&prog_list, user_string);
      print_str_list(prog_list, 3, max_y, 1);
      wrefresh(stdscr);
      //shouldn't be freeing and allocing after every key press. refactor.
      free(prog_list);
      //be sure to free these ^
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
  run();
  endwin();

  return 0;
}
