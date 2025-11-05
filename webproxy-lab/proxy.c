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

void read_req_line(int fd, rio_t* rp, char* method, char* uri, char* version);
void parse_uri(char* uri, char* destHost, char* destPort, char* destSuffix);
void read_requesthdrs(int originConnFd,
                      rio_t* rp,
                      appendHeaders* headerPtr,
                      char* destHost);
void forward_request(int fwdClieFd,
                     char* method,
                     char* destSuffix,
                     char* destVersion,
                     appendHeaders* headerPtr);
void recieve_response(int fwdClieFd, int originConnFd);

int main(int argc, char** argv) {
    rio_t rp;
    int listenfd, originConnFd;
    char originHost[MAXLINE], originPort[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char destHost[MAXLINE], destPort[MAXLINE], destSuffix[MAXLINE],
        destVersion[MAXLINE];

    appendHeaders header;
    appendHeaders* headerPtr = &header;

    // Forwarding Client File Descriptor with `open_clientfd()`
    int fwdClieFd;

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
        originConnFd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
        Getnameinfo((SA*)&clientaddr, clientlen, originHost, MAXLINE,
                    originPort, MAXLINE, 0);
        rio_readinitb(&rp, originConnFd);
        read_req_line(originConnFd, &rp, method, uri, version);
        parse_uri(uri, destHost, destPort, destSuffix);
        strcpy(destVersion, "HTTP/1.0");

        /* START_debug: */
        // printf("----- originHost: %s\n", originHost);
        // printf("----- originPort: %s\n", originPort);
        // printf("----- originVersion: %s\n", version);
        // printf("------- destHost: %s\n", destHost);
        // printf("------- destPort: %s\n", destPort);
        // printf("------- destSuffix: %s\n", destSuffix);
        // printf("------- destVersion: %s\n", destVersion);
        /* END___debug: */

        read_requesthdrs(originConnFd, &rp, headerPtr, destHost);

        fwdClieFd = Open_clientfd(destHost, destPort);
        forward_request(fwdClieFd, method, destSuffix, destVersion, headerPtr);
        recieve_response(fwdClieFd, originConnFd);

        Close(originConnFd);
    }

    return 0;
}

void read_req_line(int fd, rio_t* rp, char* method, char* uri, char* version) {
    char buf[MAXBUF];
    char filename[MAXLINE], cgiargs[MAXLINE];

    rio_readlineb(rp, buf, MAXLINE);

    sscanf(buf, "%s %s %s", method, uri, version);
}

void parse_uri(char* uri, char* destHost, char* destPort, char* destSuffix) {
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

void read_requesthdrs(int originConnFd,
                      rio_t* rp,
                      appendHeaders* headerPtr,
                      char* destHost) {
    char headers[MAXBUF] = "\0";
    size_t readSize;
    char destHostCopy[MAXBUF] = "\0";

    headerPtr->remain[0] = '\0';
    strcpy(destHostCopy, destHost);
    strcat(destHostCopy, "\r\n");
    strcpy(headerPtr->Host, destHostCopy);

    do
    {
        readSize = rio_readlineb(rp, headers, MAXBUF);
        if (strcmp(headers, "\r\n") == 0) break;
        strncat(headerPtr->remain, headers, readSize);
    } while (readSize > 0);
    /* START_debug: */
    // printf(">>>>>>>>>>>>>>>>>>>>\n");
    // printf("%s", headerPtr->remain);
    // printf("<<<<<<<<<<<<<<<<<<<<\n");
    /* END___debug: */
    strcpy(headerPtr->UserAgent, (char*)user_agent_hdr);
    strcpy(headerPtr->Connection, "close\r\n");
    strcpy(headerPtr->ProxyConnection, "close\r\n");
}

void forward_request(int fwdClieFd,
                     char* method,
                     char* destSuffix,
                     char* destVersion,
                     appendHeaders* headerPtr) {
    char reqLine[MAXLINE];
    char _Host[MAXLINE] = "\0";
    char _UserAgent[MAXLINE] = "\0";  // User-Agent
    char _Connection[MAXLINE] = "\0";
    char _ProxyConnection[MAXLINE] = "\0";  // Proxy-Connection
    char _remain[MAXLINE] = "\0";

    // create reqLine
    strcpy(reqLine, method);
    strcat(reqLine, " ");
    strcat(reqLine, destSuffix);
    strcat(reqLine, " ");
    strcat(reqLine, destVersion);
    strcat(reqLine, "\r\n");

    // req line
    rio_writen(fwdClieFd, reqLine, strlen(reqLine));

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

    /* START_debug: */
    // printf("-------- header: %s", _Host);
    // printf("-------- header: %s", _remain);
    // printf("-------- header: %s", _Connection);
    // printf("-------- header: %s", _ProxyConnection);
    // printf("-------- header: %s", _UserAgent);
    /* END___debug: */

    rio_writen(fwdClieFd, _Host, strlen(_Host));
    rio_writen(fwdClieFd, _remain, strlen(_remain));
    rio_writen(fwdClieFd, _Connection, strlen(_Connection));
    rio_writen(fwdClieFd, _ProxyConnection, strlen(_ProxyConnection));
    rio_writen(fwdClieFd, _UserAgent, strlen(_UserAgent));
    rio_writen(fwdClieFd, "\r\n", strlen("\r\n"));
}

// recieve: line, headers, empty-line, body
void recieve_response(int fwdClieFd, int originConnFd) {
    rio_t rp;
    char readBuf[MAXBUF];
    size_t readSize;
    size_t totalReadSize = 0;
    // Content lenth string pointer
    char* p;
    size_t contentLength = 0;

    rio_readinitb(&rp, fwdClieFd);

    do
    {
        readSize = rio_readlineb(&rp, readBuf, MAXBUF);
        rio_writen(originConnFd, readBuf, readSize);
        /* printf("recieved line & headers: %s\n", readBuf); */
        if ((p = strstr(readBuf, "Content-Length: ")) != NULL)
        {
            // 문자열에서 포멧을 지정해 바로 담을 수 있다.
            sscanf(p, "Content-Length: %zu", &contentLength);
        }
        /* } while (strcmp(readBuf, "\r\n") != 0); */
    } while (readSize != 0);

    if (contentLength <= 0) return;
    do
    {
        readSize = rio_readlineb(&rp, readBuf, MAXBUF);
        totalReadSize += readSize;
        rio_writen(originConnFd, readBuf, strlen(readBuf));
        /* printf("recieved body: %s\n", readBuf); */
    } while (totalReadSize != contentLength);
}
