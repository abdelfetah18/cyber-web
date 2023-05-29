#include "headers/HttpParser.h"

void hexPrint(char* data,uint size){
    printf("\n==============================[ SIZE: %d bytes ]==============================\n", size);
    for(uint i = 0; i < size; i++){
        if(i % 16 == 0)
            printf("\n");
        printf("%x ", data[i]);
    }
    printf("\n====================================================================\n");
}

// Helper functions
char* getParsingState(ParserState state){
    switch (state)
    {
    case PS_BODY:
        return "PS_BODY";
        break;
    case PS_DONE:
        return "PS_DONE";
        break;
    case PS_FIELD_NAME:
        return "PS_FIELD_NAME";
        break;
    case PS_FIELD_VALUE:
        return "PS_FIELD_VALUE";
        break;
    case PS_HTTP_VERSION_NOT_SUPPORTED:
        return "PS_HTTP_VERSION_NOT_SUPPORTED";
        break;
    case PS_INVALID:
        return "PS_INVALID";
        break;
    case PS_REQUEST_LINE:
        return "PS_REQUEST_LINE";
        break;
    case PS_CHUNK:
        return "PS_CHUNK";
        break;
    default:
        return "State not exist";
        break;
    }
}

bool isEndOfLine(ParserInput* input,uint pos){
    return (input->data[pos] == '\r' && input->data[pos+1] == '\n');
}

bool skipWhiteSpace(ParserInput* input,uint* pos){
    if(!(input->data[*pos] == ' '))
        return false;
    while(input->data[*pos] == ' '){
        *pos += 1;
    }
    return true;
}

bool isDoubleEndOfLine(ParserInput* input,uint pos){
    return isEndOfLine(input, pos) && isEndOfLine(input, pos + 2);
}

bool isEOF(ParserInput* input,uint pos){
    return !(pos < input->size);
}

bool match(ParserInput* input,uint pos,char c){
    return input->data[pos] == c;
}

bool consume(ParserInput* input,uint* pos,char c){
    if(input->data[*pos] == c){
        *pos += 1;
        return true;
    }
    return false;
}

HTTP_METHOD parseHttpMethod(ParserInput* input,uint* pos){
    if(strncmp(input->data+*pos, "GET", 3) == 0){
        *pos += 3;
        return HM_GET;
    }
    
    if(strncmp(input->data+*pos, "POST", 4) == 0){
        *pos += 4;
        return HM_POST;
    }
    
    if(strncmp(input->data+*pos, "CONNECT", 7) == 0){
        *pos += 7;
        return HM_CONNECT;
    }
    // TODO: Support other methods as well.

    return HM_INVALID;
}

HTTP_VERSION parseHttpVersion(ParserInput* input,uint* pos){
    if(strncmp(input->data+*pos, "HTTP/1.1", 8) == 0){
        *pos += 8;
        return HV_HTTP_1_1;
    }

    return HV_NOT_SUPPORTED;
}

// Passing a pointer to pos to be able to update the acctual pos.
char* parseFieldName(ParserInput* input,uint* pos){
    uint len = 0;
    while(!isEOF(input, *pos + len) && !isEndOfLine(input, *pos + len) && !match(input, *pos + len, ' ') && !match(input, *pos + len, ':'))
        len += 1;
    
    char* token = malloc((sizeof(char) * (len)) + 1);
    strncpy(token, input->data + *pos, len);
    token[len] = '\0';
    *pos += len;
    return token;
}

char* parseFieldValue(ParserInput* input,uint* pos){
    uint len = 0;
    while(!isEOF(input, *pos + len) && !isEndOfLine(input, *pos + len))
        len += 1;

    char* token = malloc((sizeof(char) * (len)) + 1);
    strncpy(token, input->data + *pos, len);
    token[len] = '\0';
    *pos += len;
    
    return token;
}

