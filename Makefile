all: server.c client.c
	gcc server.c socket.c client-info.c -o server
	gcc client.c socket.c client-info.c -o client
clean:
	rm -f server client