/**
* File : parse.h
* Author: John wee (33126204)
* Description: header file for parse.c
*
*
*/

#define MAX_NUM_TOKENS 100
#define tokenSeperators " \t\n" // characters that seperate a token

/**
* breaks up an array of chars by whitespace characters into individual tokens
* return >=0 largest index of token array
* -1 on failure
*/

int parser(char input[],char *args[]);
