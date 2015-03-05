CC=g++
XFLAG =-Wall -ansi -pedantic
CFLAG =
main: passPattern 


passPattern: passPattern.o 
	$(CC) $(CFLAG) passPattern.o -o passPattern 
passPattern.o: passPattern.c
	$(CC) $(CFLAG) -c passPattern.c
clean:
	rm -f passPattern 
	rm -f *.o
