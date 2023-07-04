#include "headers/Json.h"

// JSON Lexer
JSON_TOKEN* json_lexer_create_token(JSON_TOKENS type,char* value,uint start_pos,uint length){
    JSON_TOKEN* token = malloc(sizeof(JSON_TOKEN));
    token->type = type;
    token->value = value;
    token->start_pos = start_pos;
    token->length = length;
    return token;
}

JSON_LEXER* json_lexer_create_lexer(JSON_INPUT input){
    JSON_LEXER* lexer = malloc(sizeof(JSON_LEXER));
    lexer->pos = 0;
    lexer->input = input;
    return lexer;
}

bool json_lexer_is_end(JSON_LEXER* lexer){
    return json_lexer_current_char(lexer) == '\0';
}

char json_lexer_current_char(JSON_LEXER* lexer){
    return lexer->input.data[lexer->pos];
}

void json_lexer_consume(JSON_LEXER* lexer){
    lexer->pos += 1;
}

bool json_lexer_is_digit(JSON_LEXER* lexer){
    return json_lexer_current_char(lexer) >= '0' && json_lexer_current_char(lexer) <= '9';
}

void json_lexer_skip_white_space(JSON_LEXER* lexer){
    while(!json_lexer_is_end(lexer) && (json_lexer_current_char(lexer) == ' ' || json_lexer_current_char(lexer) == '\r' || json_lexer_current_char(lexer) == '\n'))
        json_lexer_consume(lexer);
}

JSON_TOKEN* json_lexer_next(JSON_LEXER* lexer){
    if(json_lexer_is_end(lexer)){
        return json_lexer_create_token(JT_EOF, NULL, lexer->pos, 0);
    }

    json_lexer_skip_white_space(lexer);

    if(json_lexer_current_char(lexer) == '{'){
        json_lexer_consume(lexer);
        return json_lexer_create_token(JT_OPEN_CURLY_BRACKET, strdup("{"), lexer->pos, 1);
    }

    if(json_lexer_current_char(lexer) == '}'){
        json_lexer_consume(lexer);
        return json_lexer_create_token(JT_CLOSE_CURLY_BRACKET, strdup("}"), lexer->pos, 1);
    }

    // String
    if(json_lexer_current_char(lexer) == '"'){
        json_lexer_consume(lexer);
        uint start_pos = lexer->pos;
        // FIXME: Replace Escaped characters with its real value.
        while(!json_lexer_is_end(lexer) && json_lexer_current_char(lexer) != '"')
            json_lexer_consume(lexer);
        
        if(json_lexer_current_char(lexer) == '"'){
            uint length = lexer->pos - start_pos;
            json_lexer_consume(lexer);
            char* value = malloc(sizeof(char) * length + 1);
            memset(value, 0, length + 1);
            return json_lexer_create_token(JT_STRING, strncpy(value, lexer->input.data + start_pos, length), start_pos, length);
        }

    }

    // Number
    if(json_lexer_is_digit(lexer) || json_lexer_current_char(lexer) == '-'){
        uint start_pos = lexer->pos;
        json_lexer_consume(lexer);

        // FIXME: Add Support for fraction and exponent.
        while(!json_lexer_is_end(lexer) && json_lexer_is_digit(lexer))
            json_lexer_consume(lexer);

        uint length = lexer->pos - start_pos;

        char* value = malloc(sizeof(char) * length+1);
        memset(value, 0, length + 1);
        return json_lexer_create_token(JT_NUMBER, strncpy(value, lexer->input.data + start_pos, length), start_pos, length);
    }

    char* true_str = "true";
    if(strncmp(true_str, lexer->input.data, 4) == 0){
        lexer->pos += 4;
        return json_lexer_create_token(JT_BOOLEAN, strdup(true_str), lexer->pos, 4);
    }

    char* false_str = "false";
    if(strncmp(true_str, lexer->input.data, 5) == 0){
        lexer->pos += 5;
        return json_lexer_create_token(JT_BOOLEAN, strdup(false_str), lexer->pos, 5);
    }

    char* null_str = "null";
    if(strncmp(null_str, lexer->input.data, 4) == 0){
        lexer->pos += 4;
        return json_lexer_create_token(JT_BOOLEAN, strdup(null_str), lexer->pos, 4);
    }

    if(json_lexer_current_char(lexer) == ':'){
        json_lexer_consume(lexer);
        return json_lexer_create_token(JT_COL, strdup(":"), lexer->pos, 1);
    }

    if(json_lexer_current_char(lexer) == ','){
        json_lexer_consume(lexer);
        return json_lexer_create_token(JT_COMMA, strdup(","), lexer->pos, 1);
    }

    if(json_lexer_current_char(lexer) == '['){
        json_lexer_consume(lexer);
        return json_lexer_create_token(JT_OPEN_SQUARE_BRACKET, strdup("["), lexer->pos, 1);
    }

    if(json_lexer_current_char(lexer) == ']'){
        json_lexer_consume(lexer);
        return json_lexer_create_token(JT_CLOSE_SQUARE_BRACKET, strdup("]"), lexer->pos, 1);
    }
}

