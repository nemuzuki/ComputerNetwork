#include<stdio.h>
#include<string.h>
#include<iostream> 
#include<algorithm>
#include<time.h>
#include<cstdlib>
using namespace std;

#define GET_HIGH(number) ((int)number&0xFF00)>>8
#define GET_LOW(number) ((int)number&0x00FF)
#define GET_WHOLE(h,l) ((((int)h<<8)&0xff00)|((int)l&0xff))
const int bufferSize=4096;

struct udp_message{
	int server_port;//端口号
	int seq_num;//序号
	int length;//报文段二进制长度
	int check_sum;//校验和
	char *message;//报文段
	udp_message(){}
	udp_message(int server_port,int seq_num,int length,int check_sum,char *message):
		server_port(server_port),
		seq_num(seq_num),
		length(length),
		check_sum(check_sum),
		message(message){}
	void print(){
		printf("server_port:%d\nseq_num:%d\nlength:%d\ncheck_sum:%d\nmessage:%s\n",
			server_port,seq_num,length,check_sum,message);
	}
	//真正的报文，二进制串，保存到msg中 
	void real_message(char *msg){
		//存的不是每一位，而是把8位作为一个字节，存到char的一个单位里！
		msg[0]=GET_HIGH(server_port);
		msg[1]=GET_LOW(server_port);
		msg[2]=GET_HIGH(seq_num);
		msg[3]=GET_LOW(seq_num);
		msg[4]=GET_HIGH(length);
		msg[5]=GET_LOW(length);
		msg[6]=0;
		msg[7]=0;
		for(int i=0;i<length;++i){
			msg[8+i]=message[i];
		}

		int a=checksum(msg);
		msg[6]=GET_HIGH(a);
		msg[7]=GET_LOW(a);
	}
	//计算校验和，每16位转为10进制，然后求和取反
	int checksum(char *msg){
		unsigned long sum=0;
		for(int i=0;i<8+length;i+=2){//这里不能用strlen(msg)，因为如果中间有0，读到就不计算了
			sum+=GET_WHOLE(msg[i],msg[i+1]);
			sum=(sum>>16)+(sum&0xffff);
			//后16位加上自己的进位部分，相当于回卷
			//用二进制举例，因为和最大为111+111=1110=1111-1，所以加上进位后肯定不会再进位了 
		}
		return (~sum)&0xffff;
	} 
};

//解析udp报文msg，返回报文段，写入message
bool analyse(char *msg,char *message){
	int server_port=GET_WHOLE(msg[0],msg[1]);
	int seq_num=GET_WHOLE(msg[2],msg[3]);
	int length=GET_WHOLE(msg[4],msg[5]);
	int check_sum=GET_WHOLE(msg[6],msg[7]);
	for(int i=0;i<length;i++){
		message[i]=msg[8+i];
	}
//	udp_message m=udp_message(server_port,seq_num,length,check_sum,message);
//	m.print();

	int sum=0;
	for(int i=0;i<length+8;i+=2){
		sum+=GET_WHOLE(msg[i],msg[i+1]);
		sum=(sum>>16)+(sum&0xffff);
	}
	if(sum==0xffff){//校验和正确，回复ACK
		cout<<"yes"<<endl;
		return true;
	}
	else{
		cout<<"no"<<endl;
		return false;
	}
}

bool random_loss(){
	srand((int)time(0));
	int r=rand()%100;
	if(r==99){
		return false;
	}
	return true;
}
