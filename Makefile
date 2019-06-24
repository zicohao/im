PROJECT_PATH 	:= $(shell pwd)/

SRC_PATH	   	:= $(PROJECT_PATH)src/

BIN_PATH	   	:= $(SRC_PATH)bin/

LIB_PATH 	   	:= /usr/lib64/libjsoncpp.a

LD_FLAGS 	   	:= -lpthread -ljsoncpp

CC 			   	:= g++


all : $(BIN_PATH)server $(BIN_PATH)client
	
$(BIN_PATH)server : $(SRC_PATH)server.cpp $(SRC_PATH)server.h $(SRC_PATH)dataPool.h $(SRC_PATH)info.h
	[ -e $(BIN_PATH) ] || mkdir $(BIN_PATH)
	$(CC) -g $^ -o $@ $(LIB_PATH) $(LD_FLAGS) -std=c++11

$(BIN_PATH)client : $(SRC_PATH)client.cpp $(SRC_PATH)client.h
	$(CC) -g $^ -o $@ $(LIB_PATH) $(LD_FLAGS) -std=c++11 -lncurses

.PHONY:

clean :
	rm -rf $(BIN_PATH)server $(BIN_PATH)client
