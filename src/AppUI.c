#include "headers/AppUI.h"

GtkWindow* window = NULL;
GtkBuilder* builder = NULL;
HistoryItem* history_list = NULL;

HistoryItem* createHistoryItem(uint id,HttpRequest* http_request,HttpResponse* http_response){
    HistoryItem* item = malloc(sizeof(HistoryItem));
    item->id = id;
    item->http_request = http_request;
    item->http_response = http_response;
    item->next = NULL;

    return item;
}

void insertIntoHistoryList(HistoryItem* item){
    if(history_list == NULL){
        history_list = item;
        return;
    }
    item->next = history_list;
    history_list = item;
}

void updateHistoryItemHttpRequest(uint id,HttpRequest* http_request){
    HistoryItem* cur = history_list;
    while(cur != NULL){
        if(cur->id == id){
            cur->http_request = http_request;
            return;
        }
        cur = cur->next;
    }
}

void updateHistoryItemHttpResponse(uint id,HttpResponse* http_response){
    HistoryItem* cur = history_list;
    while(cur != NULL){
        if(cur->id == id){
            cur->http_response = http_response;
            return;
        }
        cur = cur->next;
    }
}

HistoryItem* getHistoryItemById(uint id){
      HistoryItem* cur = history_list;
    while(cur != NULL){
        if(cur->id == id){
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}

void updateUI(GtkWidget *widget, gpointer data){
    if(data == NULL)
        return;
    
    /*
        MAP:
            ID -> { HttpRequest, HttpResponse }
    */

    IPCMessage* ipc_message = (IPCMessage*) data;
    HistoryItem* item = (HistoryItem*) ipc_message->data;
    
    switch (ipc_message->type){
        case HTTP_REQUEST: {
            appendToHistory(item->id, item->http_request->resource_path);
            break;
        }
        case HTTP_RESPONSE:{
            
            break;    
        }
        default:
            break;
    }
}

static void activate(GtkApplication *app, gpointer user_data){

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "CyberWeb.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    window = GTK_WINDOW(gtk_builder_get_object(builder, "window"));
    
    // Register a callback to update the GUI.
    gint signal = g_signal_new("update_ui", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
    g_signal_connect(window, "update_ui", G_CALLBACK(updateUI), NULL);

    // Start the proxy server.
    pthread_t WORKER_ID;
    pthread_create(&WORKER_ID, NULL, proxyServerThread, NULL);

    gtk_window_set_application(GTK_WINDOW(window), app);
    gtk_widget_show(GTK_WIDGET (window));
}

int main_loop(int argc,char** argv){
    GtkApplication *app;
    int status;

    app = gtk_application_new("org.CyberWeb.dev", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);

    return status;
}

void onHistoryItemSelected(GtkWidget* widget,gpointer data){
    HistoryItem* item = getHistoryItemById((uint) data);
    setRequestRaw(item->http_request->raw->data, item->http_request->raw->size);
    if(item->http_response && item->http_response->raw){
        setResponseRaw(item->http_response->raw->data, item->http_response->raw->size);
    }
}

void appendToHistory(uint id, char* data){
    GObject *history_list = gtk_builder_get_object(builder, "history_list");

    int len = strlen(data);
    char* path = data;

    if(len > 80){
        path = malloc(sizeof(char) * 31);
        strncpy(path, data, 27);
        strncpy(path+27, "...\0", 4);
    }
    GtkWidget* label = gtk_label_new(path);
    gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
    gtk_label_set_xalign(GTK_LABEL(label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(label), 0.0);
    
    GtkWidget* button = gtk_button_new();
    gtk_button_set_child(button, label);
    gtk_list_box_append(GTK_WIDGET(history_list), button);
    g_signal_connect(button, "clicked", G_CALLBACK(onHistoryItemSelected), id);    
}

void clearHistory(GtkWidget* list){
    // TODO: Implement a LinkedList for history items.
}

void setRequestRaw(char* data,int size){
    GObject* http_request_raw = gtk_builder_get_object(builder, "http_request_raw");

    GtkTextBuffer* buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(buffer, data, size);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(http_request_raw), false);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(http_request_raw), buffer);
}

void setResponseRaw(char* data,int size){
    GObject* http_response_raw = gtk_builder_get_object(builder, "http_response_raw");

    GtkTextBuffer* buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(buffer, data, size);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(http_response_raw), false);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(http_response_raw), buffer);
}

void setRequestBody(GtkWidget* raw,char* data,int size){
    GObject* http_request_body = gtk_builder_get_object(builder, "http_request_body");

    GtkTextBuffer* buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(buffer, data, size);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(http_request_body), false);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(http_request_body), buffer);
}

void setResponseBody(GtkWidget* raw,char* data,int size){
    GObject* http_response_body = gtk_builder_get_object(builder, "http_response_body");

    GtkTextBuffer* buffer = gtk_text_buffer_new(NULL);
    gtk_text_buffer_set_text(buffer, data, size);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(http_response_body), false);
    gtk_text_view_set_buffer(GTK_TEXT_VIEW(http_response_body), buffer);
}

void appendToHeaders(GtkWidget* tab_content,char* field_name,char* field_value){
    GtkGrid *grid = GTK_GRID(gtk_grid_new());
    gtk_grid_set_row_homogeneous(grid, TRUE);
    gtk_grid_set_column_homogeneous(grid, TRUE);

    GtkWidget *name_label = gtk_label_new(field_name);
    GtkWidget *value_label = gtk_label_new(field_value);

    gtk_grid_attach(grid, name_label, 0, 0, 1, 1);
    gtk_grid_attach(grid, value_label, 1, 0, 1, 1);
    
    gtk_list_box_insert(GTK_LIST_BOX(tab_content), grid, -1);
}

void appendToHttpRequestHeaders(char* field_name,char* field_value){
    GObject* http_request_headers = gtk_builder_get_object(builder, "http_request_headers");
    appendToHeaders(GTK_WIDGET(http_request_headers), field_name, field_value);
}

void appendToHttpResponseHeaders(char* field_name,char* field_value){
    GObject* http_response_headers = gtk_builder_get_object(builder, "http_response_headers");
    appendToHeaders(GTK_WIDGET(http_response_headers), field_name, field_value);
}