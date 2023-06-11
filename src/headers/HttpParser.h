#ifndef HTTP_PARSER
#define HTTP_PARSER

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define uint unsigned int

#include "BufferStorage.h"

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
    char* resource_path;
    HTTP_VERSION http_version;
    HttpHeaders *headers;
    BufferStorage* body;
} HttpRequest;

typedef struct {
    HTTP_VERSION http_version;
    uint status_code;
    char* reason_phrase;
    HttpHeaders *headers;
    BufferStorage* body;
} HttpResponse;


typedef enum {
    PS_REQUEST_LINE,
    PS_RESPONSE_LINE,
    PS_FIELD_NAME,
    PS_FIELD_VALUE,
    PS_BODY,
    PS_DONE,
    PS_HTTP_VERSION_NOT_SUPPORTED,
    PS_INVALID
} ParserState;

typedef struct {
    ParserState parser_state;
    enum { S_REQUEST, S_RESPONSE } state;
    HttpRequest* http_request;
    HttpResponse* http_response;
    uint request_line_length;
    uint response_line_length;
    uint headers_length;
    uint body_length;
    uint expected_body_length;
    long uint last_saved_chunk_len;
    long uint last_chunk_size;
    BufferStorage* last_chunk_size_buffer;
    char* last_field_name;
    char* last_field_value;
    enum { BS_START, BS_CHUNK_SIZE, BS_CHUNK_CRLF_1, BS_CHUNK_DATA, BS_CHUNK_CRLF_2, BS_CONTENT_LENGTH, BS_DONE } body_state; // FIXME: This is a hack but find a better way to do it.
} Parser;

typedef struct {
    char* data;
    uint size;
} ParserInput;

typedef struct {
    Parser* parser;
    BufferStorage* full_raw_data;
} ParserResult;


void parseHttpRequest(Parser* parser,ParserInput* input);
void parseHttpResponse(Parser* parser,ParserInput* input);

HttpHeaders* createHeader(HttpHeader header);
void appendHeader(HttpHeaders** arr,HttpHeader header);
char* getHeader(HttpHeaders* arr,char* field_name);

// Helper functions
ParserInput createParserInput(char* data,uint size);
char* getParsingState(ParserState state);
bool isEndOfLine(ParserInput* input,uint pos);
bool isDoubleEndOfLine(ParserInput* input,uint pos);
bool isEOF(ParserInput* input,uint pos);
bool match(ParserInput* input,uint pos,char c);
bool consume(ParserInput* input,uint* pos,char c);
bool skipWhiteSpace(ParserInput* input,uint* pos);
HTTP_METHOD parseHttpMethod(ParserInput* input,uint* pos);
char* parseResourcePath(ParserInput* input,uint* pos);
char* parseReasonPhrase(ParserInput* input,uint* pos);
HTTP_VERSION parseHttpVersion(ParserInput* input,uint* pos);
void parseRequestLine(Parser* parser,ParserInput* input);
void parseResponseLine(Parser* parser,ParserInput* input);
char* parseFieldName(ParserInput* input,uint* pos);
char* parseFieldValue(ParserInput* input,uint* pos);
uint parseHeaders(Parser* parser,ParserInput* input);
bool doesHaveBody(HTTP_METHOD http_method);
uint parseChunckedSize(Parser* parser,ParserInput* input,uint* pos);
uint parseChunkData(Parser* parser,ParserInput* input,uint* pos);
uint parseBody(Parser* parser,ParserInput* input);

char* resize_and_cat(char* str1, char* str2);
Parser* createParser();

ParserResult* createParserResult(Parser* parser,BufferStorage* full_raw_data);

void hexPrint(char* data,uint size);

#endif