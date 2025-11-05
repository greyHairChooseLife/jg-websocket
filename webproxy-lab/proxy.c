#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define DEFAULT_DEST_PORT "80"

typedef struct {
    char Host[MAXLINE];
    char UserAgent[MAXLINE];  // User-Agent
    char Connection[MAXLINE];
    char ProxyConnection[MAXLINE];  // Proxy-Connection
    char remain[MAXLINE];
} appendHeaders;

/* You won't lose style points for including this long line in your code */
static const char* user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void readReqLine(int fd, char* method, char* uri, char* version);
void parseUri(char* uri, char* destHost, char* destPort, char* destSuffix);
void read_requesthdrs(rio_t* rp, appendHeaders* headerPtr, char* destHost);
void forwardRequest(int clientFd,
                    char* method,
                    char* destSuffix,
                    char* destVersion,
                    appendHeaders* headerPtr);

int main(int argc, char** argv) {
    int listenfd, connfd;
    char originHost[MAXLINE], originPort[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char destHost[MAXLINE], destPort[MAXLINE], destSuffix[MAXLINE],
        destVersion[MAXLINE];

    rio_t rp;
    appendHeaders header;
    appendHeaders* headerPtr = &header;

    int clientFd;

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
        printf("------- destHost: %s\n", destHost);
        printf("------- destPort: %s\n", destPort);
        printf("------- destSuffix: %s\n", destSuffix);
        printf("------- destVersion: %s\n", destVersion);

        rio_readinitb(&rp, connfd);
        read_requesthdrs(&rp, headerPtr, destHost);

        printf("-------- header: Host %s", headerPtr->Host);
        printf("-------- header: Connection %s", headerPtr->Connection);
        printf("-------- header: ProxyConn %s", headerPtr->ProxyConnection);
        printf("-------- header: UserAgent %s", headerPtr->UserAgent);
        printf("-------- header: remain %s", headerPtr->remain);

        clientFd = Open_clientfd(destHost, destPort);
        forwardRequest(clientFd, method, destSuffix, destVersion, headerPtr);
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
    char *suffixStartPtr, *portStartPtr;

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

void read_requesthdrs(rio_t* rp, appendHeaders* headerPtr, char* destHost) {
    char headers[MAXBUF];
    size_t readSize;
    char destHostCopy[MAXBUF];

    headerPtr->remain[0] = '\0';
    strcpy(destHostCopy, destHost);
    strcat(destHostCopy, "\r\n");
    strcpy(headerPtr->Host, destHostCopy);

    do
    {
        readSize = rio_readlineb(rp, headers, MAXBUF);
        strncat(headerPtr->remain, headers, readSize);
    } while (strcmp(headers, "\r\n") != 0);

    strcpy(headerPtr->UserAgent, (char*)user_agent_hdr);
    strcpy(headerPtr->Connection, "close\r\n");
    strcpy(headerPtr->ProxyConnection, "close\r\n");
}

void forwardRequest(int clientFd,
                    char* method,
                    char* destSuffix,
                    char* destVersion,
                    appendHeaders* headerPtr) {
    char reqLine[MAXLINE];
    char _Host[MAXLINE];
    char _UserAgent[MAXLINE];  // User-Agent
    char _Connection[MAXLINE];
    char _ProxyConnection[MAXLINE];  // Proxy-Connection
    char _remain[MAXLINE];

    // create reqLine
    strcpy(reqLine, method);
    strcat(reqLine, " ");
    strcat(reqLine, destSuffix);
    strcat(reqLine, " ");
    strcat(reqLine, destVersion);
    strcat(reqLine, "\r\n");

    // req line
    rio_writen(clientFd, reqLine, strlen(reqLine));

    // req headers
    strcpy(_Host, "Host: ");
    strcat(_Host, headerPtr->Host);
    strcpy(_UserAgent, "User-Agent: ");
    strcat(_UserAgent, headerPtr->UserAgent);
    strcpy(_Connection, "Connection: ");
    strcat(_Connection, headerPtr->Connection);
    strcpy(_ProxyConnection, "Proxy-Connection: ");
    strcat(_ProxyConnection, headerPtr->ProxyConnection);
    strcat(_remain, headerPtr->remain);
    rio_writen(clientFd, _Host, strlen(_Host));
    rio_writen(clientFd, _Connection, strlen(_Connection));
    rio_writen(clientFd, _ProxyConnection, strlen(_ProxyConnection));
    rio_writen(clientFd, _UserAgent, strlen(_UserAgent));
    rio_writen(clientFd, _remain, strlen(_remain));
}
