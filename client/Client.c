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
#define SIZE 1024
#define FILE_NAME_MAX_SIZE 512
#define SERVER_PORT 6666
#define LENGTH_OF_LISTEN_QUEUE 20
#define IP "127.0.0.1"

int main()
{
	/*客户端描述符*/ 
    int clientSocket;
    struct sockaddr_in server_addr;
	/*发送缓冲区、接收缓冲区*/
    char sendbuf[SIZE];
	char recvbuf[SIZE];
	int  iDataNum;
	char mode[10];
       /*客户程序开始建立 clientSocket描述符 */ 
    if((clientSocket=socket(AF_INET,SOCK_STREAM,0))==-1) 
    { 
        fprintf(stderr,"创建套接字错误:%s\a\n",strerror(errno)); 
        exit(1); 
    } 
	else
	{
        printf("创建套接字成功!\n");
    }
    
    /*客户程序填充服务端的资料 */ 
    server_addr.sin_family=AF_INET;          // IPV4
    server_addr.sin_port=htons(SERVER_PORT);  // (将本机器上的short数据转化为网络上的short数据)端口号
    server_addr.sin_addr.s_addr=inet_addr(IP); // IP地址
    /* 客户程序发起连接请求 */ 
    if(connect(clientSocket,(struct sockaddr *)(&server_addr),sizeof(server_addr))==-1) 
    { 
        fprintf(stderr,"连接失败:%s\a\n",strerror(errno)); 
        exit(1); 
    }
	else
	{
        printf("连接到主机...\n");
    }
    printf("/******************************开始聊天******************************/\n");
    while(1)
	{
			//选择模式 
			printf("输入C发送消息，输入F发送文件：");
			scanf("%s", mode);
			//告诉服务器接下来是什么内容 
			send(clientSocket, mode, strlen(mode), 0);
			mode[strlen(mode)]='\0';
			if(mode[0] == 'F')
			{
				bzero(mode, 10);
				// 输入文件名
    			char file_name[FILE_NAME_MAX_SIZE+1];
    			bzero(file_name, FILE_NAME_MAX_SIZE+1);
    			printf("请输入要发送的文件名:\t");
    			scanf("%s", file_name);

 				//告诉服务器发送的文件名 
    			bzero(sendbuf, SIZE);
    			strncpy(sendbuf, file_name, strlen(file_name)>SIZE?SIZE:strlen(file_name));
    	
    			// 向服务器发送sendbuf中的数据
    			if(send(clientSocket, sendbuf, SIZE, 0) < 0)
    			{
        			perror("文件名发送失败:");
        			exit(1);
    			}
    		
    			//发送文件内容
				if(sendFile(sendbuf, file_name, clientSocket))break;
				
        		//接收服务器发来的内容 
        		char their_name[10]="服务器";
        		while(1)
        		{
        			int res = receive(mode, recvbuf, their_name, clientSocket); 
        			if(res == 1)break;
        			else if(res == 2)continue;
        			else break;
				}
			}
			//发送消息 
			else if(mode[0] == 'C') 
			{
				bzero(mode, 10);
				//输入并发送消息 
				sendMsg(sendbuf, clientSocket);
				//输入quit断开连接 
				if(strcmp(sendbuf, "quit") == 0)break;
 				//接收服务器发来的内容 
 				char their_name[10]="服务器";
 				while(1)
 				{
 					int res = receive(mode, recvbuf, their_name, clientSocket); 
        			if(res == 1)break;
        			else if(res == 2)continue;
        			else break;
				}
			}
			else
			{
				bzero(mode, 10);
				printf("暂时还未开通此功能！敬请期待！\n");
				continue; 
			}
	}
	close(clientSocket);
	return 0;
}

