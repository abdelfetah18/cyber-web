#ifndef BUFFER_STORAGE
#define BUFFER_STORAGE

#include <stdio.h>
#include <stdlib.h>

#define uint unsigned int

typedef struct
{
    uint capacity;
    uint size;
    char* data;
} BufferStorage;


BufferStorage* createBufferStorage();
void freeBufferStorage(BufferStorage* buffer);
void appendToBuffer(BufferStorage* buffer,char* data,uint size);

#endif