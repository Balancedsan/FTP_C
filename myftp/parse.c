#include  <string.h> // strtok 
#include "parse.h"

int parser(char input[],char *args[]){
 char *tk;
 int count = 0;

 tk = strtok(input,tokenSeperators);
 args[count] = tk;

 while(tk != NULL){
  ++count;
  if(count >= MAX_NUM_TOKENS){
   count = -1;
   break;
  }

  tk = strtok(NULL,tokenSeperators);
  args[count] = tk;
 }

 return count;

}
