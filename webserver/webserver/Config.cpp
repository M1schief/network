#include "Config.h"
#include <string>

using namespace std;

Config::Config(void)
{
}

Config::~Config()
{
}
const string IP = "127.0.0.1";
const int Config::PORT = 1111;//端口1111
const u_long Config::BLOCKMODE = 1;//非阻塞模式
const int Config::MAXCONNECT = 10;//最大连接数10
const int Config::BUFLEN = 1024;//缓冲区大小1024