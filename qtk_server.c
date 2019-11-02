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

#define BUFF_SIZE 1024
#define SERVER_PORT 8000
#define OPEN_MAX 102400

/**
 * 创建一个socket
 *
 * @return
 */
int create_socket() {
    int server_socket;
    struct sockaddr_in server_addr;

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
    if (listen(server_socket, 20) < 0) {
        perror("listen error");
        return -1;
    }

    return server_socket;
}

/**
 * 注销epoll事件
 *
 * @param epoll_fd
 * @param client_fd
 * @param statu
 */
void cancel_epoll(int epoll_fd, int client_fd, int statu) {
    struct epoll_event event;
    event.events = statu;
    event.data.fd = client_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
}

/**
 * 注册epoll事件
 *
 * @param epoll_fd
 * @param client_fd
 * @param statu
 */
void register_epoll(int epoll_fd, int client_fd, int statu) {
    struct epoll_event event;
    event.events = statu;
    event.data.fd = client_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
}

/**
 * 客户端连接
 *
 * @param server_socket
 * @param epoll_fd
 */
void accept_client(int server_socket, int epoll_fd) {
    int client_fd;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    client_fd = accept(server_socket, (struct sockaddr *) &client_addr, &client_len);
    register_epoll(epoll_fd, client_fd, EPOLLIN);
}

/**
 * 处理请求
 *
 * @param client_fd
 * @param epoll_fd
 */
void deal_client(int client_fd, int epoll_fd) {
    int n;
    char buf[BUFF_SIZE];

    memset(buf, 0, BUFF_SIZE);
    n = read(client_fd, buf, BUFF_SIZE);
    if (n == 0) {
        cancel_epoll(epoll_fd, client_fd, EPOLLIN);
        close(client_fd);
    } else {
        printf("recv data:%s\n", buf);
        write(client_fd, buf, n);
    }
}
/*void deal_client(int client_fd, int epoll_fd) {
    int n;
    char buf[BUFF_SIZE];

    memset(buf, 0, BUFF_SIZE);
    n = read(client_fd, buf, BUFF_SIZE);
    if (n == 0) {
        cancel_epoll(epoll_fd, client_fd, EPOLLIN);
        close(client_fd);
    } else {
        //根据结尾标识符\n判断同一个请求包
        while (1){
            if(buf[n-1] == '\n'){
                printf("say:%s\n", buf);
                write(client_fd, buf, n);
                printf("读取结束\n");
                break;
            }else{
                write(client_fd, buf, n);
                memset(buf, 0, BUFF_SIZE);
                n = read(client_fd, buf, BUFF_SIZE);
            }
        }
    }
}*/

/**
 * main函数
 *
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[]) {
    int i, server_socket, client_fd;
    int event_num, epoll_fd;
    struct epoll_event ep[OPEN_MAX];

    server_socket = create_socket();
    epoll_fd = epoll_create(OPEN_MAX);
    register_epoll(epoll_fd, server_socket, EPOLLIN);

    while (1) {
        event_num = epoll_wait(epoll_fd, ep, OPEN_MAX, 0);
        for (i = 0; i < event_num; i++) {
            //如果是服务器socket可读，处理连接请求
            if ((ep[i].data.fd == server_socket) && (ep[i].events == EPOLLIN)) {
                accept_client(server_socket, epoll_fd);
            }
            //否则为客户端可读，处理请求信息
            else {
                client_fd = ep[i].data.fd;
                deal_client(client_fd, epoll_fd);
            }
        }
    }
}



