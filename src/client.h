#ifndef INCLUDE_CLIENT_H
#define INCLUDE_CLIENT_H

#include <iostream>
#include <string>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <boost/noncopyable.hpp>

const int MAXLEN = 1024;

class Client : boost::noncopyable {
public:
    Client() {}

    Client(std::string ip, uint16_t port) {
        _addr.sin_family = AF_INET;
        _addr.sin_port = htons(port);
        _addr.sin_addr.s_addr = inet_addr(ip.c_str());
    }

    void initClient() {
       
        //创建套接字
        _sock = socket(AF_INET, SOCK_DGRAM, 0);

        if (_sock < 0) {
            perror("socket");
            return;
        }
    }

    //客户端接收数据
    void recvData(std::string& strInfo) {
        char buffer[MAXLEN];
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        ssize_t rs = recvfrom(_sock, buffer, MAXLEN - 1, 0, (struct sockaddr*)&peer, &len);
        if (rs > 0) {
            buffer[rs] = 0;
            strInfo = buffer;
        }
    }

    //客户端发送数据
    void sendData(std::string& strInfo) {
        sendto(_sock, strInfo.c_str(), strInfo.size(), 0, (struct sockaddr*)&_addr, sizeof(_addr));
    }

    ~Client() {
        close(_sock);
    }

private:
    //套接字
    int _sock;
    struct sockaddr_in _addr;
};

#endif
