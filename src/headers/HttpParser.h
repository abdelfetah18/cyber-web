#ifndef HTTP_PARSER
#define HTTP_PARSER

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define uint unsigned int


typedef enum {
    HM_GET,
    HM_POST,
    HM_CONNECT,
    HM_INVALID
} HTTP_METHOD;

typedef enum {
    HV_HTTP_1_1,
    HV_NOT_SUPPORTED
} HTTP_VERSION;

typedef struct {
    char* field_name;
    char* field_value;
} HttpHeader;

typedef struct HttpHeaders {
    HttpHeader header;
    struct HttpHeaders* next;
} HttpHeaders;

typedef struct {
    HTTP_METHOD method;
    char* path_or_resource;
    HTTP_VERSION http_version;
    HttpHeaders *headers;
    char* body;
} HttpRequest;

typedef enum {
    PS_REQUEST_LINE,
    PS_FIELD_NAME,
    PS_FIELD_VALUE,
    PS_BODY,
    PS_DONE,
    PS_HTTP_VERSION_NOT_SUPPORTED,
    PS_INVALID
} ParserState;

typedef struct {
    ParserState parser_state;
    HttpRequest* http_request;
    uint request_line_length;
    uint headers_length;
    uint body_length;
    char* last_field_name;
    char* last_field_value;
} Parser;

void parseHttpRequest(Parser* parser,char* input);

HttpHeaders* createHeader(HttpHeader header);
void appendHeader(HttpHeaders** arr,HttpHeader header);
char* getHeader(HttpHeaders* arr,char* field_name);

// Helper functions
bool isEndOfLine(char* input,uint pos);
bool skipWhiteSpace(char* input,uint* pos);
HTTP_METHOD parseHttpMethod(char* input,uint* pos);
HTTP_VERSION parseHttpVersion(char* input,uint* pos);
char* parseFieldName(char* input,uint* pos);
char* parseFieldValue(char* input,uint* pos);
char* resize_and_cat(char* str1, char* str2);
Parser* createParser();

#endif