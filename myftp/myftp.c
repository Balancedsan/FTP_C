#include "stream.h"
#include "parse.h"
#include "command.h"

#include <dirent.h>     //DIR, opendir, scandir, closedir
#include <stdlib.h>     //free, exit, atoi
#include <stdio.h>      //printf, perror, snprintf, fgets, getc
#include <sys/types.h>  //lseek
#include <sys/socket.h>
#include <sys/stat.h>   //fstat
#include <netinet/in.h>
#include <net/if.h>     //IFF_UP
#include <arpa/inet.h>  //inet_ntop, inet_addr
#include <netdb.h>      //getaddrinfo, gai_strerror
#include <string.h>     //strlen, strchr, strerror, strcopy, memset
#include <unistd.h>     //lseek, getcwd, unlink, close, read, write, chdir
#include <fcntl.h>     //open, O_RONLY, O_WRONLY
#include <ifaddrs.h>   //getifaddrs
#include <errno.h>     //errno
#include <signal.h>    //sigfillset, sigaction, sigint, sigquit

// macro function that checks the signal

#define CHECK_SIGNAL(signal, obj) \
    if(sigaction(signal, obj , NULL) != 0) { \
    printf("[-] sigaction of "#signal" has terminated the client \n"); \
    exit(1); \
    }

int execute(int socketdesc, char *buf); // function prototype to execute the client

void createInterruptSignal();// function prototype to create an interrupt signal for the program

/**
 * Connects to the server by the number of argument it receives (0 - 2)
 * if 0 arguments
 *      Default hostname and port is used (port 40300)
 * else if 1 arguments
 *      the program uses the hostname the user has provided as a argument for example (eg ./myftp <hostname>)
 * else if 2 arguments
 *      the program uses the hostname and port the user has provided as a argument (eg /myftpd hostname port)
 *
 */
int main(int argc , char * argv[]){
    
    createInterruptSignal(); // creates the interrupt signal for the program to handle interrupts

    int socketdesc; // socket descriptor used to store the descriptor of the socket to be used later
    char buf[MAX_CMD_INPUT], host[64]; // buffer to store maximum command input whereas host stores the ip addr
    char port[6];
    snprintf(port, 6, "%d", SERV_TCP_PORT);



    // check if the user entered more arguments than the expected arguments
    if(argc > 3 || argc < 1)
    {
        printf("[-] Example Usage: %s  <server host name>  <server listening port> \n", argv[0]);
        exit(1);
    }
    // initialize the socket for the program
    if((socketdesc = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("[-] initializing the socket has caused an error");
        exit(1);
    }
    
    // used to get server host name and port number for local connection
    if (argc == 1)
    {
        struct ifaddrs *myaddrs, *ifa;
        void *in_addr;
        int flag = 0;

        if(getifaddrs(&myaddrs) != 0) // getifaddrs detects local addresses. Supports IPv4 and IPv6
        {
            perror("[-] getifaddrs failed.\n");
            exit(1);
        }
        // gets the local ip address of the program
        for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
        {
            // checks the address of the interface is null or the interface flags is not up
            if (ifa->ifa_addr == NULL || !(ifa->ifa_flags & IFF_UP)){
                        continue;
            }

            // checks the ip address family if its IPV4 or IPV6
            switch (ifa->ifa_addr->sa_family)
            {
                case AF_INET: // for ipv4 family
                {
                    struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                    in_addr = &s4->sin_addr;
                    break;
                }

                case AF_INET6: // for ipv6 family
                {
                    struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                    in_addr = &s6->sin6_addr;
                    break;
                }

                default:
                continue;
            }

            if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, host, sizeof(host)))
            {
                continue;
            }
            else
            {
                struct sockaddr_in serverAddr;
                // fils the last
                memset(&serverAddr, '\0', sizeof(serverAddr)); // fills the last address with a null terminator
                serverAddr.sin_family = AF_INET; // set to ipv4
                serverAddr.sin_port = htons(SERV_TCP_PORT);// converts port 40312 to a network byte order
                serverAddr.sin_addr.s_addr = inet_addr(host); // internet address


                if(connect(socketdesc, (struct sockaddr*)&serverAddr, sizeof(serverAddr))  >= 0)
                {
                    printf("Connected to myftp on local host: %s\n", host);
                    flag = 1;
                    break;
                }
            }
        }
        if(flag == 0) // never connects to local host
        {
            printf("[-] Error in connection: %s\n", strerror(errno));
            exit(1);
        }
    }
    else
    {
        // copies the ipaddress to the host buffer
        strcpy(host, argv[1]);
        
        // if client specifies local addr , ip addr and port number
        if(argc == 3)
        {
            int portNum = atoi(argv[2]); // converts argument to portNumber
            
            if(!(portNum >= 1024 && portNum < 65536)){ // if port not in range of 1024 and 65536 print an error and exit the program
                perror("server port number must be between 1024 and 65535\n");
                exit(1);
            }
            strcpy(port,argv[2]);
        }
        
        int status;
        struct addrinfo hints, *servinfo; // used to provide hints
        memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICSERV;
        if( (status = getaddrinfo(host, port, &hints, &servinfo)) != 0) //Use getaddrinfo because gethostbyname is an older, obsolete
        {                                                               // version. getaddrinfo supports IPv6 as well.
            printf("[-] getaddrinfo error: %s\n", gai_strerror(status));
            exit(1);
        }
        
        if(connect(socketdesc, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
        {
            printf("[-] Error in the connection: %s\n", strerror(errno));
            exit(1);
        }
        else
        {
            printf("Connected to myftp on host: %s\n", host);
        }
    }

    return execute(socketdesc, buf);

}

/**
 * executes the client program and checks for the type of command the user has inputted
 *
 **/
int execute(int socketdesc, char *buf) {
    char *tokens[2], *delim;

    while (1)
    {
        if(writeByte(socketdesc, OP_ONLINE) == -1) // make sure that the server is still up
        {
            printf("[-] Unable to initialize handshake.\n");
            exit(1);
        }
        char opcode = OP_ONLINE; // set the operation code to O which sets the protocol
        if(recv(socketdesc, (char *)&opcode, sizeof(opcode), 0) == 0) // recv will return 0 if the socket is disconnected
        {
            printf(" Server have stopped running.\nExiting ...\n");
            exit(1);
        }

        printf("FTP-client:$ "); // displays the prompt to the user

        // reads the user input and tokenize

        if(fgets(buf, MAX_CMD_INPUT, stdin) == NULL)
        {
            if(!feof(stdin))
            {
                perror("[-] Unable to read from stdin");
                exit(1);
            }
        }
        // searches for new line in the code
        delim = strchr(buf, '\n');

        if(delim == NULL) // check if there is no '\n' in the buffer
        {
            buf[MAX_CMD_INPUT-1] = '\0';
            int c;
            while((c = getc(stdin)) != '\n' && c != EOF); // clear the buffer in stdin
        }
        else
        {
            *delim = '\0'; // replace '\n' with '\0'
        }

        int argsCount = parser(buf, tokens);

        if(argsCount == 0) // if the client just click enter without typing any commands, display the prompt again
        {
            continue;
        }
        
        
        if(strcmp(tokens[0],CMD_PUT) == 0)
        {
            if(argsCount  == 2) // ensures the number of arguments is 2
            {
                command* cmd = allocCommand(socketdesc, tokens[1]);
                putCommand(cmd);
                freeCommand(cmd);
            }
            else
            printf("Example Usage: put <filename>\n");

        }
        else if(strcmp(tokens[0],CMD_GET) == 0)
        {
            if(argsCount == 2) {
                command* cmd = allocCommand(socketdesc, tokens[1]);
                getCommand(cmd);
                freeCommand(cmd);
            }
            else
            printf("Example Usage: get <filename>\n");

        }
        else if(strcmp(tokens[0],CMD_PWD) ==0)
        {
           argsCount == 1 ? pwdCommand(socketdesc) : printf(" Example Usage: pwd\n");

        }
        else if(strcmp(tokens[0],CMD_LPWD)==0)
        {
           argsCount == 1 ?  lpwdCommand() : printf(" Example Usage: lpwd\n");
        }
        else if(strcmp(tokens[0],CMD_DIR)==0)
        {
           argsCount == 1 ? dirCommand(socketdesc) :  printf("Example Usage: dir\n"); // dir is to display current working directory's files, you cannot specify any path
        }
        else if(strcmp(tokens[0],CMD_LDIR)==0)
        {
            argsCount == 1 ? ldirCommand() : printf("Example Usage: ldir\n"); // ldir is to display local current working directory's files, you cannot specify
        }
        else if(strcmp(tokens[0],CMD_CD)==0)
        {
            if(argsCount  == 2)
            {
                command* cmd = allocCommand(socketdesc, tokens[1]);
                cdCommand(cmd);
                freeCommand(cmd);
            }
            else
            printf("Example Usage: : cd <directory_pathname>\n");
        }
        else if(strcmp(tokens[0],CMD_LCD)==0)
        {
            if(argsCount == 2)
            lcdCommand(tokens[1]);
            else
            printf("Example Usage: : lcd <directory_pathname>\n");
        }
        else if(strcmp(tokens[0],CMD_HELP)==0)
        {
           argsCount == 1 ?  helpCommand():  printf("Example Usage: : help\n");
        }
        else if(strcmp(tokens[0],CMD_QUIT)==0)
        {
           argsCount == 1 ?  quitCommand(): printf("Example Usage: quit\n");
        }
        else// if none of above is true display default message
        {
            printf(" No such command is available. Please type 'help' to see list of available commands.\n");
        }
    }

    return 0;
}

/**
 * Creates The Interrupt signal for the program
 */
void createInterruptSignal(){
    struct sigaction interrupt; // initialze sigaction
    interrupt.sa_flags = 0;
    interrupt.sa_handler = quitCommand; // set the handler to capture signals
    sigfillset(&(interrupt.sa_mask)); // block all other signals
    CHECK_SIGNAL(SIGINT, &interrupt);
    CHECK_SIGNAL(SIGQUIT, &interrupt);
}

