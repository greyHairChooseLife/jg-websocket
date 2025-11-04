#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char* user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void readReqLine(int fd, char* method, char* uri, char* version);

int main(int argc, char** argv) {
    int listenfd, connfd;
    char originHost[MAXLINE], originPort[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    /* Check command line args */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA*)&clientaddr, clientlen, originHost, MAXLINE, originPort,
                    MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", originHost, originPort);
        readReqLine(connfd, method, uri, version);
        Close(connfd);
    }

    return 0;
}

void readReqLine(int fd, char* method, char* uri, char* version) {
    char buf[MAXBUF];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rp;

    rio_readinitb(&rp, fd);
    rio_readlineb(&rp, buf, MAXLINE);

    sscanf(buf, "%s %s %s", method, uri, version);
}
