#include "server.h"

void* recvMessage(void* arg) {
    Server* us = (Server*)arg;
    std::string message;
    while (1) {
        us->recvData(message);
        std::cout << "test" << message << std::endl;
    }
}

void* sendMessage(void* arg) {
    Server* us = (Server*)arg;
    while (1) {
        us->broadCast();
    }
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        std::cout << "usage:" << argv[0] << " " << "port" << std::endl;
        return 1;
    }
    Server ss(atoi(argv[1]));
    ss.initServer();
    pthread_t r, s;
    pthread_create(&r, NULL, recvMessage, (void*)&ss);
    pthread_create(&s, NULL, sendMessage, (void*)&ss);
    pthread_join(r, NULL);
    pthread_join(s, NULL);
    return 0;
}
