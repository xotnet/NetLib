C network lib for **Linux** and **Windows**


**```CLIENT```**

**connect_net**(const char* ip, const char* port, const unsigned short int protocol) - **connect to the server**

**```SERVER```**

**listen_net**(const char* ip, const char* port, const unsigned short int protocol) - **create listening server**

**accept_net**(int listener) - **accept connecting client**

**getPeerIp_net**(int socket) - **get ip of connected socket**

**```BOTH```**

**recv_net**(int socket, char* buf, int size) - **read data from socket**

**send_net**(int socket, char* buf, int size) - **send data to socket**

**resolve_net**(const char* domain, const char* port, char* ipAddrOutput) - **convert domain to ip**

**close_net**(int socket) - **close connection**
