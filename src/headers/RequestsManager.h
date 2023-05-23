#ifndef REQUESTS_MANAGER
#define REQUESTS_MANAGER

#include <stdbool.h>

typedef struct HandleDataResult {
    bool is_valid;
    char* target_host_name;
} HandleDataResult;

HandleDataResult handleData(char* input);

#endif