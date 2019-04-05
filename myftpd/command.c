#include "command.h"
#include "stream.h"
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

void commandPUT(descriptors * desc){
    
    logger(desc,"PUT"); // creates a log file for PUT command
    
    int filenameLength; // stores the length of filename
    int acknowledgeCode; // creates a ackowledge code
    char opcode; // operation code
    int fileDescriptor; // file descriptor
    
    // read the filename and length by 2 bytes, check if the return is 1 to indicate file reading is sucessful
    if(readTwoByteLength(desc->sd, &filenameLength)== -1){
        logger(desc,"Failed to read two byte length");// return to logger that read failed
        return;
    }
    
    // creates a charbuffer of the size of the filelength + 1
    char filename[filenameLength + 1];
    
    // reads the filenamelength of the file to the filename buffer, check if return is N byte to indicate file reading is sucessful
    if(readNBytes(desc->sd,filename,filenameLength) == -1){
        logger(desc,"Failed to read filename");
        return;
    }
    
    // null terminator for the filename buffer
    filename[filenameLength] = '\0';
    
    // stores put in a filename
    logger(desc,"PUT %s",filename);
    
    // attempt to create the file
    acknowledgeCode = ACK_PUT_SUCCESS;
    
    // attempts to openfile
    if((fileDescriptor= open(filename,O_RDONLY)) != -1){
        logger(desc,"File already exist : %s",filename);
        acknowledgeCode = ACK_PUT_FILENAME;
    }else if((fileDescriptor = open(filename,O_WRONLY | O_CREAT, 0777)) == -1){
        logger(desc,"Cannot create file ",filename);
        acknowledgeCode = ACK_PUT_CREATEFILE;
    }
    
    // write acknowledgement for operation put
    
    if(writeByte(desc->sd,OP_PUT) == -1){
        logger(desc,"Failed to write OP_PUT");
        return;
    }

    // write acknowledgement for acknowledge code to log
    if(writeByte(desc->sd,acknowledgeCode) == -1){
        logger(desc,"[-] Failed to write ackcode: %c",acknowledgeCode);
        return;
    }
    // write compeleted to log if acknowledge code not equal to sucess
    if(acknowledgeCode != ACK_PUT_SUCCESS)
    {
        logger(desc,"PUT completed.");
        return;
    }
    
    // reads the response from the client
    if(readByte(desc->sd,&opcode) == -1){
        logger(desc,"Failed to read code");
    }
    
    // expects to read from OP_DATA
    if(opcode != OP_DATA){
        logger(desc,"[-] Unexpected opcode:%c, expected: %c",opcode,OP_DATA);
        return;

    }
    
    int fileSize; // stores the size of file
    
    // reads the file size
    
    if(readFourByteLength(desc->sd,&fileSize)== -1){
        logger(desc,"[-] Failed to read filesize.");
        return;
    }
    
    
    int blockSize = FILE_BLOCK_SIZE;
    if(FILE_BLOCK_SIZE > fileSize){
        blockSize = fileSize;
    }
    
    // creates a file buffer of block size
    char fileBuffer[blockSize];
    int bytesRead = 0 , byteswritten = 0;
    
    // iterate through the filesize and to read and write file
    while(fileSize > 0){
        if(blockSize > fileSize){
            blockSize = fileSize;
        }
        
        if((bytesRead = readNBytes(desc->sd,fileBuffer,blockSize)) == -1){
            logger(desc,"[-] Failed to read bytes.");
            close(fileDescriptor);
            return;
        }
        
        if((byteswritten = write(fileDescriptor,fileBuffer,bytesRead)) < bytesRead){
            logger(desc,"[-] Failed to write %d bytes, wrote %d bytes instead.",bytesRead,byteswritten);
            close(fileDescriptor);
            acknowledgeCode = ACK_PUT_WRERROR;
        }
        
        fileSize -= byteswritten;
    }
    
    // close file descriptor
    close(fileDescriptor);
    
    if(writeByte(desc->sd,OP_DATA) == -1){
        logger(desc,"[-] Failed to write OP_DATA.");
        return;
    }
    
    if(writeByte(desc->sd,acknowledgeCode) == -1){
        logger(desc,"[-] Failed to write OP_DATA.");
        return;
    }
    
    if(acknowledgeCode == ACK_PUT_SUCCESS){
        logger(desc,"PUT success.");
    }
    
    logger(desc,"PUT completed.");
}


