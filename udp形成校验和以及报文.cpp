/*
用于rdt3.0
这里将udp报文定义为[端口号16位/序号1位/报文段转为二进制的长度16位/校验和16位/报文段n位]
*/
#include <iostream>
#include <cstring>
#include <sstream> 
#include <algorithm>
#include <cstdio>
using namespace std;
//整数转string 
string to_string(unsigned long num){
	stringstream ss;
	string str;
	ss<<num;
	ss>>str;
	return str;
}
string int2bin(int num,int bit_num){//整型转二进制
	string s="";
	while(num>0){
		s+=to_string(num%2);
		num/=2;
	}
	reverse(s.begin(), s.end());
	int n=s.size();
	//补0
	for(int i=0;i<bit_num-n;++i){
		s="0"+s;
	}
	return s;
}
string str2bin(string s){
	string r="";
	for(int i=0;i<s.size();++i){
		unsigned char c=s[i];
		r+=int2bin((int)c,8);//把char转为8位二进制数asc
	}
	return r;
}
int bin2dec(string b){//16位二进制数转为10进制
	int n=b.size();
	int d=0;
	for(int i=0;i<n;++i){
		d=d*2+b[i]-'0';
	}
	//cout<<d<<endl;
	return d;
}
struct udp_message{
	int server_port;//端口号
	int seq_num;//序号
	int length;//报文段二进制长度
	int check_sum;//校验和
	string message;//报文段
	udp_message(){}
	udp_message(int server_port,int seq_num,int length,int check_sum,string message):
		server_port(server_port),
		seq_num(seq_num),
		length(length),
		check_sum(check_sum),
		message(message){}
	void print(){
		printf("server_port:%d\nseq_num:%d\nlength:%d\ncheck_sum:%d\nmessage:%s\n",
			server_port,seq_num,length,check_sum,message.c_str());
	}
	//真正的报文，二进制串
	string real_message(){
		string s_p=int2bin(server_port,16);
		string s_n=to_string(seq_num);
		string l=int2bin(length,16);
		string c_s=int2bin(check_sum,16);
		string m=str2bin(message);
		string need0="";

		string result=s_p+s_n+l+c_s+m;
		int need_zero=16-result.size()%16;
		for(int i=0;i<need_zero;++i){
			need0+='0';
		}
		result+=need0;

		c_s=checksum(result);
		result.replace(33,16,c_s);
		cout<<result<<endl;
		return result;
	}
	//计算校验和，每16位转为10进制，然后求和取反
	string checksum(string s){
		unsigned long sum=0;
		int n=s.size();
		for(int i=0;i<n;i+=16){
			sum+=bin2dec(s.substr(i,16));
			sum=(sum>>16)+(sum&0xffff);
			//后16位加上自己的进位部分，相当于回卷
			//用二进制举例，因为和最大为111+111=1110=1111-1，所以加上进位后肯定不会再进位了 
		}
		string sum_bin=int2bin(sum,16);//将校验和转为二进制
		//按位取反
		string result="";
		for(int i=0;i<16;++i){
			if(sum_bin[i]=='0'){
				result+="1";
			}
			else{
				result+="0";
			}
		}
		return result;
	} 
};
//解析udp报文，返回报文段
string analyse(string s){
	int server_port=bin2dec(s.substr(0,16));
	int seq_num=bin2dec(s.substr(16,1));
	int length=bin2dec(s.substr(17,16));
	int check_sum=bin2dec(s.substr(33,16));
	string message_asc=s.substr(49,length);
	string message="";
	for(int i=0;i<length;i+=8){
		message+=bin2dec(message_asc.substr(i,8));
	}
	udp_message m=udp_message(server_port,seq_num,length,check_sum,message);
	m.print();
	return message;
}
int main(){
	string text="abc";
	udp_message mes=udp_message(6666,1,text.size()*8,0,text);
	string s=mes.real_message();
	analyse(s);
}
