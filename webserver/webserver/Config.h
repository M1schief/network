#pragma once
#include <string>
#include <winsock2.h>

using namespace std;

//����������Ϣ
class Config
{
public:
	static const int MAXCONNECT;		//���������
	static const int BUFLEN;		//��������С
    static const string SERADDR;  //��������ַ
	static const int PORT;				//�������˿�
	static const u_long BLOCKMODE;			//SOCKET����ģʽ
private:
	Config(void);
	~Config(void);
};
