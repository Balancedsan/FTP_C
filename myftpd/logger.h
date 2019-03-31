/**
* File: logger.h
* Author: John Wee
* Purpose: the header file for function prototype for logger
*
*/



// struct for managing socket descriptor, client id and logfile path

typedef struct{
int sd; // socket
int cid; // the client id
char logfile[256]; // absolute log path of the file being 256 bytes in length
} descriptors;

/**
* accepts a client id, formated output string and variable list of arguments
* returns: current time, client id and passwed format string to log file
*
*/
void logger(descriptors *d, char * argformat,...);
