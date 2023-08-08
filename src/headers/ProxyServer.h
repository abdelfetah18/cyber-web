#ifndef PROXY_SERVER
#define PROXY_SERVER

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

#include "ServerManager.h"
#include "AppUI.h"
#include "RequestsManager.h"

typedef struct WorkerParams {
    SSL* client_ssl;
    SOCKET_HANDLE client;
    SSL* host_ssl;
    SOCKET_HANDLE host;
} WorkerParams;

void proxyServerThread(void* args);
void workerThread(void* args);
void closePeersConnectionsAndExit(WorkerParams* peers);

WorkerParams* createWorkerParams(SSL* client_ssl,SOCKET_HANDLE client,SSL* host_ssl,SOCKET_HANDLE host);

#endif