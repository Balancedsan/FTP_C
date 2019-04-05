#include "logger.h"
#include <stdarg.h>// used for multiple arguments in a function va_start, va_end , vdprintf
#include <time.h> //used to get the timestamp functions localtime asctime_r
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>  // for opening files
#include <unistd.h> // for closing files



void getLocalTime(char * timeFormat){
    time_t seconds; // used to store seconds since 1970 utc time
    struct tm * timevalue;// used to convert raw time from time function to local current time
    time(&seconds); // stores the time into seconds variable since 1970 utc time
    timevalue = localtime(&seconds); // gets the local time from passing in time_t seconds
    asctime_r(timevalue,timeFormat); // converts local time to a string value
    
}

// outputs current time , client id and passed format string to the log file

void logger(descriptors *d, char* argformat, ...) {

int fileDescriptor; // stores the value of file descriptor when opening the file

if((fileDescriptor = open(d->logfile,O_WRONLY | O_APPEND | O_CREAT, 0766)) == -1){
    perror("[-] Failed writting to log \n");
    exit(0);
}
    va_list arguments;


    char * loggerFormat; // used to store relevant data in string format 
    char * cidformat = "Client %d - "; // used for outputing clientid details later
    char cidstring[64] = " "; // used to store the client id output later
    
    
    char timeFormat[64];// stores the current time value in string format
    getLocalTime(timeFormat); // gets the local time
    
    
    
    
    if(d->cid != 0){
        sprintf(cidstring,cidformat,d->cid);// stores the output of client id into the buffer
    }
    
    
    // create a log of dynamic size data of the stringlength of the timeformat and argumentformat + 2 bytes * size of char
    loggerFormat = (char*) malloc((strlen(timeFormat) + strlen(cidstring) + strlen(argformat) + 2) * sizeof(char));

    
    // copies the timeFormat string to the loggerformat pointer of strings
    strcpy(loggerFormat,timeFormat);


    strcat(loggerFormat,cidstring); // appends cidstring to the loggerformat string
    strcat(loggerFormat,argformat);// appends argformat to loggerformat string
    strcat(loggerFormat,"\n"); // appends the new line to the loggerformat

    va_start(arguments,argformat); // used to access aditional variables passed to the function
    vdprintf(fileDescriptor,loggerFormat,arguments);//used to print the logger arguments to the filedescriptor
    va_end(arguments);//used to end access to additional variables
    
    free(loggerFormat); // release dynamic memory from loggerformat that allocates memory dynamacially earlier
    close(fileDescriptor); // close the file descriptor
}
