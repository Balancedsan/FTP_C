/**
 * File: command.h
 * Author: John Wee
 * Purpose: used to create function prototypes for input commands
 *
 **/
#include "logger.h"
#define FILE_BLOCK_SIZE 512

// operation code constants
#define OP_PWD 'W'
#define OP_DIR 'I'
#define OP_CD 'C'
#define OP_DATA 'D'
#define OP_GET 'G'
#define OP_PUT 'P'
#define OP_ONLINE 'O'

// acknowledge code for operation change directory (OP_CD)
#define ACK_CD_SUCCESS '0'
#define ACK_CD_FIND '1'

// acknowledge code for operation 'GET' (OP_GET)
#define ACK_GET_FIND '0'
#define ACK_GET_OTHER '1'

// error message for operation 'GET' acknowledge codes (OP_GET)
#define ACK_GET_FIND_MSG "The sever cannot find the requested file"
#define ACK_GET_OTHER_MSG "The server is unable to request the file due to other reasons"

// acknowledge code for operation 'PUT' OP_PUT
#define ACK_PUT_SUCCESS '0'
#define ACK_PUT_FILENAME '1'
#define ACK_PUT_CREATEFILE '2'
#define ACK_PUT_WRERROR '3'

// error messages for operation 'PUT' OP_PUT acknowledge codes
#define ACK_PUT_FILENAME_MSG "The server cannot accept the file as there is a clask of filename"
#define ACK_PUT_CREATEFILE_MSG "The server cannot accept the file because it cannot create the named file"

#define UNEXPECTED_ERROR_MSG "Unexpected behaviour"

#define SERV_TCP_PORT 40300 // default server listening port
#define LOGPATH "/myftpd.log" //log path

/**
 * Protocol process PUT to put a file from the client to the server.
 * desc -> struct that manages the client connection information
 *
 */
void commandPUT(descriptors *desc);

/**
 * Protocol process to GET a requested file from the server to the client.
 * desc the structure used to manage the client connection information
 */
void commandGET(descriptors *desc);


/**
 * Protocol process to display the current directory path of the server
 * desc - struc used to manage client connection information
 */
void commandPWD(descriptors *desc);

/**
 * Protocol process to display directory listing of the server
 * desc - struct used to manage client connection information
 */
void commandDIR(descriptors * desc);


/**
 * Protocol process to change directory of the server
 * desc - struct used to manage client connection information
 */
void commandCD(descriptors *desc);



