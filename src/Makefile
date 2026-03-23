CC=gcc
CFLAGS= -D FRAME_STORE_SIZE=$(framesize) -D MEM_SIZE=$(varmemsize) -g -pthread
FMT=indent
framesize ?= 900
varmemsize ?= 1000

mysh: *.c
	$(CC) $(CFLAGS) -c *.c
	$(CC) $(CFLAGS) -o mysh *.o

style:
	$(FMT) *.h *.c

clean: 
	$(RM) mysh; $(RM) *.o; $(RM) *~

