#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "headers/ServerManager.h"
#include "headers/RequestsManager.h"
#include "headers/HttpParser.h"
#include "headers/BufferStorage.h"

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

        Parser* http_parser = createParser();
        ParserInput input;
        input.data = buf;
        input.size = bytes;
        parseHttpRequest(http_parser,&input);
        
        if(http_parser->parser_state == PS_INVALID){
            printf("[*] Invalid Data.\n");
            printf("\n\n%s\n\n", buf);
            // Close the connection.
            SSL_shutdown(host_ssl);
            SSL_free(host_ssl);
            SSL_shutdown(client_ssl);
            SSL_free(client_ssl);
            close(client);
            close(host);
            exit(1);
        }
        
       printf("\n[*] Checking Parse Http Request State: %s\n", getParsingState(http_parser->parser_state));

        BufferStorage* full_request_buffer = createBufferStorage();
        appendToBuffer(full_request_buffer, buf, bytes);
        while(http_parser->parser_state != PS_DONE){
            memset(buf, 0, MAX_BUFFER_SIZE);
            bytes = SSL_read(client_ssl, buf, sizeof(buf));
            input.data = buf;
            input.size = bytes;
            parseHttpRequest(http_parser, &input);
            appendToBuffer(full_request_buffer, buf, bytes);
        }
        
        // Forward http request to the target server.
        SSL_write(host_ssl, full_request_buffer->data, full_request_buffer->size);
        
        // Receive the Http Response.
        char response_buf[MAX_BUFFER_SIZE];
        memset(response_buf, 0, MAX_BUFFER_SIZE);

        bytes = SSL_read(host_ssl, response_buf, MAX_BUFFER_SIZE);
        
        if(bytes == 0){
            // NOTE: For some reasons if i am not doing this check i got a segfault problem.
            // FIXME: Investigate the issue.
            goto close_connection;
        }
        http_parser->parser_state = PS_REQUEST_LINE;
        input.data = response_buf;
        input.size = bytes;
        parseHttpResponse(http_parser, &input);
       printf("[*] parseHttpResponse: %s\n", getParsingState(http_parser->parser_state));
        
        
        BufferStorage* full_response_buffer = createBufferStorage();
        appendToBuffer(full_response_buffer, response_buf, bytes);

        while(http_parser->parser_state != PS_DONE){
            memset(response_buf, 0, MAX_BUFFER_SIZE);
            bytes = SSL_read(host_ssl, response_buf, MAX_BUFFER_SIZE);
            printf("[*] Bytes: %d\n", bytes);
            appendToBuffer(full_response_buffer, response_buf, bytes);
            input.data = response_buf;
            input.size = bytes;
            parseHttpResponse(http_parser, &input);
            printf("[*] ParseHttpResponse: %s\n", getParsingState(http_parser->parser_state));
        }
       
       printf("[*] Checking Parse Http Response State: %s\n", getParsingState(http_parser->parser_state));
        if(http_parser->parser_state == PS_DONE){
            printf("[*] Parsing Response Done.\n");
            SSL_write(client_ssl, full_response_buffer->data, full_response_buffer->size);
        }
        
        close_connection:
        // Close the connections.
        SSL_shutdown(host_ssl);
        SSL_free(host_ssl);
        SSL_shutdown(client_ssl);
        SSL_free(client_ssl);
        close(client);
        close(host);
       printf("[*] Client connection closed.\n");
       printf("[*] Host connection closed.\n");
    }
    
   printf("[*] Thread have completed its work.\n");
}

int main(int argc,char** argv){
    uint port = 8080;
    if(argc == 2){
        port = atoi(argv[1]);
    }
    Server* server = createServer(port);
    printf("[*] Server is up running on port %d.\n", port);
    while(true){
        Client* client = acceptConnections(server->socket);
       printf("[*] New client. \n");
        RecvData recv_data = receiveData(client->socket);
       printf("[*] Client has sent some data.\n");
        ParserInput parser_input;
        parser_input.data = recv_data.data;
        parser_input.size = recv_data.size;
        HandleDataResult res = handleData(&parser_input);
       printf("[*] Client data has been handled.\n");
        
        if(res.is_valid){
            printf("[*] Target hostname: %s.\n", res.target_host_name);
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
        }else{
           printf("[*] Invalid Request. (Http Request not supported yet, only https.)\n");
            close(client);
        }
    }
    
    return 0;
}