#ifdef __WIN32
	#include <winsock2.h>
#else
	#include <arpa/inet.h>
	#include <unistd.h>
#endif
#include <cstdlib>
#include <string>
int listen_net(const char* port) {
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	const int enable = 1;
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = INADDR_ANY;
	bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	listen(listener, SOMAXCONN);
	return listener;
}

int accept_net(int listener) {
	return accept(listener, 0, 0);
}

int connect_net(const char* ip, const char* port) {
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int conn = socket(AF_INET, SOCK_STREAM, 0);
	const int enable = 1;
	setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = inet_addr(ip);
	connect(conn, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	return conn;
}

int send_net(int socket, char* buf, size_t size) {
	return send(socket, buf, (int)size, 0);
}

int recv_net(int socket, char* buf, int size) {
	return recv(socket, buf, size, 0);
}

char* resolve_net(const char* domain) {
	#ifdef __WIN32
		WSADATA wsaData;
		int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	#endif
    struct hostent* remoteHost = gethostbyname(domain);
	struct in_addr addr;
	memcpy(&addr, remoteHost->h_addr_list[0], sizeof(struct in_addr));
	return inet_ntoa(addr);
}

char* getPeerIp_net(int socket) {
	struct sockaddr_in client_addr;
	int addr_len = sizeof(client_addr);
	getpeername(socket, (struct sockaddr *)&client_addr, &addr_len);
	return inet_ntoa(client_addr.sin_addr);
}

int close_net(int conn) {
	#ifdef __WIN32
		return closesocket(conn);
	#elif __linux__
		return close(conn);
	#endif
}
