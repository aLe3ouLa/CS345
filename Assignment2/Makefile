CC = gcc
CFLAGS = -ansi -Wall -pedantic -pthread

serverobj = server.o
clientobj = client.o

executables = server client

all:	$(executables)
server:	$(serverobj)
	$(CC) -o server $(serverobj) $(CFLAGS)
server.o: server.c
	$(CC) -c server.c
client: $(clientobj)
	$(CC) -o client $(clientobj) $(CFLAGS)
client.o: client.c
	$(CC) -c client.c
clean:
	-rm -f guess_socket client server $(serverobj) $(clientobj)
