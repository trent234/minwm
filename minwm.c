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
  int* match_len;
  int len;
  int size;
  struct str_list* next;
} str_list;

int free_list(str_list* in);
int print_list(str_list* in, int match_len, int cur_y, int max_y, int cur_x);
int widen_list(str_list* in_list, char* in);
int narrow_list(str_list* in_list, char* in);
int get_prog_list(str_list** in_list);
int get_input(char* user_string, int max_element, int cur_y, int cur_x);
int run();

//frees all str_lists that may be connected as well as their str contents
//in must be either allocated or NULL. same goes for its next ptr
int free_list(str_list* in){
  str_list* temp;
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
int print_list(str_list* in, int match_len, int cur_y, int max_y, int cur_x){
  //while there are lists and we haven't filled available y axis space already
  while(in){
    for(int i = 0; i < in->len && cur_y < max_y; i++){
      if(in->match_len[i] == match_len){
        wmove(stdscr, cur_y++, 1);
        clrtoeol();
        printw("%s", in->p[i]); 
      }
    }
    in = in->next;
  }
  return 0;
}

//used after user deletes a char thus widening the pool
//in = user input 
int widen_list(str_list* in_list, char* in){
  if(!in || !in_list)
    return -1;
  //x = the len of new shortened string 
  int x = strlen(in);
  for(int i = 0; i <= in_list->len; i++){
    if(in_list->match_len[i] > x)
      in_list->match_len[i] = x;
  }
  return 0;
}
//used after user adds a char thus narrowing the pool
//in = user input 
int narrow_list(str_list* in_list, char* in){
  if(!in || !in_list)
    return -1;
  //x = the last element (newly added) of the input string
  int x = strlen(in) - 1;
  for(int i = 0; i < in_list->len; i++){
    //if in is longer than prog name and we're at the end.. go no further
    if(in_list->match_len[i] == strlen(in_list->p[i]))
      continue;
    //if the substr up to this point matches and the latest in char also matches..
    if(in_list->match_len[i] == x && in_list->p[i][x] == in[x])
      in_list->match_len[i]++;
  }
  return 0;
}

//in_list is ref to caller's ptr where results will be stored
//substr is the user input used to narrow the results
//caller is responsible to free memory. be sure to call free_list()
//return codes: 0 = success; -1 = stream failed to open; 2 = stream close fail
int get_prog_list(str_list** in_list){
  int ret_code;
  char buf[100];
  FILE* stream;
  str_list* list = (str_list*) malloc(sizeof(str_list));
  //initializing struct data members that need it
  list->next = NULL;
  list->len = 0;

  //open stream to get count and malloc space for that many strings
  stream = popen("compgen -c | sort | uniq | wc -l | tr -d \'\n\'", "r");
  if (stream == NULL) 
    return -1; //log it once that exists
  fgets(buf, sizeof(buf), stream);
  list->size = atoi(buf);
  list->p = (char**) malloc(sizeof(char*) * list->size);
  //alloc match lengths all to 0. this is the len to which 
  //the string matches the user_string. 
  list->match_len = (int*) malloc(sizeof(int*) * list->size);
  for(int i = 0; i < list->size; i++)
    list->match_len[i] = 0;

  //open stream to get prog names 
  stream = popen("compgen -c | sort | uniq", "r");
  if (stream == NULL) 
    return -1; //log it once that exists
  while(fgets(buf, sizeof(buf), stream)){
    //clever strcspn use from https://stackoverflow.com/questions/2693776/
    buf[strcspn(buf, "\n")] = 0;
    list->p[list->len] = (char*) malloc(sizeof(char) * strlen(buf) + 1);
    strncpy(list->p[list->len++], buf, strlen(buf) + 1); 
  }
  ret_code = pclose(stream);
  if(ret_code == -1)
    return -2;

  //avoid leaking mem. input should either be null or alloc'd.
  //following those rules, this should be safe
  if(*in_list)
    free_list(*in_list);
  //assigning to our param so passed in ptr now has it
  *in_list = list;
  return 0; 
}

//for now input max is width of screen. should be more than enough but
//perhaps the future will be more dynamic and nice
//return codes 0 = success; -1 = no more room; -2 = unrecognized input 
// 1 = it was a delete. **this is used to manage str_len.match_len
int get_input(char* user_string, int max_element, int cur_y, int cur_x){
  int next_position = strnlen(user_string, max_element + 1);
  int user_key = mvwgetch(stdscr, cur_y, cur_x);

  if(next_position > max_element)
    return -1;
  if(user_key == KEY_BACKSPACE || user_key == KEY_DC){
    user_string[next_position - 1] = 0; 
    return 1;
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
  str_list* prog_list = NULL;

  getmaxyx(stdscr, max_y, max_x);
  user_string = (char*) malloc(sizeof(char) * (max_x + 1));
  for(int i = 0; i <= max_x; i++)
    user_string[i] = 0;
  while(true){
    //get full prog list from bash. only when user_string is empty
    if(!strlen(user_string))
      ret_code = get_prog_list(&prog_list);
    ret_code = get_input(user_string, max_x - 1, cur_y, cur_x);
    if (ret_code == 0 || ret_code == 1){
      //inefficient, so refactor this to update only whats needed
      //print_str does clear line but what of old lines not where its printing?
      erase();
      mvwprintw(stdscr, 1, 0, "%s", user_string);
      if(!ret_code) //user added a char
        ret_code = narrow_list(prog_list, user_string);
      else //user deleted a char
        ret_code = widen_list(prog_list, user_string);
      print_list(prog_list, strlen(user_string), 3, max_y, 1);
      //this isn't working at the moment. want cursor to blink after text.
      wmove(stdscr, 1, strlen(user_string));
      wrefresh(stdscr);
    }
  }
  free(user_string); 
  free(prog_list);
}

int main(){
  //start ncurses & global settings
  initscr();
  //allows exotic keys. using so we can use delete as well. maybe unnecessary.
  keypad(stdscr, TRUE);
  //allows input to be immediately available instead of waiting for newline 
  cbreak(); 
  //hide auto output input at cursor location so we have more control 
  noecho();
  //main action
  run();
  //end ncurses
  endwin();

  return 0;
}
