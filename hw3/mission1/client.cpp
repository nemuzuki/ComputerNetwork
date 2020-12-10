#include"tool.h"
#include<Winsock2.h>
#include<time.h> 
#include<vector>
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666

SOCKET localSocket;
struct sockaddr_in serverAddr,clientAddr;//接收端的ip和端口号信息 
DWORD WINAPI handlerRequest(LPVOID lpParam);
DWORD WINAPI handlerTimer(LPVOID lpParam);

bool begin_recv=false;//可以开始接收 
bool waiting=false; //等待发送 
string sendbuf="";
string temp;

int seq_num=1;//序号
clock_t start;
bool begin_timer=false;
 
//发送报文
void send_to(string sendbuf){
	udp_message message=udp_message(SERVER_PORT,seq_num,sendbuf.size()*8,0,sendbuf);//打包成udp 
	string real_message=message.real_message();
	const char *buffer=real_message.c_str();
	//发送数据
	sendto(localSocket,buffer,real_message.size(),0,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));
	//开始计时
	start=clock(); 
	begin_timer=true;
}


//接收 
string recv_from(){
	char recvbuf[200];
	int size=sizeof(serverAddr);
	recvfrom(localSocket,recvbuf,200,0,(SOCKADDR*)&serverAddr,&size);
	//cout<<"服务器端口号："<<ntohs(serverAddr.sin_port); 
	string s=recvbuf;
	return s;
}
vector<string>v;
//发送文件
void send_file(string path){
	FILE *fin=fopen(path.c_str(),"rb");
	char buffer[bufferSize];
	
	while(!feof(fin))
	{
		fread(buffer,1,sizeof(buffer),fin);
		cout<<"start sending..."<<endl;
		sendto(localSocket,buffer,sizeof(buffer),0,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));
		memset(buffer,0,sizeof(buffer));
	}
	fclose(fin);
} 

void send_test(){
   	send_file("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\1.jpg");
//    send_file("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\2.jpg");
//    send_file("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\3.jpg");
//    send_file("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\helloworld.txt");
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

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(6665);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	
	HANDLE hThread=CreateThread(NULL, 0, handlerRequest,LPVOID(), 0,NULL);
	HANDLE hThread2=CreateThread(NULL, 0, handlerTimer,LPVOID(), 0,NULL);
	
	
	cout<<">>>";
	cin>>sendbuf;
	send_test();
	
//	while(1) {
//		cin>>sendbuf;
//		Sleep(10);//等待waiting更新 
//		//当现在不在停等状态时，允许发送报文 
//		if(waiting==false){
//			seq_num=!seq_num; //新的报文，序号变化 
//			send_to(sendbuf);//发送
//						
//			temp=sendbuf;//缓存，便于重传 
//			waiting=true;
//		}
//		begin_recv=true;//***可以开始接收服务器端消息啦***
//		if(sendbuf=="quit"){
//			begin_recv=false;
//			break;
//		}
//	}
	//CloseHandle(hThread);
	closesocket(localSocket);
	WSACleanup();
}

//负责接收的线程 
DWORD WINAPI handlerRequest(LPVOID lpParam){
	while(1){
		Sleep(10);//改变begin_recv后，需要等一下再接收，不然会丢掉第一个ACK 
		string s=recv_from();
		if(begin_recv==false)continue;//还不能接收 
		
		begin_timer=false;//收到ack，停止计时 
		
		int server_port=bin2dec(s.substr(0,16));
		int seq_num_2=bin2dec(s.substr(16,16));
		int length=bin2dec(s.substr(32,16));
		int check_sum=bin2dec(s.substr(48,16));
		string message_asc=s.substr(64,length);
		string message="";
		for(int i=0;i<length;i+=8){
			message+=bin2dec(message_asc.substr(i,8));
		}
		udp_message m=udp_message(server_port,seq_num_2,length,check_sum,message);
		m.print();

		int sum=0;
		for(int i=0;i<s.size();i+=16){
			sum+=bin2dec(s.substr(i,16));
			sum=(sum>>16)+(sum&0xffff);
		}
		if(sum==0xffff&&seq_num_2==seq_num){//校验和正确、序号正确，可以发送下一条
			waiting=false;
			cout<<">>>";
		}
		else{//继续等待正确ACK，若超时重传
			waiting=true; 
		}
	} 
}
DWORD WINAPI handlerTimer(LPVOID lpParam){
	double dt;
    clock_t end;
    while(1){
    	end = clock();
    	dt = (double)(end - start);
    	if(begin_timer==true && dt==300 ){
		//如果超过300毫秒，这里必须是==，否则会出现dt>1000的异常情况 
			cout<<"timeout="<<dt<<",resend"<<endl; 
			begin_timer=false;
			send_to(temp);//超时重传
		}
	}
}
