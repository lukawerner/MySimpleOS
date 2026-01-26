CC=gcc
CFLAGS= -g
FMT=indent

mysh: shell.c interpreter.c shellmemory.c helper.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c helper.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o helper.o

style: shell.c shell.h interpreter.c interpreter.h shellmemory.c shellmemory.h helper.c helper.h
	$(FMT) $?

clean: 
	$(RM) mysh; $(RM) *.o; $(RM) *~

