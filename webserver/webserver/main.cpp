#include <winsock2.h>
#include <string>
#include <assert.h>
#include <iostream>

#include <regex>
#include <Windows.h>
#include <atlstr.h>

#pragma comment(lib,"ws2_32.lib")

#define PORT 1111

std::string TCHAR2STRING(TCHAR* STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0, STR, -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen * sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, STR, -1, chRtn, iLen, NULL, NULL);
	std::string str(chRtn);
	return str;

}

static bool endWith(const std::string& fullStr, const std::string& endStr)
{
	if (endStr.size() > fullStr.size())
	{
		return false;
	}
	int indexFull = fullStr.size() - 1;
	int indexEnd = endStr.size() - 1;
	while (indexEnd >= 0)
	{
		if (fullStr[indexFull] != endStr[indexEnd])
		{
			return false;
		}
		indexFull--;
		indexEnd--;
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
	cout << "wait link...\n";
	while (1)
	{
		sockaddr_in client_address;
		int len = sizeof client_address;
		accept_sock = accept(tcp_sock, (LPSOCKADDR)&client_address, &len);
		char recv_buf[1024];
		char send_buf[1024];
		n = recv(accept_sock, recv_buf, sizeof recv_buf, 0);
		recv_buf[n] = '\0';
		if (!n)
		{
			cout << "555" << endl;
			continue;
		}
		printf("recv:\n%s\n", recv_buf);

		string recv_str;
		recv_str.assign(recv_buf);
		n = recv_str.find("\r\n\r\n");
		recv_str = recv_str.substr(0, n);
		//正则表达式提取url
		std::smatch match;
		std::regex reg(R"(/.*?(?= ))");
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
		char szFilePath[MAX_PATH + 1] = { 0 };
		GetModuleFileNameA(NULL, szFilePath, MAX_PATH);    //编码方式ANSI
		string fullPath = szFilePath;
		int idx = fullPath.find_last_of('\\');
		string main_addr = fullPath.substr(0, idx);
		idx = main_addr.find_last_of('\\');
		main_addr = main_addr.substr(0, idx);
		fullPath = main_addr + "\\file" + url;
		DWORD ftyp = GetFileAttributesA(fullPath.c_str());
		if ((ftyp != INVALID_FILE_ATTRIBUTES) && (ftyp & FILE_ATTRIBUTE_DIRECTORY))
		{//是一个目录，打开该目录下的索引index.html
			fullPath = fullPath + "\index.html";
		}
		FILE* infile = fopen(fullPath.c_str(), "rb");
		if (!infile) cout << "hhh" << endl;
		string statusCode, firstHeader, respondHttpHeader;
		respondHttpHeader = "";
		statusCode = "200 OK";
		firstHeader = "HTTP/1.1 200 OK\r\n";
		string typeStr;
		if (endWith(url, ".html") || endWith(url, "htm") || endWith(url, "\\"))
		{
			typeStr = "Content-Type: text/html\r\n";
		}
		else if (endWith(url, ".txt"))
		{
			typeStr = "Content-Type: text/plain\r\n";
		}
		else if (endWith(url, ".jpg"))
		{
			typeStr = "Content-Type: image/jpeg\r\n";
		}
		else if (endWith(url, ".jpeg") || endWith(url, ".png"))
		{
			typeStr = "Content-Type: image/"
				+ (url.substr(url.rfind('.') + 1))
				+ "\r\n";
		}
		else if (endWith(url, ".ico"))
		{
			typeStr = "Content-Type: image/x-icon\r\n";
		}
		else
		{
			//发送自定义501页面
			infile = fopen((main_addr + "\\501.html").c_str(), "rb");
			//infile=fopen("501.html","rb");
			statusCode = "501 Not Implemented";
			firstHeader = "HTTP/1.1 501 Not Inplemented\r\n";
			typeStr = "Content-Type: text/html\r\n";
		}
		if (!infile)
		{//文件不存在
			//发送自定义404页面
			infile = fopen((main_addr + "\\404.html").c_str(), "rb");
			//infile=fopen("404.html","rb");
			statusCode = "404 Not Found";
			firstHeader = "HTTP/1.1 404 Not Found\r\n";
			typeStr = "Content-Type: text/html\r\n";
		}
		//获取文件大小
		fseek(infile, 0, SEEK_END);
		int fileLength = ftell(infile);
		//文件指针归位
		fseek(infile, 0, SEEK_SET);

		respondHttpHeader = firstHeader
			+ typeStr
			+ "Content-Length: " + std::to_string(fileLength) + "\r\n"
			+ "Server: csr_http1.1\r\n"
			+ "Connection: close\r\n"
			+ "\r\n";

		//发送报头
		n = send(accept_sock,
			respondHttpHeader.c_str(),
			(int)(respondHttpHeader.length()),
			0);
		int bufReadNum;
		//发送请求的文件        
		while (true)
		{
			//缓存清零
			memset(send_buf, 0, sizeof(send_buf));
			bufReadNum = fread(send_buf, 1, 1024, infile);
			if (SOCKET_ERROR == (send(accept_sock, send_buf, bufReadNum, 0)))
			{//发送失败
				n = SOCKET_ERROR;
				break;
			}
			if (feof(infile))
				break;
		}
		fclose(infile);

		closesocket(accept_sock);
	}
	closesocket(tcp_sock);
	getchar();
}
