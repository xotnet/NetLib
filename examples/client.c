#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "net.c"
unsigned int PACKAGE_LEN = 1024;

void sleepMS(int ms) {
	#ifdef __WIN32
		Sleep(ms);
	#else
		sleep(ms);
	#endif
}

int char_count(char *str) {
  int count = 0;
  while (*str != 0) {
    count++;
    str++;
  }
  return count;
}

unsigned short int cmpWL(const char* msg, const char* msg2) {
	unsigned int msgLen = sizeof(msg);
	int i = 0;
	while (i<msgLen) {
		char c = msg[i];
		char h = msg2[i];
		if (c == 0) {break;}
		if (c != h) {return 0;}
		i++;
	}
	return 1;
}

void commandHandler(const char* cmd) {
	if (cmpWL(cmd, "exit") == 1) {
		exit(0);
	}
}

int main() {
	int socket = connect_net("127.0.0.1", "1234", 0);
	char* buf = (char*)malloc(PACKAGE_LEN);
	
	char* recvBuffer = (char*)malloc(PACKAGE_LEN);
	recv_net(socket, recvBuffer, PACKAGE_LEN);
	
	printf("\nType help to get command list. Type exit to close connection.\n");

	memset(recvBuffer, 0, PACKAGE_LEN);

	while (1) {
		memset(buf, 0, PACKAGE_LEN);
		printf("\n user ~ ");
		scanf("%[^\n]s", buf);
		while(getchar() != '\n');
		commandHandler(buf);
		send_net(socket, buf, char_count(buf));
		memset(recvBuffer, 0, PACKAGE_LEN);
		int status = recv_net(socket, recvBuffer, PACKAGE_LEN);
		if (status <= 0) {printf("[ERR] Connection terminated\n"); exit(0);}
		unsigned short keywordExists = 0;
		printf(" serv ~ %s", recvBuffer);
	}
	free(recvBuffer);
	free(buf);
	return 0;
}