// JSON Parser
JSON_PARSER* create_json_parser(JSON_INPUT input){
    JSON_PARSER* parser = malloc(sizeof(JSON_PARSER));
    parser->lexer = json_lexer_create_lexer(input);
    parser->current_token = json_lexer_next(parser->lexer);
    return parser;
}

void consume_token(JSON_PARSER* parser){
    parser->current_token = json_lexer_next(parser->lexer);
}

JSON_VALUE* create_json_value(JSON_TYPES type,void* value){
    JSON_VALUE* json_value = malloc(sizeof(JSON_VALUE));
    json_value->type = type;

    switch(type){
        case JSON_TYPE_STRING:
            json_value->value.string = value;
            break;
        case JSON_TYPE_ARRAY:
            json_value->value.array = value;
            break;
        case JSON_TYPE_OBJECT:
            json_value->value.object = value;
            break;
        case JSON_TYPE_BOOLEAN:
            json_value->value.boolean = value;
            break;
        case JSON_TYPE_NUMBER:
            json_value->value.number = value;
            break;
        default:
            break;
    }

    return json_value;
}

JSON_OBJECT* create_json_object(char* key, JSON_VALUE* value){
    JSON_OBJECT* json_object = malloc(sizeof(JSON_OBJECT));
    
    json_object->key = key;
    json_object->value = value;
    json_object->next = NULL;
    
    return json_object;
}

JSON_ARRAY* create_json_array(int key, JSON_VALUE* value){
    JSON_ARRAY* json_array = malloc(sizeof(JSON_ARRAY));
    
    json_array->key = key;
    json_array->value = value;
    json_array->next = NULL;
    
    return json_array;
}

// This function always returns the head of the list.
JSON_OBJECT* json_object_set(JSON_OBJECT* json_object,char* key, JSON_VALUE* value){
    if(json_object == NULL)
        return create_json_object(key, value);
    
    JSON_OBJECT* cur = json_object;
    while (cur->next != NULL)
        cur = cur->next;
    
    cur->next = create_json_object(key, value);
    
    return json_object;
}

// This function always returns the head of the list.
JSON_ARRAY* json_array_set(JSON_ARRAY* json_array,int key, JSON_VALUE* value){
    if(json_array == NULL)
        return create_json_array(key, value);
    
    JSON_ARRAY* cur = json_array;
    while (cur->next != NULL)
        cur = cur->next;
    
    cur->next = create_json_array(key, value);
    
    return json_array;
}


JSON_ARRAY* parse_array_object(JSON_PARSER* parser){
    JSON_ARRAY* json_array = NULL;
    if(parser->current_token->type != JT_OPEN_SQUARE_BRACKET)
        return NULL;
    consume_token(parser);

    int key = 0;
    JSON_VALUE* value = NULL;

    while(parser->current_token->type != JT_EOF && parser->current_token->type != JT_CLOSE_SQUARE_BRACKET){
        switch (parser->current_token->type){
            case JT_OPEN_CURLY_BRACKET:
                value = create_json_value(JSON_TYPE_OBJECT, parse_json_object(parser));
                break;
            case JT_OPEN_SQUARE_BRACKET:
                value = create_json_value(JSON_TYPE_ARRAY, parse_array_object(parser));
                break;
            case JT_STRING:
                value = create_json_value(JSON_TYPE_STRING, parser->current_token->value);
                consume_token(parser);
                break;
            case JT_BOOLEAN:
                // FIXME: Until it get running i am using true bool.
                value = create_json_value(JSON_TYPE_BOOLEAN, true);
                consume_token(parser);
                break;
            case JT_NUMBER:
                value = create_json_value(JSON_TYPE_NUMBER, atoi(parser->current_token->value));
                consume_token(parser);
                break;
            default:
                break;
        }

        json_array = json_array_set(json_array, key, value);

        if(parser->current_token->type == JT_COMMA){
            consume_token(parser);
            key += 1;
        }
    }

    if(parser->current_token->type != JT_CLOSE_SQUARE_BRACKET)
        return NULL;
    consume_token(parser);

    return json_array;
}

