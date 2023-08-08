#include "headers/AppUI.h"

static void activate(GtkApplication *app, gpointer user_data){

    /* Construct a GtkBuilder instance and load our UI description */
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "CyberWeb.ui", NULL);

    /* Connect signal handlers to the constructed widgets. */
    GObject *window = gtk_builder_get_object(builder, "window");
    
    // Start the proxy server.
    pthread_t WORKER_ID;
    pthread_create(&WORKER_ID, NULL, proxyServerThread, NULL);

    gtk_window_set_application(GTK_WINDOW (window), app);
    gtk_widget_show(GTK_WIDGET (window));

    // TEST: append To History
    for(int i = 0; i < 20; i++){
        appendToHistory("https://abdelfetah.dev/i am the best developer in the world");
    }

    char* example_request = "GET /foo/bar HTTP/1.1\r\n"
                            "Host: example.org\r\n"
                            "User-Agent: Mozilla/5.0 (Macintosh; U; Intel Mac OS X 10.6; fr; rv:1.9.2.8) Gecko/20100722 Firefox/3.6.8\r\n"
                            "Accept: *\r\n"
                            "Accept-Language: fr,fr-fr;q=0.8,en-us;q=0.5,en;q=0.3\r\n"
                            "Accept-Encoding: gzip,deflate\r\n"
                            "Accept-Charset: ISO-8859-1,utf-8;q=0.7,*;q=0.7\r\n"
                            "Keep-Alive: 115\r\n"
                            "Connection: keep-alive\r\n"
                            "Content-Type: application/x-www-form-urlencoded\r\n"
                            "X-Requested-With: XMLHttpRequest\r\n"
                            "Referer: http://example.org/test\r\n"
                            "Cookie: foo=bar; lorem=ipsum;\r\n\r\n\0";
    
    setRequestRaw(example_request, strlen(example_request));
    setResponseRaw(example_request, strlen(example_request));

    appendToHttpRequestHeaders("Connection", "keep-alive");
    appendToHttpResponseHeaders("Server", "CyberWeb");
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


void appendToHistory(char* data){
    GObject *history_list = gtk_builder_get_object(builder, "history_list");
    GtkWidget* label = gtk_button_new_with_label(data);
    gtk_list_box_append(GTK_WIDGET(history_list), label);
    // g_signal_connect(label, "clicked", G_CALLBACK(onClick), params);    
}

void addToHistory(ParserResult* parser_result){
    char* path = parser_result->parser->http_request->resource_path;    
    appendToHistory(path);
    printf("[*] addToHistory Done.\n");
}

void clearHistory(GtkWidget* list){
    // TODO: Implement a LinkedList for history items.
}

void setRequestRaw(char* data,int size){
    GObject* http_request_raw = gtk_builder_get_object(builder, "http_request_raw");

    char* str = malloc((sizeof(char) * size) + 1);
    strncpy(str, data, size);

    gtk_label_set_label(GTK_WIDGET(http_request_raw), str);
    gtk_label_set_wrap(GTK_WIDGET(http_request_raw), true);
}

void setResponseRaw(char* data,int size){
    GObject* http_response_raw = gtk_builder_get_object(builder, "http_response_raw");

    char* str = malloc((sizeof(char) * size) + 1);
    strncpy(str, data, size);

    gtk_label_set_label(GTK_WIDGET(http_response_raw), str);
    gtk_label_set_wrap(GTK_WIDGET(http_response_raw), true);
}

void setRequestBody(GtkWidget* raw,char* data,int size){
    // FIXME: Use a TextView instead of label.
    gtk_label_set_label(raw, data);
}

void setResponseBody(GtkWidget* raw,char* data,int size){
    // FIXME: Use a TextView instead of label.
    gtk_label_set_label(raw, data);
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