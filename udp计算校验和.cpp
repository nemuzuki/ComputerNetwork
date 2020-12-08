/*
用于rdt3.0
这里将udp报文定义为[端口号16位/序号1位/报文段转为二进制的长度16位/校验和16位/报文段n位]
*/
#include <iostream>
#include <cstring>
#include <sstream> 
#include <algorithm>
using namespace std;
//整数转string 
string to_string(unsigned long num){//整数转string
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
string str2bin(string s){//字符串中的字符转为asc码，8位拼接
	string r="";
	for(int i=0;i<s.size();++i){
		unsigned char c=s[i];
		r+=int2bin((int)c,8);//把char转为8位二进制数asc
	}
	return r;
}
int bin_dec(string b){//16位二进制数转为10进制
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
	udp_message(int sp,int sn,int l,int cs,string m){
		server_port=sp;
		seq_num=sn;
		length=l;
		check_sum=cs;
		message=m;
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
			sum+=bin_dec(s.substr(i,16));
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
int main(){
	string text="abc";//要发送的信息
	udp_message mes=udp_message(6666,1,text.size()*8,0,text);
	mes.real_message();
}
