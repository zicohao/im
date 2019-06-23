#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8000
#define IP "127.0.0.1"

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("Usage:client IP\n");
        return 1;
    }
    (void)argv;
    char buf[1024];
    memset(buf, '\0', sizeof(buf));
    struct sockaddr_in server_sock;
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&server_sock, sizeof(server_sock));
    server_sock.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &server_sock.sin_addr);
    server_sock.sin_port = htons(PORT);

    int ret = connect(sock, (struct sockaddr*)&server_sock, sizeof(server_sock));

    if (ret < 0) {
        printf("connect failed ···. errno is : %d, errstring is : %s\n", errno, strerror(errno));
        return 1;
    }
    printf("connect success···\n");

    while (1) {
        printf("client:#");
        fgets(buf, sizeof(buf), stdin);
        buf[strlen(buf) - 1] = '\0';
        write(sock, buf, sizeof(buf));
        if (strncasecmp(buf, "quit", 4) == 0) {
            printf("quit\n");
            break;
        }
        printf("please wait···\n");
        read(sock, buf, sizeof(buf));
        printf("server:$ %s\n", buf);
    }
    close(sock);
    return 0;
}
