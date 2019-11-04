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
	
#define TRUE 1 
#define FALSE 0 
#define PORT 8888 

#define MaxResponse 1024
#define max_clients 30

void handle(int arg)
{
    printf("12345\n");
    //return;
}

int main(int argc , char *argv[]) 
{   
    signal(SIGPIPE, handle);//避免client ctrl+c 時
	int opt = TRUE; 
	int master_socket , addrlen , new_socket , client_socket[max_clients] , 
		activity, i , valread , sd; 
	int max_sd; 
	struct sockaddr_in address; 
		
	char buffer[1025]; //data buffer of 1K 

	int isClientGetTheLast[max_clients] = {0};
		
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
				
			if (FD_ISSET( sd , &readfds)) 
			{ 
				//Check if it was for closing , and also read the 
				//incoming message 
				if ((valread = read( sd , buffer, 1024)) == 0) 
				{ 
					//Somebody disconnected , get his details and print 
					getpeername(sd , (struct sockaddr*)&address ,(socklen_t*)&addrlen); 
					printf("Host disconnected , ip %s , port %d \n" , 
						inet_ntoa(address.sin_addr) , ntohs(address.sin_port)); 
						
					//Close the socket and mark as 0 in list for reuse 
					close( sd ); 
					client_socket[i] = 0;
					isClientGetTheLast[i] = 0;
				} 
					
				//Echo back the message that came in 
				else
				{ 
					//set the string terminating NULL byte on the end 
					//of the data read 
					buffer[valread] = '\0';
                    char response[MaxResponse];
					//小心GetIt還沒讀走command就進來了
					if(strcmp(buffer,"GetIt")==0){
						printf("server know\n");
						isClientGetTheLast[i] = 1;
					}
                    if(strcmp(buffer,"ls")==0){
                        struct dirent **entry_list;
						int FileNumber = scandir(".", &entry_list, 0, alphasort);
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
                    }
                    if(strcmp(buffer,"put")==0){
            
                    }
                    if(strcmp(buffer,"get")==0){
                        
                    }
                    if(strcmp(buffer,"play")==0){
                        
                    }
				} 
			}
		} 
	} 
		
	return 0; 
} 