long uint parseChunckedSize(ParserInput* input,uint* pos){
    uint len = 0;
    while(*pos + len < input->size && !isEndOfLine(input, *pos + len))
        len += 1;
    
    char* size = malloc(sizeof(char) * (len+1));
    strncpy(size, input->data+(*pos), len);
    size[len] = '\0';
    
    *pos += len;

    char *endPtr;
    long uint decimalValue;

    decimalValue = strtol(size, &endPtr, 16);

    if (*endPtr != '\0') {
        printf("[*] Invalid hexadecimal string\n");
    }
    return decimalValue;
}

char* resize_and_cat(char* str1, char* str2){
    uint str1_len = strlen(str1);
    uint str2_len = strlen(str2);
    char* new_str = malloc(sizeof(char) * (str1_len + str2_len + 1));
    strncpy(new_str, str1, str1_len);
    strncpy(new_str + str1_len, str2, str2_len);
    return new_str;
}

Parser* createParser(){
    Parser* parser = malloc(sizeof(Parser));
    memset(parser, 0, sizeof(Parser));

    parser->http_request = malloc(sizeof(HttpRequest));
    memset(parser->http_request, 0, sizeof(HttpRequest));

    parser->http_response = malloc(sizeof(HttpResponse));
    memset(parser->http_response, 0, sizeof(HttpResponse));

    parser->http_request->body = createBufferStorage();
    parser->http_response->body = createBufferStorage();

    parser->parser_state = PS_REQUEST_LINE;
    return parser;
}

// Headers list maniplulation
void appendHeader(HttpHeaders** arr,HttpHeader header){
    // TODO: Use HashMap instead of a LinkedList for better performance.
    // Appending new item at the head can be more useful since we dont care about the order.
    HttpHeaders* new_header = malloc(sizeof(HttpHeaders));
    new_header->header = header;
    new_header->next = NULL;
    if(*arr == NULL){
        *arr = new_header;
    }else{
        new_header->next = *arr;
        *arr = new_header;
    }
}

char* getHeader(HttpHeaders* arr,char* field_name){
    HttpHeaders* ptr = arr;
    while(ptr != NULL){
        if(strcmp(ptr->header.field_name, field_name) == 0)
            return ptr->header.field_value;
        ptr = ptr->next;
    }
    return NULL;
}


HttpHeaders* createHeader(HttpHeader header){
    HttpHeaders* new_header = malloc(sizeof(HttpHeaders));
    new_header->header = header;
    new_header->next = NULL;
    return new_header;
}

char* parseResourcePath(ParserInput* input,uint* pos){
    uint len = 0;
    while(!isEOF(input, *pos + len) && !match(input, *pos + len, ' ') && !isEndOfLine(input, *pos + len))
        len += 1;
    
    char* resource_path = malloc(sizeof(char) * (len+1));
    strncpy(resource_path, input->data + *pos, len);
    resource_path[len] = '\0';
    *pos += len;

    return resource_path;
}


void parseRequestLine(Parser* parser,ParserInput* input){
    uint pos = 0;

   // Parse HttpMethod
    // NOTE: We are passing a pointer to a position which mean we also consume that token.
    printf("[*] Parsing Http Method.\n");
    HTTP_METHOD http_method = parseHttpMethod(input, &pos);
    if(http_method == HM_INVALID){
        parser->parser_state = PS_INVALID;
        return;
    }
    parser->http_request->method = http_method;

    if(!skipWhiteSpace(input, &pos)){
        parser->parser_state = PS_INVALID;
        return;
    }
        
    // Parse Resource Path.
    printf("[*] Parsing Resource Path.\n");
    char* resource_path = parseResourcePath(input, &pos);
    parser->http_request->resource_path = resource_path;
    
    if(!skipWhiteSpace(input, &pos)){
        parser->parser_state = PS_INVALID;
        return;
    }

    // Parse http version
    printf("[*] Parsing Http Version.\n");
    HTTP_VERSION http_version = parseHttpVersion(input, &pos);
    if(http_version == HV_NOT_SUPPORTED){
        parser->parser_state = PS_HTTP_VERSION_NOT_SUPPORTED;
        return;
    }
    
    if(isEndOfLine(input, pos)){
        pos += 2;
        parser->headers_length = 0;
        parser->request_line_length = pos;
        parser->parser_state = PS_FIELD_NAME;
    }
}

