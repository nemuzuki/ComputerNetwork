#include"tool.h"
#include<Winsock2.h> 
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666
#define CLIENT_PORT 6665
SOCKET localSocket; 
//发送的ip和端口号信息 
struct sockaddr_in clientAddr,serverAddr;

//%1的概率比特错误或者丢包 
bool random_loss(){
	int r=rand()%100;
	cout<<r<<endl;
	if(r>98){
		return false;
	}
	return true;
}
//发送报文
void send_to(char *message,int seq_num){
	udp_message u=udp_message(SERVER_PORT,seq_num,strlen(message),0,message);//打包成udp 
	//打包后的报文msg
	char msg[bufferSize];
	u.real_message(msg);
	sendto(localSocket,msg,sizeof(msg),0,(SOCKADDR*)&clientAddr,sizeof(SOCKADDR));
}


bool recv_from(){
	char msg[bufferSize],message[bufferSize];
	int size=sizeof(clientAddr);
	recvfrom(localSocket,msg,sizeof(msg),0,(SOCKADDR*)&clientAddr,&size);
	return analyse(msg,message);
}
//无校验和接收文件
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
	cout<<"transport picture finish!"<<endl;
	fclose(fout);
}

int expected_seqnum=0;
void recv_file_2(string path){
	char msg[bufferSize],message[bufferSize];
	FILE *fout=fopen(path.c_str(),"wb");
	while(1)
	{
		int size=sizeof(clientAddr);
		recvfrom(localSocket,msg,sizeof(msg),0,(SOCKADDR*)&clientAddr,&size);
		bool error_msg=random_loss();
		if(error_msg==false)msg[1]/=2; 
		
		bool check=analyse(msg,message);
		
		//一定概率丢掉ACK包 
		bool lost_ack=random_loss();
		if(lost_ack==false)continue; 
		int recv_seqnum=GET_WHOLE(msg[2],msg[3]);
		//如果校验和正确，并且收到的序号是想要的
		if(check==true && expected_seqnum==recv_seqnum){
			cout<<"yes"<<endl;
			fwrite(message,1,GET_WHOLE(msg[4],msg[5]),fout);
			send_to("ACK",expected_seqnum);
			expected_seqnum++;//更新想要的序号 
		}
		else{
			//否则发送最近正确接收的序号
			cout<<"no"<<endl;
			send_to("ACK",expected_seqnum-1);
		}
		memset(message,0,sizeof(message));
	}
	fclose(fout);
}

void recv_test(){
	recv_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\接收到的文件\\1.jpg");
//    recv_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\接收到的文件\\2.jpg");
 //   recv_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\接收到的文件\\3.jpg");
 //   recv_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\接收到的文件\\helloworld.txt");
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
	

	srand(time(0));
	recv_test();
	closesocket(localSocket);
	WSACleanup();
}
