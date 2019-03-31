# Project Title
A FTP Client and Server Program Done In C programming includes list of commands 

PWD - print working directory
cd - change directory
PUT - put a file onto the server
GET - Get a file from the server
ldir - displays files name in current directory
lpwd - displays current directory of the client

## Preresquites
Any Unix Machine with shell prompt 

Ensure GCC and Makefile are installed on your machine to compile the program

To install GCC on Linux
```
$ sudo apt install gcc
```

To install makefile on linux
```
$ sudo apt install make
```


## Getting Started
Clone or install the repo onto your local machine

compile the program using the command make clean and then make in both folders before running the program

Example in myftp folder

```
$ make clean
```

```
$ make 
```

To run the server and client program on your local machine , run two terminals and go into the client directory myftp and server directory myftpd to run the executable.

The server program creates a daemon process , please ensure you kill the process after running the server or the program will continue to run on your machine

Ensure you run the server before the client program

Example of running the server program in the server directory

```
$./myftpd
```

Example of running the client program in the client directory

```
$ ./myftp
```

Use the command help to show list of commands from client machine. 


The client is also able to connect using the ipaddress and ports by specifying the arguments , for example

```
$./myftp <ip add> <port num>
```

## Authors
John Wee


