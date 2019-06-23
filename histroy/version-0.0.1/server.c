#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

//const int PORT = 8000;
//const int  BACKLOG = 10;

#define PORT 8000
#define BACKLOG 10


int main()
{
    // 创建套接字
    int sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock < 0) {
        printf("create socket error, error is : %d, errstring is : %s\n", errno, strerror(errno));
    }
    struct sockaddr_in server_socket;
    struct sockaddr_in client_socket;
    bzero(&server_socket, sizeof(server_socket));
    server_socket.sin_family = AF_INET;
    server_socket.sin_addr.s_addr = htonl(INADDR_ANY);
    server_socket.sin_port = htons(PORT);
    
    // 绑定端口号
    if (bind(sock, (struct sockaddr*)&server_socket, sizeof(struct sockaddr_in)) < 0) {
        printf("bind error, error code is : %d, error string is : %s\n", errno, strerror(errno));
        close(sock);
        return 1;
    }

    // 监听套接字
    if (listen(sock, BACKLOG) < 0) {
        printf("listen error, error code is : %d, error string is : %s\n", errno, strerror(errno));
        close(sock);
        return 2;
    }
    printf("bind and listen success, wait accept···\n");

    for (;;) {
        socklen_t len = 0;
        int client_sock = accept(sock, (struct sockaddr*)&client_sock, &len);

        if (client_sock < 0) {
            printf("accept error, error is : %d, errstring is : %s\n", errno, strerror(errno));
            close(sock);
            return 3;
        }
        char buf[INET_ADDRSTRLEN];
        memset(buf, '\0', sizeof(buf));
        //inet_ntop(AF_INET, &client_socket.sin_addr, buf, sizeof(buf));
        printf("get connect, ip is : %s port is : %d\n", buf, ntohs(client_socket.sin_port));
        while (1) {
            char buffer[1024];
            memset(buffer, '\0', sizeof(buffer));
            read(client_sock, buffer, sizeof(buffer));
            printf("client :# %s\n", buffer);
            printf("server :$");
            memset(buffer, '\0', sizeof(buffer));
            fgets(buffer, sizeof(buffer), stdin);
            buffer[strlen(buffer) - 1] = '\0';
            write(client_sock, buffer, strlen(buffer) + 1);
            printf("please wait ···\n");
        }
    }
    close(sock);
    return 0;
}
