#include "Config.h"
#include <string>

using namespace std;

Config::Config(void)
{
}

Config::~Config(void)
{
}

const string Config::SERADDR = "127.0.0.1";	//������IP��ַ
const int Config::MAXCONNECT = 10;				//���������5
const int Config::BUFLEN = 1024;				//��������С256�ֽ�
const int Config::PORT = 1111;						//�������˿�1111
const u_long Config::BLOCKMODE = 1;					//SOCKETΪ������ģʽ
