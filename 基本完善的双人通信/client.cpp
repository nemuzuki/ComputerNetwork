#include<Winsock2.h> 
#include<stdio.h>
#include<string.h>
#include<iostream>
#pragma comment(lib,"ws2_32")
using namespace std;
#define SERVER_PORT 6666
#define server_ip 127.0.0.1
//10.130.102.15

DWORD WINAPI handlerRequest(LPVOID lpParam);
void connect_server();

string start,endx="server",sendbuf;
SOCKET serverSocket;
bool connect_flag=0;//这个flag专门用于接下来应该输入对方姓名时 

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

int main(){
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2,1); 
    WSAStartup(wVersionRequested, &wsaData);

    serverSocket=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    //去和6666端口服务器连接
    while(connect(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr))){
        Sleep(1000);
    }
    Sleep(10);
    //创建线程，专门用来接收
    HANDLE hThread=CreateThread(NULL, 0, handlerRequest,LPVOID(serverSocket), 0, NULL);
    Sleep(10);//等连上服务器再进行下面操作

    printf("请输入昵称：");
    cin>>start;
    send_to(serverSocket,start);
    Sleep(10);
    
    connect_flag=1;

    while(1){
        if(connect_flag==1){
            printf("与谁建立连接：\n");
            cin>>sendbuf;
            endx=sendbuf; 
            if(connect_flag==1){
                sendbuf="con"+sendbuf; 
                connect_flag=0;
            }
        }
        //要发送的信息 
        else{
            getline(cin,sendbuf);//读入一整行，包括空格 
            if(sendbuf.size()==0)continue;
            else if(connect_flag==1){
                sendbuf="con"+sendbuf; 
                connect_flag=0;
            }
        }
        if(sendbuf=="conquit"){
            send_to(serverSocket,"quit");
            break;
        }
        //正常发送 
        send_to(serverSocket,sendbuf);
        if(sendbuf=="quit")break;
    }
    closesocket(serverSocket);
    WSACleanup();
}

DWORD WINAPI handlerRequest(LPVOID lpParam){
    int len;
    string recvbuf;
    SOCKET serverSocket=(SOCKET)lpParam;
    
    while(1){
        recvbuf=recv_from(serverSocket);
        //收到对方下线消息后 
        if(recvbuf=="quit"){
            cout<<endx<<"下线"<<endl; 
            Sleep(10);
            recvbuf=recv_from(serverSocket);//接收当前人数 
            cout<<recvbuf<<endl;
            connect_flag=1;
            endx="server"; 
            printf("与谁建立连接：\n");
        }
        else if(recvbuf=="该用户未上线"){
            cout<<recvbuf<<endl;
            connect_flag=1;
        } 
        //被邀请聊天 
        else if(recvbuf.substr(0,6)=="invite"){
            connect_flag=0;//直接进入聊天 
            int n=recvbuf.length();
            int i;
            for(i=7;i<n;++i){
                if(recvbuf[i]=='$')break;
            }
            endx=recvbuf.substr(7,i-7);
            cout<<recvbuf.substr(7,n-7)<<endl; 
        }
        //正常输出信息 
        else{
            cout<<endx<<":"<<recvbuf<<endl;
        } 
    }
}
