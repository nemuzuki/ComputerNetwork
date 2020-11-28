#include"tool.h"
#include<Winsock2.h>
#include<fstream>
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666

SOCKET localSocket;
struct sockaddr_in serverAddr,clientAddr;//接收端的ip和端口号信息 
DWORD WINAPI handlerRequest(LPVOID lpParam);


bool begin_recv=false;//可以开始接收 
bool waiting=false; //等待发送 
string sendbuf="";
string temp;

int seq_num=0;//序号
 
//发送报文 ，包含[序号$信息$校验和] 
void send_to(string sendbuf){
	//校验和 
	unsigned long checksum_seg=checksum(sendbuf);
	sendbuf=sendbuf+"$"+to_string(checksum_seg);
	
	//序号
	if(seq_num==0){
		sendbuf="0$"+sendbuf;
	} 
	else{
		sendbuf="1$"+sendbuf;
	}
	
	//发送数据
	send_to(sendbuf);
}

string recv_from(){
	char recvbuf[200];
	int size=sizeof(serverAddr);
	recvfrom(localSocket,recvbuf,200,0,(SOCKADDR*)&serverAddr,&size);
	//cout<<"服务器端口号："<<ntohs(serverAddr.sin_port); 
	string s=recvbuf;
	return s;
}

//发送文件，先把文件转换为字符串，放入buffer 
void send_file(string path){
	char buffer[bufferSize];
	FILE *fin=fopen(path.c_str(),"rb");
	fread(buffer,1,sizeof(buffer),fin);
	cout<<"start sending..."<<endl;
	cout<<buffer<<endl;
	fclose(fin);
//	string s=buffer;
//	send_to(s);
	sendto(localSocket,buffer,sizeof(buffer),0,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));
}

//发送图片 
void send_pic(string path){
	FILE *fin=fopen(path.c_str(),"rb");
	char buffer[bufferSize];
	
	while(!feof(fin))
	{
		fread(buffer,1,sizeof(buffer),fin);
		cout<<"start sending..."<<endl;
		//cout<<buffer<<endl;
		sendto(localSocket,buffer,sizeof(buffer),0,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));
		memset(buffer,0,sizeof(buffer));
	}
	fclose(fin);
} 
 
int main(){
	//初始化
    WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,1); 
	WSAStartup(wVersionRequested, &wsaData);

	//去连接服务器的socket
    localSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    
    //服务器的ip和端口号 
    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(6666);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	HANDLE hThread=CreateThread(NULL, 0, handlerRequest,LPVOID(), 0,NULL);
	int send_status=0;
	cout<<">>>";
	cin>>sendbuf;
	
//	send_pic("C:/Users/Mika/Desktop/计算机网络/大作业3/任务1测试文件/helloworld.txt");
//	send_pic("C:/Users/Mika/Desktop/计算机网络/大作业3/任务1测试文件/1.jpg");
//	send_pic("C:/Users/Mika/Desktop/计算机网络/大作业3/任务1测试文件/2.jpg");
	send_pic("C:/Users/Mika/Desktop/计算机网络/大作业3/任务1测试文件/3.jpg");
	Sleep(10000);
	
//	while(1) {
//		cin>>sendbuf;
//		Sleep(10);//等待waiting更新 
//		//允许发送 
//		if(waiting==false){
//			seq_num=!seq_num; 
//			send_to(sendbuf);
//			temp=sendbuf;//缓存，便于重传 
//			waiting=true;
//		}
//		begin_recv=true;//可以开始接收服务器端消息啦
//		if(sendbuf=="quit"){
//			begin_recv=false;
//			break;
//		}
//	}
	CloseHandle(hThread);
	closesocket(localSocket);
	WSACleanup();
}

//负责接收的线程 
DWORD WINAPI handlerRequest(LPVOID lpParam){
	while(1){
		Sleep(10);//改变begin_recv后，需要等一下再接收，不然会丢掉第一个ACK 
		string recvbuf=recv_from();
		if(begin_recv==true){//可以开始接收了 
			cout<<recvbuf<<endl;
			//如果收到ACK的序号是刚刚发送的序号 
			if(recvbuf[0]==seq_num+'0'){
				waiting=false;
				cout<<">>>";
			} 
			//否则重传
			else{
				waiting=true; 
				sendbuf=temp;
				send_to(sendbuf);
			}
		}
	} 
}
