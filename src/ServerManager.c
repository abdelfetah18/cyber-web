#include "headers/ServerManager.h"

Server* createServer(int port){
    Server* server = malloc(sizeof(Server));
    memset(server, 0, sizeof(Server));
    
    server->address = malloc(sizeof(struct sockaddr_in));
    memset(server->address, 0, sizeof(struct sockaddr_in));

    server->address->sin_port = htons(port);
    server->address->sin_family = AF_INET;
    SOCKET_HANDLE socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0)
        perror("socket()");

    server->socket = socket_fd;
    if(bind(socket_fd, (struct sockaddr *) server->address, sizeof(struct sockaddr)) < 0){
        perror("bind()");
        exit(1);
    }

    if(listen(socket_fd, MAX_CONNECTIONS) < 0)
        perror("listen()");
    
    return server;
}

Client* acceptConnections(SOCKET_HANDLE server_socket_fd){
    Client* client = malloc(sizeof(Client));
    memset(client, 0, sizeof(Server));
    
    client->address = malloc(sizeof(struct sockaddr_in));
    memset(client->address, 0, sizeof(struct sockaddr_in));

    int len = sizeof(struct sockaddr_in);
    SOCKET_HANDLE client_socket_fd = accept(server_socket_fd, (struct sockaddr*) client->address, &len);
    if(client_socket_fd < 0)
        perror("accept()");
    
    client->socket = client_socket_fd;

    return client;
}

RecvData receiveData(SOCKET_HANDLE client_socket_fd){
    RecvData recv_data = { 0, 0 };
    
    recv_data.data = malloc(sizeof(char) * MAX_BUFFER_SIZE);
    memset(recv_data.data, 0, MAX_BUFFER_SIZE);

    recv_data.size = recv(client_socket_fd, recv_data.data, MAX_BUFFER_SIZE, 0);
    if(recv_data.size < 0)
        perror("recv()");
    
    return recv_data;
}

Host* connectToHost(char* hostname){
    Host* host_socket = malloc(sizeof(Host));
    host_socket->address = malloc(sizeof(struct sockaddr_in));
    
    int pos = 0;
    while(hostname[pos] != ':')
        pos++;
    hostname[pos] = '\0';
    char* str_port = hostname + pos + 1;
    
    int port = atoi(str_port);

    struct hostent *host;

    host = gethostbyname(hostname);
    if(host == NULL){
        perror(hostname);
        abort();
    }

    host_socket->socket = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(host_socket->address, 0, sizeof(struct sockaddr_in));
    
    host_socket->address->sin_family = AF_INET;
    host_socket->address->sin_port = htons(port);
    host_socket->address->sin_addr.s_addr = *(long*)(host->h_addr_list[0]);
    
    printf("[*] Trying to connect in %s on port %d\n", hostname, port);

    if(connect(host_socket->socket, (struct sockaddr*) host_socket->address, sizeof(struct sockaddr)) != 0){
        close(host_socket->socket);
        perror(hostname);
        abort();
    }
    return host_socket;
}


SSL_CTX *create_context()
{
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    method = TLS_server_method();

    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    return ctx;
}

void configure_context(SSL_CTX *ctx,char* hostname)
{
    char* path = "hosts/";
    char* target_host_file = malloc((sizeof(char) * strlen(hostname)) + 11);
    strcpy(target_host_file, path);
    strcpy(target_host_file+6, hostname);
    strcpy(target_host_file+6+strlen(hostname), ".crt");

    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, target_host_file, SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    strcpy(target_host_file+6+strlen(hostname), ".key");
    if (SSL_CTX_use_PrivateKey_file(ctx, target_host_file, SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

SSL* acceptSSLConnection(SOCKET_HANDLE connection, char* hostname){
    SSL_CTX *ctx;
    SSL* ssl_connection;
    
    ctx = create_context();
    configure_context(ctx, hostname);

    ssl_connection = SSL_new(ctx);
    SSL_set_fd(ssl_connection, connection);

    if (SSL_accept(ssl_connection) <= 0) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return ssl_connection;
}

SSL* upgradeToSSL(SOCKET_HANDLE connection){
    SSL *ssl;

    // Init SSL Context
    SSL_METHOD *method;
    SSL_CTX *ctx;
    OpenSSL_add_all_algorithms();  /* Load cryptos, et.al. */
    SSL_load_error_strings();   /* Bring in and register error messages */
    method = TLSv1_2_client_method();  /* Create new client-method instance */
    ctx = SSL_CTX_new(method);   /* Create new context */
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }


    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, connection);    /* attach the socket descriptor */
    if ( SSL_connect(ssl) == -1 )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else{
        // printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        // ShowCerts(ssl);        /* get any certs */
        return ssl;
    }
    return NULL;
}

void sendData(SOCKET_HANDLE socket_fd){
    // TODO: In case of Http Client.   
}