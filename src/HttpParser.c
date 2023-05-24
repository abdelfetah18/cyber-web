#include "headers/HttpParser.h"

// Helper functions
bool isEndOfLine(char* input,uint pos){
    return (input[pos] == '\r' && input[pos+1] == '\n');
}

bool skipWhiteSpace(char* input,uint* pos){
    if(!(input[*pos] == ' '))
        return false;
    while(input[*pos] == ' '){
        *pos += 1;
    }
    return true;
}

HTTP_METHOD parseHttpMethod(char* input,uint* pos){
    if(strncmp(input+*pos, "GET", 3) == 0){
        *pos += 3;
        return HM_GET;
    }
    
    if(strncmp(input+*pos, "POST", 4) == 0){
        *pos += 4;
        return HM_POST;
    }
    
    if(strncmp(input+*pos, "CONNECT", 7) == 0){
        *pos += 7;
        return HM_CONNECT;
    }
    // TODO: Support other methods as well.

    return HM_INVALID;
}

HTTP_VERSION parseHttpVersion(char* input,uint* pos){
    if(strncmp(input+*pos, "HTTP/1.1", 8) == 0){
        *pos += 8;
        return HV_HTTP_1_1;
    }

    return HV_NOT_SUPPORTED;
}

// Passing a pointer to pos to be able to update the acctual pos.
char* parseFieldName(char* input,uint* pos){
    uint len = 0;
    while(input[*pos + len] != '\0' && !isEndOfLine(input, *pos + len) && input[*pos + len] != ' ' && input[*pos + len] != ':'){
        len += 1;
    }

    char* token = malloc((sizeof(char) * (len)) + 1);
    strncpy(token, input + *pos, len);
    token[len] = '\0';
    *pos += len;
    return token;
}

char* parseFieldValue(char* input,uint* pos){
    uint len = 0;
    while(input[*pos + len] != '\0' && !isEndOfLine(input, *pos + len)){
        len += 1;
    }

    char* token = malloc((sizeof(char) * (len)) + 1);
    strncpy(token, input + *pos, len);
    token[len] = '\0';
    *pos += len;
    return token;
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

// Here we assume that we are given the full request.
void parseHttpRequest(Parser* parser,char* input){
    char* http_raw_request = input;
    if(parser->parser_state == PS_REQUEST_LINE){
        uint pos = 0;
        while(input[pos] != '\0' && !isEndOfLine(input, pos)){
            // Parse HttpMethod
            // NOTE: We are passing a pointer to a position which mean we also consume that token.
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
            
            // Requested resource.
            uint len = 0;
            while(input[pos+len] != '\0' && input[pos+len] != ' ')
                len += 1;
            
            char* path = malloc(sizeof(char) * (len+1));
            parser->http_request->path_or_resource = path;
            strncpy(path, input+pos, len);
            path[len] = '\0';
            pos += len;

            if(!skipWhiteSpace(input, &pos)){
                parser->parser_state = PS_INVALID;
                return;
            }
            // Parse HttpVersion
            HTTP_VERSION http_version = parseHttpVersion(input, &pos);
            if(http_version == HV_NOT_SUPPORTED){
                parser->parser_state = PS_HTTP_VERSION_NOT_SUPPORTED;
                return;
            }
        }
        if(isEndOfLine(input, pos)){
            // NOTE: the request line length without the end of line characters.
            parser->request_line_length = pos;
            parser->parser_state = PS_FIELD_NAME;
            http_raw_request += (pos + 2);
        }
    }
    if(parser->parser_state == PS_FIELD_NAME || parser->parser_state == PS_FIELD_VALUE){
        // NOTE: If the parser state was at field_name or field_value we need to check the saved value
        //       and continue parsing and adding it to that value.
        uint pos = 0;
        /*
        
            FieldName -> (WhiteSpace) -> SemiCol -> (WhiteSpace) -> FieldValue -> HttpBody

        */
        while(http_raw_request[pos] != '\0' && !(isEndOfLine(http_raw_request, pos) && isEndOfLine(http_raw_request, pos + 2))){
            skipWhiteSpace(http_raw_request, &pos);
            if(http_raw_request[pos] == ':'){
                pos += 1;
                skipWhiteSpace(http_raw_request, &pos);
                parser->parser_state = PS_FIELD_VALUE;
            }

            if(isEndOfLine(http_raw_request, pos)){
                pos += 2;
            }

            if(http_raw_request[pos] == '\0'){
                return;
            }

            if(parser->parser_state == PS_FIELD_NAME){
                char* field_name = parseFieldName(http_raw_request, &pos);
                if(parser->last_field_name != NULL){
                    char* new_field_name = resize_and_cat(parser->last_field_name, field_name);
                    field_name = new_field_name;
                    parser->last_field_name = new_field_name;
                }else{
                    parser->last_field_name = field_name;
                }

                skipWhiteSpace(http_raw_request, &pos);
                if(http_raw_request[pos] == ':'){
                    pos += 1;
                    skipWhiteSpace(http_raw_request, &pos);
                    parser->parser_state = PS_FIELD_VALUE;
                }else if(http_raw_request[pos] == '\0'){
                    return;
                }else{
                    parser->parser_state = PS_INVALID;
                    return;
                }
            }

            if(parser->parser_state == PS_FIELD_VALUE){
                char* field_value = parseFieldValue(http_raw_request, &pos);
                if(parser->last_field_value != NULL){
                    char* new_field_value = resize_and_cat(parser->last_field_value, field_value);
                    field_value = new_field_value;
                    parser->last_field_value = new_field_value;
                }else{
                    parser->last_field_value = field_value;
                }
                if(!isEndOfLine(http_raw_request, pos)){
                    return;
                }else{
                    HttpHeader http_header;
                    http_header.field_name = parser->last_field_name;
                    http_header.field_value = parser->last_field_value;
                    appendHeader(&(parser->http_request->headers), http_header);
                    parser->last_field_name = NULL;
                    parser->last_field_value = NULL;
                    parser->parser_state = PS_FIELD_NAME;
                }
            }
        }    

        if(http_raw_request[pos] == '\0'){
            return;
        }else if(isEndOfLine(http_raw_request, pos) && isEndOfLine(http_raw_request, pos + 2)){
            http_raw_request += (pos + 4);
        }else{
            // Return and close the connection expected Double EndOfLine.
            parser->parser_state = PS_INVALID;
            return;
        }

        // TODO: Implement a function to check if the method is supported.
        if(parser->http_request->method == HM_GET || parser->http_request->method == HM_CONNECT){
            parser->parser_state = PS_DONE;
            return;
        }

        parser->parser_state == PS_BODY;
    }
    
    if(parser->parser_state == PS_BODY){
        // TODO: Add support for json and xml.
        // FIXME: Adding support for Chunked Content-Type where the size of the body is not stored at 'Content-Length' header field.
        uint body_length = atoi(getHeader(parser->http_request->headers, "Content-Length"));
        uint pos = 0;
        while(http_raw_request[pos] != '\0' && pos < body_length){
            pos++;
        }

        char* body = malloc((sizeof(char) * body_length) + 1);
        memcpy(body, http_raw_request, pos);
        body[pos] == '\0';
        parser->http_request->body = body;
        if(pos == body_length)
            parser->parser_state == PS_DONE;
        return;
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