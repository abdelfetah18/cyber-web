#include "headers/RequestsManager.h"
#include "headers/HttpParser.h"
#include "headers/CertificateManager.h"


// Helper functions
char* getHostNameFromTargetHostname(char* target_hostname){
    int len = 0;
    while(target_hostname[len] != ':')
        len += 1;
    
    char* hostname = malloc((sizeof(char) * len) + 1);
    
    strncpy(hostname, target_hostname, len);
    hostname[len] = '\0';
    return hostname;
}

HandleDataResult handleData(char* input){
    HandleDataResult result;
    Parser* parser = createParser();
    parseHttpRequest(parser, input);
    
    if(parser->parser_state == PS_INVALID){
        // TODO: Return and close the connection.
        result.is_valid = false;
        return result;
    }

    if(parser->parser_state == PS_DONE){
        printf("[*] Parsing Done.\n");
        if(parser->http_request->method == HM_CONNECT){
            printf("[*] Its a CONNECT Request.\n");
            // Get for the certificate.
            char* host_name = getHostNameFromTargetHostname(parser->http_request->path_or_resource);
            char* certificate = getCertificate(host_name);
            // Return.
            result.is_valid = true;
            result.target_host_name = parser->http_request->path_or_resource;
            return result;
        }
    }else{
        printf("[*] Parsing is not OK. [state: %d]\n", parser->parser_state);
    }
}