#include "command.h"
#include "daemon.h"
#include "stream.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>   // waitpid
#include <arpa/inet.h> // inet_ntoa
#include <stdio.h>
#include <string.h>
#include <errno.h>


int main(int argc, char * argv[]){
    int nsd;
    pid_t pid; // pid for the fork
    unsigned short port = SERV_TCP_PORT; // listening port for the server
    socklen_t cli_addrlen; // size of address = client address length
    struct sockaddr_in ser_addr, cli_addr;
    
    descriptors desc;
    desc.cid = 0; // client id = 0
    desc.sd = 0;
    
    char init_dir[256] = "."; // initialize directory
    char curr_dir[256] = ""; // current directory
    
    // get the current directory
    getcwd(curr_dir,sizeof(curr_dir));
    
    if(argc > 2){
        printf("usage: %s initial_current_directory\n",argv[0]);
    }
    
    
    // get the initial directory
    if(argc == 2){
        strcpy(init_dir,argv[1]);
    }
    
    if(chdir(init_dir) == -1){
        printf("failed to set initial directory to  %s\n", init_dir);
        exit(1);
    }
    
    // set absolute path to the logfile
    getcwd(curr_dir,sizeof(curr_dir));
    strcpy(desc.logfile,curr_dir);
    strcat(desc.logfile,LOGPATH);
    
    // make the server a daemon
    
    daemonInit();
    
    logger(&desc,"server initialized.");
    logger(&desc,"initial dir set to &s",curr_dir);
    
    // set up listening socket sd
    if((desc.sd = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("failed to create socket for server");
        exit(1);
    }
    
    // build server internet socket address
    bzero((char *)&ser_addr,sizeof(ser_addr));
    ser_addr.sin_family = AF_INET; // address family
    ser_addr.sin_port = htons(port); // network ordered port number
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY); // any interface
    
    
    // bind server address to socket sd
    if(bind(desc.sd,(struct sockaddr *)&ser_addr,sizeof(ser_addr)) < 0){
        perror("failed to bind server socket");
        exit(1);
    }
    
    // becomes a listening socket
    listen(desc.sd,5); // 5 maximum connections can be in the queue
    logger(&desc,"myftp server now listening on port %hu",port);
    //ntohs converts the unsigned short integer netshort from network byte order to host byte order
    printf("myftp server is running on port : %d\n",ntohs(ser_addr.sin_port));
    
    while(1){
        // wait to accept a client request for connection
        cli_addrlen = sizeof(cli_addr);
        nsd = accept(desc.sd,(struct sockaddr *)&cli_addr,&cli_addrlen);
        if(nsd < 0){
            if(errno == EINTR){
                continue; // if interrupted by  SIGCHILD
            }
            perror("failed to accept connection");
            exit(1);
        }
        
        // iterate client id before forking
        desc.cid++;
        
        if((pid = fork()) < 0){
            perror("failed to create child process");
            exit(1);
        }
        else if(pid > 0){
            close(nsd);
            continue; // parrent to wait for next client
        }else {
            close(desc.sd);// now in child, serve the current client
            desc.sd = nsd;
            char * cli_address = inet_ntoa(cli_addr.sin_addr);
            int cli_port = ntohs(cli_addr.sin_port);
            serveClient(&desc,cli_address,cli_port);
            exit(0);
        }
    }
    
    
    
    
    
    
    
    
}
