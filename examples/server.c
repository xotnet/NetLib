#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "net.c"

int char_count(char *str);
unsigned short int cmpWL(char* msg, char* msg2, int from, int to);

int PACKAGE_LEN = 1024;

void kickClient(int sock, short int outConsole) {
	close_net(sock);
	if (outConsole == 1) {printf("[%d] Client disconnected!\n", sock);}
}

void messageHandler(int socket) {
	char buf[PACKAGE_LEN];
	strcpy(buf, "200");
	send_net(socket, buf, 3);
	// INIT END
	srand(time(0));

	printf("[%d] Socket connected\n", socket);
	char* msg = (char*)malloc(PACKAGE_LEN);
	while (1) {
		memset(buf, 0, sizeof(buf));
		int received = recv_net(socket, buf, PACKAGE_LEN);
		strcpy(msg, buf);
		if (received < 1) {kickClient(socket, 1); break;}
		else if (cmpWL(buf, "help", 0, 4) == 1) {
			strcpy(buf, "Hello! - say hello to the server");
			send_net(socket, buf, char_count(buf));
		} else if (cmpWL(buf, "Hello!", 0, 6) == 1) {
			strcpy(buf, "Hello from test server!");
			send_net(socket, buf, char_count(buf));
		} else {
			strcpy(buf, "Unknown command!");
			send_net(socket, buf, char_count(buf));
		}
		printf("[%d] Says: %s\n", socket, msg);
	}
	free(msg);
}

void* accepter(void* server) {
	int socket = accept_net(*((int*)server));
	
	pthread_t id;
	pthread_create(&id, NULL, accepter, server);
	pthread_detach(id);
	
	messageHandler(socket);
}

int main() {
	int server = listen_net("0.0.0.0", "1234", 0);

	printf("Server started\n");

	pthread_t id;
	pthread_create(&id, NULL, accepter, (void*)&server);
	pthread_detach(id);

	while (1) {
		sleep(10000);
	}
}

int char_count(char *str) {
  int count = 0;
  while (*str != 0) {
    count++;
    str++;
  }
  return count;
}

unsigned short int cmpWL(char* msg, char* msg2, int from, int to) {
	unsigned int msgLen = char_count(msg);
	if (msgLen == 0 || char_count(msg2) == 0) {return 0;}
	int p = 0;
	while (from<msgLen && from<to) {
		if (*(msg+from) != *(msg2+p)) {return 0;}
		p++;
		from++;
	}
	return 1;
}