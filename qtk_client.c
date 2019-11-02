#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, const char * argv[]) {
    int client_socket;
    struct sockaddr_in ser_addr;
    char sendbuf[400];
    char recbuf[400];
    int sennum,recnum;
    if((client_socket= socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        perror("socket");
        return -1;
    }

    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(8090);
    ser_addr.sin_addr.s_addr = inet_addr("192.168.0.98");

    if(connect(client_socket, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("connect with destination host...\n");

    while(1)
    {
        printf("Input your world:>");
        scanf("%s", sendbuf);
        printf("\n");
        sendbuf[strlen(sendbuf)] = '\n';
        send(client_socket, sendbuf, strlen(sendbuf), 0);

        recnum = recv(client_socket, recbuf, 4000, 0);
        recbuf[recnum] = '\0';
        printf("recv is: %s\n", recbuf);
        if(strcmp(recbuf, "quit") == 0)
            break;
    }
    close(client_socket);

    return 0;
}
