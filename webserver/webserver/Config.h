#pragma once
#include <string>
#include <winsock2.h>

using namespace std;

//����������Ϣ
class Config
{
public:
	static const string IP;//�������˿�
	static const int PORT;//�������˿�
	static const u_long BLOCKMODE;//������
	static const int MAXCONNECT;//���������
	static const int BUFLEN;//��������С
private:
	Config(void);
	~Config();
};
