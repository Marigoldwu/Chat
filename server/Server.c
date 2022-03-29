#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "main.h"
#define SERVER_PORT 6666
#define SIZE 1024
#define SIZE_SHMADD 2048
#define BACKLOG 20
#define FILE_NAME_MAX_SIZE 512
int sockfd;
int fd[BACKLOG];
int i=0;
/*********套接字描述符*******/
int get_sockfd()
{    
	struct sockaddr_in server_addr; 
	if((sockfd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
	    fprintf(stderr,"创建套接字错误:%s\n\a",strerror(errno));         
		exit(1);
	}
	else
	{
        printf("创建套接字成功!\n");
	}     
	
	/*sockaddr结构 */ 
	bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;         
	server_addr.sin_port=htons(SERVER_PORT);        
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY); 
    
    int opt = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    /*绑定服务器的ip和服务器端口号*/
    if(bind(sockfd,(struct sockaddr *)(&server_addr),sizeof(server_addr))==-1)
    {      
		fprintf(stderr,"端口绑定错误:%s\n\a",strerror(errno));       
        exit(1);     
    } 
	else
	{
		printf("端口绑定成功!\n");    
	}  
     /* 设置允许连接的最大客户端数 */     
    if(listen(sockfd,BACKLOG)==-1)     
    {    
	 	fprintf(stderr,"监听端口错误:%s\n\a",strerror(errno)); 
		exit(1);  
	} 
	else
	{
        printf("端口监听中......\n"); 
	} 
    return sockfd;
}

int main() 
{   
	//	char shmadd_buffer[SIZE_SHMADD],buffer[SIZE];
	//调用socket函数返回的文件描述符 
	int serverSocket;
	//声明两个套接字sockaddr_in结构体变量，分别表示客户端和服务器
	struct sockaddr_in server_addr;
	struct sockaddr_in client_addr;  
	int addr_len = sizeof(client_addr);
	int client;
	char sendbuf[SIZE];
	char recvbuf[SIZE];
	int iDataNum;
	//创建套接字serverSocket 
    serverSocket = get_sockfd();
    char mode[10]; 
    printf("监听端口: %d\n", SERVER_PORT);
    /*循环接收客户端*/
    while(1)
	{
		
		//调用accept函数后，会进入阻塞状态
		//accept返回一个套接字的文件描述符，这样服务器端便有两个套接字的文件描述符，
		//serverSocket和client。
		//serverSocket仍然继续在监听状态，client则负责接收和发送数据
		//client_addr是一个传出参数，accept返回时，传出客户端的地址和端口号
		//addr_len是一个传入-传出参数，传入的是调用者提供的缓冲区的client_addr的长度，以避免缓冲区溢出。
		//传出的是客户端地址结构体的实际长度。
		//出错返回-1
 		
		client = accept(serverSocket, (struct sockaddr*)&client_addr, (socklen_t*)&addr_len);
		if(client < 0)
		{
			perror("accept");
			continue;
		}
		printf("已连接用户： %s：%d\n", inet_ntoa(client_addr.sin_addr),htons(client_addr.sin_port));
		//printf("Port is %d\n", htons(client_addr.sin_port));
		printf("/******************************开始聊天******************************/\n");
		printf("等待对方发送消息......\n");
		//inet_ntoa ip地址转换函数，将网络字节序IP转换为点分十进制IP
		//表达式：char *inet_ntoa (struct in_addr);
//		printf(!flag++);
		while(1)
		{
           		//父进程用于接收信息/
           		mode[0] = '\0';
				iDataNum = recv(client, mode, SIZE, 0);
				if(iDataNum < 0)
				{
					perror("接收缓冲区为空");
					continue;
				}
				mode[iDataNum] = '\0';
				if(strcmp(mode, "quit") == 0)
				{
					bzero(recvbuf, SIZE);
					printf("客户端已断开连接！");
					break;
				}
				else if(mode[0] == 'F')
				{
					// recv函数接收数据到缓冲区buffer中
        			bzero(recvbuf, SIZE);
        			if(recv(client, recvbuf, SIZE, 0) < 0)
        			{
            			perror("数据接收失败:");
            			break;
        			}
 
        			// 然后从recvbuf(缓冲区)拷贝到file_name中
        			char file_name[FILE_NAME_MAX_SIZE+1];
        			bzero(file_name, FILE_NAME_MAX_SIZE+1);
        			strncpy(file_name, recvbuf, strlen(recvbuf)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(recvbuf));
        		
        			//接收文件内容 
        			while(1){
        				int res = receiveFile(recvbuf, file_name, client);
        				if(res==0)break;
					}; 
        			
        			//发送
					while(1)
					{
						int res = sendC(mode, sendbuf, client); 
						if(res == 1)break;
						else if(res == 2)continue;
						else break;
					}
				} 
				else if(mode[0] == 'C')
				{
					char their_name[10]="客户端";
					bzero(recvbuf, SIZE);
					//接收消息 
					receiveMsg(recvbuf, their_name, client);
				
					//发送
					while(1)
					{
						int res = sendC(mode, sendbuf, client); 
						if(res == 1)break;
						else if(res == 2)continue;
						else break;
					}
				} 
				else
				{
					bzero(recvbuf, SIZE);
					continue;
				}
		}		
	}
	close(client);
	close(serverSocket);
	return 0;
}
