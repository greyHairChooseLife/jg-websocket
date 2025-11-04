/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"

void doit(int fd);
void read_requesthdrs(rio_t* rp);
int parse_uri(char* uri, char* filename, char* cgiargs);
void serve_static(int fd, char* filename, int filesize);
void get_filetype(char* filename, char* filetype);
void serve_dynamic(int fd, char* filename, char* cgiargs);
void clienterror(int fd,
                 char* cause,
                 char* errnum,
                 char* shortmsg,
                 char* longmsg);

int main(int argc, char** argv) {
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

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
        Getnameinfo((SA*)&clientaddr, clientlen, hostname, MAXLINE, port,
                    MAXLINE, 0);
        printf("Accepted connection from (%s, %s)\n", hostname, port);
        doit(connfd);
        Close(connfd);
    }
}

void doit(int connfd) {
    rio_t rp;
    char rawReq[MAXBUF];
    char method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    rio_readinitb(&rp, connfd);
    rio_readlineb(&rp, rawReq, MAXBUF);
    printf("req in raw ----------> %s", rawReq);

    sscanf(rawReq, "%s %s %s", method, uri, version);  // GET / HTTP/1.1
    printf("req in [%s %s %s]\n", method, uri, version);

    if (strcasecmp(method, "GET"))  // returns not "\0" when it's different
    {
        clienterror(connfd, method, "501", "GET method only",
                    "Server does not serve such thing.");
        return;
    }
}

void clienterror(int fd,
                 char* cause,
                 char* errnum,
                 char* shortmsg,
                 char* longmsg) {
    char header[MAXLINE], body[MAXBUF];

    /* Build the HTTP response body */
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body,
            "%s<body style=\"background-color: #000000; color: #FFFFFF\">\r\n",
            body);
    sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s</p>\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);
    sprintf(body, "%s</body></html>\r\n", body);

    /* Print the HTTP response */
    sprintf(header, "HTTP/1.1 %s %s\r\n", errnum, shortmsg);
    sprintf(header, "%sContent-type: text/html\r\n", header);
    sprintf(header, "%sContent-length: %d\r\n\r\n", header, (int)strlen(body));

    Rio_writen(fd, header, strlen(header));
    Rio_writen(fd, body, strlen(body));
}
