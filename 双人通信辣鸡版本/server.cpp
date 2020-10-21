#include<Winsock2.h> 
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<string>
#include<map>
#pragma comment(lib,"ws2_32")
#define SERVER_PORT 6666
using namespace std;

DWORD WINAPI handlerRequest(LPVOID client);

int main(){
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,1); 
	WSAStartup(wVersionRequested, &wsaData);
	
	SOCKET sockSrv=socket(AF_INET,SOCK_STREAM,0);//创建服务器端socket
	struct sockaddr_in server_addr;//服务器端地址
	
	
	int addrlen=sizeof(server_addr);
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERVER_PORT);//服务器端口号定为6666
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	bind(sockSrv,(SOCKADDR*)&server_addr,sizeof(SOCKADDR));//套接字绑定服务器端地址
	listen(sockSrv,5);//队列的最大长度为5

	//服务器需要将各客户端的端口号存起来，和客户的名字形成一个映射，才能进行转发
	//所以先问客户叫什么名字
	int len;
	printf("监听端口：%d\n",SERVER_PORT);
	int i=0;
	while(1){
		struct sockaddr_in addrClient;//客户端地址
		SOCKET client = accept(sockSrv, (SOCKADDR*)&addrClient, &addrlen);//监听到了用户
		cout<<"当前客户的端口号："<<addrClient.sin_port<<endl;//输出当前客户的端口号

		socklist[i++]=client;
		HANDLE hThread=CreateThread(NULL, NULL, handlerRequest,LPVOID(client), 0, NULL);//创建线程,lpvoid传的是指针
		CloseHandle(hThread);

	}
	closesocket(sockSrv);
	WSACleanup();
}

DWORD WINAPI handlerRequest(LPVOID lpParam){
	int len;
	while(1){
		char buffer[200];
		SOCKET client=(SOCKET)lpParam;
		len=recv(client, buffer, 50, 0);//收到要转发的信息
		buffer[len]='\0';//后面的不要了
		if(strcmp(buffer,"quit")==0)break;
		
		for(int i=0;i<2;++i){
			if (socklist[i]!=client){
				send(socklist[i], buffer, strlen(buffer), 0);
			}
		}
		if(strcmp(buffer,"quit")==0)break;
	}
	return 0;
}
