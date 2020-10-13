#pragma once
#include <winsock2.h>
#include <iostream>
#include <regex>
#include <Ws2tcpip.h>

#include "Server.h"
#include "WinsockEnv.h"
#include "Config.h"

#pragma comment(lib, "Ws2_32.lib")

u_long mode = Config::BLOCKMODE;//��������ģʽΪ������
const unordered_map<string, const string> Server::mymap= {
	//��׺��Ӧ
	{"html", "Content-Type: text/html\r\n"},
	{"\\", "Content-Type: text/html\r\n"},
	{"txt", "Content-Type: text/plain\r\n"},
	{"jpg", "Content-Type: image/jpeg\r\n"},
	{"ico", "Content-Type: image/x-icon\r\n"},
};
//��������ʼ��
Server::Server(void)
{
	this->recv_buf = new char[Config::BUFLEN];
	this->send_buf = new char[Config::BUFLEN];
	this->sessions_ok = new list<SOCKET>();
	this->sessions_err = new list<SOCKET>();
}
//�������
Server::~Server()
{
	if (this->recv_buf != NULL)
	{
		delete this->recv_buf;
		this->recv_buf = NULL;
	}

	if (this->send_buf != NULL)
	{
		delete this->send_buf;
		this->send_buf = NULL;
	}

	if (this->sessions_ok != NULL)
	{
		for (auto itor = this->sessions_ok->begin(); itor != this->sessions_ok->end(); itor++)
			closesocket(*itor);
		delete this->sessions_ok;
		this->sessions_ok = NULL;
	}

	if (this->sessions_err != NULL)
	{
		for (auto itor = this->sessions_err->begin(); itor != this->sessions_err->end(); itor++)
			closesocket(*itor);
		delete this->sessions_err;
		this->sessions_err = NULL;
	}
	WSACleanup();
}
//������ʼ��
int Server::winsock_start()
{
	if (WinsockEnv::Startup() == -1)
		return -1;
	return 0;
}
//����������
int Server::server_start() {
	//������ʽsocket
	this->tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (this->tcp_sock == INVALID_SOCKET) {
		//����ʧ��
		cout << "Server socket creare error !" << endl;
		WSACleanup();
		return -1;
	}
	cout << "Server socket create ok!\n";

	//���÷�����IP��ַ�Ͷ˿ں�
	this->addr.sin_family = AF_INET;
	this->addr.sin_port = htons(Config::PORT);
	this->addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//this->addr.sin_addr.S_un.S_addr = inet_addr(Config::IP.c_str());
	//��socket��������ַ
	int n = bind(this->tcp_sock, (LPSOCKADDR) & (this->addr), sizeof(this->addr));
	if (n == SOCKET_ERROR) {
		//��ʧ��
		cout << "Server socket bind error!\n";
		closesocket(this->tcp_sock);
		WSACleanup();
		return -1;
	}

	cout << "Server socket bind ok!\n";
	return 0;
}
//�ȴ�����
int Server::listen_start() {
	int n = listen(this->tcp_sock, Config::MAXCONNECT);
	if (n == SOCKET_ERROR) {
		cout << "Server socket listen error!\n";
		closesocket(this->tcp_sock);
		WSACleanup();
		return -1;
	}

	cout << "Server socket listen ok!\n";
	return 0;
}
//����socket������
int Server::set_unblock(SOCKET s)
{
	int n = ioctlsocket(s, FIONBIO, &mode);
	if (n == SOCKET_ERROR)
	{
		//FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
		cout << "ioctlsocket() failed with error!\n";
		return false;
	}
}
//ɾ��������Ч�Ự
void Server::remove_closed_socket(SOCKET s) {
	if (s != INVALID_SOCKET) {
		auto itor = find(this->sessions_ok->begin(), this->sessions_ok->end(), s);
		if (itor != this->sessions_ok->end())
			this->sessions_ok->erase(itor);
	}
}
//ɾ��������Ч�Ự
void Server::remove_closed_socket() {
	for (auto itor = this->sessions_err->begin(); itor != this->sessions_err->end(); itor++) {
		this->remove_closed_socket(*itor);
	}
}
//������Ч�Ự
void Server::add_session(SOCKET s) {
	if (s != INVALID_SOCKET) {
		this->sessions_ok->insert(this->sessions_ok->end(), s);
	}
}
//���ӿͻ���
int Server::client_link()
{
	sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);

	//����Ƿ��յ��û���������
	if (FD_ISSET(this->tcp_sock, &rfds)) 
	{  
		this->total--;

		//�����Ựsocket
		SOCKET accept_sock = accept(this->tcp_sock, (LPSOCKADDR)&client_addr, &addr_len);
		if (accept_sock == INVALID_SOCKET) 
		{
			//�Ựsocket����ʧ��
			cout << "Server accept connection request error!\n";
			return -1;
		}
		cout << "\nconnect from addr = "<< inet_ntoa(client_addr.sin_addr)<< "port =" << client_addr.sin_port << endl;
		add_session(accept_sock);
	}
}
//������Ϣ
int Server::recv_msg(SOCKET s)
{
	if (FD_ISSET(s, &this->rfds))
	{
		//s�ɶ�
		int n = recv(s, recv_buf, Config::BUFLEN, 0);
		if (n)
			//���ճɹ������
		{
			string recv = recv_buf;
			int n = recv.find("\r\n");
			recv = recv.substr(0, n);
			cout << "recv:" << recv << endl;
		}
		else
		{
			//����ʧ�ܣ�������Ч����
			this->sessions_err->insert(this->sessions_err->end(), s);
			return -1;
		}
	}
	return 0;
}
//��ȡ�����ַ
void Server::get_main_path()
{
	if (!main_path.size()) {
		//��ȡ�����ַ���ļ���ַ�ڳ����ϼ�Ŀ¼�µ�file�ļ�����
		char file_path[MAX_PATH + 1] = { 0 };
		GetModuleFileNameA(NULL, file_path, MAX_PATH);//���뷽ʽANSI

		string path = file_path;
		int idx = path.find_last_of('\\');
		path = path.substr(0, idx);
		idx = path.find_last_of('\\');
		path = path.substr(0, idx);
		path = path + "\\file";

		this->main_path = path;
	}
}
//��ȡ�����ļ���ַ
void Server::get_url(char recv_buf[])
{
	string recv_str = recv_buf;

	//������ʽ��ȡurl
	smatch match;
	regex reg(R"(/.*?(?= ))");
	string url;
	if (std::regex_search(recv_str, match, reg))
	{
		url = match[0].str();
	}
	for (int i = 0; i < url.size(); i++)
	{
		//�滻��Windows·��
		if (url[i] == '/')
			url[i] = '\\';
	}
	this->url = main_path + url;
}
//������Ϣ
int Server::send_msg(SOCKET s,const char* buf,int len,int mode)
{
	int n = send(s, buf, len, 0);
	if (n == SOCKET_ERROR)
	{
		//����ʧ��
		if(mode == 0)
			cout << "send head\n";
		else
			cout << "send file\n";
		cout << WSAGetLastError() << endl;
		sessions_err->insert(sessions_err->end(), s);
		return -1;
	}
}
int Server::send_msg(SOCKET s)
{
	if (FD_ISSET(s, &this->wfds)) {
		//s��д
		string first_head, file_type, response_head,suffix;
		first_head = "HTTP/1.1 200 OK\r\n";
		if (recv_buf[0] == '\0')
			return -1;

		get_main_path();
		get_url(recv_buf);
		DWORD ftyp = GetFileAttributesA(url.c_str());
		if ((ftyp != INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY))
			//Ŀ¼���򿪸�Ŀ¼�µ�����index.html
			url = url + "\\index.html";

		suffix = url.substr(url.find_last_of('.') + 1);
		auto find_type = mymap.find(suffix);
		FILE* infile = fopen(url.c_str(), "rb");
		if (find_type != mymap.end())
		{
			file_type = find_type->second;
		}
		else
		{
			//�����Զ���501ҳ��
			infile = fopen((main_path + "\\501.html").c_str(), "rb");
			first_head = "HTTP/1.1 501 Not Inplemented\r\n";
			file_type = "Content-Type: text/html\r\n";
		}
		if (!infile)
		{
			//�ļ�������
			//�����Զ���404ҳ��
			infile = fopen((main_path + "\\404.html").c_str(), "rb");
			first_head = "HTTP/1.1 404 Not Found\r\n";
			file_type = "Content-Type: text/html\r\n";
		}

		//��ȡ�ļ���С
		fseek(infile, 0, SEEK_END);
		int fileLength = ftell(infile);
		//�ļ�ָ���λ
		fseek(infile, 0, SEEK_SET);

		//�������ͱ���ͷ��
		response_head = first_head
			+ file_type
			+ "Content-Length: " + std::to_string(fileLength) + "\r\n"
			+ "Server: csr_http1.1\r\n"
			+ "Connection: close\r\n"
			+ "\r\n";

		//����head
		if (send_msg(s, response_head.c_str(), (int)(response_head.length()), 0) == -1)
			return -1;
		//�����ļ�
		while (true)
		{
			//��������
			memset(send_buf, 0, sizeof(send_buf));
			int file_buf = fread(send_buf, 1, 1024, infile);
			if (send_msg(s, send_buf, file_buf, 1) == -1)
				return -1;
			if (feof(infile))
				//����
				break;
		}
		fclose(infile);
	}
	return 0;
}
//��������
int Server::accept_request() {
	if (client_link() == -1)
		//���������ʧ��
		return -1;
	if (this->total > 0) 
	{
		//�пɶ����д
		for (auto itor = this->sessions_ok->begin(); itor != this->sessions_ok->end(); itor++)
		{
			memset(recv_buf, '\0', Config::BUFLEN);
			if (recv_msg(*itor) == -1)
				continue;
			if (send_msg(*itor) == -1)
				continue;
		}
	}
	return 0;
}
//ѭ������
int Server::loop()
{
	if (set_unblock(this->tcp_sock) == -1)
		return -1;
	while (true)
	{
		//ȥ����Ч�Ự
		this->remove_closed_socket();
		//���socket����
		FD_ZERO(&this->rfds);
		FD_ZERO(&this->wfds);
		//�ȴ��û���������
		FD_SET(tcp_sock, &this->rfds);

		for (auto itor = this->sessions_ok->begin(); itor != this->sessions_ok->end(); itor++)
		{
			//�������лỰΪ������ģʽ
			set_unblock(*itor);
			//���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
			FD_SET(*itor, &rfds);
			FD_SET(*itor, &wfds);
		}

		if ((this->total = select(0, &this->rfds, &this->wfds, NULL, NULL)) == SOCKET_ERROR) { 
			cout << "select() failed with error!\n";
			return -1;
		}
		if(this->accept_request()==-1)
			return -1;
	}
	return 0;
}