// Handles protocol process to send a requested file from the server to the client
void commandGET(descriptors * desc){

    logger(desc,"GET");

    int fileDescriptor; // used to store file descriptor values
    struct stat inf;
    int filesize; // stores the size of the file
    int filenameLength;
    char ackcode; // acknowledge code

    // read filename and length
    if(readTwoByteLength(desc->sd,&filenameLength) == -1){
        printf("failed to read 2 byte length");
        return;
    }

    char filename[filenameLength + 1];

    if(readNBytes(desc->sd,filename,filenameLength) == -1){
        printf("failed to read filename.");
        return;
    }

    // set last element to null terminator

    filename[filenameLength] = '\0';

    logger(desc,"GET %s",filename);

    // process the file

    if((fileDescriptor = open(filename,O_RDONLY)) == -1){
        ackcode = ACK_GET_FIND;
        logger(desc,"%s",ACK_GET_FIND_MSG);
        if(writeByte(desc->sd,OP_GET)== -1){
            logger(desc,"Failed to write opcode : %c " , OP_GET);
            return;
        }
        if(writeByte(desc->sd,ackcode) == -1){
            logger(desc,"Failed to write ackcode %c",ackcode);
        }
        return;
    }

    // used to determine information about the file, check if file type is directory
    if(fstat(fileDescriptor,&inf) < 0 || (inf.st_mode & S_IFMT) == S_IFDIR){
        if((inf.st_mode & S_IFMT) == S_IFDIR){
            logger(desc, "File requested is a directory");
        }else {
            logger(desc,"fstat error");
        }
        ackcode = ACK_GET_OTHER;
        logger(desc,"%s",ACK_GET_OTHER_MSG);
        if(writeByte(desc->sd,OP_GET) == -1){
            logger(desc,"failed to write opcode %c",OP_GET);
            return;
        }
        if(writeByte(desc->sd,ackcode) == -1){
            logger(desc,"Failed to write ackcode %c",ackcode);
        }
        return;

    }

    filesize = (int)inf.st_size;

    //reset the file pointer
    lseek(fileDescriptor,0,SEEK_SET);

    //send the data
    if(writeByte(desc->sd,OP_DATA) == -1){
        logger(desc,"Failed to send OP_DATA");
        return;
    }

    if(writeFourByteLength(desc->sd,filesize) == -1){
        logger(desc,"Failed to send file size");
        return;
    }
    // stores the number of bytes read
    int bytesRead = 0;
    char buf[FILE_BLOCK_SIZE];

    while((bytesRead = read(fileDescriptor,buf,FILE_BLOCK_SIZE)) > 0){
        if(writeNBytes(desc->sd,buf,bytesRead) == -1){
            logger(desc,"failed to send file content");
            return;
        }
    }

    logger(desc,"GET success");
    logger(desc,"GET complete");

}

void commandPWD(descriptors * desc){
    logger(desc,"PWD");
    
    char cwd[1024];
    
    // get pathname of working directory
    getcwd(cwd,sizeof(cwd));
    
    if(writeByte(desc->sd,OP_PWD) == -1){
        logger(desc,"Failed to write opcode");
        return;
    }
    
    if(writeTwoByteLength(desc->sd,strlen(cwd)) == -1){
        logger(desc,"failed to write length");
        return;
    }
    
    if(writeNBytes(desc->sd,cwd,strlen(cwd)) == -1){
        logger(desc,"failed to write directory");
        return;
    }
    
    logger(desc,"PWD completed");
    
    
}
// handles protocol process to display directory listing of the server

void commandDIR(descriptors * desc){
    logger(desc,"DIR");
    char files[1024] = "";
    
    DIR *dir; // pointer to directory
    struct dirent **fileList; //
    dir = opendir("."); // opens the directory stream named by the dirname argument
    if(dir){
        char tempChar[2];
        // scans the directory by filelist and sort them alphatically return number of files
        int numFiles = scandir(".",&fileList,NULL,alphasort);
        if(numFiles < 0){
            logger(desc,"failed to open scan directory");
            
        }
        for(int i = 0; i < numFiles; i++){
            if(fileList[i]->d_type == DT_REG){
                tempChar[0] = 'f'; // file
            }
            else if(fileList[i]->d_type == DT_DIR){
                tempChar[0] = 'd'; // directory
            }else{
                tempChar[0] = 'o'; // other
            }
            tempChar[1] = '\0';
            strcat(files,"[");
            strcat(files,tempChar);
            strcat(files,"]");
            strcat(files,fileList[i]->d_name);
            strcat(files,"\n");
        }
        files[strlen(files) -1] = '\0';
        closedir(dir);
        free(fileList);
        logger(desc,"DIR success.");
        
    }else{
        logger(desc,"DIR failed");
    }
    
    if(writeByte(desc->sd,OP_DIR) == -1){
        logger(desc,"failed to write opcode");
        return;
    }
    if(writeFourByteLength(desc->sd,strlen(files)) == -1){
        logger(desc,"failed to write length");
        return;
    }
    if(writeNBytes(desc->sd,files,strlen(files)) == -1){
        logger(desc,"failed to write the file list");
        return;
    }
    logger(desc,"DIR complete");
    
    
}

// protocol for changing directory
void commandCD(descriptors *desc){
    logger(desc,"CD");
    
    int size;
    int ackcode;
    
    if(readTwoByteLength(desc->sd,&size) == -1){
        logger(desc,"failed to read size");
        return;
    }
    
    char token[size+1];
    
    if(readNBytes(desc->sd,token,size) == -1){
        logger(desc,"failed to read token");
        return;
    }
    
    token[size] = '\0';
    
    logger(desc,"CD %s",token);
    
    
    if(chdir(token) == 0){
        ackcode = ACK_CD_SUCCESS;
    }else {
        ackcode = ACK_CD_FIND;
        logger(desc,"cd cannot find directory");
    }
    
    
    if(writeByte(desc->sd,OP_CD) == -1){
        logger(desc,"failed to send cd");
        return;
    }
    
    if(writeByte(desc->sd,ackcode) == -1){
        logger(desc,"failed to send ackcode");
        return;
    }
    
    logger(desc,"cd complete");
    
    
    
}

