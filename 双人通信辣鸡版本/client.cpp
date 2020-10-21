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

    char sendbuf[200],recvbuf[200];
    int serverSocket=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//去和6666端口服务器连接
    connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
	

	int len;
    while(1)
	{
		printf("发送消息：");
		scanf("%s",sendbuf);
		send(serverSocket, sendbuf, strlen(sendbuf), 0); //向服务端发送消息
		if(strcmp(sendbuf, "quit") == 0) break;

		printf("接受到消息：");
		recvbuf[0]='\0';
		len=recv(serverSocket, recvbuf, 200, 0); //接收服务端发来的消息
		recvbuf[len]='\0';//后面的不要了
		printf("%s\n", recvbuf);
	}
    closesocket(serverSocket);
	WSACleanup();
}
