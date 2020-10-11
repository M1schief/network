#include <winsock2.h>
#include <string>
#include <assert.h>
#include <iostream>
#include <regex>
#include <Windows.h>
#include <unordered_map>
#include <list>
#include <algorithm>
#pragma comment(lib,"ws2_32.lib")

#define PORT 1111

std::unordered_map<std::string, const std::string> mymap = {
	//��׺��Ӧ
	{"html", "Content-Type: text/html\r\n"},
	{"htm", "Content-Type: text/html\r\n"},
	{"\\", "Content-Type: text/html\r\n"},
	{"txt", "Content-Type: text/plain\r\n"},
	{"jpg", "Content-Type: image/jpeg\r\n"},
	{"ico", "Content-Type: image/x-icon\r\n"},
};

bool get_suffix(const std::string& full_str, const std::string& end_str)
{
	//��ȡ��׺
	if (end_str.size() > full_str.size())
	{
		return false;
	}
	int size_full = full_str.size() - 1;
	int size_end = end_str.size() - 1;
	while (size_end >= 0)
	{
		if (full_str[size_full] != end_str[size_end])
		{
			return false;
		}
		size_full--;
		size_end--;
	}
	return true;
}

std::string get_main_path()
{
	//��ȡ�����ַ���ļ���ַ�ڳ����ϼ�Ŀ¼�µ�file�ļ�����
	char file_path[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, file_path, MAX_PATH);//���뷽ʽANSI
	std::string main_path = file_path;
	int idx = main_path.find_last_of('\\');
	main_path = main_path.substr(0, idx);
	idx = main_path.find_last_of('\\');
	main_path = main_path.substr(0, idx);
	main_path = main_path + "\\file";
	return main_path;
}

std::string get_url(char recv_buf[])
{
	//��ȡ�����url
	std::string recv_str = recv_buf;
	int n = recv_str.find("\r\n\r\n");
	recv_str = recv_str.substr(0, n);
	//������ʽ��ȡurl
	std::smatch match;
	std::regex reg(R"(/.*?(?= ))");
	std::string url;
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
	return url;
}

int main() {
	using namespace std;
	SOCKET tcp_sock;
	SOCKET accept_sock;
	std::list<SOCKET> sessions_ok;
	std::list<SOCKET> sessions_err;
	WSADATA wsa_data;
	fd_set rfds;				//���ڼ��socket�Ƿ������ݵ����ĵ��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�������ݵ�����
	fd_set wfds;				//���ڼ��socket�Ƿ���Է��͵��ļ�������������socket������ģʽ�µȴ������¼�֪ͨ�����Է������ݣ�

	WSAStartup(0x0202, &wsa_data);
	tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcp_sock == -1)
	{
		return false;
	}
	sockaddr_in addr, client_addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);
	int n = bind(tcp_sock, (LPSOCKADDR)&addr, sizeof(addr));
	assert(n == 0);
	n = listen(tcp_sock, 10);
	assert(n == 0);
	u_long mode = 1;//��srvSock��Ϊ������ģʽ�Լ����ͻ���������
	char recv_buf[1024];
	char send_buf[1024];
	if ((n = ioctlsocket(tcp_sock, FIONBIO, &mode) == SOCKET_ERROR)) 
	{ 
		//FIONBIO��������ֹ�׽ӿ�s�ķ�����ģʽ��
		cout << "ioctlsocket() failed with error!\n";
		return false;
	}
	
	while (1)
	{
		//��sessions_ok��ɾ���Ѿ���������ĻỰSOCKET
		for (auto itor1 = sessions_err.begin(); itor1 != sessions_err.end(); itor1++)
		{
			auto itor = find(sessions_ok.begin(), sessions_ok.end(), *itor1);
			if (itor != sessions_ok.end())
				sessions_ok.erase(itor);
		}
		//���read,write������
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_SET(tcp_sock, &rfds);
			
		for (auto itor = sessions_ok.begin(); itor != sessions_ok.end(); itor++)
		{
			//�����лỰSOCKET��Ϊ������ģʽ
			mode = 1;
			ioctlsocket(*itor, FIONBIO, &mode);
			//���õȴ��ỰSOKCET�ɽ������ݻ�ɷ�������
			FD_SET(*itor, &rfds);
			FD_SET(*itor, &wfds);
		}
		int total = select(0, &rfds, &wfds, NULL, NULL);
		int len = sizeof client_addr;
		if (FD_ISSET(tcp_sock, &rfds))
		{
			total--;
			accept_sock = accept(tcp_sock, (LPSOCKADDR)&client_addr, &len);
			sessions_ok.insert(sessions_ok.end(), accept_sock);
		}
		if (total > 0) 
		{
			for (auto itor = sessions_ok.begin(); itor != sessions_ok.end(); itor++)
			{
				memset(recv_buf, '\0', sizeof(recv_buf));
				if (FD_ISSET(*itor, &rfds)) 
				{
					//receiving data from client
					n = recv(*itor, recv_buf, sizeof recv_buf, 0);
					if (n) 
						printf("recv:\n%s\n", recv_buf);
					else 
					{
						sessions_err.insert(sessions_err.end(), *itor);
						continue;
					}
				}
				if (FD_ISSET(*itor, &wfds)) {
					//write data to client
					if (recv_buf[0] == '\0')
						continue;
					string url = get_url(recv_buf);
					string main_path = get_main_path();
					string full_path = main_path + url;
					DWORD ftyp = GetFileAttributesA(full_path.c_str());
					if ((ftyp != INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY))					
						//Ŀ¼���򿪸�Ŀ¼�µ�����index.html
						full_path = full_path + "\\index.html";
					FILE* infile = fopen(full_path.c_str(), "rb");
					if (!infile)
					{
						cout << "hhh" << endl;
						cout << full_path << endl;
					}
					string first_head, file_type, response_head;
					first_head = "HTTP/1.1 200 OK\r\n";
					string suffix = url.substr(url.find_last_of('.') + 1);

					auto find_type = mymap.find(suffix);
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

					response_head = first_head
						+ file_type
						+ "Content-Length: " + std::to_string(fileLength) + "\r\n"
						+ "Server: csr_http1.1\r\n"
						+ "Connection: close\r\n"
						+ "\r\n";
					n = send(*itor, response_head.c_str(), (int)(response_head.length()), 0);
					if (n == SOCKET_ERROR)
					{
						cout << "first_head" << first_head;
						cout << WSAGetLastError() << endl;
						sessions_err.insert(sessions_err.end(), *itor);
						continue;
					}
					int i = 0;
					while (true)
					{
						//��������
						memset(send_buf, 0, sizeof(send_buf));
						int file_buf = fread(send_buf, 1, 1024, infile);
						if (SOCKET_ERROR == (send(*itor, send_buf, file_buf, 0)))
						{//����ʧ��
							cout << "send err 2" << endl;
							cout << WSAGetLastError() << endl;
							sessions_err.insert(sessions_err.end(), *itor);
							i = 1;
							break;
						}				
						if (feof(infile))
							break;
					}
					if (i)
					{
						i = 0;
						continue;
					}
					fclose(infile);
				}
			}
		}
	}
	closesocket(tcp_sock);
	WSACleanup();
	return 0;
}
