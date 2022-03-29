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
#define SIZE 1024
#define FILE_NAME_MAX_SIZE 512
#define SERVER_PORT 6666
#define LENGTH_OF_LISTEN_QUEUE 20
#define IP "127.0.0.1"
//发送消息 
void sendMsg(char sendbuf[], int socket)
{
	printf("你:");
	scanf("%s", sendbuf);
	if(send(socket, sendbuf, strlen(sendbuf), 0) < 0)
	{
		printf("消息发送失败！\n");
	}
}
//发送文件 
int sendFile(char sendbuf[], char file_name[], int socket)
{
	// 打开文件并读取文件数据
   	FILE *fp = fopen(file_name, "r");
    if(NULL == fp)
    {
        printf("%s 文件不存在！\n", file_name);
    }
    else
    {
        bzero(sendbuf, SIZE);
        int length = 0;
        // 每读取一段数据，便将其发送给服务器，循环直到文件读完为止
        while((length = fread(sendbuf, sizeof(char), SIZE, fp)) > 0)
        {
            if(send(socket, sendbuf, length, 0) < 0)
            {
            	printf("%s 发送失败\n", file_name);
                break;
            }
            bzero(sendbuf, SIZE);
        }
//        send(socket, "", 0, 0);
        // 关闭文件
        fclose(fp);
        //close(socket);
        printf("%s 发送成功!\n", file_name);
        int iDataNum;
        char recvbuf[SIZE];
        bzero(recvbuf,SIZE);
		recvbuf[0] = '\0';
		if((iDataNum = recv(socket, recvbuf, SIZE, 0)) < 0)
		{
			printf("数据接收失败！\n");
		}
		recvbuf[iDataNum] = '\0';
		if(strcmp(recvbuf, "success")==0)
		{
			printf("对方已接收文件！\n");
		}
    }
    return 0;
}
//接收消息 
void receiveMsg(char recvbuf[], char their_name[], int socket)
{
	int iDataNum;
	printf("%s:", their_name);
	recvbuf[0] = '\0';
	if((iDataNum = recv(socket, recvbuf, SIZE, 0)) < 0)
	{
		printf("数据接收失败！\n");
	}
	recvbuf[iDataNum] = '\0';
	printf("%s\n", recvbuf);
} 
//接收文件 
int receiveFile(char recvbuf[], char file_name[], int socket)
{
	// 打开文件，准备写入
    FILE *fp = fopen(file_name, "w");
    if(NULL == fp)
    {
        printf("无法写入文件%s.\n", file_name);
        exit(1);
    }
 
    // 从服务器接收数据到recvbuf中
    // 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止
    bzero(recvbuf, SIZE);
    int length = 0;
		while(1)
		{
			length = recv(socket, recvbuf, SIZE, 0);
			if(length > 0){
				if(fwrite(recvbuf, sizeof(char), length, fp) < length)
        		{
            		printf("写入文件%s失败\n", file_name);
            		break;
        		}
        		bzero(recvbuf, SIZE);
			}
			break;
		}	

			fclose(fp);
			printf("成功接收文件：%s! 文件存储在默认文件夹下（程序所在文件夹）.\n", file_name);
			char sendbuf[SIZE]="success";
			send(socket, sendbuf, strlen(sendbuf), 0);
			return 0;
}
int sendC(char mode[], char sendbuf[], int socket)
{
	//选择模式 
	printf("输入C发送消息，输入F发送文件：");
	scanf("%s", mode);
	//告诉服务器接下来是什么内容 
	send(socket, mode, strlen(mode), 0);
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
    	if(send(socket, sendbuf, SIZE, 0) < 0)
    	{
        	perror("文件名发送失败:");
        	exit(1);
    	}
    		
    	//发送文件内容
		if(sendFile(sendbuf, file_name, socket))return 1;
	}
	//发送消息 
	else if(mode[0] == 'C') 
	{
		bzero(mode, 10);
		//输入并发送消息 
		sendMsg(sendbuf, socket);
		//输入quit断开连接 
		if(strcmp(sendbuf, "quit") == 0)return 1;
	}
	else
	{
		bzero(mode, 10);
		printf("暂时还未开通此功能！敬请期待！\n");
		return 2; 
	}
	return 0;
}
//接收 用在发送完之后 
int receive(char mode[], char recvbuf[], char their_name[], int socket)
{ 
	//判断接收消息类型 
 	mode[0] = '\0';
	int iDataNum = recv(socket, mode, 1024, 0);
	if(iDataNum < 0)
	{
		perror("啥也没有！\n");
		return 1;
	}
	mode[iDataNum] = '\0';
	//断开连接 
	if(strcmp(mode, "quit") == 0)
	{
		printf("客户端已断开连接！");
		return 1;
	}
	//接收文件 
	else if(mode[0] == 'F')
	{
		// recv函数接收文件名到缓冲区recvbuf中
        bzero(recvbuf, SIZE);
        if(recv(socket, recvbuf, SIZE, 0) < 0)
        {
            perror("数据接收失败:");
            return 1;
        }
 
        // 然后从recvbuf(缓冲区)拷贝到file_name中
        char file_name[FILE_NAME_MAX_SIZE+1];
        bzero(file_name, FILE_NAME_MAX_SIZE+1);
        strncpy(file_name, recvbuf, strlen(recvbuf)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(recvbuf));
		//接收文件内容 
		return receiveFile(recvbuf, file_name, socket);
	}
	//接收消息 
	else if(mode[0] == 'C')
	{
		//接收消息 
		receiveMsg(recvbuf, their_name, socket);
	}
	else
	{
		bzero(mode, 10);
		return 2;
	}
	return 0;
}
