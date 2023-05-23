#ifndef SERVER_MANAGER
#define SERVER_MANAGER

#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>  

#define MAX_CONNECTIONS 64
#define MAX_BUFFER_SIZE 1024 * 4
#define SOCKET_HANDLE int

/*
    sockaddr_in* server_address
    int socket_fd
*/

typedef struct Server {
    struct sockaddr_in* address;
    SOCKET_HANDLE socket;
} Server;

typedef struct Client {
    struct sockaddr_in* address;
    SOCKET_HANDLE socket;
} Client;

typedef struct Host {
    struct sockaddr_in* address;
    SOCKET_HANDLE socket;
} Host;

typedef struct RecvData {
    char* data;
    unsigned int size;
} RecvData;

Server* createServer(int port);
Client* acceptConnections(SOCKET_HANDLE socket_fd);
RecvData receiveData(SOCKET_HANDLE client_socket_fd);
void sendData(SOCKET_HANDLE socket_fd);
Host* connectToHost(char* hostname);
SSL* acceptSSLConnection(SOCKET_HANDLE client,char* host_name);
SSL* upgradeToSSL(SOCKET_HANDLE connection);

#endif