#include <winsock2.h>
#include <string>
#include <assert.h>
#include <iostream>
#include <regex>
#include <Windows.h>
#include <atlstr.h>
#include <unordered_map>
#pragma comment(lib,"ws2_32.lib")

#define PORT 1111

std::unordered_map<string, string> mymap = {
			{"html", "Content-Type: text/html\r\n"},
			{"htm", "Content-Type: text/html\r\n"},
			{"\\", "Content-Type: text/html\r\n"},
			{"txt", "Content-Type: text/plain\r\n"},
			{"jpg", "Content-Type: image/jpeg\r\n"},
			{"png", "Content-Type: image/"
				+ (url.substr(url.rfind('.') + 1))
				+ "\r\n"},
			{"ico", "Content-Type: image/x-icon\r\n"},
};

bool get_suffix(const std::string& full_str, const std::string& end_str)
{
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

int main() {
	using std::cout;
	using std::endl;
	using std::string;

	SOCKET tcp_sock;
	SOCKET accept_sock;

	WSADATA wsa_data;
	WSAStartup(0x0202, &wsa_data);

	tcp_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (tcp_sock == -1)
	{
		return false;
	}

	sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(PORT);

	int n = bind(tcp_sock, (LPSOCKADDR)&addr, sizeof(addr));
	assert(n == 0);

	n = listen(tcp_sock, 10);
	assert(n == 0);
	while (1)
	{
		cout << "wait link..." << endl << endl;
		sockaddr_in client_address;
		int len = sizeof client_address;
		accept_sock = accept(tcp_sock, (LPSOCKADDR)&client_address, &len);
		char recv_buf[1024];
		char send_buf[1024];
		n = recv(accept_sock, recv_buf, sizeof recv_buf, 0);
		if (!n)
			continue;
		recv_buf[n] = '\0';
		printf("recv:\n%s\n", recv_buf);

		string recv_str;
		recv_str.assign(recv_buf);
		n = recv_str.find("\r\n\r\n");
		recv_str = recv_str.substr(0, n);
		//������ʽ��ȡurl
		std::smatch match;
		std::regex reg(R"(/.*?(?= ))");
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
		char file_path[MAX_PATH + 1] = { 0 };
		GetModuleFileNameA(NULL, file_path, MAX_PATH);    //���뷽ʽANSI
		string full_path = file_path;
		int idx = full_path.find_last_of('\\');
		string main_path = full_path.substr(0, idx);
		idx = main_path.find_last_of('\\');
		main_path = main_path.substr(0, idx);
		full_path = main_path + "\\file" + url;
		DWORD ftyp = GetFileAttributesA(full_path.c_str());
		if ((ftyp != INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY))
		{//��һ��Ŀ¼���򿪸�Ŀ¼�µ�����index.html
			full_path = full_path + "\\index.html";
		}
		FILE* infile = fopen(full_path.c_str(), "rb");
		if (!infile) 
			cout << "hhh" << endl;
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

		//���ͱ�ͷ
		n = send(accept_sock, response_head.c_str(), (int)(response_head.length()), 0);
		int file_buf;
		//����������ļ�        
		while (true)
		{
			//��������
			memset(send_buf, 0, sizeof(send_buf));
			file_buf = fread(send_buf, 1, 1024, infile);
			if (SOCKET_ERROR == (send(accept_sock, send_buf, file_buf, 0)))
			{//����ʧ��
				n = SOCKET_ERROR;
				break;
			}
			if (feof(infile))
				break;
		}
		fclose(infile);

		closesocket(accept_sock);
		cout << "send msg end" << endl << endl;
	}
	closesocket(tcp_sock);
	getchar();
}