// Here we assume that we are given the full request.
void parseHttpRequest(Parser* parser,ParserInput* input){
    parser->state = S_REQUEST;
    if(parser->parser_state == PS_REQUEST_LINE){
        parseRequestLine(parser, input);
        
        // FIXME: This code has a memory leak issues.
        input->data += parser->request_line_length;
        input->size -= parser->request_line_length;
    }

    if(parser->parser_state == PS_FIELD_NAME || parser->parser_state == PS_FIELD_VALUE){
        printf("[*] Parsing Http Headers.\n");
        
        // NOTE: If the parser state was at field_name or field_value we need to check the saved value
        //       and continue parsing and adding it to that value.
        
        // FieldName -> (WhiteSpace) -> SemiCol -> (WhiteSpace) -> FieldValue -> HttpBody

        uint cur_parsed_headers_len = parseHeaders(parser, input);
        printf("[*] Done with Http Headers.\n");

        // FIXME: This code has a memory leak issues.
        input->data += cur_parsed_headers_len;
        input->size -= cur_parsed_headers_len;
    }
    if(parser->parser_state == PS_BODY || parser->parser_state == PS_CHUNK){
        parseBody(parser, input);
    }  
}

char* parseStatusCode(ParserInput* input,uint* pos){
    char* status_code = malloc(sizeof(char) * 4);
    uint status_code_len = 0;
    while(input->data[*pos + status_code_len] >= '0' && input->data[*pos + status_code_len] <= '9'){
        status_code[status_code_len] = input->data[*pos + status_code_len];
        status_code_len += 1;
    }
    status_code[status_code_len] = '\0';
    *pos += status_code_len;
    return status_code;
}

char* parseReasonPhrase(ParserInput* input,uint* pos){
    uint len = 0;
    while(!isEOF(input, *pos + len) && !isEndOfLine(input, *pos + len))
        len += 1;
    
    char* reason_phrase = malloc(sizeof(char) * (len+1));
    strncpy(reason_phrase, input->data + *pos, len);
    reason_phrase[len] = '\0';
    *pos += len;

    return reason_phrase;
}

void parseResponseLine(Parser* parser,ParserInput* input){
    uint pos = 0;

    // Parse http version
    HTTP_VERSION http_version = parseHttpVersion(input, &pos);
    if(http_version == HV_NOT_SUPPORTED){
        parser->parser_state = PS_HTTP_VERSION_NOT_SUPPORTED;
        return;
    }
        
    if(!skipWhiteSpace(input, &pos)){
        parser->parser_state = PS_INVALID;
        return;
    }
    
    // Parse status code.
    uint status = atoi(parseStatusCode(input, &pos));
    parser->http_response->status_code = status; 

    if(!skipWhiteSpace(input, &pos)){
        parser->parser_state = PS_INVALID;
        return;
    }
        
    // Parse reason phrase.
    char* reason_phrase = parseReasonPhrase(input, &pos);
    parser->http_response->reason_phrase = reason_phrase;
    
    if(isEndOfLine(input, pos)){
        pos += 2;
        parser->headers_length = 0;
        parser->response_line_length = pos;
        parser->parser_state = PS_FIELD_NAME;
    }
}

bool doesHaveBody(HTTP_METHOD http_method){
    switch (http_method)
    {
        case HM_GET:
            return false;
        case HM_CONNECT:
            return false;
        case HM_POST:
            return true;
            
        // TODO: Add the other methods.
    }
}

