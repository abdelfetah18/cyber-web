#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "headers/ServerManager.h"
#include "headers/RequestsManager.h"

typedef struct WorkerParams {
    SSL* client_ssl;
    SOCKET_HANDLE client;
    SSL* host_ssl;
    SOCKET_HANDLE host;
} WorkerParams;

void workerThread(void* args){
    printf("[*] Thread Created Successfuly.\n");

    WorkerParams* thread_params = (WorkerParams*) args;

    SSL* client_ssl = thread_params->client_ssl;
    SOCKET_HANDLE client = thread_params->client;
    SSL* host_ssl = thread_params->host_ssl;
    SOCKET_HANDLE host = thread_params->host;

    if(client_ssl != NULL){
        // 1. Recieve client http request.
        char buf[MAX_BUFFER_SIZE];
        memset(buf, 0, MAX_BUFFER_SIZE);
        unsigned int bytes = SSL_read(client_ssl, buf, sizeof(buf));
        // Forward http request to the target server.
        SSL_write(host_ssl, buf, strlen(buf));

        // FIXME: Parse Http Request to know the end of the request.
        while(1){
            // Receive the response.
            char result_buf[MAX_BUFFER_SIZE];
            memset(result_buf, 0, MAX_BUFFER_SIZE);
            bytes = SSL_read(host_ssl, result_buf, MAX_BUFFER_SIZE);
            // Forward response to client.
            SSL_write(client_ssl, result_buf, bytes);

            // Check if the SSL connection has been shutdown
            int ssl_shutdown = SSL_get_shutdown(host_ssl);
            if (ssl_shutdown & SSL_RECEIVED_SHUTDOWN) {
                // Client has disconnected
                SSL_shutdown(host_ssl);
                SSL_free(host_ssl);
                SSL_shutdown(client_ssl);
                SSL_free(client_ssl);
                close(client);
                close(host);
                printf("client connection closed.\n");
                break;
            }
        }
    }
    
    exit(0);
}

int main(){
    Server* server = createServer(8080);
    printf("[*] Server is up running on port 8080.\n");
    while(true){
        Client* client = acceptConnections(server->socket);
        printf("[*] New client.\n");
        RecvData recv_data = receiveData(client->socket);
        printf("[*] Client has sent some data.\n");
        HandleDataResult res = handleData(recv_data.data);
        printf("[*] Client data has been handled.\n");
        // If it a CONNECT Request then establish a connection with target host and reply with status 200.
        /*
            HostName
        */
        if(res.is_valid){
            Host* host = connectToHost(res.target_host_name);
            printf("[*] Host Connected.\n");
            SSL* host_ssl = upgradeToSSL(host->socket);
            printf("[*] Upgrade host connection to SSL.\n");
            
            const char* reply = "HTTP/1.1 200 Connection established\r\nProxy-agent: CyberWeb\r\n\r\n";
            send(client->socket, reply, strlen(reply), 0);
            
            printf("[*] Reply with Connection established to the client.\n");
            
            
            SSL* client_ssl = acceptSSLConnection(client->socket, res.target_host_name);
            printf("[*] Accept Client SSL connection.\n");

            WorkerParams* params = malloc(sizeof(WorkerParams));
            params->client_ssl = client_ssl;
            params->client = client;
            params->host_ssl = host_ssl;
            params->host = host;
            pthread_t WORKER_ID;
            pthread_create(&WORKER_ID, NULL, workerThread, params);
            
        }
    }
    
    return 0;
}