#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>

#define MAXLINE 80
#define SERVER_PORT 8000
#define OPEN_MAX 102400

int main(int argc, char *argv[]) {
    int i, j, maxi, server_socket, client_fd, sockfd;
    int event_num, epoll_fd, res;
    ssize_t n;
    char buf[MAXLINE], str[INET_ADDRSTRLEN];
    socklen_t clilen;
    int client[OPEN_MAX];
    struct sockaddr_in client_addr, server_addr;
    struct epoll_event tep, ep[OPEN_MAX];

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error");
        return -1;
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);
    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("bind error");
        return -1;
    }
    //
    if (listen(server_socket, 20) < 0) {
        perror("listen error");
        return -1;
    }
    for (i = 0; i < OPEN_MAX; i++) {
        client[i] = -1;
    }
    maxi = -1;
    if ((epoll_fd = epoll_create(OPEN_MAX)) < 0) {
        perror("epoll_create error");
        return -1;
    }
    tep.events = EPOLLIN;
    tep.data.fd = server_socket;
    if ((res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &tep)) < 0) {
        perror("epoll_ctl error");
        return -1;
    }
    while (1) {
        printf("==========start monitor...\n");
        //阻塞式监听
        event_num = epoll_wait(epoll_fd, ep, OPEN_MAX, -1);
        if (event_num == -1) {
            perror("epoll_wait error");
        }
        printf("有%d文件描述符就绪\n", event_num);
        //如果是服务器socket可读，处理连接请求
        for (i = 0; i < event_num; i++) {
            if ((ep[i].data.fd == server_socket) && (ep[i].events == EPOLLIN)) {
                clilen = sizeof(client_addr);
                client_fd = accept(server_socket, (struct sockaddr *) &client_addr,
                                   &clilen);
                printf("received from %s at port %d \n",
                       inet_ntop(AF_INET, &client_addr.sin_addr, str, sizeof(str)),
                       ntohs(client_addr.sin_port));
                //累计连接数
                for (j = 0; j < OPEN_MAX; j++) {
                    if (client[j] < 0) {
                        client[j] = client_fd;
                        break;
                    }
                }
                if (j == OPEN_MAX) {
                    perror("too many clients");
                    return -1;
                }
                //现连接数
                if (j > maxi) {
                    maxi = j;
                }
                printf("已连接客户端数：%d\n", j+1);
                //注册客户端socket读事件
                tep.events = EPOLLIN;
                tep.data.fd = client_fd;
                res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &tep);
                if (res == -1) {
                    perror("epoll_ctl error");
                    return -1;
                }
                //如果是客户端socket可读，则获取请求信息，相应客户端
            } else {
                sockfd = ep[i].data.fd;
                n = read(sockfd, buf, MAXLINE);
                if (n == 0) {
                    for (j = 0; j < maxi; j++) {
                        if (client[j] == sockfd) {
                            client[j] = -1;
                            break;
                        }
                    }
                    res = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sockfd, NULL);
                    if (res == -1) {
                        perror("epoll_ctl error");
                        return -1;
                    }
                    close(sockfd);
                    printf("client[%d] closed connection\n", j);
                } else {
                    for (j = 0; j < n; j++) {
                        buf[j] = toupper(buf[j]);
                    }
                    write(sockfd, buf, n);
                }
            }
        }
    }
}


