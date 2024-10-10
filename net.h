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
#include <stdint.h>

int32_t listen_net(const char* ip, const char* port, const uint8_t protocol /* 0 - TCP | 1 - UDP*/ ) {
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int32_t listener = 0;
	if (protocol == 0) {listener = socket(AF_INET, SOCK_STREAM, 0);} // TCP
	else if (protocol == 1) {listener = socket(AF_INET, SOCK_DGRAM, 0);} // UDP
	else {return -1;}
	const uint8_t enable = 1;
	int32_t netbuf_size = 1024 * 1024;
	setsockopt(listener, SOL_SOCKET, SO_SNDBUF, (char*)&netbuf_size, sizeof(netbuf_size));
	setsockopt(listener, SOL_SOCKET, SO_RCVBUF, (char*)&netbuf_size, sizeof(netbuf_size));
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(uint8_t));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = atoi(ip);
	bind(listener, (struct sockaddr*)&addr, sizeof(addr));
	listen(listener, SOMAXCONN);
	return listener;
}

int32_t accept_net(int32_t listener) {
	return accept(listener, 0, 0);
}

int32_t connect_net(const char* ip, const char* port, const uint8_t protocol /* 0 - TCP | 1 - UDP*/ ) {
	#ifdef __WIN32
		WSADATA wsa;
		WSAStartup(MAKEWORD(2,2), &wsa);
	#endif
	int32_t conn = 0;
	if (protocol == 0) {conn = socket(AF_INET, SOCK_STREAM, 0);}
	else if (protocol == 1) {conn = socket(AF_INET, SOCK_DGRAM, 0);}
	const uint8_t enable = 1;
	setsockopt(conn, SOL_SOCKET, SO_REUSEADDR, (char*)&enable, sizeof(uint8_t));
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(port));
	addr.sin_addr.s_addr = inet_addr(ip);
	if (connect(conn, (struct sockaddr*)&addr, sizeof(addr)) < 0) {return -1;}
	return conn;
}

int32_t send_net(int32_t socket, char* buf, uint32_t size) {
	#ifdef __WIN32
		return send(socket, buf, size, 0);
	#else
		return send(socket, buf, size, MSG_NOSIGNAL);
	#endif
}

int32_t recv_net(int32_t socket, char* buf, uint32_t size) {
	return recv(socket, buf, size, 0);
}

void getPeerIp_net(int32_t socket, char* ip) {
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	getpeername(socket, (struct sockaddr *)&client_addr, &addr_len);
	strcpy(ip, inet_ntoa(client_addr.sin_addr));
}

int32_t close_net(int32_t conn) {
	#ifdef __WIN32
		return closesocket(conn);
	#elif __linux__
		return close(conn);
	#endif
}

uint8_t socks5_connect(int32_t sock, const char *ip, uint16_t port) {
    char buf[64];

    buf[0] = 0x05;
    buf[1] = 0x01;
    buf[2] = 0x00;
    send_net(sock, buf, 3);
    recv_net(sock, buf, 2);
    if (buf[1] != 0x00) {
        return 1;
    }
    buf[0] = 0x05;
    buf[1] = 0x01;
    buf[2] = 0x00;
    buf[3] = 0x01;
    inet_pton(AF_INET, ip, &buf[4]);
    buf[8] = (port >> 8) & 0xFF;
    buf[9] = port & 0xFF;
    send_net(sock, buf, 10);
    recv_net(sock, buf, 10);
    if (buf[1] != 0x00) {
        return (unsigned short int)buf[1];
    }
    return 0;
}

char dnsIP[16] = "1.1.1.1";
enum dnsType {dnsANY, dnsA = 1, dnsNS, dnsMD, dnsMF, dnsCNAME, dnsSOA, dnsMB, dnsMG, dnsMR, dnsMX=15, dnsTXT=16};
void resolve_net(char* domain, char* output, uint8_t nsType) {
	int32_t conn = connect_net(dnsIP, "53", 1);
	char buf[128];
	memset(buf, 0, 128);
	buf[0] = 0x12;
	buf[2] = 0x01;
    buf[5] = 0x01;
    uint16_t nameLen = strlen(domain);
    char domainCpy[nameLen+1];
    uint8_t lblLen = 0;
    for (uint16_t nameLenCpy = nameLen; nameLenCpy != 0; nameLenCpy--) {
		if (*(domain+nameLenCpy-1) == '.') {
			domainCpy[nameLenCpy] = 0x00 + lblLen;
			lblLen = 0;
			continue;
		}
		domainCpy[nameLenCpy] = domain[nameLenCpy-1];
		lblLen++;
    }
    domainCpy[0] = 0x00 + lblLen;
    strcpy(buf+12, domainCpy);
    uint16_t pos = nameLen + 13;
    buf[pos] = 0x00;
    buf[pos+1] = 0x00;
    buf[pos+2] = 0x00 + nsType;
    buf[pos+3] = 0x00;
    buf[pos+4] = 0x01;
    send_net(conn, buf, pos+5);
    memset(buf, 0, 128);
    if (recv_net(conn, buf, 128) < 1) {strcpy(output, "DNS server error"); return;}
    if ((buf[6] << 8) + buf[7] == 0) {strcpy(output, "DNS server error"); return;}
    uint16_t answer_start = 12 + strlen(buf+12) + 5 + 12;
	if (nsType == dnsA) {
		char ipbytes[4];
		strcpy(ipbytes, buf+answer_start);
		inet_ntop(AF_INET, ipbytes, output, 15);
	}
	else {
	    uint16_t labelLen = 0;
	    uint16_t p = -1;
	    while (1) {
	    	labelLen = buf[answer_start];
		    if (labelLen == 0) {break;}
	        answer_start++;
	        p++;
	        for (int i = 0; i<labelLen; i++) {
	            *(output+p) = buf[answer_start+i];
	        	p++;
		    }
		    answer_start = answer_start + labelLen;
		    if (buf[answer_start] != 0) {*(output+p) = '.';}
		}
    }
    close_net(conn);
}