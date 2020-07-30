//minwm - a Window Manager Made in NCURSES for mobile Linux
//Copyright (c) 2020 Trent Wilson 
//See README for LICENSE

#include <stdlib.h>
#include <string.h>
#include <ncurses.h>

#define BAR_WIN 0
#define INPUT_WIN 1
#define LIST_WIN 2

//some fundamentals on how a window manager works from evilwm
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
int print_list(WINDOW* win, str_list* in, int match_len);
int widen_list(str_list* in_list, char* in);
int narrow_list(str_list* in_list, char* in);
int get_prog_list(str_list** in_list);
int process_input(WINDOW* win, char* user_string);
WINDOW** create_windows();
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

//in = str_list to print off. should be alloc'd or NULL, not dangling 
int print_list(WINDOW* win, str_list* in, int match_len){
  //spacing = 1 for border 1 for space (1 side) when need both sides: 2*spacing
  int max_x, max_y, i = 0, spacing_y = 1, spacing_x = 2;

  getmaxyx(win, max_y, max_x);
  //while there are lists 
  while(in){
    //while there are still unvisited rows in our list window
    for(int cur_y = spacing_y; cur_y < max_y - spacing_y; cur_y++){
      //set the cursor on working row and clear any old junk from this row 
      wmove(win, cur_y, spacing_x);
      wclrtoeol(win);
      //find next prog in list that matches the user string
      while(i < in->len && in->match_len[i] != match_len)
        i++;
      //if we haven't run out of matches in the prog list
      if(i < in->len){
	//print each char taking care to not go past max_x width for this win
	for(int j = 0; j < strlen(in->p[i]) && j < max_x - (2 * spacing_x); j++)
          mvwaddch(win, cur_y, j + spacing_x, in->p[i][j]);
	i++;
      }
    }
    in = in->next;
  }
  wrefresh(win);
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
  stream = popen("echo -n $PATH | tr : \'\\0\' | xargs -0 -I {} find {} -maxdepth 1 -executable -type f -print |  sed \'s/.*\\///g\' | sort -u | wc -l", "r");

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
  stream = popen("echo -n $PATH | tr : \'\\0\' | xargs -0 -I {} find {} -maxdepth 1 -executable -type f -print |  sed \'s/.*\\///g\' | sort -u", "r");
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
//refactor: ESC, middle mouse, and probs others still result in bad behavior
int process_input(WINDOW* win, char* user_string){
  int ret_code, max_x, next_position, user_key, spacing;

  spacing = 2; //border char + space = 2 (one side)
  next_position = strlen(user_string);
  max_x = getmaxx(win);
  //this puts our cursor in the right spot while waiting for input
  wmove(win, 1, spacing + next_position);
  user_key = wgetch(win);
  if(user_key == KEY_BACKSPACE || user_key == KEY_DC){
    user_string[next_position - 1] = 0; 
    //wipe last char from screen bc we are micromanaging this now...
    mvwaddch(win, 1, spacing + next_position - 1, ' ');
    wmove(win, 1, spacing + next_position - 1);
    ret_code = 1;
  }
  // magic bits. -1 bc next_pos is 0 based element. 
  else if(next_position > max_x - (2 * spacing) - 1)
    ret_code = -1;
  else if(user_key >= 32 || user_key <= 126){
    user_string[next_position] = (char) user_key;
    //add the new char to screen 
    mvwaddch(win, 1, spacing + next_position, user_key);
    ret_code = 0;
  }  
  else
    ret_code = -2;

  wrefresh(win);
  return ret_code;
}

//here we'll init all the windows in the layout we want
//this is to hardcoded and will be made more dynamic later. 1 step at a time
WINDOW** create_windows(){
  int max_y, max_x;

  getmaxyx(stdscr, max_y, max_x);
  //i don't understand why i need this refresh when i have wrefresh below?
  refresh();
  WINDOW** win_arr = (WINDOW**) malloc(sizeof(WINDOW*) * 3);
  //newwin params: len_y, len_x, start_y, start_x
  //this is the top bar. y dimensions and location never change
  win_arr[0] = newwin(1, max_x, 0, 0);
  //this is the user input box. y dimensions and location never change
  win_arr[1] = newwin(3, max_x, 1, 0);
  wborder(win_arr[1], '|', '|', '-', '-', '+', '+', '+', '+');
  //this is the list box. starts at y = 4 and goes till bottom
  win_arr[2] = newwin(max_y - 4, max_x, 4, 0);
  wborder(win_arr[2], '|', '|', '-', '-', '+', '+', '+', '+');

  wrefresh(win_arr[BAR_WIN]);
  wrefresh(win_arr[INPUT_WIN]);
  wrefresh(win_arr[LIST_WIN]);
  return win_arr;
}

int run(){
  int ret_code = 0;
  char* user_string;
  str_list* prog_list = NULL;
  WINDOW** windows = NULL;

  windows = create_windows(); 
  //allows exotic keys. using so we can use backspace/delete
  keypad(windows[INPUT_WIN], TRUE);
  
  //refactor: good way to get max_x here? trying to keep stdscr stuff away
  //instead use the appropriate window in each function called
  user_string = (char*) malloc(sizeof(char) * (200));
  for(int i = 0; i < 200; i++)
    user_string[i] = 0;
  while(true){
    if(!strlen(user_string))
      ret_code = get_prog_list(&prog_list); 
    //here we sit waiting for user. process the key (get, update str, and print)
    ret_code = process_input(windows[INPUT_WIN], user_string);

    if (ret_code == 0){ //user added a char.. list narrows
      ret_code = narrow_list(prog_list, user_string);
      print_list(windows[LIST_WIN], prog_list, strlen(user_string));
    }
    else if (ret_code == 1){ //user deleted a char.. list widens
      ret_code = widen_list(prog_list, user_string);
      print_list(windows[LIST_WIN], prog_list, strlen(user_string));
    }
  }
  free(user_string); 
  free(prog_list);
  return 0; //add error reporting
}

int main(){
  //start ncurses & global settings
  initscr();
  //allows input to be immediately available instead of waiting for newline 
  cbreak(); 
  //hide auto output input at cursor location so we have more control 
  noecho();
  //main action
  run();
  //end ncurses
  endwin();
  return 0; //add error reporting
}
