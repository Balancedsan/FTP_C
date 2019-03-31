#include <signal.h> // sigaction
#include <stdio.h>
#include "command.h" // protocols
#include <unistd.h>
#include <fcntl.h>
#include "stream.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>


void serveClient(descriptors *desc , char * address, int port){
    char opcode;
    
    logger(desc,"Connection accepted from %s:%d",address,port);
    
    while(readByte(desc->sd,&opcode) > 0){
        switch(opcode){
            case OP_PUT:
                commandPUT(desc);
                break;
            case OP_PWD:
                commandPWD(desc);
                break;
            case OP_DIR:
                commandDIR(desc);
                break;
            case OP_CD:
                commandCD(desc);
                break;
            case OP_ONLINE:
                writeByte(desc->sd,OP_ONLINE);
                break;
            default:
                logger(desc,"invalid opcode received"); // invalid disregard
                break;
        }
    }
    
    logger(desc,"disconnected");
    return;
}

// Terminates any hanging children processes.
void claimZombies()
{
    pid_t pid = 1;
    while (pid > 0)
    { /* claim as many zombies as we can */
        pid = waitpid(0, (int *)0, WNOHANG);
    }
}

// Sets process as daemon.
void daemonInit(void)
{
    pid_t pid;
    struct sigaction act;
    
    if ( (pid = fork()) < 0)
    {
        perror("[-] Failed to create child process");
        exit(1);
    }
    else if (pid > 0)
    {
        /* parent */
        printf("myftpd PID: %d\n", pid);
        exit(0);
    }
    else
    {
        /* child */
        setsid();        /* become session leader */
        umask(0);        /* clear file mode creation mask */
        
        /* catch SIGCHLD to remove zombies from system */
        act.sa_handler = claimZombies; /* use reliable signal */
        sigemptyset(&act.sa_mask);       /* not to block other signals */
        act.sa_flags   = SA_NOCLDSTOP;   /* not catch stopped children */
        sigaction(SIGCHLD,(struct sigaction *)&act,(struct sigaction *)0);
        
    }
}








