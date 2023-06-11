#ifndef REQUESTS_MANAGER
#define REQUESTS_MANAGER

#include <stdbool.h>

#include "HttpParser.h"

typedef struct HandleDataResult {
    bool is_valid;
    bool is_https;
    char* target_host_name;
} HandleDataResult;

HandleDataResult handleData(ParserInput* input);

#endif