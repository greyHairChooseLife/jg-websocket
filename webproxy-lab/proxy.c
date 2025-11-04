#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define DEFAULT_DEST_PORT "80"

/* You won't lose style points for including this long line in your code */
static const char* user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void readReqLine(int fd, char* method, char* uri, char* version);
void parseUri(char* uri, char* destHost, char* destPort, char* destSuffix);

int main(int argc, char** argv) {
    int listenfd, connfd;
    char originHost[MAXLINE], originPort[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char destHost[MAXLINE], destPort[MAXLINE], destSuffix[MAXLINE],
        destVersion[MAXLINE];

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
        Getnameinfo((SA*)&clientaddr, clientlen, originHost, MAXLINE,
                    originPort, MAXLINE, 0);
        readReqLine(connfd, method, uri, version);
        parseUri(uri, destHost, destPort, destSuffix);
        strcpy(destVersion, "HTTP/1.0");
        printf("----- originHost: %s\n", originHost);
        printf("----- originPort: %s\n", originPort);
        printf("----- originVersion: %s\n", version);
        printf("----- destHost: %s\n", destHost);
        printf("----- destPort: %s\n", destPort);
        printf("----- destSuffix: %s\n", destSuffix);
        printf("----- destVersion: %s\n", destVersion);
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

void parseUri(char* uri, char* destHost, char* destPort, char* destSuffix) {
    char* hostPtr;
    char result[MAXLINE];
    char *portMayStartPtr, *suffixStartPtr, *portStartPtr;

    // uri = http://www.cmu.edu:9555/hub/index.html

    hostPtr = strstr(uri, "//") + 2;

    strcpy(result, hostPtr);  // "www.cmu.edu:9555/hub/index.html"

    suffixStartPtr = strchr(result, '/');  // "/hub/index.html"
    strcpy(destSuffix, suffixStartPtr);

    *suffixStartPtr = '\0';  // now result is "www.cmu.edu:9555"

    if ((portStartPtr = strchr(result, ':')) == NULL)
        strcpy(destPort, DEFAULT_DEST_PORT);
    else
    {
        strcpy(destPort, portStartPtr + 1);
        *portStartPtr = '\0';  // now result is "www.cmu.edu"
    }

    strcpy(destHost, result);
}
