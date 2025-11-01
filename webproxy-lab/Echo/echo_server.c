#include "../csapp.h"

void echo(int);

int main(int argc, char** argv) {
    int listenfd, connfd;
    char* port;
    socklen_t client_len;
    struct sockaddr_storage client_addr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    char userbuf[MAXLINE];

    rio_t rio;

    if (argc != 2)
    {
        printf("input fuck");
        exit(-1);
    }

    port = argv[1];
    listenfd = Open_listenfd(port);

    if (listenfd < 0)
    {
        printf("socket covert into listen fail.. fuck");
        exit(-1);
    }

    while (1)
    {
        client_len = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA*)&client_addr, &client_len);
        if (connfd < 0) continue;

        rio_readinitb(&rio, connfd);
        rio_readlineb(&rio, userbuf, strlen(userbuf));
        Fputs("ok bro\n", stdout);
        while ((rio_readlineb(&rio, userbuf, strlen(userbuf))) != NULL)
        {
            rio_writen(connfd, userbuf, strlen(userbuf));
        }
        Close(connfd);
    }

    // echo(connfd);
}

void echo(int connect_fd) {
    rio_t rio;

    Rio_readinitb(&rio, connect_fd);
}
