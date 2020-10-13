#pragma once
#include <winsock2.h>
#include <iostream>
#include <regex>
#include <Ws2tcpip.h>

#include "Server.h"
#include "WinsockEnv.h"
#include "Config.h"

#pragma comment(lib, "Ws2_32.lib")

u_long mode = Config::BLOCKMODE;//设置阻塞模式为非阻塞
const unordered_map<string, const string> Server::mymap= {
	//后缀对应
	{"html", "Content-Type: text/html\r\n"},
	{"\\", "Content-Type: text/html\r\n"},
	{"txt", "Content-Type: text/plain\r\n"},
	{"jpg", "Content-Type: image/jpeg\r\n"},
	{"ico", "Content-Type: image/x-icon\r\n"},
};
//服务器初始化
Server::Server(void)
{
	this->recv_buf = new char[Config::BUFLEN];
	this->send_buf = new char[Config::BUFLEN];
	this->sessions_ok = new list<SOCKET>();
	this->sessions_err = new list<SOCKET>();
}
//服务结束
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
//环境初始化
int Server::winsock_start()
{
	if (WinsockEnv::Startup() == -1)
		return -1;
	return 0;
}
//服务器启动
int Server::server_start() {
	//创建流式socket
	this->tcp_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (this->tcp_sock == INVALID_SOCKET) {
		//创建失败
		cout << "Server socket creare error !" << endl;
		WSACleanup();
		return -1;
	}
	cout << "Server socket create ok!\n";

	//设置服务器IP地址和端口号
	this->addr.sin_family = AF_INET;
	this->addr.sin_port = htons(Config::PORT);
	this->addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//this->addr.sin_addr.S_un.S_addr = inet_addr(Config::IP.c_str());
	//绑定socket到主机地址
	int n = bind(this->tcp_sock, (LPSOCKADDR) & (this->addr), sizeof(this->addr));
	if (n == SOCKET_ERROR) {
		//绑定失败
		cout << "Server socket bind error!\n";
		closesocket(this->tcp_sock);
		WSACleanup();
		return -1;
	}

	cout << "Server socket bind ok!\n";
	return 0;
}
//等待连接
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
//设置socket非阻塞
int Server::set_unblock(SOCKET s)
{
	int n = ioctlsocket(s, FIONBIO, &mode);
	if (n == SOCKET_ERROR)
	{
		//FIONBIO：允许或禁止套接口s的非阻塞模式。
		cout << "ioctlsocket() failed with error!\n";
		return false;
	}
}
//删除单个无效会话
void Server::remove_closed_socket(SOCKET s) {
	if (s != INVALID_SOCKET) {
		auto itor = find(this->sessions_ok->begin(), this->sessions_ok->end(), s);
		if (itor != this->sessions_ok->end())
			this->sessions_ok->erase(itor);
	}
}
//删除所有无效会话
void Server::remove_closed_socket() {
	for (auto itor = this->sessions_err->begin(); itor != this->sessions_err->end(); itor++) {
		this->remove_closed_socket(*itor);
	}
}
//增加有效会话
void Server::add_session(SOCKET s) {
	if (s != INVALID_SOCKET) {
		this->sessions_ok->insert(this->sessions_ok->end(), s);
	}
}
//连接客户端
int Server::client_link()
{
	sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);

	//检查是否收到用户连接请求
	if (FD_ISSET(this->tcp_sock, &rfds)) 
	{  
		this->total--;

		//产生会话socket
		SOCKET accept_sock = accept(this->tcp_sock, (LPSOCKADDR)&client_addr, &addr_len);
		if (accept_sock == INVALID_SOCKET) 
		{
			//会话socket创建失败
			cout << "Server accept connection request error!\n";
			return -1;
		}
		cout << "\nconnect from addr = "<< inet_ntoa(client_addr.sin_addr)<< "port =" << client_addr.sin_port << endl;
		add_session(accept_sock);
	}
}
//接收信息
int Server::recv_msg(SOCKET s)
{
	if (FD_ISSET(s, &this->rfds))
	{
		//s可读
		int n = recv(s, recv_buf, Config::BUFLEN, 0);
		if (n)
			//接收成功，输出
		{
			string recv = recv_buf;
			int n = recv.find("\r\n");
			recv = recv.substr(0, n);
			cout << "recv:" << recv << endl;
		}
		else
		{
			//接收失败，加入无效队列
			this->sessions_err->insert(this->sessions_err->end(), s);
			return -1;
		}
	}
	return 0;
}
//获取程序地址
void Server::get_main_path()
{
	if (!main_path.size()) {
		//获取程序地址，文件地址在程序上级目录下的file文件夹中
		char file_path[MAX_PATH + 1] = { 0 };
		GetModuleFileNameA(NULL, file_path, MAX_PATH);//编码方式ANSI

		string path = file_path;
		int idx = path.find_last_of('\\');
		path = path.substr(0, idx);
		idx = path.find_last_of('\\');
		path = path.substr(0, idx);
		path = path + "\\file";

		this->main_path = path;
	}
}
//获取请求文件地址
void Server::get_url(char recv_buf[])
{
	string recv_str = recv_buf;

	//正则表达式提取url
	smatch match;
	regex reg(R"(/.*?(?= ))");
	string url;
	if (std::regex_search(recv_str, match, reg))
	{
		url = match[0].str();
	}
	for (int i = 0; i < url.size(); i++)
	{
		//替换成Windows路径
		if (url[i] == '/')
			url[i] = '\\';
	}
	this->url = main_path + url;
}
//发送信息
int Server::send_msg(SOCKET s,const char* buf,int len,int mode)
{
	int n = send(s, buf, len, 0);
	if (n == SOCKET_ERROR)
	{
		//发送失败
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
		//s可写
		string first_head, file_type, response_head,suffix;
		first_head = "HTTP/1.1 200 OK\r\n";
		if (recv_buf[0] == '\0')
			return -1;

		get_main_path();
		get_url(recv_buf);
		DWORD ftyp = GetFileAttributesA(url.c_str());
		if ((ftyp != INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY))
			//目录，打开该目录下的索引index.html
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
			//发送自定义501页面
			infile = fopen((main_path + "\\501.html").c_str(), "rb");
			first_head = "HTTP/1.1 501 Not Inplemented\r\n";
			file_type = "Content-Type: text/html\r\n";
		}
		if (!infile)
		{
			//文件不存在
			//发送自定义404页面
			infile = fopen((main_path + "\\404.html").c_str(), "rb");
			first_head = "HTTP/1.1 404 Not Found\r\n";
			file_type = "Content-Type: text/html\r\n";
		}

		//获取文件大小
		fseek(infile, 0, SEEK_END);
		int fileLength = ftell(infile);
		//文件指针归位
		fseek(infile, 0, SEEK_SET);

		//制作发送报文头部
		response_head = first_head
			+ file_type
			+ "Content-Length: " + std::to_string(fileLength) + "\r\n"
			+ "Server: csr_http1.1\r\n"
			+ "Connection: close\r\n"
			+ "\r\n";

		//发送head
		if (send_msg(s, response_head.c_str(), (int)(response_head.length()), 0) == -1)
			return -1;
		//发送文件
		while (true)
		{
			//缓存清零
			memset(send_buf, 0, sizeof(send_buf));
			int file_buf = fread(send_buf, 1, 1024, infile);
			if (send_msg(s, send_buf, file_buf, 1) == -1)
				return -1;
			if (feof(infile))
				//读完
				break;
		}
		fclose(infile);
	}
	return 0;
}
//接受请求
int Server::accept_request() {
	if (client_link() == -1)
		//服务端连接失败
		return -1;
	if (this->total > 0) 
	{
		//有可读或可写
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
//循环任务
int Server::loop()
{
	if (set_unblock(this->tcp_sock) == -1)
		return -1;
	while (true)
	{
		//去除无效会话
		this->remove_closed_socket();
		//清空socket集合
		FD_ZERO(&this->rfds);
		FD_ZERO(&this->wfds);
		//等待用户连接请求
		FD_SET(tcp_sock, &this->rfds);

		for (auto itor = this->sessions_ok->begin(); itor != this->sessions_ok->end(); itor++)
		{
			//设置所有会话为非阻塞模式
			set_unblock(*itor);
			//设置等待会话SOKCET可接受数据或可发送数据
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