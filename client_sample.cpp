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
            int32_t Client_FileNumber;
            char *FileNumberPtr = (char*)&Client_FileNumber;
            read(localSocket, FileNumberPtr, 4);
            Client_FileNumber = ntohl(Client_FileNumber);
            printf("Client_FileNumber %d\n",Client_FileNumber);
            for(int i=0;i<Client_FileNumber;i++){
                int32_t Client_charNumber;
                char *CharNumberPtr = (char*)&Client_charNumber;
                read(localSocket, CharNumberPtr, 4);
                Client_charNumber = ntohl(Client_charNumber);
                read(localSocket, buffer, Client_charNumber);
                buffer[Client_charNumber] = '\0';
                printf("FileName == %s\n",buffer);
            }
            /*
            while(read(localSocket, data, 4)>0){
                Client_charNumber = ntohl(Client_charNumber);
                printf("Char number == %d\n",Client_charNumber);
                read(localSocket, buffer, Client_charNumber);
                buffer[Client_charNumber] = '\0';
                printf("File name == %s\n",buffer);
            }
            */
            /*
            int valread;
            //should be done in the if scope
            //first trans a number to ensure the file number
            if ((valread = read( localSocket , buffer, 1024)) > 0) { 
                buffer[valread] = '\0';
                printf("number is %s\n",buffer);
            }
            for(int i=0;i<fileNum;i++){
                if ((valread = read( localSocket , buffer, 1024)) > 0) { 
                    buffer[valread] = '\0';
                    printf("Name:%s\n",buffer);
                }
            }
            */
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
