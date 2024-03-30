#ifdef __WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <cstring>
#endif
#include <cstdlib>
#include <string>
int listen_net(const char* ip, const char* port, const unsigned short int protocol=0) { // 0 for TCP | 1 for UDP
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int listener = 0;
	if (protocol == 0) {listener = socket(AF_INET, SOCK_STREAM, 0);} // TCP
	else if (protocol == 1) {listener = socket(AF_INET, SOCK_DGRAM, 0);} // UDP
	const int enable = 1;
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = atoi(ip);
	bind(listener, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
	listen(listener, SOMAXCONN);
	return listener;
}

int accept_net(int listener) {
	return accept(listener, 0, 0);
}

int connect_net(const char* ip, const char* port, const unsigned short int protocol=0) { // 0 for TCP | 1 for UDP
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int conn = 0;
	if (protocol == 0) {conn = socket(AF_INET, SOCK_STREAM, 0);}
	else if (protocol == 1) {conn = socket(AF_INET, SOCK_DGRAM, 0);}
	const int enable = 1;
	setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = inet_addr(ip);
	if (connect(conn, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {return -1;}
	return conn;
}

int send_net(int socket, char* buf, size_t size) {
	return send(socket, buf, (int)size, 0);
}

int recv_net(int socket, char* buf, int size) {
	return recv(socket, buf, size, 0);
}

std::string resolve_net(const char* domain, const char* port) {
	#ifdef __WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	#endif
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    getaddrinfo(domain, port, &hints, &res);
    struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(res->ai_family, &addr->sin_addr, ipstr, sizeof(ipstr));
	return ipstr;
}

char* getPeerIp_net(int socket) {
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
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
