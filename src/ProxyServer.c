#include "headers/ProxyServer.h"

void closePeersConnectionsAndExit(WorkerParams* peers){
    // Close the connection.
    SSL_shutdown(peers->host_ssl);
    SSL_free(peers->host_ssl);
    SSL_shutdown(peers->client_ssl);
    SSL_free(peers->client_ssl);
    close(peers->client);
    close(peers->host);

    // NOTE: Using 'pthread_exit' because this function will only be called on a thread and calling 'exit' will exit the main process.
    pthread_exit(1);
}

WorkerParams* createWorkerParams(SSL* client_ssl,SOCKET_HANDLE client,SSL* host_ssl,SOCKET_HANDLE host){
    WorkerParams* worker_params = malloc(sizeof(WorkerParams));
    worker_params->client = client;
    worker_params->client_ssl = client_ssl;
    worker_params->host = host;
    worker_params->host_ssl = host_ssl;
    
    return worker_params;
}

void workerThread(void* args){
   printf("[*] workerThread started successfuly.\n");

    WorkerParams* thread_params = (WorkerParams*) args;

    SSL* client_ssl = thread_params->client_ssl;
    SOCKET_HANDLE client = thread_params->client;
    SSL* host_ssl = thread_params->host_ssl;
    SOCKET_HANDLE host = thread_params->host;

    Peers peers = createPeers(client_ssl, client, host_ssl, host);
    // Get full http request from the client.
    ParserResult* parsing_request_result = receiveFullHttpRequest(client_ssl, &peers);
    BufferStorage* full_request_buffer = parsing_request_result->full_raw_data;

    parsing_request_result->parser->http_request->raw = full_request_buffer;

    uint id = history_list == NULL ? 0 : (history_list->id + 1);
    HistoryItem* item = createHistoryItem(id, parsing_request_result->parser->http_request, NULL);
    insertIntoHistoryList(item);
    IPCMessage* ipc_message = malloc(sizeof(IPCMessage));
    ipc_message->data = item;
    ipc_message->type = HTTP_REQUEST;


    g_signal_emit_by_name(window, "update_ui", ipc_message);    

    // Forward http request to the target server.
    SSL_write(host_ssl, full_request_buffer->data, full_request_buffer->size);
    
    // Get full http response from the host.
    ParserResult* parsing_response_result = receiveFullHttpResponse(host_ssl, &peers);
    BufferStorage* full_response_buffer = parsing_response_result->full_raw_data;
    
    parsing_response_result->parser->http_response->raw = full_response_buffer;

    updateHistoryItemHttpResponse(id, parsing_response_result->parser->http_response);

    // Forward http response to the client.
    SSL_write(client_ssl, full_response_buffer->data, full_response_buffer->size);
    
    printf("[*] workerThread has completed its job.\n");
    // Close the connections.
    closePeersConnectionsAndExit(&peers);
}

void proxyServerThread(void* args){
    uint port = 8080;
    
    // TODO: Pass the port throw args.
    // if(argc == 2){
    //     port = atoi(argv[1]);
    // }

    Server* server = createServer(port);
    printf("[*] Server is up running on port %d.\n", port);
    if(!doesRootCertificateExists()){
        printf("[*] RootCA Certificate not exists.\n");
        createRootCertificate();
        printf("[*] RootCA Certificate have been Created.\n");
    }else{
        printf("[*] RootCA Certificate already exists.\n");
    }
    while(true){
        Client* client = acceptConnections(server->socket);
        printf("[*] New client. \n");
        RecvData recv_data = receiveData(client->socket);
        printf("[*] Client has sent some data.\n");

        ParserInput parser_input = createParserInput(recv_data.data, recv_data.size);

        HandleDataResult res = handleData(&parser_input);
        printf("[*] Client data has been handled.\n");
        
        // MUST_BE_FUNCTION: Handle result.
        if(res.is_valid){
            if(res.is_https){
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

                WorkerParams* params = createWorkerParams(client_ssl, client, host_ssl, host);
                pthread_t WORKER_ID;
                pthread_create(&WORKER_ID, NULL, workerThread, params);
            }else{
                if(strcmp(res.target_host_name, "/getCert") == 0){
                    // MUST_BE_FUNCTION: handle GetCert.
                    // Open File.
                    FILE* f = fopen("hosts/CyberWeb.crt", "r");
                    // GetSize.
                    fseek(f, 0L, SEEK_END);
                    uint file_size = ftell(f);
                    rewind(f);
                    // Read File to a Buffer.
                    char* file_content = malloc(sizeof(char) * (file_size + 1));
                    memset(file_content, 0 , sizeof(char) * (file_size + 1));
                    fread(file_content, sizeof(char), file_size, f);
                    // Convert Size To ascii.
                    char file_size_buffer[10];
                    memset(file_size_buffer, 0, 10);
                    sprintf(file_size_buffer, "%d", file_size);
                    // itoa(file_size, file_size_buffer, 10);
                    // Prepare Response.
                    const char* reply_p1 = "HTTP/1.1 200 OK\r\n"
                    "Proxy-agent: CyberWeb\r\n"
                    "Content-Disposition: attachment; filename=\"CyberWeb.crt\"\r\n"
                    "Content-Length: ";
                    BufferStorage* reply_buffer = createBufferStorage();
                    appendToBuffer(reply_buffer, reply_p1, 113);
                    appendToBuffer(reply_buffer, file_size_buffer, strlen(file_size_buffer));
                    appendToBuffer(reply_buffer, "\r\n\r\n", 4);
                    appendToBuffer(reply_buffer, file_content, file_size);
                    appendToBuffer(reply_buffer, "\r\n\r\n", 4);
                    
                    // Reply.
                    printf("\n\nGetCert.\n\n");
                    send(client->socket, reply_buffer->data, reply_buffer->size, 0);
                }else{
                    printf("[*] Http Request is not supported yet.\n");
                }
                close(client);
            }
        }else{
            printf("[*] Invalid Request.\n");
            close(client);
        }
    }
    
    return 0;
}
