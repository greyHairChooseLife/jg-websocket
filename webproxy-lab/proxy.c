#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define DEFAULT_DEST_PORT "80"

typedef struct {
    char Host[MAXLINE];
    char UserAgent[MAXLINE];  // User-Agent
    char Conn[MAXLINE];       // Connection
    char PConn[MAXLINE];      // Proxy-Connection
    char remain[MAXLINE];
} appendHeaders;

/* You won't lose style points for including this long line in your code */
static const char* user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3";

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

    int fwdClieFd;  // Forwarding Client File Descriptor with `open_clientfd()`

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

    sprintf(headerPtr->Host, "Host: %s\r\n", destHost);
    sprintf(headerPtr->UserAgent, "User-Agent: %s\r\n", (char*)user_agent_hdr);
    sprintf(headerPtr->Conn, "Connection: close\r\n");
    sprintf(headerPtr->PConn, "Proxy-Connection: close\r\n");

    strcpy(headerPtr->remain, "");
    do
    {
        readSize = rio_readlineb(rp, headers, MAXBUF);
        if (strcmp(headers, "\r\n") == 0) break;
        strncat(headerPtr->remain, headers, readSize);
    } while (readSize > 0);
}

void forward_request(int fwdClieFd,
                     char* method,
                     char* destSuffix,
                     char* destVersion,
                     appendHeaders* headerPtr) {
    char reqLine[MAXLINE];

    // create request Line
    sprintf(reqLine, "%s %s %s\r\n", method, destSuffix, destVersion);

    // send request Line
    rio_writen(fwdClieFd, reqLine, strlen(reqLine));
    // send request headers
    rio_writen(fwdClieFd, headerPtr->Host, strlen(headerPtr->Host));
    rio_writen(fwdClieFd, headerPtr->remain, strlen(headerPtr->remain));
    rio_writen(fwdClieFd, headerPtr->Conn, strlen(headerPtr->Conn));
    rio_writen(fwdClieFd, headerPtr->PConn, strlen(headerPtr->PConn));
    rio_writen(fwdClieFd, headerPtr->UserAgent, strlen(headerPtr->UserAgent));
    rio_writen(fwdClieFd, "\r\n", strlen("\r\n"));
}

// recieve & toss: line, headers, empty-line, body
void recieve_response(int fwdClieFd, int originConnFd) {
    rio_t rp;
    size_t readSize;
    char buf[MAXBUF] = "\0";
    char headers[MAXBUF] = "\0";  // res line & headers to toss
    // Content lenth string pointer
    char* p;
    size_t remain = 0;  // content length

    rio_readinitb(&rp, fwdClieFd);

    // process response line & headers
    do
    {
        readSize = rio_readlineb(&rp, buf, MAXBUF);
        strcat(headers, buf);
        if (strcmp(buf, "\r\n") == 0) break;
        if ((p = strstr(buf, "Content-Length: ")) != NULL)
            sscanf(p, "Content-Length: %zu", &remain);
    } while (readSize > 0);
    rio_writen(originConnFd, headers, strlen(headers));

    // process response body
    while (remain)
    {
        readSize = rio_readnb(&rp, buf, MAXBUF);
        rio_writen(originConnFd, buf, readSize);
        remain -= readSize;
        if (readSize <= 0) break;  // EOF or error
    }
}
