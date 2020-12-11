#include"tool.h"
#include<Winsock2.h>
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666
#define CLIENT_PORT 6665
SOCKET localSocket;
struct sockaddr_in serverAddr,clientAddr;//接收端的ip和端口号信息 
DWORD WINAPI handlerRequest(LPVOID lpParam);

bool begin_recv=false;//可以开始接收 
bool waiting=false; //等待发送 
string temp;

int seq_num=1;//序号


//发送报文
void send_to(char *message){
	udp_message u=udp_message(SERVER_PORT,seq_num,bufferSize-10,0,message);//打包成udp 
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


//发送文件
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


bool begin_timer=false;
double dt;
clock_t start,end;
double overtime=1000;//超时时间 
//先把文件打包存储在v中，先设置小一点
void send_file_2(string path){
	FILE *fin=fopen(path.c_str(),"rb");
	char message[4096];
	while(!feof(fin))
	{
		end=clock();
		dt = (double)(end - start);
		if(waiting==true&&dt>=overtime){
			cout<<"timeout="<<dt<<endl; 
			send_to(message);//超时重传
			cout<<seq_num<<' '<<strlen(message)<<endl;
			start=clock(); 
			continue;
		}
		//当现在不在停等状态时，允许发送报文 
		if(waiting==false)
		{
			fread(message,1,sizeof(message)-10,fin);
			
			seq_num=1-seq_num; //新的报文，序号变化
			 
			send_to(message);
			cout<<seq_num<<' '<<strlen(message)<<endl;
			start=clock();  
			waiting=true;
		}
		begin_recv=true;//***可以开始接收服务器端消息啦***
	}
	cout<<"finish"<<endl;
	fclose(fin);
} 
double space=1859130;
void send_test(){
 	send_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\1.jpg");
 //   send_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\2.jpg");
  //  send_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\3.jpg");
  // send_file_2("C:\\Users\\Mika\\Desktop\\计算机网络\\大作业3\\任务1测试文件\\helloworld.txt");
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

	cout<<">>>";
	string sendbuf;
	cin>>sendbuf;	
	
	clock_t all_start=clock();
	send_test();
	clock_t all_end=clock();
	double all_pass=(double)(all_end - all_start);
	cout<<"传输时间："<<all_pass<<"ms"<<endl; 
	cout<<"吞吐率："<<space/all_pass<<"KB/s"<<endl;
	Sleep(10000);//接收最后的ACK 
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
		
//		udp_message m=udp_message(server_port,seq_num_2,length,check_sum,message);
//		m.print();
		if(GET_WHOLE(msg[2],msg[3])==seq_num){//校验和正确、序号正确，可以发送下一条
			waiting=false;
			cout<<">>>";
		}
		else{//继续等待正确ACK，若超时重传
			waiting=true; 
		}
	} 
}


