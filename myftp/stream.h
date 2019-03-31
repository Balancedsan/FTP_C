/*
* FileName : stream.h
* Author : John Wee
* Description: Header file for stream.c
* Last Modified : 25/3/2019
*/

/**
* Reads a stream of bytes from the buffer to the socket
* returns:
*  	   > 0  number of bytes read
*  	   else error
*
*/
int readNBytes(int sd, char * buf, int nbytes);



/**
* Writes a stream of bytes on the socket from the buffer
* returns: > 0 number of bytes written
*	   else error
*/
int writeNBytes(int sd,char* buf, int nbytes);

/**
* writes a one byte char from opcode to socket
* return -1 if write failed
*	 1 if write is sucessful
*/

int writeByte(int sd, char code);

/**
* Reads a one byte char from the socket to opcode
* returns -1 if the read failed
*	 1 if the read is sucessful
*/
int readByte(int sd, char * code);

/**
*  write a two byte integer from the length to the socket
*  returns: -1 if failed
	    1 if successful
*/
int writeTwoByteLength(int sd, int length);



/**
* Reads a two byte integer from the socket to length
* returns: -1 if the the read failed
*	   1 if the read is sucessful
*
*/
int readTwoByteLength(int sd, int *length);

/*
* wrties a four byte integer from the length to the socket
* returns: -1 if the the read failed
*	   1 if the read is sucessful
*/
int writeFourByteLength(int sd, int length);


/*
* Reads a four byte integer from the socket to length
* returns: -1 if the read failed
*	    1 if the read is sucessful
*
*/
int readFourByteLength(int sd , int * length);












