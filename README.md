# IM

IM 是一个基于 epoll + 多线程实现的聊天室(类似于 QQ 聊天)

### 基本功能

- IM 支持 QQ 中的基本功能

- IM 支持单人聊天

- IM 支持多人聊天，即群聊

- IM 支持文件传输

- IM 支持用户状态信息推送，在线 or 离开

- IM 支持消息回执，不丢失，及可靠性保证

- IM 支持离线信息，聊天信息的存取与拉取

- IM 支持心跳检测异常断开的客户端，检测客户端验证超时

- IM 支持 TLS 安全传输层协议

- 支持 IM 服务的开启、关闭及重启(通过命令控制 ./ctl.sh -s ···)

- 支持优雅关闭连接(shutdown)

### 我实现了··· ···

- IM 聊天室的客户端、服务端(epoll)

- 线程池、内存池

- 定时器(基于时间轮)

- 缓冲队列

- 维护消息可靠性(参考TCP)

### 版本历史

- version-0.0.1

2019.05.02：实现简单的聊天 C/S 通信

- version-0.0.2

2019.05.05：实现基于 poll 的聊天 C/S 通信

- version-0.0.3

2019.05.08：实现基于 epoll 的聊天 C/S 通信

- version-0.0.4

2019.05.14：实现了基于 epoll + 多线程 + 异步日志的多人聊天室

- version-0.0.5

2019.05.18：实现了基于 epoll + 线程池 + 异步日志 + ncurses 的多人聊天室程序

- version-0.0.6

基于上述层面，就关闭套接字后输出缓冲区数据的丢失问题、消息的不可靠问题、离线消息的存取问题、超时客户端的异常连接做了进一步优化

- im

2019.05.25：项目描述

### demo

![](https://github.com/Apriluestc/img.org/blob/master/demo.png)

### 详情见 CSDN

URL：![https://blog.csdn.net/qq_41880190/article/details/90235055](https://blog.csdn.net/qq_41880190/article/details/90235055)
