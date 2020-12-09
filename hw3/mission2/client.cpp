#include"tool.h"
#include<Winsock2.h>
#include<time.h> 
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666
#define CLIENT_PORT 6665
#define WINDOW_SIZE 4

SOCKET localSocket;
struct sockaddr_in serverAddr,clientAddr;//接收端的ip和端口号信息 
DWORD WINAPI handlerRequest(LPVOID lpParam);
DWORD WINAPI handlerTimer(LPVOID lpParam);

bool begin_recv=false;//可以开始接收 
bool waiting=false; //等待发送 
bool begin_timer=false;//启动计时器
string sendbuf="";
string temp;

int seq_num=0;//序号
clock_t start;

string send_queue[20];
int base=0;

int nxtseqnum=0;

//发送报文
void send_to(string sendbuf){
	udp_message message=udp_message(SERVER_PORT,seq_num,sendbuf.size()*8,0,sendbuf);//打包成udp 
	string real_message=message.real_message();
	const char *buffer=real_message.c_str();
	//发送数据
	sendto(localSocket,buffer,real_message.size()*8,0,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));
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

int main(){
	//初始化
    WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2,1); 
	WSAStartup(wVersionRequested, &wsaData);

	//去连接服务器的socket
    localSocket=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    
    //服务器的ip和端口号 
    serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//用户端也设定
	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(CLIENT_PORT);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//绑定本地socket和端口号 
	bind(localSocket,(SOCKADDR*)&clientAddr,sizeof(SOCKADDR));

	HANDLE hThread=CreateThread(NULL, 0, handlerRequest,LPVOID(), 0,NULL);
	HANDLE hThread2=CreateThread(NULL, 0, handlerTimer,LPVOID(), 0,NULL);
	
	for(int i=0;i<20;++i){
		send_queue[i]=i+'a';
	} 
	cout<<">>>";
	cin>>sendbuf;
	Sleep(10);//等待waiting更新
	begin_recv=true;//***可以开始接收服务器端消息啦***
	
	while(seq_num<20){
		Sleep(200);
		if(nxtseqnum<base+WINDOW_SIZE && waiting==false){
			cout<<base<<' '<<nxtseqnum<<' '<<seq_num<<endl;
			cout<<send_queue[nxtseqnum]<<endl;
			send_to(send_queue[nxtseqnum]);//只要窗口有空，立即发送
			seq_num++;
			if(base==nxtseqnum){//最早待确认的就是刚发的
				begin_timer=true;
			}
			nxtseqnum++;
		}
		else{
			Sleep(1000);
		}
	}
	
	Sleep(10000);//一定要等一会，否则线程立即关掉，没法打印结果 
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
		cout<<endl;
		
		int sum=0;
		for(int i=0;i<s.size();i+=16){
			sum+=bin2dec(s.substr(i,16));
			sum=(sum>>16)+(sum&0xffff);
		}
		if(sum==0xffff){//校验和正确
			base=seq_num_2+1;//窗口滑动
			if(base==nxtseqnum){//######全部发完，停止计时
				begin_timer==false;
			}
			else{//#######如果base<nxt，窗口滑动，可以发送新的分组，启动计时
				waiting=false;
				begin_timer==true;
			}
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
			waiting=true;
			for(int i=base;i<nxtseqnum;++i){//#######超时重传所有未确认的
				send_to(send_queue[i]);
			}
		}
	}
}
