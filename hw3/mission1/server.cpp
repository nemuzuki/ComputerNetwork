#include"tool.h"
#include<Winsock2.h> 
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666

SOCKET localSocket; 
//发送的ip和端口号信息 
struct sockaddr_in clientAddr,serverAddr;


int send_to(string s){
	const char *c=s.c_str();
	//发送数据到地址serverAddr
	return sendto(localSocket,c,200,0,(SOCKADDR*)&clientAddr,sizeof(SOCKADDR));
}
//解析报文 
string analyse(string recvbuf){
	int n=recvbuf.length();
	char seq_num=recvbuf[0];//第一位是序号
	
	//获得校验和 
	int i=2;
	for(;i<n;++i){
		if(recvbuf[i]=='$'){
			break;
		}
	}
	string checksum_seg=get_substr(recvbuf,i+1,n-1);//校验和字段 
	cout<<"校验和字段："<<checksum_seg<<" ";
	string checksum_result=to_string(checksum(get_substr(recvbuf,2,i-1)));
	cout<<"计算出的校验和："<<checksum_result<<' ';
	//如果正确，发送[正确序列号$ACK] 
	if(checksum_seg==checksum_result){
		cout<<"校验和正确"<<endl; 
		if(seq_num=='0')send_to("0$ACK"); 
		else send_to("1$ACK"); 
	}
	//错误则发送[错误序列号$ACK]  
	else{
		cout<<"校验和错误"<<endl; 
		if(seq_num=='0')send_to("1$ACK"); 
		else send_to("0$ACK"); 
	}
	//返回报文段 
	return get_substr(recvbuf,2,i-1);
}

string recv_from(){
	char recvbuf[200];
	int size=sizeof(clientAddr);
	recvfrom(localSocket,recvbuf,200,0,(SOCKADDR*)&clientAddr,&size);
	string s=recvbuf;
	return analyse(s);
}

//接收文件，并保存到path
void recv_file(string path){
	//string recv_s=recv_from();
	char buffer[bufferSize];
	//strcpy(buffer,recv_s.c_str());
	int size=sizeof(clientAddr);
	recvfrom(localSocket,buffer,sizeof(buffer),0,(SOCKADDR*)&clientAddr,&size);
	cout<<"get!"<<endl;
	
	FILE *fout=fopen(path.c_str(),"wb");
	fwrite(buffer,1,sizeof(buffer),fout);
	fclose(fout);
} 

//接收图片 或者需要分块的文件 
void recv_pic(string path){
	char buffer[bufferSize]; 
	
	FILE *fout=fopen(path.c_str(),"wb");
	while(1)
	{
		int size=sizeof(clientAddr);
		recvfrom(localSocket,buffer,sizeof(buffer),0,(SOCKADDR*)&clientAddr,&size);
		//cout<<"get!"<<endl;
		fwrite(buffer,1,sizeof(buffer),fout);
		memset(buffer,0,sizeof(buffer));
	} 
	cout<<"transport picture finish!"<<endl;
	fclose(fout);
}
int main(){
    WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,1); 
	WSAStartup(wVersionRequested, &wsaData);
	
	//本地socket，只负责接收 
    localSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    //服务器的ip和端口号 
    struct sockaddr_in clientAddr,serverAddr;
    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(6666);//htons把unsigned short类型从主机序转换到网络序
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	//绑定本地socket和端口号 
	bind(localSocket,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));
	cout<<"本地端口："<<ntohs(serverAddr.sin_port)<<endl;//ntohs把unsigned short类型从网络序转换到主机序
	int recv_status=0;
	
//	recv_pic("C:/Users/Mika/Desktop/计算机网络/大作业3/接收到的文件/helloworld.txt");
//	recv_pic("C:/Users/Mika/Desktop/计算机网络/大作业3/接收到的文件/1.jpg");
//	recv_pic("C:/Users/Mika/Desktop/计算机网络/大作业3/接收到的文件/2.jpg");
	recv_pic("C:/Users/Mika/Desktop/计算机网络/大作业3/接收到的文件/3.jpg");
//	while(1) {
//		string recvbuf=recv_from();
//		cout<<recvbuf<<endl;
//	}
	closesocket(localSocket);
	WSACleanup();
}
