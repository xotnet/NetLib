#ifdef __WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <netdb.h>
	#include <string.h>
#endif
#include <stdlib.h>

int listen_net(const char* ip, const char* port, const unsigned short int protocol /* 0 - TCP | 1 - UDP*/ ) {
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int listener = 0;
	if (protocol == 0) {listener = socket(AF_INET, SOCK_STREAM, 0);} // TCP
	else if (protocol == 1) {listener = socket(AF_INET, SOCK_DGRAM, 0);} // UDP
	else {return -1;}
	const int enable = 1;
	int netbuf_size = 1024 * 1024;
	setsockopt(listener, SOL_SOCKET, SO_SNDBUF, (char*)&netbuf_size, sizeof(netbuf_size));
	setsockopt(listener, SOL_SOCKET, SO_RCVBUF, (char*)&netbuf_size, sizeof(netbuf_size));
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(int));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = atoi(ip);
	bind(listener, (struct sockaddr*)&addr, sizeof(addr));
	listen(listener, SOMAXCONN);
	return listener;
}

int accept_net(int listener) {
	return accept(listener, 0, 0);
}

int connect_net(const char* ip, const char* port, const unsigned short int protocol /* 0 - TCP | 1 - UDP*/ ) {
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
	if (connect(conn, (struct sockaddr*)&addr, sizeof(addr)) < 0) {return -1;}
	return conn;
}

int send_net(int socket, char* buf, int size) {
	#ifdef __WIN32
		return send(socket, buf, size, 0);
	#else
		return send(socket, buf, size, MSG_NOSIGNAL);
	#endif
}

int recv_net(int socket, char* buf, int size) {
	return recv(socket, buf, size, 0);
}

void resolve_net(const char* domain, const char* port, char* ipOutput /* 16 chars */ ) {
	#ifdef __WIN32
		WSADATA wsaData;
		WSAStartup(MAKEWORD(2, 2), &wsaData);
	#endif
	struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
	if (getaddrinfo(domain, port, &hints, &res) != 0) {strcpy(ipOutput, "DNS serv error"); return;}
	struct sockaddr_in* addr = (struct sockaddr_in*)res->ai_addr;
    char ipstr[INET_ADDRSTRLEN];
    inet_ntop(res->ai_family, &addr->sin_addr, ipstr, sizeof(ipstr));
	strcpy(ipOutput, ipstr);
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

unsigned short int socks5_connect(int sock, const char *ip, unsigned short port) {
    char buf[64];

    // Handshake
    buf[0] = 0x05; // version
    buf[1] = 0x01; // 1 Authentication method
    buf[2] = 0x00; // No auth
    send_net(sock, buf, 3);
    
    // Handshake result
    recv_net(sock, buf, 2);
    if (buf[1] != 0x00) {
        return 1;
    }

    // Connect
    buf[0] = 0x05; // SOCKS5
    buf[1] = 0x01; // CMD_CONNECT
    buf[2] = 0x00; // RSV
    buf[3] = 0x01; // addr IPv4
    inet_pton(AF_INET, ip, &buf[4]); // IP addr
    buf[8] = (port >> 8) & 0xFF;
    buf[9] = port & 0xFF;
    send_net(sock, buf, 10);
    recv_net(sock, buf, 10);
    // Connection result
    if (buf[1] != 0x00) {
        return (unsigned short int)buf[1];
    }
    return 0;
}