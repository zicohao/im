# IM

IM 是一个基于 epoll + 多线程实现的聊天室(类似于 QQ 聊天)

### 基本功能

- IM 支持 QQ 中的基本功能

- IM 支持单人聊天

- IM 支持多人聊天，即群聊

- 支持消息回执，消息不丢失，保证数据传输的可靠性

- IM 支持用户状态信息推送，在线 or 离开

- IM 支持离线信息，聊天信息的存取与拉取

- IM 支持心跳检测异常断开的客户端，检测客户端验证超时

- 支持 IM 服务的开启、关闭及重启(通过命令控制 ./ctl.sh -s ···)

- 支持优雅关闭连接(shutdown)----说明：version-0.0.3 支持

### 我实现了··· ···

- IM 聊天室的客户端、服务端(epoll)-版本 3

- UDP 可靠性保证、丢包重传、离线消息拉取

- 线程池、内存池

- 定时器(基于时间轮)

- 缓冲队列

### 版本历史

- version-0.0.1

2019.05.02：实现简单的聊天 C/S 通信

- version-0.0.2

2019.05.05：实现基于 poll 的聊天 C/S 通信

- version-0.0.3

2019.05.08：实现基于 epoll 的聊天 C/S 通信

- version-0.0.4

2019.05.14：实现了基于 epoll + 多线程 + 异步日志的聊天程序

- version-0.0.5

2019.05.23：实现了基于 UDP socket + 多线程 + 双缓冲区异步日志 + ncurses 字符终端界面库的多人聊天室

### demo

![](https://github.com/Apriluestc/img.org/blob/master/demo.png)

### 详情见 CSDN

URL：https://blog.csdn.net/qq_41880190/article/details/90235055
