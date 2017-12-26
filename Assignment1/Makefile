CC = gcc
CFLAGS = -ansi -Wall -pedantic

myshobj = mysh.o

all:	mysh
mysh:	 $(myshobj)
	 $(CC) -o mysh $(myshobj) $(CFLAGS)
mysh.o:  mysh.c
	 $(CC) -c mysh.c
clean:
	-rm -f mysh $(myshobj)
