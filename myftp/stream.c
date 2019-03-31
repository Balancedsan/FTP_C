/**
* File: stream.c
* Author: John Wee
* Description: helper functions to read and write data on the sockets
*
*/

#include "stream.h" // header file for prototype functions
#include <netinet/in.h> // to access funcs like httons,ntohs,htonl.ntohl
#include <sys/types.h>
#include <unistd.h>

// iterates through nbytes until entire n bytes is written from the buffer and returns the number of bytes written
int writeNBytes(int sd, char * buf, int nbytes){
    
    int bytesWritten = 0;
    int totalWritten = 0;
    
    for(totalWritten = 0; totalWritten < nbytes; totalWritten += bytesWritten){
        // write to file for each byte and if any write failed return -1
        if((bytesWritten = write(sd,buf+bytesWritten, nbytes -totalWritten)) <= 0){
            return (bytesWritten);
        }
    }
    return totalWritten;
}

// iterates through the read from socket until entire nbytes read into buf
int readNBytes(int sd, char* buf, int nbytes){
    
    int bytesRead = 1;
    int totalRead = 0;
    
    for(totalRead = 0; (totalRead < nbytes) && (bytesRead > 0); totalRead += bytesRead){
        if((bytesRead = read(sd,buf + totalRead,nbytes - totalRead) < 0)){
            return (bytesRead);
        }
    }
           return totalRead;
}

// reads a byte char from the socket
int readByte(int sd, char * code) {

    char data;
    
    // reads one byte of code from the socket
    if(read(sd,(char *) &data ,1) != 1){ // check if exactly 1 byte is read to the buffer , if not its an error
        return -1;
    }
    
    *code = data;
    
    return 1; // return 1 back to inform the read is sucessful
}
           
// writes a byte char to the socket
int writeByte(int sd, char code){

    // writes 1 byte of code to the socket
    if(write(sd,(char*)&code,1) != 1){
        return -1;
    }
    
    return 1;// return 1 back to inform the write is sucessful
}
           
// Writes a two byte integer to the socket
int writeTwoByteLength(int sd, int length){
    
    // short stores exactly 2 bytes in size
    short data = length;
    data = htons(data); // host to network convert to network byte
    
    if(write(sd,&data,2) != 2){// check if bytes written is exactly 2 bytes
        return -1;
    }
    return 1;
}

// reads a two byte integer from the socket
        
int readTwoByteLength(int sd, int * length){
    // short stores exactly 2 bytes size
    short data = 0;
    
    if(read(sd,&data,2) != 2){
        return -1;
    }
    
    short conv = ntohs(data); // convert to host byte order
    int t = (int)conv;
    *length = t;
    
    return 1;
}
           
// writes a four byte integer from the socket
           
int writeFourByteLength(int sd, int length){
    int data = htonl(length); // converts to network byte order
    
    if(write(sd,&data,4) != 4){// check if bytes written is exactly 4 bytes
        return -1;
    }
    
    return 1;
}

// reads a four byte integer from the socket

int readFourByteLength(int sd, int * length){
    
    int data = 0;
    
    if(read(sd,&data,4)!=4){// check if bytes read is exactly 4 bytes 
        return -1;
    }
    
    int conv = ntohl(data); // converts to host byte order
    *length = conv;
    
    return 1;
}
