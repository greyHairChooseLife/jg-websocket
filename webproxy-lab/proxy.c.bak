#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define MAXBUF_SIZE 8192

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

// 요청 처리 함수
void doit(int clientfd);

// URI 파싱
int parse_uri(char *uri, char *hostname, char *path, int *port);

// 요청 생성
void build_request(char *request, char *hostname, char *path, rio_t *client_rio);

// 서버 연결
int connect_to_server(char *hostname, int port);

// 응답 전달
void forward_response(int serverfd, int clientfd);

int main(int argc, char **argv) {
    int listenfd, clientfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    char port[10];

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);

    while (1) {
        clientlen = sizeof(clientaddr);
        clientfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
        Getnameinfo((SA *)&clientaddr, clientlen, NULL, 0, port, sizeof(port), 0);
        printf("Accepted connection on port %s\n", port);

        doit(clientfd);
        Close(clientfd);
    }
}

void doit(int clientfd) {
    char buf[MAXBUF_SIZE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char hostname[MAXLINE], path[MAXLINE], request[MAXBUF_SIZE];
    int port;
    rio_t client_rio;

    Rio_readinitb(&client_rio, clientfd);
    if (!Rio_readlineb(&client_rio, buf, MAXBUF_SIZE))
        return;

    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET")) {
        printf("Unsupported method: %s\n", method);
        return;
    }

    if (!parse_uri(uri, hostname, path, &port)) {
        printf("Invalid URI: %s\n", uri);
        return;
    }

    build_request(request, hostname, path, &client_rio);

    int serverfd = connect_to_server(hostname, port);
    if (serverfd < 0) {
        printf("Failed to connect to server: %s\n", hostname);
        return;
    }

    Rio_writen(serverfd, request, strlen(request));
    forward_response(serverfd, clientfd);
    Close(serverfd);
}

int parse_uri(char *uri, char *hostname, char *path, int *port) {
    // 기본 포트는 80
    *port = 80;

    // 예: http://www.cmu.edu:8080/hub/index.html
    char *hostbegin = strstr(uri, "//");
    hostbegin = hostbegin ? hostbegin + 2 : uri;
    // hostbegin = www.cmu.edu:8080/hub/index.html

    char *hostend = strpbrk(hostbegin, ":/");
    // hostend = :8080/hub/index.html
    ///hostend(noport) = hub/index.html

    int hostlen = hostend ? hostend - hostbegin : strlen(hostbegin);
    // hostlen = 11 www.cmu.edu

    strncpy(hostname, hostbegin, hostlen);
    // hostname = www.cmu.edu
    hostname[hostlen] = '\0';

    if (hostend && *hostend == ':')  // hostend가 포트나 서픽스가 있으면 주소값이 있으므로 True 그리고 *hostend는 첫번째 값 그리고 그게 :면 포트가 포함된 서픽스
    {
      sscanf(hostend + 1, "%d%s", port, path); // hostend + 1 주소부터 정수와 문자열을 port 와 path에 복사
    } 
    else if (hostend && *hostend == '/') // 만약 포트가 없다면 서픽스만 있음
    {
      strcpy(path, hostend); // path에 바로 서픽스를 복사
    } 
    else // 만약 포트도 서픽스도 없으면 
    {
      strcpy(path, "/"); // path에 바로 /를 복사 
    }

    return 1;
}

void build_request(char *request, char *hostname, char *path, rio_t *client_rio) {
    char buf[MAXBUF_SIZE];

    sprintf(request, "GET %s HTTP/1.0\r\n", path);
    sprintf(request + strlen(request), "Host: %s\r\n", hostname);
    sprintf(request + strlen(request), "%s", user_agent_hdr);
    sprintf(request + strlen(request), "Connection: close\r\n");
    sprintf(request + strlen(request), "Proxy-Connection: close\r\n");

    // 클라이언트가 보낸 나머지 헤더 복사
    while (Rio_readlineb(client_rio, buf, MAXBUF_SIZE) > 0) {
        if (!strcmp(buf, "\r\n")) break;
        if (strncasecmp(buf, "Host:", 5) &&
            strncasecmp(buf, "User-Agent:", 11) &&
            strncasecmp(buf, "Connection:", 11) &&
            strncasecmp(buf, "Proxy-Connection:", 17)) {
            strcat(request, buf);
        }
    }

    strcat(request, "\r\n");
}

int connect_to_server(char *hostname, int port) {
    char port_str[10];
    sprintf(port_str, "%d", port);
    return Open_clientfd(hostname, port_str);
}

void forward_response(int serverfd, int clientfd) {
    char buf[MAXBUF_SIZE];
    ssize_t n;
    rio_t server_rio;

    Rio_readinitb(&server_rio, serverfd);
    while ((n = Rio_readlineb(&server_rio, buf, MAXBUF_SIZE)) > 0) {
        Rio_writen(clientfd, buf, n);
    }
}

