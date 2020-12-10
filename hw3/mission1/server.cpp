#include"tool.h"
#include<Winsock2.h> 
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666
#define CLIENT_PORT 6665
SOCKET localSocket; 
//发送的ip和端口号信息 
struct sockaddr_in clientAddr,serverAddr;


int send_to(string s){
	const char *c=s.c_str();
	//发送数据到地址serverAddr
	return sendto(localSocket,c,200,0,(SOCKADDR*)&clientAddr,sizeof(SOCKADDR));
}
//解析udp报文，返回报文段
string analyse(string s){
	int server_port=bin2dec(s.substr(0,16));
	int seq_num=bin2dec(s.substr(16,16));
	int length=bin2dec(s.substr(32,16));
	int check_sum=bin2dec(s.substr(48,16));
	string message_asc=s.substr(64,length);
	string message="";
	for(int i=0;i<length;i+=8){
		message+=bin2dec(message_asc.substr(i,8));
	}
	udp_message m=udp_message(server_port,seq_num,length,check_sum,message);
	m.print();

	int sum=0;
	for(int i=0;i<s.size();i+=16){
		sum+=bin2dec(s.substr(i,16));
		sum=(sum>>16)+(sum&0xffff);
	}
	//Sleep(1000);
	if(sum==0xffff){//校验和正确，回复ACK
		cout<<"校验和正确"<<endl;
		udp_message m=udp_message(CLIENT_PORT,seq_num,24,0,"ACK");
		string message=m.real_message();
		send_to(message);
	}
	else{
		cout<<"校验和错误"<<sum<<endl;
		udp_message m=udp_message(CLIENT_PORT,1-seq_num,24,0,"ACK");
		string message=m.real_message();
		send_to(message);
	}
	return message;
}

string recv_from(){
	char recvbuf[200];
	int size=sizeof(clientAddr);
	recvfrom(localSocket,recvbuf,200,0,(SOCKADDR*)&clientAddr,&size);
	string s=recvbuf;
	return analyse(s);
}

void recv_file(string path){
	char buffer[bufferSize]; 
	FILE *fout=fopen(path.c_str(),"wb");
	while(1)
	{
		int size=sizeof(clientAddr);
		recvfrom(localSocket,buffer,sizeof(buffer),0,(SOCKADDR*)&clientAddr,&size);
		fwrite(buffer,1,sizeof(buffer),fout);
		memset(buffer,0,sizeof(buffer));
	}
	fclose(fout);
}

void recv_test(){
	recv_file("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\接收到的文件\\1.jpg");
//    recv_file("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\接收到的文件\\2.jpg");
//    recv_file("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\接收到的文件\\3.jpg");
//    recv_file("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\接收到的文件\\helloworld.txt");
}

int main(){
    WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,1); 
	WSAStartup(wVersionRequested, &wsaData);
	
	//本地socket，只负责接收 
    localSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    //服务器的ip和端口号 
    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(6666);//htons把unsigned short类型从主机序转换到网络序
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	//绑定本地socket和端口号 
	bind(localSocket,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));
	cout<<"本地端口："<<ntohs(serverAddr.sin_port)<<endl;//ntohs把unsigned short类型从网络序转换到主机序
	
//	while(1) {
//		string recvbuf=recv_from();
//		cout<<recvbuf<<endl;
//	}
	recv_test();
	closesocket(localSocket);
	WSACleanup();
}
