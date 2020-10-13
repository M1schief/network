#pragma once
#include <string>
#include <winsock2.h>

using namespace std;

//保存配置信息
class Config
{
public:
	static const int MAXCONNECT;		//最大连接数
	static const int BUFLEN;		//缓冲区大小
    static const string SERADDR;  //服务器地址
	static const int PORT;				//服务器端口
	static const u_long BLOCKMODE;			//SOCKET阻塞模式
private:
	Config(void);
	~Config(void);
};
