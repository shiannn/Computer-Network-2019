//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux 
#include <stdio.h> 
#include <string.h> //strlen 
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h> //close 
#include <arpa/inet.h> //close 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#include <signal.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;
	
#define TRUE 1 
#define FALSE 0 
//#define PORT 8888 

#define MaxResponse 1024
#define max_clients 30
#define Client_Get "clientGetIt"
#define Server_Get "serverGetIt"
#define StopVideo "stopvideo"
#define MaxFileName 1000
#define MaxCommand 10
#define MyEOF "@@@@@@"
#define EOFnum 6
#define FileNotFound "@@FileNotFound@@"
#define FileFound "@@FileFound@@"

void handle(int arg)
{
    printf("12345\n");
    //return;
}

int main(int argc , char *argv[]) 
{   
	char InputPort[10]={0};
    int PORT = -1;
    if(argc == 2){
        printf("%s\n",argv[1]);
        for(int InputIndex=0;argv[1][InputIndex]!='\0';InputIndex++){
            InputPort[InputIndex] = argv[1][InputIndex];
        }
        printf("Port==%s\n",InputPort);
        //InputPortNumber = atoi(InputPort);
		PORT = atoi(InputPort);
    }
    else{
        printf("wrong commandline parameter !\n");
        return 0;
    }

    signal(SIGPIPE, handle);//避免client ctrl+c 時
	mkdir("serverDir", S_IRWXU);
	chdir("serverDir");

	int opt = TRUE; 
	int master_socket , addrlen , new_socket , client_socket[max_clients] , 
		activity, i , valread , sd; 
	int max_sd; 
	struct sockaddr_in address; 
		
	char buffer[MaxResponse]; //data buffer of 1K 

	int isClientGetTheLast[max_clients] = {0};
	int ClientLs[max_clients] = {0};
	int ClientPut[max_clients] = {0};
	int ClientGet[max_clients] = {0};
	int ClientPlay[max_clients] = {0};
	FILE* ClientPutfp[max_clients]={0};
	FILE* ClientGetfp[max_clients]={0};
	struct dirent **entry_list[max_clients];
	int ClientLsEntry[max_clients] = {0};
	int ClientLsFileNumber[max_clients] = {0};
	char FileName[max_clients][MaxFileName];
	char bufferPut[max_clients][MaxResponse];
	char bufferGet[max_clients][MaxResponse];
	char bufferls[max_clients][MaxResponse];

	VideoCapture cap[max_clients];
	Mat imgServer[max_clients];
		
	//set of socket descriptors 
	fd_set readfds; 
	fd_set writefds;
		
	//a message 
	char *message = "ECHO Daemon v1.0 \r\n"; 
	
	//initialise all client_socket[] to 0 so not checked 
	for (i = 0; i < max_clients; i++) 
	{ 
		client_socket[i] = 0;
	} 
		
	//create a master socket 
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	//set master socket to allow multiple connections , 
	//this is just a good habit, it will work without this 
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, 
		sizeof(opt)) < 0 ) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	
	//type of socket created 
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
		
	//bind the socket to localhost port 8888 
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	printf("Listener on port %d \n", PORT); 
		
	//try to specify maximum of 3 pending connections for the master socket 
	if (listen(master_socket, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
		
	//accept the incoming connection 
	addrlen = sizeof(address); 
	puts("Waiting for connections ..."); 
		
	while(TRUE) 
	{ 
		//clear the socket set 
		FD_ZERO(&readfds);
		FD_ZERO(&writefds); 
	
		//add master socket to set 
		FD_SET(master_socket, &readfds);
		FD_SET(master_socket, &writefds); 
		max_sd = master_socket; 
			
		//add child sockets to set 
		for ( i = 0 ; i < max_clients ; i++) 
		{ 
			//socket descriptor 
			sd = client_socket[i]; 
				
			//if valid socket descriptor then add to read list 
			if(sd > 0) 
				FD_SET( sd , &readfds);
				FD_SET( sd , &writefds);
				
			//highest file descriptor number, need it for the select function 
			if(sd > max_sd) 
				max_sd = sd; 
		} 
	
		//wait for an activity on one of the sockets , timeout is NULL , 
		//so wait indefinitely 
		activity = select( max_sd + 1 , &readfds , &writefds , NULL , NULL); 
	
		if ((activity < 0) && (errno!=EINTR)) 
		{ 
			printf("select error"); 
		} 
			
		//If something happened on the master socket , 
		//then its an incoming connection 
		if (FD_ISSET(master_socket, &readfds)) 
		{ 
			if ((new_socket = accept(master_socket, 
					(struct sockaddr *)&address, (socklen_t*)&addrlen))<0) 
			{ 
				perror("accept"); 
				exit(EXIT_FAILURE); 
			} 
			
			//inform user of socket number - used in send and receive commands 
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" 
                , new_socket , inet_ntoa(address.sin_addr) , ntohs 
				(address.sin_port)); 
		
			//send new connection greeting message 
            /*
			if( send(new_socket, message, strlen(message), 0) != strlen(message) ) 
			{ 
				perror("send"); 
			} 
            */
				
			puts("Welcome message sent successfully"); 
				
			//add new socket to array of sockets 
			for (i = 0; i < max_clients; i++) 
			{ 
				//if position is empty 
				if( client_socket[i] == 0 ) 
				{ 	
					client_socket[i] = new_socket;
					isClientGetTheLast[i] = 1;
					printf("Adding to list of sockets as %d\n" , i); 
						
					break; 
				} 
			} 
		} 
			
		//else its some IO operation on some other socket 
		for (i = 0; i < max_clients; i++) 
		{ 
			sd = client_socket[i]; 
			//client傳來的東西可能被上面攔截
			if (FD_ISSET( sd , &readfds)) 
			{ 
				//Check if it was for closing , and also read the 
				//incoming message
				if ((valread = read( sd , buffer, MaxResponse)) == 0) 
				{ 
					//Somebody disconnected , get his details and print 
					getpeername(sd , (struct sockaddr*)&address ,(socklen_t*)&addrlen); 
					printf("Host disconnected , ip %s , port %d \n" , 
						inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
						
					//Close the socket and mark as 0 in list for reuse 
					int shutdownStatus = shutdown( sd , 2);
					int closeStatus = close( sd );
					printf("closed sd == %d closeStatus== %d shutdownStatus == %d\n",sd,closeStatus,shutdownStatus);
					//set it can't be written read
					FD_CLR( sd , &readfds);
					FD_CLR( sd , &writefds);

					client_socket[i] = 0;
					isClientGetTheLast[i] = 0;

					memset(FileName[i],'\0',MaxFileName);
					memset(bufferPut[i],'\0',MaxResponse); 
					memset(bufferGet[i],'\0',MaxResponse);
					memset(bufferls[i],'\0',MaxResponse);
					
					isClientGetTheLast[i] = 0;
					ClientPut[i] = 0;
					ClientGet[i] = 0;
					ClientPlay[i] = 0;
					ClientPutfp[i]= 0;
					ClientGetfp[i]= 0;

					ClientLs[i] = 0;
					entry_list[i] = NULL;
					ClientLsEntry[i] = 0;
					ClientLsFileNumber[i] = 0;
					

					cap[i].release();
					//imgServer[max_clients];
				} 
					
				//Echo back the message that came in 
				else
				{ 
					//set the string terminating NULL byte on the end 
					//of the data read

					//給予 client 服務
					//1.greeting message 2.command  3.data 4.Stop Video
					if(strncmp(buffer,StopVideo,strlen(StopVideo))==0){
						//stop video
						printf("server stop video\n");
						ClientPlay[i] = 0;
						cap[i].release();
						isClientGetTheLast[i] = 1;
					}
					else if(strncmp(buffer,Client_Get,strlen(Client_Get))==0){
						//greeting message
						//printf("server know\n");
						isClientGetTheLast[i] = 1;
					}
					else if(ClientPut[i]==1){
						//int count = read(sd,bufferPut[i],sizeof(bufferPut[i]));
						int count = valread;
						memcpy(bufferPut[i],buffer,count);
						if(strncmp(bufferPut[i],MyEOF,EOFnum)==0){
							printf("the client .jpg EOF\n");
							ClientPut[i] = 0;
							fclose(ClientPutfp[i]);
							continue;
						}
						send(sd , Server_Get , strlen(Server_Get) , 0 );
						printf("read count==%d\n",count);
						fwrite(bufferPut[i],sizeof(char),count,ClientPutfp[i]);
					}
					else{
						buffer[valread] = '\0';
                    	char response[MaxResponse];
						//如果client不是任何服務下 那讀到的就是command
						if(strncmp(buffer,"ls",2)==0){
							printf("after Getting\n");
							ClientLs[i] = 1;
							ClientLsFileNumber[i] = scandir(".", &entry_list[i], 0, alphasort);
							ClientLsEntry[i] = 0;
							/*
							int charNumber = 0;
							for(int i=0;i<FileNumber;i++){
								struct dirent *entry = entry_list[i];
								//printf("%s\n",entry->d_name);
								sprintf(&response[charNumber],"%s\n",entry->d_name);
								charNumber += strlen(entry->d_name)+1;
							}
							if(FD_ISSET( sd , &writefds) && isClientGetTheLast[i]==1){
								write(sd, response,strlen(response));
								isClientGetTheLast[i] = 0;
							}
							*/
						}
						if(strncmp(buffer,"put",3)==0){
							//client upload
							char commandDummy[MaxCommand];
							
							sscanf(buffer,"%s%s",commandDummy,FileName[i]);
							printf("commandDummu==%s FileName==%s\n",commandDummy,FileName[i]);
							ClientPut[i] = 1;
							ClientPutfp[i] = fopen(FileName[i], "wb");
						}
						if(strncmp(buffer,"get",3)==0){
							//client download
							char commandDummy[MaxCommand];
							sscanf(buffer,"%s%s",commandDummy,FileName[i]);
							printf("commandDummu==%s FileName==%s\n",commandDummy,FileName[i]);

							ClientGetfp[i] = fopen(FileName[i], "rb");
							if(ClientGetfp[i] == NULL){
								char NotFound[] = FileNotFound;
								write(sd, NotFound,strlen(NotFound));
								isClientGetTheLast[i] = 0;
							}
							else{
								char Found[] = FileFound;
								write(sd, Found,strlen(Found));
								isClientGetTheLast[i] = 0;
								ClientGet[i] = 1;	
							}
						}
						if(strncmp(buffer,"play",4)==0){
							//client play
							char commandDummy[MaxCommand];
							sscanf(buffer,"%s%s",commandDummy,FileName[i]);
							printf("commandDummu==%s FileName==%s\n",commandDummy,FileName[i]);

							//需要一個array for cap
							//VideoCapture cap(FileName);
							bool VideoOpen = cap[i].open(FileName[i]);
							printf("VideoOpen == %d\n",VideoOpen);
							if(VideoOpen == false){
								char NotFound[] = FileNotFound;
								write(sd, NotFound,strlen(NotFound));
								isClientGetTheLast[i] = 0;
							}
							else{
								char Found[] = FileFound;
								write(sd, Found,strlen(Found));
								int Count = read(sd,buffer,MaxResponse);
								buffer[0] = '\0';
								//isClientGetTheLast[i] = 0;
								ClientPlay[i] = 1;

								// get the resolution of the video
								int width = cap[i].get(CV_CAP_PROP_FRAME_WIDTH);
								int height = cap[i].get(CV_CAP_PROP_FRAME_HEIGHT);
								cout  << width << ", " << height << endl;

								imgServer[i] = Mat::zeros(height,width, CV_8UC3);

								//send width 和 height回去給client
								//send size first
								// get the size of a frame in bytes 
								//int imgSize = width * height * 3;
								int32_t conv = htonl(width);
								char *dataPtr = (char*)&conv;
								write(sd,dataPtr,sizeof(conv));
								Count = read(sd,buffer,MaxResponse);
								buffer[0] = '\0';

								conv = htonl(height);
								dataPtr = (char*)&conv;
								write(sd,dataPtr,sizeof(conv));
								Count = read(sd,buffer,MaxResponse);
								buffer[0] = '\0';
								cout << "Video disappear" << endl;
							}
						}
					}
				} 
			}
			if(FD_ISSET( sd , &writefds)){
				if(ClientLs[i]==1){
					if(isClientGetTheLast[i] == 1){
						if(ClientLsEntry[i] != ClientLsFileNumber[i]){
							struct dirent *entry = entry_list[i][ClientLsEntry[i]];
							sprintf(bufferls[i],"%s\n",entry->d_name);
							write(sd, bufferls[i],strlen(bufferls[i]));
							isClientGetTheLast[i] = 0;

							ClientLsEntry[i] += 1;
						}
						else{
							sprintf(bufferls[i],"%s",MyEOF);
							printf("is ToSend EOF %s\n",bufferls[i]);
							send(sd , bufferls[i] , EOFnum , 0 );
							ClientLs[i] = 0;
							ClientLsEntry[i] = 0;
							isClientGetTheLast[i] = 0;
						}
						/*
						int charNumber = 0;
						for(int i=ClientLsEntry[i];i<ClientLsFileNumber[i];i++){
							struct dirent *entry = entry_list[i];
							//printf("%s\n",entry->d_name);
							sprintf(&response[charNumber],"%s\n",entry->d_name);
							charNumber += strlen(entry->d_name)+1;
						}
						if(FD_ISSET( sd , &writefds) && isClientGetTheLast[i]==1){
							write(sd, response,strlen(response));
							isClientGetTheLast[i] = 0;
						}
						*/
					}
				}
				if(ClientGet[i]==1){
					if(isClientGetTheLast[i] == 1){
						if(!feof(ClientGetfp[i])){
							//give file content
							int NumItems = fread(bufferGet[i],sizeof(char),MaxResponse,ClientGetfp[i]);
							printf("fread items %d\n",NumItems);
							if(NumItems != 0){
								int NumSend = send(sd , bufferGet[i] , NumItems*sizeof(char) , 0 );
								printf("Send items %d\n",NumSend);
								isClientGetTheLast[i] = 0;
							}
						}
						else{
							//give EOF
							sprintf(bufferGet[i],"%s",MyEOF);
							printf("is ToSend EOF %s\n",bufferGet[i]);
							send(sd , bufferGet[i] , EOFnum , 0 );
							ClientGet[i] = 0;
							fclose(ClientGetfp[i]);
							isClientGetTheLast[i] = 0;
						}
					}
				}
				if(ClientPlay[i]==1){
					if(isClientGetTheLast[i] == 1){
						//Mat imgServer;
						// ensure the memory is continuous (for efficiency issue.)
						if(!imgServer[i].isContinuous()){
							imgServer[i] = imgServer[i].clone();
						}
						//allocate container to load frames

						//send VideoImage
						cap[i] >> imgServer[i];
						// get the size of a frame in bytes 
						int width = cap[i].get(CV_CAP_PROP_FRAME_WIDTH);
						int height = cap[i].get(CV_CAP_PROP_FRAME_HEIGHT);
						int imgSize = width*height*3;
						uchar VideoImagebuffer[imgSize];
						if(imgServer[i].empty()){
							printf("imgsize == %d\n",imgSize);
							memcpy(VideoImagebuffer,"@@@@@@", 6);
							send(sd , VideoImagebuffer , imgSize , 0 );
						}
						else{
							// allocate a buffer to load the frame (there would be 2 buffers in the world of the Internet)
							imgSize = imgServer[i].total() * imgServer[i].elemSize();
							// copy a frame to the buffer
							memcpy(VideoImagebuffer,imgServer[i].data, imgSize);
							//memcpy(VideoImagebuffer,"@@@@@", 5);
							int NumSend = send(sd , VideoImagebuffer , imgSize*sizeof(uchar) , MSG_WAITALL);	
						}
						isClientGetTheLast[i] = 0;
					}
				}
			}
		} 
	} 
		
	return 0; 
} 
