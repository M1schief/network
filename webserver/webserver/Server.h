#pragma once
#include <winsock2.h>
#include <list>
#include <unordered_map>
using namespace std;

class Server
{
private:
	SOCKET tcp_sock;
	sockaddr_in addr;
	char* recv_buf;
	char* send_buf;
	list<SOCKET> *sessions_ok;//��Ч�Ự
	list<SOCKET> *sessions_err;//ʧЧ�Ự
	string main_path;//�����ַ
	string url;//Ŀ���ַ
	fd_set rfds;//���ڼ��socket�Ƿ������ݵ����ĵ��ļ�������
	fd_set wfds;//���ڼ��socket�Ƿ���Է��͵��ļ�������
	int total;
	static const unordered_map<string, const string> mymap;//��׺��Ӧ
public:
	Server(void);
	~Server();
	int winsock_start();
	int server_start();
	int listen_start();
	int set_unblock();
	int set_unblock(SOCKET s);
	void remove_closed_socket(SOCKET s);
	void remove_closed_socket();
	void add_session(SOCKET s);
	int client_link();
	int recv_msg(SOCKET s);
	int send_msg(SOCKET s, const char* buf, int len,int mode);
	int send_msg(SOCKET s);
	void get_url(char recv_buf[]);
	void get_main_path();
	int accept_request();
	int loop();
};