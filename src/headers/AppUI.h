#ifndef APP_UI
#define APP_UI

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>


#include "ProxyServer.h"
static GtkBuilder* builder = NULL;

void addToHistory(ParserResult* parser_result);
void appendToHistory(char* data);
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