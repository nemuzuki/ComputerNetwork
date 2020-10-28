#include<Winsock2.h> 
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<string>
#include<map>
#include<vector>
#include<list>
#pragma comment(lib,"ws2_32")
using namespace std;
#define server_ip 127.0.0.1
#define SERVER_PORT 6666

DWORD WINAPI handlerRequest(LPVOID client);

map<string,string>name_name;//一张映射表，发信->收信
map<string,SOCKET>name_socket;//存储名字和socket
typedef list<string>LIST_STRING;//建立链表存在线用户名
LIST_STRING users;
LIST_STRING::iterator user;


int send_to(SOCKET socket,string s){
	const char *c=s.c_str();
	return send(socket,c,200,0);
}

string recv_from(SOCKET socket){
	char c[200];
	recv(socket,c,200,0);
	string s=c;
	return s;
}

string show_online_users(){
	string sendbuf="当前在线用户：";
	for(user=users.begin();user!=users.end();++user){
		sendbuf+=*user+' ';
	}
	return sendbuf;
}

bool user_is_online(string name){
	for(user=users.begin();user!=users.end();++user){
		if(*user==name){
			break;
		}
	}
	if(user==users.end())return false;
	return true;
}
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
		SOCKET client = accept(sockSrv, (SOCKADDR*)&addrClient, &addrlen);//监听到了用户，创建一个socket
		cout<<"当前客户的端口号："<<addrClient.sin_port<<endl;//输出当前客户的端口号

		
		HANDLE hThread=CreateThread(NULL, 0, handlerRequest,LPVOID(client), 0,NULL);//给每个用户创建一个线程，lpvoid传的是指针
		CloseHandle(hThread);
	}
	closesocket(sockSrv);
	WSACleanup();

}
DWORD WINAPI handlerRequest(LPVOID lpParam){
	SOCKET client=(SOCKET)lpParam;
	
	string sendbuf="welcome";
	send_to(client,sendbuf);
			

	//接收用户名
	string start=recv_from(client);
	cout<<start<<"上线"<<endl;
	name_socket[start]=client;
	users.push_back(start);
	sendbuf=show_online_users();
	//向所有人发送当前人数
	for(user=users.begin();user!=users.end();++user){
		send_to(name_socket[*user],sendbuf);
	}
	
	//接收聊天对象
	string endx;
	
	while(1){
		string buffer=recv_from(client);//收到要转发的信息
		if(buffer.substr(0,3)=="con"){
			endx=buffer.substr(3,buffer.size()-3);
			name_name[start]=endx;
			name_name[endx]=start;//反向也强制连接 
			cout<<start<<"申请和"<<endx<<"连接"<<endl;
			if(!user_is_online(endx)){
				send_to(client,"该用户未上线");
			}
			else{ 
				send_to(client,"服务器收到建立连接申请");
				send_to(name_socket[endx],"invite$"+start+"$邀请你聊天");
			}
			continue;
		}
		cout<<start<<"->"<<name_name[start]<<":"<<buffer<<endl;
		//正常转发信息 
		send_to(name_socket[name_name[start]],buffer);
		//如果其中一个用户退出了，要让另一个用户重新选择用户聊天
		if(buffer=="quit"){
			users.remove(start);
			cout<<start<<"已下线"<<endl;
			string sendbuf=show_online_users();
			for(user=users.begin();user!=users.end();++user){
				send_to(name_socket[*user],sendbuf);
			}
			return 0;
		}
	}
	
	return 0;
}
