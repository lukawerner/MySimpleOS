CC=gcc
CFLAGS= -g -pthread
FMT=indent

mysh: *.c
	$(CC) $(CFLAGS) -c *.c
	$(CC) $(CFLAGS) -o mysh *.o

style:
	$(FMT) *.h *.c

clean: 
	$(RM) mysh; $(RM) *.o; $(RM) *~

