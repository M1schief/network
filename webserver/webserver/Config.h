#pragma once
#include <string>
#include <winsock2.h>

using namespace std;

//保存配置信息
class Config
{
public:
	static const string IP;//服务器端口
	static const int PORT;//服务器端口
	static const u_long BLOCKMODE;//非阻塞
	static const int MAXCONNECT;//最大连接数
	static const int BUFLEN;//缓冲区大小
private:
	Config(void);
	~Config();
};
