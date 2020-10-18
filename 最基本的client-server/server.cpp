#include<Winsock2.h> 
#include<stdio.h>
#include<string.h>
#include<iostream>
#pragma comment(lib,"ws2_32")
#define SERVER_PORT 6666
using namespace std;
int main(){
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,1); 
	WSAStartup(wVersionRequested, &wsaData);
	
	SOCKET sockSrv=socket(AF_INET,SOCK_STREAM,0);//创建服务器端socket
	struct sockaddr_in server_addr;//服务器端地址
	
	char buffer[200];
	int addrlen=sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);//服务器端口号定为6666
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sockSrv,(SOCKADDR*)&server_addr,sizeof(SOCKADDR));//套接字绑定服务器端地址
	listen(sockSrv,5);//队列的最大长度为5
	
	while(1){
		printf("监听端口：%d\n",SERVER_PORT);
		struct sockaddr_in addrClient;//客户端地址
		SOCKET client = accept(sockSrv, (SOCKADDR*)&addrClient, &addrlen);//创建新套接字连接客户
		cout<<"当前客户的端口号："<<addrClient.sin_port<<endl;//输出当前客户的端口号
		while(1){
			//buffer[0]='\0';//每次都要初始化
			int len=recv(client, buffer, 50, 0);
			buffer[len]='\0';//后面的不要了
			if(strcmp(buffer,"quit")==0)break;
			printf("接收到消息：%s",buffer);
			printf("\n");	

			printf("发送消息：");
			scanf("%s",buffer);
			send(client, buffer, strlen(buffer), 0);
			if(strcmp(buffer,"quit")==0)break;
		}
	}
	closesocket(sockSrv);
	WSACleanup();

}
//g++ server.cpp -o server.exe -lwsock32
