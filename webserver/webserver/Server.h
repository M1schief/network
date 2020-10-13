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
	list<SOCKET> *sessions_ok;//有效会话
	list<SOCKET> *sessions_err;//失效会话
	string main_path;//程序地址
	string url;//目标地址
	fd_set rfds;//用于检查socket是否有数据到来的的文件描述符
	fd_set wfds;//用于检查socket是否可以发送的文件描述符
	int total;
	static const unordered_map<string, const string> mymap;//后缀对应
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