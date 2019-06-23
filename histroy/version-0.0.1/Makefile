all : server client

server : server.c
	gcc $^ -o $@

client : client.c
	gcc $^ -o $@

.PHONY :

clean :
	rm -rf server client
