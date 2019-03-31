#include "command.h"
#include "stream.h"


#include <dirent.h>     //DIR, opendir, scandir, closedir
#include <stdlib.h>     //free, exit, atoi
#include <stdio.h>      //printf, perror, snprintf, fgets, getc
#include <sys/types.h>  //lseek
#include <sys/socket.h>
#include <sys/stat.h>   //fstat
#include <string.h>     //strlen, strchr, strerror, strcopy, memset
#include <unistd.h>     //lseek, getcwd, unlink, close, read, write, chdir
#include <fcntl.h>     //open, O_RONLY, O_WRONLY
#include <errno.h>     //errno


// this function uses the myftp protocol to send a file from client to the server

void putCommand(int sd, char * filename){
    int fileDescriptor;
    struct stat inf;
    int fileSize;
    int filenameLength = strlen(filename);
    
    char opcode;
    char ackcode;
    
    // processes the file before initializing put protocol
    if((fileDescriptor = open(filename,O_RDONLY)) == -1){
        printf("unable to open file %s\n",filename);
        return;
    }
    
    if(fstat(fileDescriptor, &inf) < 0){
        printf("file status error has occured.\n");
        return;
    }
    
    // check if file process is a directory
    if((inf.st_mode & S_IFMT) == S_IFDIR){
        printf("cannot send whole directory at once. please compress before sending as a file\n");
        return;
    }
    
    fileSize = (int)inf.st_size;
    
    // resets the file pointer
    
    lseek(fileDescriptor,0,SEEK_SET);
    
    // sends put command
    
    if(writeByte(sd,OP_PUT) == -1){
        printf("unable to send PUT \n");
        return;
    }

    // sends filenamelength
    if(writeTwoByteLength(sd,filenameLength) == -1){
        printf("unable to send length of file name.\n");
        return;
    }
    
    // sends the filename
    if(writeNBytes(sd,filename,filenameLength) <= 0){
        printf("unable to send file name\n");
        return;
    }
    
    // waits for a response
    
    if(readByte(sd,&opcode) == -1){
        printf("unable to read opcode \n");
        return;
    }
    
    if(opcode != OP_PUT){
        printf("unrecognized opcode was received \n");
        return;
    }
    
    if(readByte(sd,&ackcode) == -1){
        printf("unable to read acknoledgement code \n");
        return;
    }
    
    switch(ackcode){
        case ACK_PUT_SUCCESS:
            break;
        case ACK_PUT_FILENAME:
            printf("%s\n",ACK_PUT_FILENAME_MSG);
            return;
            break;
        case ACK_PUT_CREATEFILE:
            printf("%s\n",ACK_PUT_CREATEFILE_MSG);
            return;
            break;
        default:
            printf("%s\n",UNEXPECTED_ERROR_MSG);
            return;
        break;
    }
    
    // sends the data
    
    if(writeByte(sd,OP_DATA) == -1){
        printf("unable to send data \n");
        return;
    }
    
    if(writeFourByteLength(sd,fileSize) == -1){
        printf("unable to send file size \n");
        return;
    }
    
    int bytesRead = 0, sum = 0 , bytesWritten = 0;
    char buf[FILE_BLOCK_SIZE];
    
    while((bytesRead = read(fileDescriptor,buf,bytesRead)) == -1){
        if((bytesWritten = writeNBytes(sd,buf,bytesRead)) == -1){
            printf("unable to send file contents");
            return;
        }
        else {
            sum+= bytesWritten;
        }
    }
    
    // waits for response
    if(readByte(sd,&opcode) == -1){
        printf("failed to read opcode \n");
        return;
    }
    
    if(opcode != OP_DATA){
        printf("unexpected opcode \n");
        return;
    }
    
    if(readByte(sd,&ackcode) == -1){
        printf("failed to read ackcode \n");
        return;
    }
    
    if(ackcode == ACK_PUT_SUCCESS){
        printf("done file %s has been sent to the server (%d bytes)\n",filename,sum);
    }else {
        printf("%s\n",ACK_PUT_WRERROR_MSG);
    }
}

