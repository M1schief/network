#include "WinsockEnv.h"
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

#define VER1 2  //Winsock���汾��
#define VER2 2	//Winsock�ΰ汾��

WinsockEnv::WinsockEnv(void)
{
}

WinsockEnv::~WinsockEnv()
{
}

int WinsockEnv::Startup(){
	WSADATA wsaData;//���ڷ���Winsock�Ļ�����Ϣ   
	WORD  ver_all = MAKEWORD(VER1, VER2); //Winsock�汾�ţ����ֽڴΰ汾�����ֽ����汾��

	//��ʼ�� winsock
	int n = WSAStartup(ver_all, &wsaData);

	if(n)
	{ 
		//�������ֵ����0����Winsock��ʼ��ʧ��
		cout<< "Winsock startup error!\n";
		return -1;
	}

	//�жϷ��ص�Winsock�汾��
	if(wsaData.wVersion!= ver_all)
	{ 
		//�汾����
		WSACleanup();  //���Winsock
		cout << "Winsock version error!\n";
		return -1;
	}

	cout << "Winsock startup ok!\n";
	return 0;
}