JSON_OBJECT* parse_json_object(JSON_PARSER* parser){
    JSON_OBJECT* json_object = NULL;
    if(parser->current_token->type != JT_OPEN_CURLY_BRACKET)
        return NULL;

    char* key = NULL;
    JSON_VALUE* value = NULL;
    consume_token(parser);

    while(parser->current_token->type != JT_EOF && parser->current_token->type != JT_CLOSE_CURLY_BRACKET){
        if(parser->current_token->type != JT_STRING)
            return NULL;
        
        key = parser->current_token->value;
        consume_token(parser);
        if(parser->current_token->type != JT_COL)
            return NULL;
        consume_token(parser);

        switch (parser->current_token->type){
            case JT_OPEN_CURLY_BRACKET:
                value = create_json_value(JSON_TYPE_OBJECT, parse_json_object(parser));
                break;
            case JT_OPEN_SQUARE_BRACKET:
                value = create_json_value(JSON_TYPE_ARRAY, parse_array_object(parser));
                break;
            case JT_STRING:
                value = create_json_value(JSON_TYPE_STRING, parser->current_token->value);
                consume_token(parser);
                break;
            case JT_BOOLEAN:
                // FIXME: Until it get running i am using true bool.
                value = create_json_value(JSON_TYPE_BOOLEAN, true);
                consume_token(parser);
                break;
            case JT_NUMBER:
                value = create_json_value(JSON_TYPE_NUMBER, atoi(parser->current_token->value));
                consume_token(parser);
                break;
            default:
                break;
        }

        if(parser->current_token->type == JT_COMMA)
            consume_token(parser);
            
        json_object = json_object_set(json_object, key, value);
    }

    if(parser->current_token->type != JT_CLOSE_CURLY_BRACKET)
        return NULL;
    consume_token(parser);
    
    return json_object;
}

bool parse_json(JSON_PARSER* parser){
    parser->object = parse_json_object(parser);
    if(parser->object == NULL)
        return false;
    return true;
}



void print_json_array(int indent,JSON_ARRAY* array){
    printf("[ ");
    JSON_ARRAY* cur = array;
    while(cur != NULL){
        if(array != cur)
            printf(", ");
        print_json_value(indent, cur->value);
        cur = cur->next;
    }
    printf("]");
}

void print_indent(int indent){
    for(int i = 0; i < indent; i++)
        for(int j = 0; j < 2; j++)
           printf(" ");
}

void print_json_object(int indent,JSON_OBJECT* object){
    // print_indent(indent);
    printf("{\n");
    JSON_OBJECT* cur = object;
    while(cur != NULL){
        if(object != cur)
            printf(",\n");
        print_indent(indent + 1);
        printf("%s : ", cur->key);
        print_json_value(indent, cur->value);
        cur = cur->next;
    }
    printf("\n");
    print_indent(indent);
    printf("}\n");
}

void print_json_value(int indent,JSON_VALUE* value){
    switch (value->type){
        case JSON_TYPE_STRING:
            printf("%s", value->value.string);
            break;
        case JSON_TYPE_OBJECT:
            print_json_object(indent + 1, value->value.object);
            break;
        case JSON_TYPE_ARRAY:
            print_json_array(indent + 1, value->value.array);
            break;
        case JSON_TYPE_NUMBER:
            printf("%d", value->value.number);
            break;
        case JSON_TYPE_NULL:
            printf("%s", "null");
            break;
        case JSON_TYPE_BOOLEAN:
            printf("%s", value->value.boolean ? "true" : "false");
            break;
        default:
            break;
    }
}

void json_pretty_print(JSON_OBJECT* object){
    print_json_object(0, object);
}