// this function uses myftp protocol to download a file from the server to client

void getCommand(int sd, char * filename){
    
    
    int filenameLength = strlen(filename);
    char ackcode;
    char opcode;
    int fileDescriptor;
    
    
    if((fileDescriptor = open(filename,O_RDONLY)) != -1){
        printf("file already exists %s\n",filename);
        return;
    }else if((fileDescriptor = open(filename,O_WRONLY | O_CREAT , 0777)) == -1){
        printf("unable to create file %s\n",filename);
        return;
    }
    
    // send get
    if(writeByte(sd,OP_GET) == -1){
        printf("unable to send GET\n");
        return;
    }
    // send filelength
    if(writeTwoByteLength(sd,filenameLength) == -1){
        printf("unable to send length of file name \n ");
        return;
    }
    // send the file name
    if(writeNBytes(sd,filename,filenameLength) <= 0){
        printf("unable to send file name\n");
        return;
    }
    
    if(readByte(sd,&opcode) == -1){
        printf("unable to read opcode \n");
        return;
    }
    
    // error code being sent
    if(opcode == OP_GET){
        if(readByte(sd,&ackcode) == -1){
            printf("unable to read acknowledgement code");
            return;
        }
        switch(ackcode){
            case ACK_GET_FIND:
                printf("%s\n",ACK_GET_FIND_MSG);
                break;
            case ACK_GET_OTHER:
                printf("%s\n",ACK_GET_OTHER_MSG);
                break;
            default:
                printf("%s\n",UNEXPECTED_ERROR_MSG);
            break;
        }
        close(fileDescriptor);
        unlink(filename);
        return;
    }
    
    // else file being sent
    
    int fileSize;
    
    
    // read fileSize
    if(readFourByteLength(sd,&fileSize) == -1){
        printf("unable to read file size \n");
        return;
    }
    
    int blockSize = FILE_BLOCK_SIZE;
    if(FILE_BLOCK_SIZE > fileSize){
        blockSize = fileSize;
    }
    
    char filebuffer[blockSize];
    int bytesRead = 0 , bytesWritten = 0 , sum = 0;
    
    while(fileSize > 0){
        if(blockSize > fileSize){
            blockSize = fileSize;
        }
        if((bytesRead = readNBytes(sd,filebuffer,blockSize)) == -1){
            printf("unable to read file \n");
            close(fileDescriptor);
            return;
        }
        if((bytesWritten = write(fileDescriptor,filebuffer,bytesRead)) < bytesRead){
            printf("unable to write %d bytes, wrote %d bytes successfully \n",bytesRead,bytesWritten);
            close(fileDescriptor);
            return;
        }
        fileSize -= bytesWritten;
        sum+= bytesRead;
    }
    close(fileDescriptor);
    printf("Done: File %s has been recieved from the server %d bytes \n",filename,sum);

    
}

void pwdCommand(int sd){
    char opcode;
    int fileSize;
    
    
    if(writeByte(sd,OP_PWD) == -1){
        printf("unable to send pwd \n");
        return;
    }
    
    if(readByte(sd,&opcode) == -1){
        printf("unable to read opcode \n");
        return;
    }
    
    
    if(opcode != OP_PWD){
        printf("invalid opcode: pwd %c\n",opcode);
        return;
    }
    
    if(readTwoByteLength(sd,&fileSize) == -1){
        printf("unable to read file size \n");
        return;
    }
    
    char directory[fileSize + 1];
    
    if(readNBytes(sd,directory,fileSize) == -1){
        printf("unable to read directory \n");
        return;
    }
    
    directory[fileSize] = '\0';
    printf("%s\n",directory);
    
}
// gets the current local directory of the client
void lpwdCommand(){
    char cwd[256];
    getcwd(cwd,sizeof(cwd));
    printf("%s\n",cwd);
}

