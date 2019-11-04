#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

using namespace std;
int main(int argc , char *argv[])
{

    int localSocket, recved;
    localSocket = socket(AF_INET , SOCK_STREAM , 0);

    if (localSocket == -1){
        printf("Fail to create a socket.\n");
        return 0;
    }

    struct sockaddr_in info;
    bzero(&info,sizeof(info));

    info.sin_family = PF_INET;
    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(8888);

    int err = connect(localSocket,(struct sockaddr *)&info,sizeof(info));

    sockaddr_in getsockaddr;
    socklen_t len = sizeof(sockaddr_in);
    int r=getsockname(localSocket,(sockaddr *)&getsockaddr,&len);
    int port=ntohs(getsockaddr.sin_port);
    char dst[100];
    printf("The Client address is %s\n",inet_ntop(AF_INET,&getsockaddr,dst,100));
    printf("my port is %d\n",port);


    if(err==-1){
        printf("Connection error\n");
        return 0;
    }
    char receiveMessage[BUFF_SIZE] = {};
    while(1){
        /*
        bzero(receiveMessage,sizeof(char)*BUFF_SIZE);
        if ((recved = recv(localSocket,receiveMessage,sizeof(char)*BUFF_SIZE,0)) < 0){
            cout << "recv failed, with received bytes = " << recved << endl;
            break;
        }
        else if (recved == 0){
            cout << "<end>\n";
            break;
        }
        printf("Is receive Blocked?\n");
        char buffer[100] = "hello";
        send(localSocket , buffer , strlen(buffer) , 0 ); 
        //printf("%d:%s\n",recved,receiveMessage);
        */
        printf("here\n");
        char command[10];
        scanf("%s",command);
        if(strcmp(command,"ls")==0){
            send(localSocket , command , strlen(command) , 0 );
            char buffer[1024];
            read(localSocket,receiveMessage,BUFF_SIZE);
            printf("receive %s\n",receiveMessage);
            send(localSocket , "GetIt" , strlen("GetIt") , 0 );
        }
        else if(strcmp(command,"put")==0){
            
        }
        else if(strcmp(command,"get")==0){
            
        }
        else if(strcmp(command,"play")==0){
            
        }
    }
    printf("close Socket\n");
    close(localSocket);
    return 0;
}