uint parseHeaders(Parser* parser,ParserInput* input){
    uint pos = 0;
    while (!isEOF(input, pos) && !isDoubleEndOfLine(input, pos)){
        skipWhiteSpace(input, &pos);
        if(consume(input, &pos, ':')){
            skipWhiteSpace(input, &pos);
            parser->parser_state = PS_FIELD_VALUE;
        }

        if(isEndOfLine(input, pos))
            pos += 2;

        if(isEOF(input, pos))
            return;

        if(parser->parser_state == PS_FIELD_NAME){
            char* field_name = parseFieldName(input, &pos);
            if(parser->last_field_name != NULL){
                char* new_field_name = resize_and_cat(parser->last_field_name, field_name);
                field_name = new_field_name;
                parser->last_field_name = new_field_name;
            }else{
                parser->last_field_name = field_name;
            }
            skipWhiteSpace(input, &pos);
            if(consume(input, &pos, ':')){
                skipWhiteSpace(input, &pos);
                parser->parser_state = PS_FIELD_VALUE;
            }else if(isEOF(input, pos)){
                return;
            }else{
                parser->parser_state = PS_INVALID;
                return;
            }
        }

        if(parser->parser_state == PS_FIELD_VALUE){
            char* field_value = parseFieldValue(input, &pos);

            if(parser->last_field_value != NULL){
                char* new_field_value = resize_and_cat(parser->last_field_value, field_value);
                field_value = new_field_value;
                parser->last_field_value = new_field_value;
            }else{
                parser->last_field_value = field_value;
            }
            if(!isEndOfLine(input, pos)){
                return;
            }else{
                HttpHeader http_header;
                http_header.field_name = parser->last_field_name;
                http_header.field_value = parser->last_field_value;

                if(parser->state == S_REQUEST)
                    appendHeader(&(parser->http_request->headers), http_header);
                else
                    appendHeader(&(parser->http_response->headers), http_header);
                
                
                parser->last_field_name = NULL;
                parser->last_field_value = NULL;
                parser->parser_state = PS_FIELD_NAME;
            }
        }
    }

    if(pos >= input->size)
            return;
    else if(isDoubleEndOfLine(input, pos)){
        pos += 4;
    }else{
        // Return and close the connection expected Double EndOfLine.
        parser->parser_state = PS_INVALID;
        return;
    }
    
    parser->headers_length = pos;

    if(doesHaveBody(parser->http_request->method) || parser->state == S_RESPONSE){
        parser->body_length = 0;
        parser->parser_state = PS_BODY;
    }else{
        // FIXME: implement a state to tell if we are parsing request or response.
        parser->parser_state = PS_DONE;
    }

    return pos;
}

