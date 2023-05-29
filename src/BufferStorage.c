#include "headers/BufferStorage.h"

BufferStorage* createBufferStorage(){
    BufferStorage* buffer_storage = malloc(sizeof(BufferStorage));
    buffer_storage->data = malloc(sizeof(char) * 8);
    buffer_storage->capacity = 8;
    buffer_storage->size = 0;
    return buffer_storage;
}

void appendToBuffer(BufferStorage* buffer,char* data,uint size){
    if((buffer->size + size) < buffer->capacity){
        memcpy(buffer->data + buffer->size, data, size);
    }else{
        buffer->capacity = buffer->capacity + (((size / 8) + 1) * 8);
        char* new_buffer = malloc(sizeof(char) * buffer->capacity);
        memcpy(new_buffer, buffer->data, buffer->size);
        memcpy(new_buffer + buffer->size, data, size);
        free(buffer->data);
        buffer->data = new_buffer;
    }
    buffer->size += size;
}