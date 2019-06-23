PROJECT			= $(shell pwd)
BIN 			= $(PROJECT)/bin
CC		= gcc

all : $(BIN)/server $(BIN)/client

$(BIN)/server : $(PROJECT)/*.cpp
	[ -e $(BIN) ] || mkdir $(BIN)
	g++ $^ -o $@ -std=c++11 -lpthread

$(BIN)/client : $(PROJECT)/client.c
	gcc $^ -o $@

.PHONY :

clean:
	rm -rf $(BIN)/server client
