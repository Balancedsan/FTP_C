#makefile for client

myftp: myftp.o stream.o parse.o command.o
	gcc -Wall myftp.o stream.o parse.o command.o -o myftp
command.o: command.c command.h
	gcc -c -Wall command.c -o command.o
stream.o: stream.c stream.h
	gcc -c -Wall stream.c -o stream.o
pase.o: parse.c parse.h
	gcc -c -Wall parse.c -o parse.o
clean:
	rm -f *.o myftp
