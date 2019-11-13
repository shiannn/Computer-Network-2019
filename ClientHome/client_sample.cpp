#include <iostream>
#include <sys/socket.h> 
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h> 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "opencv2/opencv.hpp"
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;
using namespace cv;

#define BUFF_SIZE 1024
#define MaxFileName 1000
#define Client_Get "clientGetIt"
#define Server_Get "serverGetIt"
#define StopVideo "stopvideo"
#define MyEOF "@@@@@@"
#define EOFnum 6
#define FileNotFound "@@FileNotFound@@"
#define FileFound "@@FileFound@@"

using namespace std;
int main(int argc , char *argv[])
{

    int localSocket, recved;
    localSocket = socket(AF_INET , SOCK_STREAM , 0);

    if (localSocket == -1){
        printf("Fail to create a socket.\n");
        return 0;
    }

    mkdir("clientDir", S_IRWXU);
	chdir("clientDir");

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
        // EOF should give greeting message back, too
        printf("here\n");
        char command[10];
        char fileName[MaxFileName];
        char ToSend[BUFF_SIZE];
        //scanf("%s",command);
        char Input[MaxFileName];
        char Dummy[MaxFileName];
        fgets(Input, MaxFileName, stdin);
        int retScan = sscanf(Input,"%s%s%s",command,fileName,Dummy);
        printf("retScan == %d\n",retScan);
        if(retScan >= 3){
            printf("Command format error.\n");
            continue;
        }
        if(strcmp(command,"ls")==0){
            if(retScan == 2){
                printf("Command format error.\n");
                continue;
            }
            strcpy(ToSend,command);
            send(localSocket , ToSend , strlen(ToSend) , 0 );
            //char buffer[1024];
            int count = read(localSocket,receiveMessage,BUFF_SIZE);
            receiveMessage[count] = '\0';
            printf("receive %s\n",receiveMessage);
            send(localSocket , Client_Get , strlen(Client_Get) , 0 );
        }
        else if(strcmp(command,"put")==0){
            //scanf("%s",fileName);
            if(retScan == 1){
                printf("Command format error.\n");
                continue;
            }
            sprintf(ToSend,"%s %s",command,fileName);
            send(localSocket , ToSend , strlen(ToSend) , 0 );
            //printf("ToSend==%s\n",ToSend);
            FILE *file = fopen(fileName, "rb");
            if(file == NULL){
                printf("The ‘%s’ doesn’t exist\n",fileName);
                continue;
            }
            int flagServerGet = 1;
            while(!feof(file)){
                int NumItems = fread(ToSend,sizeof(char),BUFF_SIZE,file);
                if(NumItems == 0){
                    break;
                }
                else{
                    if(flagServerGet == 1){
                        int NumSend = send(localSocket , ToSend , NumItems*sizeof(char) , 0 );
                        printf("Send items %d\n",NumSend);
                        flagServerGet = 0;
                        //flagServerGet = 1;
                    }
                    //sleep(0.1);
                    int Count = read(localSocket,receiveMessage,BUFF_SIZE);
                    receiveMessage[Count] = '\0';
                    printf("rece==%s\n",receiveMessage);
                    if(strcmp(receiveMessage,Server_Get)==0){
                        flagServerGet = 1;
                    }
                }
            }
            sprintf(ToSend,"%s",MyEOF);
            printf("is ToSend EOF %s\n",ToSend);
            send(localSocket , ToSend , EOFnum , 0 );
            //char buffer[1024];
            //fread the file into buffer
            //read(localSocket,receiveMessage,BUFF_SIZE);
        }
        else if(strcmp(command,"get")==0){
            //scanf("%s",fileName);
            if(retScan == 1){
                printf("Command format error.\n");
                continue;
            }
            sprintf(ToSend,"%s %s",command,fileName);
            send(localSocket , ToSend , strlen(ToSend) , 0 );

            FILE *file;
            read(localSocket,receiveMessage,BUFF_SIZE);
            send(localSocket , Client_Get , strlen(Client_Get) , 0 );
            if(strncmp(receiveMessage,FileFound,strlen(FileFound))==0){
                file = fopen(fileName, "wb");
            }
            else if(strncmp(receiveMessage,FileNotFound,strlen(FileNotFound))==0){
                printf("The ‘%s’ doesn’t exist.\n",fileName);
                continue;
            }

            int count;
            while((count = read(localSocket,receiveMessage,BUFF_SIZE))>0){
                if(strncmp(receiveMessage,MyEOF,EOFnum)==0){
                    send(localSocket , Client_Get , strlen(Client_Get) , 0 );
                    break;
                }
                else{
                    send(localSocket , Client_Get , strlen(Client_Get) , 0 );
                    printf("read count==%d\n",count);
                    fwrite(receiveMessage,sizeof(char),count,file);
                }
            }
            printf("get break\n");
            fclose(file);
        }
        else if(strcmp(command,"play")==0){
            //scanf("%s",fileName);
            if(retScan == 1){
                printf("Command format error.\n");
                continue;
            } 
            if(strncmp(&fileName[strlen(fileName)-4],".mpg",4) != 0){
                printf("The ‘%s’ is not a mpg file.\n",fileName);
                continue;
            }
            sprintf(ToSend,"%s %s",command,fileName);
            send(localSocket , ToSend , strlen(ToSend) , 0 );

            read(localSocket,receiveMessage,BUFF_SIZE);
            send(localSocket , Client_Get , strlen(Client_Get) , 0 );
            if(strncmp(receiveMessage,FileNotFound,strlen(FileNotFound))==0){
                printf("The ‘%s’ doesn’t exist.\n",fileName);
                continue;
            }
            
            int32_t ret;
            char *data = (char*)&ret;
            read(localSocket,data,sizeof(ret));
            send(localSocket , Client_Get , strlen(Client_Get) , 0 );
            int width = ntohl(ret);

            read(localSocket,data,sizeof(ret));
            send(localSocket , Client_Get , strlen(Client_Get) , 0 );
            int height = ntohl(ret);
            
            printf("width==%d height==%d\n",width,height);
            
            //int height = 540;
            //int width = 960;
            
            Mat imgClient;
            imgClient = Mat::zeros(height, width, CV_8UC3);
            if(!imgClient.isContinuous()){
                imgClient = imgClient.clone();
            }
            
            int imgSize = height*width*3;
            uchar VideoImagebuffer[imgSize];
            uchar *iptr = imgClient.data;
            uchar EndVideo[] = MyEOF;

            while(recv(localSocket,VideoImagebuffer,imgSize,MSG_WAITALL)>0){
                if(memcmp(VideoImagebuffer,EndVideo,EOFnum)==0){
                    printf("server tell client empty frame\n");
                    send(localSocket , StopVideo , strlen(StopVideo) , 0 );
                    break;
                }
                else{
                    memcpy(iptr,VideoImagebuffer,imgSize);
                    imshow("Video", imgClient);
                    //Press ESC on keyboard to exit
                    // notice: this part is necessary due to openCV's design.
                    // waitKey means a delay to get the next frame.
                    char c = (char)waitKey(33.3333);
                    if(c==27){
                        printf("Client Stop Video %s\n",StopVideo);
                        send(localSocket , StopVideo , strlen(StopVideo) , 0 );
                        break;
                    }
                    else{
                        send(localSocket , Client_Get , strlen(Client_Get) , 0 );
                    }
                }
            }
            destroyWindow("Video");
            for(int i=0;i<5;i++){
                waitKey(1);
            }
        }
        else{
            printf("Command not found.\n");
        }
    }
    printf("close Socket\n");
    close(localSocket);
    return 0;
}
