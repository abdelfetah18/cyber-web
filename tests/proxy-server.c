#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024*8
#define MAX_CONNECTIONS 1
#define PORT 8080
#define uint unsigned int

#include <openssl/ssl.h>
#include <openssl/err.h>      


// =========================[ SSL ]===============================
#define FAIL    -1
int OpenConnection(const char *hostname, int port)
{
    int sd;
    struct hostent *host;
    struct sockaddr_in addr;
    if ( (host = gethostbyname(hostname)) == NULL )
    {
        perror(hostname);
        abort();
    }
    sd = socket(PF_INET, SOCK_STREAM, 0);
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = *(long*)(host->h_addr_list[0]);
    if ( connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0 )
    {
        close(sd);
        perror(hostname);
        abort();
    }
    return sd;
}
SSL_CTX* InitCTX(void)
{
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
    return ctx;
}
void ShowCerts(SSL* ssl)
{
    X509 *cert;
    char *line;
    cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
    if ( cert != NULL )
    {
        printf("Server certificates:\n");
        line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
        printf("Subject: %s\n", line);
        free(line);       /* free the malloc'ed string */
        line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
        printf("Issuer: %s\n", line);
        free(line);       /* free the malloc'ed string */
        X509_free(cert);     /* free the malloc'ed certificate copy */
    }
    else
        printf("Info: No client certificates configured.\n");
}
//===============================================================
struct sockaddr_in server_address;

int createTCPServer(){
    memset(&server_address, 0, sizeof(struct sockaddr_in));
    server_address.sin_port = htons(PORT);
    server_address.sin_family = AF_INET;
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        perror("socket()");
        exit(1);
    }

    int b = bind(socket_fd, (struct sockaddr *) &server_address, sizeof(struct sockaddr));
    if(b == -1){
        perror("bind()");
        exit(1);
    }

    int l = listen(socket_fd, MAX_CONNECTIONS);
    if(l == -1){
        perror("listen()");
        exit(1);
    }
    return socket_fd;
}

int acceptClients(int socket_fd,struct sockaddr_in* client_address){
    memset(client_address, 0, sizeof(struct sockaddr_in));
    int len = sizeof(struct sockaddr_in);
    int client = accept(socket_fd, (struct sockaddr*) client_address, &len);
    if(client == -1){
        perror("accept");
        exit(1);
    }
    return client;
}

void receiveDataFromClient(int client,char* buff){
    memset(buff, 0, MAX_BUFFER_SIZE);
    int r = recv(client, buff, MAX_BUFFER_SIZE, 0);
    if(r == -1){
        perror("recv");
    }
}

typedef enum ParseResult {
    CONNECT_REQUEST,
    INVALID_DATA
} ParseResult;

bool consume(char* input, char* data){
    int i = 0;
    while(data[i] != '\0'){
        if(input[i] != data[i])
            return false;
        i++;
    }
    return true;
}

bool is_white_space(char input){
    return input == ' ';
}

void skip_white_space(char* input,uint* pos){
    while(is_white_space(input[*pos]))
        *pos++;
}



char* parseData(char* buff){
    uint pos = 0;
    bool is_connect = consume(buff, "CONNECT");
    if(!is_connect)
        return INVALID_DATA;
    pos += 8;
    skip_white_space(buff, &pos);

    uint len = 0;
    while(!is_white_space(buff[pos+len])){
        len++;
    }
    char* host = malloc(len+1);
    memcpy(host,buff+pos, len);
    host[len] = '\0';
    printf("host: %s\n", host);
    // TODO: Complete the parsing.
    return host;
}

SSL* establishSSLConnection(char* host){
    SSL_CTX *ctx;
    int server;
    SSL *ssl;
    char buf[1024];
    char acClientRequest[1024] = {0};
    int bytes;
    char *hostname, *portnum;
    
    // TODO: Make the function accept hostname and port.
    int pos = 0;
    while(host[pos] != ':')
        pos++;
    host[pos] = '\0';

    SSL_library_init();
    hostname = host;
    portnum = host+pos+1;
    ctx = InitCTX();
    server = OpenConnection(hostname, atoi(portnum));
    ssl = SSL_new(ctx);      /* create new SSL connection state */
    SSL_set_fd(ssl, server);    /* attach the socket descriptor */
    if ( SSL_connect(ssl) == FAIL )   /* perform the connection */
        ERR_print_errors_fp(stderr);
    else{
        // printf("Connected with %s encryption\n", SSL_get_cipher(ssl));
        // ShowCerts(ssl);        /* get any certs */
        return ssl;
    }
    return NULL;
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

void configure_context(SSL_CTX *ctx)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, "config/cert.pem", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "config/key.pem", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

SSL* acceptClientSSL(int client){
    SSL_CTX *ctx;
    SSL* client_ssl;

    ctx = create_context();
    configure_context(ctx);

    client_ssl = SSL_new(ctx);
    SSL_set_fd(client_ssl, client);

    if (SSL_accept(client_ssl) <= 0) {
        ERR_print_errors_fp(stderr);
        return NULL;
    }

    return client_ssl;
}

int main()
{
    // 1. Create a TCP Server.
    int server = createTCPServer();
    printf("Server is running on port: %d\n", PORT);
    
    ACCEPT:
    // 2. Accept Client Connection.
    struct sockaddr_in client_address;
    int client = acceptClients(server, &client_address);
    
    // 3. Receive and Parse The Client data.
    char buff[MAX_BUFFER_SIZE];
    receiveDataFromClient(client, buff);
    printf("data:\n%s\n", buff);
    char* host = parseData(buff);
    
    // 4. If it is a 'CONNECT' Request then Establish a SSL Connection with the target host and reply with status 200 Connection Established.
    SSL* ssl = establishSSLConnection(host);
    const char* reply = "HTTP/1.1 200 Connection established\r\nProxy-agent: <proxy-agent-name>\r\n\r\n";
    send(client, reply, strlen(reply), 0);

    // 5. Accept Client SSL connection.
    SSL* client_ssl = acceptClientSSL(client);
    if(client_ssl != NULL){
        printf("client ssl success.\n");
        // 1. Recieve client http request.
        char buf[MAX_BUFFER_SIZE];
        memset(buf, 0, MAX_BUFFER_SIZE);
        uint bytes = SSL_read(client_ssl, buf, sizeof(buf));
        // Forward http request to the target server.
        SSL_write(ssl, buf, strlen(buf));

        // FIXME: Parse Http Request to know the end of the request.
        while(1){
            // Receive the response.
            char result_buf[MAX_BUFFER_SIZE];
            memset(result_buf, 0, MAX_BUFFER_SIZE);
            bytes = SSL_read(ssl, result_buf, MAX_BUFFER_SIZE);
            // Forward response to client.
            SSL_write(client_ssl, result_buf, bytes);

            // Check if the SSL connection has been shutdown
            int ssl_shutdown = SSL_get_shutdown(ssl);
            if (ssl_shutdown & SSL_RECEIVED_SHUTDOWN) {
                // Client has disconnected
                SSL_shutdown(ssl);
                SSL_free(ssl);
                SSL_shutdown(client_ssl);
                SSL_free(client_ssl);
                close(client);
                printf("client connection closed.\n");
                break;
            }
        }
    }

    goto ACCEPT;

    return 0;
}
