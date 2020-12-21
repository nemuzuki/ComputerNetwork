#include"tool.h"
#include<Winsock2.h>
#include<vector>
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666
#define CLIENT_PORT 6665
#define WINDOW_SIZE 8	//窗口大小

SOCKET localSocket;
struct sockaddr_in serverAddr,clientAddr;//接收端的ip和端口号信息 
DWORD WINAPI handlerRequest(LPVOID lpParam);

bool begin_recv=false;//可以开始接收 
bool waiting=false; //等待发送 
string temp;

int seq_num=0;//序号，就是nxtseqnum


//发送报文
void send_to(char *message){
	udp_message u=udp_message(SERVER_PORT,seq_num,1024-10,0,message);//打包成udp 
	//打包后的报文msg
	char msg[bufferSize];
	u.real_message(msg);
	//发送数据
	sendto(localSocket,msg,sizeof(msg),0,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));//大小很重要
}


//接收 
void recv_from(char *message){
	char msg[bufferSize];
	int size=sizeof(serverAddr);
	recvfrom(localSocket,msg,sizeof(msg),0,(SOCKADDR*)&serverAddr,&size);
	analyse(msg,message);
}


//发送文件，无校验和
 void send_file(string path){
 	FILE *fin=fopen(path.c_str(),"rb");
 	char buffer[4096];
 	while(!feof(fin))
 	{
 		fread(buffer,1,sizeof(buffer),fin);
 		cout<<strlen(buffer)<<endl;
 		sendto(localSocket,buffer,sizeof(buffer),0,(SOCKADDR*)&serverAddr,sizeof(SOCKADDR));
 		memset(buffer,0,sizeof(buffer));
 	}
 	cout<<"finish"<<endl;
 	fclose(fin);
 } 


double dt;
clock_t start,end;
double overtime=100;//超时时间 
//把文件分块存储在v中
vector<char*>v;
char block[2048][1024];


int base=0;
void send_file_2(string path){
	//先保存到数组里
	FILE *fin=fopen(path.c_str(),"rb");
	int i=0;
	while(!feof(fin)){
		fread(block[i],1,sizeof(block[i])-10,fin);
		v.push_back(block[i++]);//加入数组
	}
	fclose(fin);
	
	char *message;
	int n=v.size();
	while(seq_num<n)
	{
		end=clock();
		dt = (double)(end - start);
		//如果超时，未确认的全部重传
		if(dt>=overtime){
			cout<<"timeout="<<dt<<endl; 
			seq_num=base; 
			
		}
		//当现在有空间，且不在停等状态时，允许发送报文 
		if(seq_num<base+WINDOW_SIZE)
		{
			message=v[seq_num];
			send_to(message);
			start=clock(); 
			begin_recv=true;//***可以开始接收服务器端消息啦***
			cout<<seq_num<<' '<<strlen(message)<<endl;
			seq_num++; //新的报文，序号变化
			if(base==seq_num){//当base=nxt时，开始计时，这相当于初始条件
				start=clock(); 
			}
		}
	}
	cout<<"finish"<<endl;
} 
double space=1859130;
void send_test(){
 	send_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\1.jpg");
 //   send_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\2.jpg");
  //  send_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\3.jpg");
 //  send_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\helloworld.txt");
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

	clientAddr.sin_family = AF_INET;
	clientAddr.sin_port = htons(CLIENT_PORT);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	bind(localSocket,(SOCKADDR*)&clientAddr,sizeof(SOCKADDR));
	cout<<"本地端口："<<ntohs(clientAddr.sin_port)<<endl;//ntohs把unsigned short类型从网络序转换到主机序
	HANDLE hThread=CreateThread(NULL, 0, handlerRequest,LPVOID(), 0,NULL);

	cout<<">>>";
	string sendbuf;
	cin>>sendbuf;
	
	clock_t all_start=clock();
	send_test();
	clock_t all_end=clock();
	double all_pass=(double)(all_end - all_start);
	cout<<"传输时间："<<all_pass<<"ms"<<endl; 
	cout<<"吞吐率："<<space/all_pass<<"KB/s"<<endl;

	
	Sleep(10000);//一定要等一会，否则线程立即关掉，没法打印结果 
	//CloseHandle(hThread);
	closesocket(localSocket);
	WSACleanup();
}

//负责接收的线程 
DWORD WINAPI handlerRequest(LPVOID lpParam){
	while(1){
		Sleep(10);//改变begin_recv后，需要等一下再接收，不然会丢掉第一个ACK 
		char msg[bufferSize],message[bufferSize];
		int size=sizeof(serverAddr);
		if(begin_recv==false){
			continue;
		}
		recvfrom(localSocket,msg,sizeof(msg),0,(SOCKADDR*)&serverAddr,&size);

		if(analyse(msg,message)){
			cout<<"ACK"<<GET_WHOLE(msg[2],msg[3])<<endl; 
			base=GET_WHOLE(msg[2],msg[3])+1;
			//当收到一个ack，则下一个包开始计时
			start=clock();
		}
	} 
}
