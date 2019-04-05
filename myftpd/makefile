#makefile for the server

myftpd: command.o daemon.o logger.o stream.o
	gcc -Wall myftpd.c  command.o daemon.o logger.o stream.o -o myftpd

command.o: command.h command.c
	gcc -c -Wall command.c -o command.o	

daemon.o: daemon.h daemon.c
	gcc -c -Wall daemon.c -o daemon.o

logger.o: logger.h logger.c
	gcc -c -Wall logger.c -o logger.o

stream.o: stream.c stream.h
	gcc -c -Wall stream.c -o stream.o

clean:
	rm -f *.o myftpd
