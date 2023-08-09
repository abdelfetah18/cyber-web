#ifndef APP_UI
#define APP_UI

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>

#include "ProxyServer.h"

typedef struct {
    enum { HTTP_REQUEST, HTTP_RESPONSE } type;
    void* data;
} IPCMessage;

typedef struct {
    uint id;
    HttpRequest* http_request;
    HttpResponse* http_response;
    struct HistoryItem* next;
} HistoryItem;

extern GtkBuilder* builder;
extern HistoryItem* history_list;
extern GtkWindow* window;

HistoryItem* createHistoryItem(uint id,HttpRequest* http_request,HttpResponse* http_response);
void insertIntoHistoryList(HistoryItem* item);

void updateHistoryItemHttpRequest(uint id,HttpRequest* http_request);
void updateHistoryItemHttpResponse(uint id,HttpResponse* http_response);

HistoryItem* getHistoryItemById(uint id);

void updateUI(GtkWidget *widget, gpointer data);

void appendToHistory(uint id,char* data);
void clearHistory(GtkWidget* list);

void setRequestRaw(char* data,int size);
void setResponseRaw(char* data,int size);

void appendToHttpRequestHeaders(char* field_name,char* field_value);
void appendToHttpResponseHeaders(char* field_name,char* field_value);

void setRequestBody(GtkWidget* raw,char* data,int size);
void setResponseBody(GtkWidget* raw,char* data,int size);

static void activate(GtkApplication *app, gpointer user_data);
int main_loop(int argc,char** argv);

#endif