uint parseBody(Parser* parser,ParserInput* input){
    printf("[*] Parsing Message Body.\n");
    // hexPrint(input->data, input->size);
    // TODO: Add support for json and xml.
    // FIXME: Adding support for Chunked Content-Type where the size of the body is not stored at 'Content-Length' header field.
    char* content_length = NULL;
    if(parser->state == S_REQUEST)
        content_length = getHeader(parser->http_request->headers, "Content-Length");
    else
        content_length = getHeader(parser->http_response->headers, "Content-Length");
    
    uint pos = 0;
    if(content_length != NULL){
        printf("[*] There is a Content-Length header with value: %s.\n", content_length);
        uint body_length = atoi(content_length);
        while(!isEOF(input, pos))
            pos++;
        
        if(pos == 0 && body_length != 0){
            return;
        }

        if(parser->state == S_REQUEST)
            appendToBuffer(parser->http_request->body, input->data, pos);
        else
            appendToBuffer(parser->http_response->body, input->data, pos);
        
        parser->body_length += pos;
        
        if(parser->body_length == body_length)
            parser->parser_state = PS_DONE;
    }else{
        printf("[*] There is No Content-Length header.\n");
        char* transfer_encoding = NULL;
        if(parser->state == S_REQUEST)
            transfer_encoding = getHeader(parser->http_request->headers, "Transfer-Encoding");
        else
            transfer_encoding = getHeader(parser->http_response->headers, "Transfer-Encoding");
        
        if(transfer_encoding != NULL){
            printf("[*] Found Transfer-Encoding header.\n");
            printf("[*] LastState: %s\n", getParsingState(parser->parser_state));
            
            /*
                field_name -> : -> field_value -> CRLF
                ChunckSize -> CRLF -> ChunckData -> CRLF

                ParserState:
                    headers -> field_name | field_value
                    body -> normal_body | chuck_size | chunck_value
                parseRequestLine -> RequestLine
                parseHeaders -> Headers
                parseBody -> Body

            */
            
            if(parser->parser_state == PS_CHUNK){
                uint len = 0;
                while(pos + len < input->size && (parser->last_saved_chunk_len + len) < parser->last_chunk_size){
                    len += 1;
                }
                pos += len;
                parser->last_saved_chunk_len += len;
                if(parser->last_saved_chunk_len != parser->last_chunk_size){
                    parser->parser_state = PS_CHUNK;
                }else{
                    if(isEndOfLine(input->data, pos)){
                        pos += 2;
                    }
                    parser->parser_state = PS_BODY;
                }
            }else{
                long uint size = parseChunckedSize(input, &pos);
                parser->last_chunk_size = size;
                printf("[*] Chunk Size: %lu\n", size);
                if(size == 0){
                    printf("[*] Done with chunks.\n");
                    parser->parser_state = PS_DONE;
                    if(isEndOfLine(input->data, pos)){
                        pos += 2;
                    }
                }else{
                    if(isEndOfLine(input->data, pos)){
                        pos += 2;
                    }
                    uint len = 0;
                    while(pos + len < input->size && len < size){
                        len += 1;
                    }
                    pos += len;
                    parser->last_saved_chunk_len = len;
                    if(len != size){
                        parser->parser_state = PS_CHUNK;
                    }else{
                        if(isEndOfLine(input->data, pos)){
                            pos += 2;
                        }
                    }
                }
            }
            
            /*
            while(!isEOF(input, pos) && !isDoubleEndOfLine(input, pos))
                pos += 1;
            
            if(isDoubleEndOfLine(input, pos))
                parser->parser_state = PS_DONE;
            
            */
            
            if(parser->state == S_REQUEST)
                appendToBuffer(parser->http_request->body, input->data, input->size);
            else
                appendToBuffer(parser->http_response->body, input->data, input->size);
        }
    }

    return pos;
}

void parseHttpResponse(Parser* parser,ParserInput* input){
    parser->state = S_RESPONSE;
    printf("[*] Parse Http Response.\n");

    if(parser->parser_state == PS_REQUEST_LINE){
        parseResponseLine(parser, input);

        // FIXME: This code has a memory leak issues.
        input->data += parser->response_line_length;
        input->size -= parser->response_line_length;
    }
    
    if(parser->parser_state == PS_FIELD_NAME || parser->parser_state == PS_FIELD_VALUE){
        // NOTE: If the parser state was at field_name or field_value we need to check the saved value
        //       and continue parsing and adding it to that value.
        
        // FieldName -> (WhiteSpace) -> SemiCol -> (WhiteSpace) -> FieldValue -> HttpBody

        uint cur_parsed_headers_len = parseHeaders(parser, input);

        // FIXME: This code has a memory leak issues.
        input->data += cur_parsed_headers_len;
        input->size -= cur_parsed_headers_len;
    }

    if(parser->parser_state == PS_BODY || parser->parser_state == PS_CHUNK){
        parseBody(parser, input);
    }    
}

/*
    In order to be able to parse the steaming data we need to keep track of some properties
    like:
        - ParserState. (done, request_line, headers, body).
        - the parsed data like the avialable headers.
    We may have some struct like Parser which will have these properties.
    
    ParserState will determine the state of the parser where the parser may stoped last time.
    The states will be two:
        headers or body
    our buffer is 4096 length which mean the parsing may stop only in headers or body.
    but we are adding request_line as the start state of the parsing.    
*/