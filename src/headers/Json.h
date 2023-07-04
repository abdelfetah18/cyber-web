#ifndef JSON
#define JSON

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define uint unsigned int

// Lexer
typedef enum { 
    JT_OPEN_CURLY_BRACKET,
    JT_CLOSE_CURLY_BRACKET,
    JT_OPEN_SQUARE_BRACKET,
    JT_CLOSE_SQUARE_BRACKET,
    JT_COL,
    JT_COMMA,
    JT_STRING,
    JT_NUMBER,
    JT_BOOLEAN,
    JT_NULL,
    JT_EOF
} JSON_TOKENS;

typedef struct JSON_TOKEN {
    JSON_TOKENS type;
    char* value;
    uint start_pos;
    uint length;
} JSON_TOKEN;

typedef struct {
    char* data;
    uint size;
} JSON_INPUT;

typedef struct {
    JSON_INPUT input;
    uint pos;
} JSON_LEXER;

JSON_LEXER* json_lexer_create_lexer(JSON_INPUT input);
JSON_TOKEN* json_lexer_create_token(JSON_TOKENS type,char* value,uint start_pos,uint length);
bool json_lexer_is_end(JSON_LEXER* lexer);
char json_lexer_current_char(JSON_LEXER* lexer);
bool json_lexer_is_digit(JSON_LEXER* lexer);
void json_lexer_consume(JSON_LEXER* lexer);
void json_lexer_skip_white_space(JSON_LEXER* lexer);
JSON_TOKEN* json_lexer_next(JSON_LEXER* lexer);

// Parser
typedef enum { 
    JSON_TYPE_STRING,
    JSON_TYPE_NUMBER,
    JSON_TYPE_ARRAY,
    JSON_TYPE_OBJECT,
    JSON_TYPE_BOOLEAN,
    JSON_TYPE_NULL
} JSON_TYPES;

typedef struct JSON_ARRAY {
  uint key;
  struct JSON_VALUE* value;
  struct JSON_ARRAY* next;  
} JSON_ARRAY;

typedef struct JSON_OBJECT {
  char* key;
  struct JSON_VALUE* value;
  struct JSON_OBJECT* next;  
} JSON_OBJECT;

typedef struct JSON_PARSER {
    struct JSON_LEXER* lexer;
    struct JSON_TOKEN* current_token;
    struct JSON_OBJECT* object;
} JSON_PARSER;

typedef struct JSON_VALUE {
    JSON_TYPES type;
    union {
        char* string;
        struct JSON_ARRAY* array;
        struct JSON_OBJECT* object;
        bool boolean;
        int number;
    } value;
} JSON_VALUE;


JSON_VALUE* create_json_value(JSON_TYPES type,void* value);
JSON_OBJECT* create_json_object(char* key, JSON_VALUE* value);
JSON_ARRAY* create_json_array(int key, JSON_VALUE* value);
JSON_OBJECT* json_object_set(JSON_OBJECT* json_object,char* key, JSON_VALUE* value);
JSON_ARRAY* json_array_set(JSON_ARRAY* json_object,int key, JSON_VALUE* value);

JSON_PARSER* create_json_parser(JSON_INPUT input);
bool parse_json(JSON_PARSER* parser);
void consume_token(JSON_PARSER* parser);
JSON_OBJECT* parse_json_object(JSON_PARSER* parser);
JSON_ARRAY* parse_array_object(JSON_PARSER* parser);

// JSON Object is a Map KeyValue pairs.
// Key is always a string.
// Value can be one of the following: string, number, object, array, boolean, null

void print_indent(int indent);
void print_json_array(int indent,JSON_ARRAY* value);
void print_json_object(int indent,JSON_OBJECT* value);
void print_json_value(int indent,JSON_VALUE* value);
void json_pretty_print(JSON_OBJECT* object);

#endif