// prints the file name under current directory of the server

void dirCommand(int sd){
    char opcode;
    int fileSize;
    
    if(writeByte(sd,OP_DIR) == -1){
        printf("unable to send dir \n");
        return;
    }
    
    if(readByte(sd,&opcode) == -1){
        printf("unable to read opcode \n");
        return;
    }
    
    if(opcode != OP_DIR){
        printf("invalid opcode : dor %c\n",opcode);
        return;
    }
    
    if(readFourByteLength(sd, &fileSize) == -1){
        printf("unable to read file size \n");
        return;
    }
    
    
    char directory[fileSize + 1];
    
    if(readNBytes(sd,directory,fileSize) == -1){
        printf("unable to read directory \n");
        return;
    }
    
    directory[fileSize] = '\0';
    
    printf("%s\n",directory);
    
}

// this function displays the file names under the current directory of the client

void ldirCommand(){
    DIR *d;
    struct dirent **fileList;
    d = opendir(".");
    if(d){
        int numFiles = scandir(".",&fileList,NULL,alphasort);
        if(numFiles < 0){
            printf("failed to scan directory \n");
        }
        for(int i = 0; i < numFiles; ++i){
            if(fileList[i]->d_type == DT_REG){
                printf("[f]"); // f : file
            }
            else if(fileList[i]->d_type == DT_DIR){
                printf("[d]");// d : directory
            }else {
                printf("[o]"); // o : other
            }
            printf("%s\n",fileList[i]->d_name);
        }
        closedir(d);
        free(fileList);
    }else {
        printf("failed to scan directory \n");
    }
}


// this function changes the current directory of the server that is serving the client


void cdCommand(int sd, char * token){
    char opcode;
    char ackcode;
    int length = strlen(token);
    
    // unable to send change directory
    
    if(writeByte(sd,OP_CD) == -1){
        printf("unable to send cd \n");
        return;
    }
    // unable to write length of file
    if(writeTwoByteLength(sd,length) == -1){
        printf("unable to write length \n");
        return;
    }
    
    if(writeNBytes(sd,token,strlen(token)) == -1){
        printf("unable to write to directory name \n");
        return;
    }
    
    if(readByte(sd,&opcode) == -1){
        printf("unable to read opcode \n");
        return;
    }
    
    if(opcode != OP_CD){
        printf("invalid opcode for cs %c\n",opcode);
        return;
    }
    
    if(readByte(sd,&ackcode) == -1){
        printf("unable to read acknowledgement code \n");
        return;
    }
    
    if(ackcode == ACK_CD_SUCCESS){
        return;
    }
    
    if(ackcode == ACK_CD_FIND){
        printf("the server cannot find directory \n");
        return;
    }
    
}

// the function changes the current directory of the client

void lcdCommand(char * token){
    if(chdir(token) != 0){
        printf("directory not found \n");
    }
}

// displays a message stating current session is terminated
void quitCommand()
{
    printf("session is terminated \n");
    exit(0);
}
// displays all available commands and their description
void helpCommand(){
    printf("Commands and Descriptions:\n");
    printf("\tpwd - to display the current directory of the server that is serving the client\n");
    printf("\tlpwd - to display the current directory of the client\n");
    printf("\tdir - to display the file names under the current directory of the server that is serving the client\n");
    printf("\tldir - to display the file names under the current directory of the client\n");
    printf("\tcd <directory_pathname> - to change the current directory of the server that is serving the client\n");
    printf("\tlcd <directory_pathname> - to change the current directory of the client\n");
    printf("\tget <filename> - to download the named file from the current directory of the remote server and save it in the current directory of the client\n");
    printf("\tput <filename> - to upload the named file from the current directory of the client to the current directory of the remote server\n");
    printf("\tquit - to terminate the session\n");
    printf("\thelp - display this information\n");
}
