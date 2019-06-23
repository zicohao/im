
.PHONY:all
all:client server

client:client.c
	gcc -o $@ $^
server:server.c
	g++ -o $@ $^

.PHONY:clean
clean:
	rm -r client server
