/* server.c */  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <unistd.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
  
#include "base64.h"  
#include "sha1.h"  
#include "intLib.h"  
  
  
#define REQUEST_LEN_MAX 1024  
#define DEFEULT_SERVER_PORT 8000  
#define WEB_SOCKET_KEY_LEN_MAX 256  
#define RESPONSE_HEADER_LEN_MAX 1024  
#define LINE_MAX 256  
  
  
void shakeHand(int connfd,const char *serverKey);  
char * fetchSecKey(const char * buf);  
char * computeAcceptKey(const char * buf);  
char * analyData(const char * buf,const int bufLen);  
char * packData(const char * message,unsigned long * len);  
void response(const int connfd,const char * message);  
  
int main(int argc, char *argv[])  
{  
    struct sockaddr_in servaddr, cliaddr;  
    socklen_t cliaddr_len;  
    int listenfd, connfd;  
    char buf[REQUEST_LEN_MAX];  
    char *data;  
    char str[INET_ADDRSTRLEN];  
    char *secWebSocketKey;  
    int i,n;  
    int connected=0;//0:not connect.1:connected.  
    int port= DEFEULT_SERVER_PORT;  
  
    if(argc>1)  
      {  
        port=atoi(argv[1]);  
      }  
    if(port<=0||port>0xFFFF)  
      {  
        printf("Port(%d) is out of range(1-%d)\n",port,0xFFFF);  
        return;  
      }  
    listenfd = socket(AF_INET, SOCK_STREAM, 0);  
  
    bzero(&servaddr, sizeof(servaddr));  
    servaddr.sin_family = AF_INET;  
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  
    servaddr.sin_port = htons(port);  
      
    bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));  
  
    listen(listenfd, 20);  
  
    printf("Listen %d\nAccepting connections ...\n",port);  
    cliaddr_len = sizeof(cliaddr);  
    connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);  
    printf("From %s at PORT %d\n",  
               inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),  
               ntohs(cliaddr.sin_port));  
  
    while (1)  
      {  
      
        memset(buf,0,REQUEST_LEN_MAX);  
        n = read(connfd, buf, REQUEST_LEN_MAX);   
        printf("---------------------\n");  
      
      
        if(0==connected)  
          {  
            printf("read:%d\n%s\n",n,buf);  
            secWebSocketKey=computeAcceptKey(buf);    
            shakeHand(connfd,secWebSocketKey);  
            connected=1;  
            continue;  
          }  
  
        data=analyData(buf,n);  
        response(connfd,data);  
    }  
    close(connfd);  
}  