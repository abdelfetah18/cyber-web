#include "../src/headers/Json.h"

int main(int argc,char** argv){
    char* data = "{ \"username\": \"Abdelfetah\", \"Skills\": 18 }";
    uint length = strlen(data);

    JSON_INPUT input = {
        .data = data,
        .size = length
    };

    JSON_PARSER* json_parser = create_json_parser(input);

    bool result = parse_json(json_parser);

    printf("RESULT: %s\n", result ? "true" : "false");
    json_pretty_print(json_parser->object);

    return 0;
}