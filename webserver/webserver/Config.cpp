#include "Config.h"
#include <string>

using namespace std;

Config::Config(void)
{
}

Config::~Config(void)
{
}

const string Config::SERADDR = "127.0.0.1";	//服务器IP地址
const int Config::MAXCONNECT = 10;				//最大连接数5
const int Config::BUFLEN = 1024;				//缓冲区大小256字节
const int Config::PORT = 1111;						//服务器端口1111
const u_long Config::BLOCKMODE = 1;					//SOCKET为非阻塞模式
