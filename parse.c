#include <stdio.h>
#include <string.h>
#include "parse.h"

int parse(char **u_cmd, char *parse_string){
  char *word_seg = strtok(parse_string, " ");
  int cnt = 0;
  for(; word_seg!=NULL; cnt++)
  {
      u_cmd[cnt]=word_seg;
      word_seg = strtok(NULL, " ");
  }
  u_cmd[cnt]=NULL;
  return cnt;
}