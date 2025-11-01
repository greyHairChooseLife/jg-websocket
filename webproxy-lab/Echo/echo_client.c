#include "../csapp.h"

int main (int argc, char ** argv)
{
    int clientfd; // host port를 넣어주기 위해
    char * host, *port;
    rio_t rio;
    char buf[MAXLINE];
    
    if(argc != 3)
    {
        printf("input fuck.");
        exit(-1);
    }
    host = argv[1];
    port = argv[2];
    clientfd = Open_clientfd(host, port);

    if(clientfd < 0)
    {
        if(clientfd == -1)
        {
            printf("connect fuck.");
            exit(-1);
        }
        
        else if(clientfd == -2)
        {
            printf("something fuck.");
            exit(-1);
        }

        else
        {
            printf("somthing really fucked");
            exit(-1);
        }
    }

    Rio_readinitb(&rio, clientfd); // 리오 버퍼 초기화
    Rio_writen(clientfd, buf, MAXLINE); 
    fputs(buf,stdout);
    Close(clientfd);
    exit(0);

}