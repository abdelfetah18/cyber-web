#ifndef PROXY_SERVER
#define PROXY_SERVER

#include <stdbool.h>
#include <X11/Xlib.h>

#include "ServerManager.h"

typedef struct WorkerParams {
    SSL* client_ssl;
    SOCKET_HANDLE client;
    SSL* host_ssl;
    SOCKET_HANDLE host;
} WorkerParams;

void startProxyServer(void* args);
void* workerThread(void* args);
void closePeersConnectionsAndExit(WorkerParams* peers);

WorkerParams* createWorkerParams(SSL* client_ssl,SOCKET_HANDLE client,SSL* host_ssl,SOCKET_HANDLE host);

#endif