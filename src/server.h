#ifndef INCLUDE_SERVER_H
#define INCLUDE_SERVER_H

#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <stdlib.h>

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <error.h>

#include <jsoncpp/myjson/myjson.h>
#include <jsoncpp/json/json.h>
#include <boost/noncopyable.hpp>

#include "dataPool.h"
#include "info.h"

const int MAXLEN = 1024;

class Server : boost::noncopyable {
public:
    Server() {}
    
    Server(uint16_t port)
        :_port(port)
    {}

    void initServer() {
        
        //创建套接字
        _sock = socket(AF_INET, SOCK_STREAM, 0);
        
        if (_sock < 0) {
            perror("socket");
            return;
        }

        //绑定端口号
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_port = htons(_port);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        //绑定 ip 地址
        if (bind(_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("bind");
            return;
        }
        //保证 UDP 可靠性，可以仿照 TCP
    }

    //服务端接收数据
    void recvData(std::string& strInfo) {
        char buffer[MAXLEN];

        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        ssize_t rs = recvfrom(_sock, buffer, MAXLEN - 1, 0, (struct sockaddr*)&peer, &len);
        
        if (rs > 0) {
            buffer[rs] = 0;
            strInfo = buffer;
            
            //异步处理
            //服务器收到消息后放到消息队列中，而不是立即处理
            _dataPool.putMessage(strInfo);
            
            //将收到的数据进行反序列化
            Info info;
            info.DeSerialize(strInfo);
            
            //某种字段回显客户端状态
            if (info._token == "quit") {
                std::map<uint32_t, struct sockaddr_in>::iterator it = _onlineList.find(peer.sin_addr.s_addr);
                if (it != _onlineList.end()) {
                    _onlineList.erase(it->first);
                }
            } else {
                _onlineList.insert(std::pair<uint32_t, struct sockaddr_in>(peer.sin_addr.s_addr, peer));
            }
        }
    }

    //一对一单聊
    //一对一消息模式
    //服务端发送数据至客户端具体 userId
    void sendData(std::string& strInfo, const struct sockaddr_in& peer) {
        sendto(_sock, strInfo.c_str(), strInfo.size(), 0, (struct sockaddr*)&peer, sizeof(peer));
    }

    //一对多群聊----
    //发布/订阅消息模式
    void broadCast() {
        std::string data;
        _dataPool.getMessage(data);
        
        //unordered_map 是好友列表的容器
        std::map<uint32_t, struct sockaddr_in>::iterator it = _onlineList.begin();

        for (; it != _onlineList.end(); it++) {
            sendData(data, it->second);
        }
    }

    ~Server() {
        close(_sock);
        _port = -1;
    }

private:
    int _sock;
    int _port;
    DataPool _dataPool;     //消息队列
    std:: map<uint32_t, struct sockaddr_in> _onlineList;   //在线好友列表
};

#endif
