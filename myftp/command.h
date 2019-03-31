#define FILE_BLOCK_SIZE 512
#define MAX_CMD_INPUT 64
#define SERV_TCP_PORT 40300 // Default server listening port

// client commands available
#define CMD_PWD "pwd"
#define CMD_LPWD "lpwd"
#define CMD_DIR "dir"
#define CMD_LDIR "ldir"
#define CMD_CD "cd"
#define CMD_LCD "lcd"
#define CMD_GET "get"
#define CMD_PUT "put"
#define CMD_QUIT "quit"
#define CMD_HELP "help"

// opcodes

#define OP_PWD 'W'
#define OP_DIR 'I'
#define OP_CD 'C'
#define OP_GET 'G'
#define OP_PUT 'P'
#define OP_DATA 'D'
#define OP_ONLINE 'O'


// acknowledge code for OP_CD
#define ACK_CD_SUCCESS '0'
#define ACK_CD_FIND '1'

// ackowledge code for OP_GET
#define ACK_GET_FIND '0'
#define ACK_GET_OTHER '1'

// acknowledge codes for OP_PUT
#define ACK_PUT_SUCCESS '0'
#define ACK_PUT_FILENAME '1'
#define ACK_PUT_CREATEFILE '2'
#define ACK_PUT_WRERROR '3'

// error messages for OP_CD ack codes
#define ACK_CD_OTHER_MSG "[-] The server cannot change directory due to other reasons"

// error messages for OP_GET ack codes
#define ACK_GET_FIND_MSG "[-] the server cannot find requested file"
#define ACK_GET_OTHER_MSG "[-] the server cannot send the file due to other reasons"

// error messages for OP_PUT ack codes
#define ACK_PUT_FILENAME_MSG "[-] the server cannot accept the file as there is a filename clash"
#define ACK_PUT_CREATEFILE_MSG "[-]the server cannot accept the file because it cannot create the named file"
#define ACK_PUT_WRERROR_MSG "[-]write has failed (unknown error or disc full)"

// other error messages
#define UNEXPECTED_ERROR_MSG "unexpected behaviour"

/**
 * uses myfrp protocol to send a file from client to server
 * sd - client socket
 * filename - name of file to be sent from client to server
 */
void putCommand(int sd , char * filename);


/**
 * uses the myftp protocol to send a file from client to server
 * sd client's socket
 * filename - name of file to be sent from server to client
 */
void getCommand(int sd , char * filename);


/**
 * uses the myftp protocol to print the current directory path of the server
 * sd - client socket
 */

void pwdCommand(int sd);


/**
 * prints the current local directory path of the client
 */
void lpwdCommand();

/**
 * print the list of files in the current directory
 * sd - client's socket
 */
void dirCommand();

/**
 * prints a list of files in the current directory of the client
 */
void ldirCommand();

/**
 * uses the myftp protocol to change directory of the server
 * token - path
 */
void cdCommand(int sd, char* token);


/**
 * Changes current directory of the client
 * token - path
 */
void lcdCommand(char * token);

/**
 * prints a message stating myftp session has been terminated by the client
 *
 */
void quitCommand();


/**
 * prints a list of available commands to the client
 */
void helpCommand();
 
















