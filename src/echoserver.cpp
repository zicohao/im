#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <pthread.h>
#include <sys/wait.h>

const int DEFAULTPORT = 5000;
const int EVENTS = 1000;
typedef unsigned long UL;
typedef long long LL;

struct conn {
    int sock;
    char* buffer;
    size_t alloced, head, tail;
    bool readOfEnd;
    bool error;
    conn(int sock)
        :sock(sock)
        ,buffer(0)
        ,head(0)
        ,tail(0)
        ,readOfEnd(false)
        ,error(false)
    {
        alloced = 32;
        buffer = (char*)malloc(alloced);
        if (!buffer) {
            puts("no memory.\n");
            exit(-1);
        }
    }
    void read() {
        for ( ; ; ) {
            if (alloced - tail < 64) {
                if (alloced - (tail - head) < 128) {
                    alloced *= 2;
                    buffer = (char*)realloc(buffer, alloced);
                    if (!buffer) {
                        puts("no memory(realloc)");
                        exit(-1);
                    }
                } else {
                    memmove(buffer, buffer + head, tail - head);
                    tail -= head;
                    head = 0;
                }
            }
            int n = ::read(sock, buffer + tail, alloced - tail);
            if (n < 0) {
                if (errno == EAGAIN) {
                    break;
                }
                perror("read");
                error = true;
                return;
            }
            if (n == 0) {
                readOfEnd = true;
            }
            tail += n;
        }
    }
    int write() {
        while (head < tail) {
            int n = ::write(sock, buffer + head, tail - head);
            if (n < 0) {
                if (errno == EAGAIN) {
                    break;
                }
                perror("write");
                error = true;
                return -1;
            }
            head += n;
        }
        return tail - head;
    }
    void handle() {
        if (error) {
            return;
        }
        read();
        if (error) {
            return;
        }
        write();
    }
    int done() const {
        return error || (readOfEnd && (tail - head));
    }
};

static void setnonblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}
static int setup_server_socket(int port) {
    int sock;
    struct sockaddr_in sin;
    int yes = 1;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(-1);
    }
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
    memset(&sin, 0, sizeof sin);
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port = htons(port);
    if (bind(sock, (struct sockaddr*)&sin, sizeof sin) < 0) {
        close(sock);
        perror("bind");
        exit(-1);
    }
    if (listen(sock, 1024) < 0) {
        close(sock);
        perror("listen");
        exit(-1);
    }
    return sock;
}


int main(int argc, char* argv[])
{
    struct epoll_event ev;
    struct epoll_event events[EVENTS];
    int listen, epfd;
    int procs = 1;
    int opt, port = DEFAULTPORT;
    /*
    int ret = fork();
    if (ret < 0) {
        perror("fork");
        return 1;
    } else if (ret == 0) {
        if (execlp("/home/shiny/workspace/chat/txproj_version", "./txproj_version", NULL) < 0) {
            printf("execl error\n");
        }
    } else {
    }*/
    while (-1 != (opt = getopt(argc, argv, "p:s:"))) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        case 's':
            procs = atoi(optarg);
            break;
        default:
            fprintf(stderr, "unknown option: %c\n", opt);
            return 1;
        }
    }
    listen = setup_server_socket(port);
    for (int i = 1; i < procs; ++i) {
        fork();
    }
    if((epfd = epoll_create(128)) < 0) {
        perror("epoll_create");
        exit(-1);
    }
    memset(&ev, 0, sizeof(ev));
    ev.events = EPOLLIN;
    ev.data.fd = listen;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen, &ev);
    printf("listening port %d\n", port);
    UL proc = 0;
    struct timeval tv, tv_prev;
    gettimeofday(&tv_prev, NULL);
    tv_prev = tv;
    for( ; ; ) {
        int i;
        int nfd = epoll_wait(epfd, events, EVENTS, -1);
        for (i = 0; i < nfd; i++) {
            if (events[i].data.fd == listen) {
                struct sockaddr_in addr;
                socklen_t len = sizeof addr;
                int client = accept(listen, (struct sockaddr*)&addr, &len);
                if (client < 0) {
                    perror("accept");
                    continue;
                }
                setnonblocking(client);
                memset(&ev, 0, sizeof ev);
                ev.events = EPOLLIN | EPOLLET;
                ev.data.ptr = (void*)new conn(client);
                epoll_ctl(epfd, EPOLL_CTL_ADD, client, &ev);
            } else {
                conn* pc = (conn*)events[i].data.ptr;
                pc->handle();
                proc++;
                if (pc->done()) {
                    epoll_ctl(epfd, EPOLL_CTL_DEL, pc->sock, &ev);
                    delete pc;
                }
            }
        }
        if (proc > 100000) {
            proc = 0;
            gettimeofday(&tv, NULL);
            timersub(&tv, &tv_prev, &tv_prev);
            LL d = tv_prev.tv_sec;
            d *= 1000;
            d += tv_prev.tv_usec / 1000;
            printf("%lld msec per 100000 req\n", d);
            printf("%lld reqs per sec\n", 100000LL * 1000 / d);
            tv_prev = tv;
        }
    }
    return 0;
}
