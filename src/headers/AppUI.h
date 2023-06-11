#ifndef APP_UI
#define APP_UI

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>
#include <X11/Xlib.h>


#include "ProxyServer.h"

#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600

static int screen_width = DEFAULT_SCREEN_WIDTH;
static int screen_height = DEFAULT_SCREEN_HEIGHT;
static int window_width = DEFAULT_SCREEN_WIDTH;
static int window_height = DEFAULT_SCREEN_HEIGHT;

typedef struct {
    GtkWidget *tab_title;
    GtkWidget *tab_content;
} Tab;

typedef struct {
    GtkWidget *list_contianer;
    GtkWidget *list_title;
    GtkWidget *list_content;
} List;

typedef struct {
    Tab* http_message_tab;
    Tab* http_raw_tab;
    Tab* http_headers_tab;
    Tab* http_body_tab;
} HttpMessage;

typedef struct {
    GtkWidget *window;
    GtkWidget *main_container;
    GtkWidget *left_container;
    List *history;

    GtkWidget *right_container;
    HttpMessage* http_request_tab;
    HttpMessage* http_response_tab;
} MyAppUI;

Tab* createTabUI(char* title,GtkWidget* tab_content);
List* createListUI(char* title);
HttpMessage* createHttpMessageUI(char* title);
void init_my_app(MyAppUI* app);
void resize_my_app(MyAppUI* app);
static void on_property_notify(GtkWidget* widget, GParamSpec* pspec, gpointer data);
void updateUI(GtkWidget *widget, gpointer data);
static void activate(GtkApplication *app, gpointer user_data);
int main_loop(int argc,char** argv);


// UI Control
typedef enum {
    ADD_TO_HISTORY,
    CLEAR_HISTORY,
    SET_REQUEST_RAW,
    SET_RESPONSE_RAW,
    SET_REQUEST_HEADERS,
    SET_RESPONSE_HEADERS,
    SET_REQUEST_BODY,
    SET_RESPONSE_BODY
} IPCCallback;

typedef struct {
    MyAppUI* my_app;
    ParserResult* data;
} IPCParam;

typedef struct {
    IPCCallback callback;
    IPCParam* params;
} IPCMessage;

typedef struct {
    MyAppUI* my_app;
    ParserResult* parser_result;
} onClickParam;

onClickParam* createOnClickParam(MyAppUI* my_app,ParserResult* parser_result);

void addToHistory(MyAppUI* my_app,ParserResult* parser_result);
void clearHistory(GtkWidget* list);

void setRequestRaw(GtkWidget* raw,char* data,int size);
void setResponseRaw(GtkWidget* raw,char* data,int size);

// void setRequestHeaders();
// void setResponseHeaders();

void setRequestBody(GtkWidget* raw,char* data,int size);
void setResponseBody(GtkWidget* raw,char* data,int size);

IPCParam* createIPCParam(MyAppUI* my_app,ParserResult* data);
IPCMessage* createIPCMessage(IPCCallback callback,IPCParam* params);

#endif