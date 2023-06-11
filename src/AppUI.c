#include "headers/AppUI.h"

Tab* createTabUI(char* title,GtkWidget* tab_content){
    Tab* tab = malloc(sizeof(Tab));
    tab->tab_title = gtk_label_new(title);
    tab->tab_content = tab_content;
    return tab;
}

List* createListUI(char* title){ 
    List* list = malloc(sizeof(List));
    list->list_contianer = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);

    list->list_title = gtk_label_new(title);
    list->list_content = gtk_list_box_new();

    gtk_box_append(list->list_contianer, list->list_title);
    gtk_box_append(list->list_contianer, list->list_content);

    return list;
}

HttpMessage* createHttpMessageUI(char* title){
    HttpMessage* http_message = malloc(sizeof(HttpMessage));
    http_message->http_message_tab = createTabUI(title, gtk_notebook_new());
    
    http_message->http_raw_tab = createTabUI("raw", gtk_label_new("raw"));
    http_message->http_headers_tab = createTabUI("headers", gtk_label_new("headers"));
    http_message->http_body_tab = createTabUI("body", gtk_label_new("body"));

    gtk_label_set_wrap(http_message->http_raw_tab, true);
    gtk_label_set_wrap(http_message->http_headers_tab, true);
    gtk_label_set_wrap(http_message->http_body_tab, true);

    gtk_notebook_append_page(http_message->http_message_tab->tab_content, http_message->http_raw_tab->tab_content, http_message->http_raw_tab->tab_title);
    gtk_notebook_append_page(http_message->http_message_tab->tab_content, http_message->http_headers_tab->tab_content, http_message->http_headers_tab->tab_title);
    gtk_notebook_append_page(http_message->http_message_tab->tab_content, http_message->http_body_tab->tab_content, http_message->http_body_tab->tab_title);
    return http_message;
}   

void init_my_app(MyAppUI* app){
    app->main_container = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 1);
    app->left_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    app->history = createListUI("History");
    
    app->right_container = gtk_notebook_new();
    app->http_request_tab = createHttpMessageUI("Request");
    app->http_response_tab = createHttpMessageUI("Response");

    gtk_widget_set_halign(app->main_container, GTK_ALIGN_CENTER);

    gtk_box_append(GTK_BOX(app->main_container), app->left_container);
    gtk_box_append(GTK_BOX(app->main_container), app->right_container);

    gtk_box_append(GTK_BOX(app->left_container), app->history->list_contianer);
   
    gtk_notebook_append_page(app->right_container, app->http_request_tab->http_message_tab->tab_content, app->http_request_tab->http_message_tab->tab_title);
    gtk_notebook_append_page(app->right_container, app->http_response_tab->http_message_tab->tab_content, app->http_response_tab->http_message_tab->tab_title);
}

void resize_my_app(MyAppUI* app){
    int c_width = window_width > DEFAULT_SCREEN_WIDTH ? window_width * 5 / 6 : DEFAULT_SCREEN_WIDTH * 5 / 6;
    int c_height = window_height - 40;

    gtk_widget_set_size_request(app->main_container, c_width, c_height);
    gtk_widget_set_size_request(app->left_container, c_width / 3, c_height);
    gtk_widget_set_size_request(app->right_container, c_width * 2 / 3, c_height);
}

static void on_property_notify(GtkWidget* widget, GParamSpec* pspec, gpointer data){
    const gchar* property_name = g_param_spec_get_name(pspec);
    gtk_window_get_default_size(widget, &window_width, &window_height);
    MyAppUI* my_app = (MyAppUI*) data;
    if(my_app){
        resize_my_app(my_app);
    }
}

void updateUI(GtkWidget *widget, gpointer data){
    if(data == NULL)
        return;

    IPCMessage* ipc_message = (IPCMessage*) data;
    switch (ipc_message->callback) {
        case ADD_TO_HISTORY:
            addToHistory(ipc_message->params->my_app, ipc_message->params->data);
            break;
        case CLEAR_HISTORY:
            clearHistory(ipc_message->params->my_app);
            break;
        case SET_REQUEST_RAW:
            setRequestRaw(ipc_message->params->my_app, ipc_message->params->data, 0);
            break;
        case SET_RESPONSE_RAW:
            setResponseRaw(ipc_message->params->my_app, ipc_message->params->data, 0);
            break;
        case SET_REQUEST_HEADERS:
            // TODO: Implement setRequestHeaders.
            break;
        case SET_RESPONSE_HEADERS:
            // TODO: Implement setResponseHeaders.
            break;
        case SET_REQUEST_BODY:
            setRequestBody(ipc_message->params->my_app, ipc_message->params->data, 0);
            break;
        case SET_RESPONSE_BODY:
            setResponseBody(ipc_message->params->my_app, ipc_message->params->data, 0);
            break;
        default:
            return;
            break;
    }
}

static void activate(GtkApplication *app, gpointer user_data){
    // Get current screen size
    Display* display = XOpenDisplay(NULL);
    Screen* screen = DefaultScreenOfDisplay(display);
    screen_width = WidthOfScreen(screen);
    screen_height = HeightOfScreen(screen);

    MyAppUI* my_app = malloc(sizeof(MyAppUI));
    
    // init widgets
    init_my_app(my_app);

    // Prepare window events
    my_app->window = gtk_application_window_new(app);

    // Start the proxy server.
    pthread_t WORKER_ID;
    pthread_create(&WORKER_ID, NULL, proxyServerThread, my_app);
    
    gtk_window_set_resizable(my_app->window, true);
    gtk_window_set_default_size(GTK_WINDOW(my_app->window), window_width, window_height);
    gtk_window_set_title(GTK_WINDOW(my_app->window), "CyberWeb");

    gtk_window_set_child(GTK_WINDOW(my_app->window), my_app->main_container);
    
    g_signal_connect(my_app->window, "notify", G_CALLBACK(on_property_notify), my_app);
    // Register a callback to update the GUI.
    gint signal = g_signal_new("update_ui", G_TYPE_OBJECT, G_SIGNAL_RUN_FIRST, 0, NULL, NULL, g_cclosure_marshal_VOID__POINTER, G_TYPE_NONE, 1, G_TYPE_POINTER);
    g_signal_connect(my_app->window, "update_ui", G_CALLBACK(updateUI), NULL);
    // resize
    resize_my_app(my_app);
    // show
    gtk_widget_show(my_app->window);
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


// UI Control 
onClickParam* createOnClickParam(MyAppUI* my_app,ParserResult* parser_result){
    onClickParam* params = malloc(sizeof(onClickParam));

    params->my_app = my_app;
    params->parser_result = parser_result;

    return params;
}

void onClick(GtkWidget* widget,gpointer data){
    onClickParam* params = (onClickParam*) data;
    gtk_label_set_text(params->my_app->http_request_tab->http_raw_tab->tab_content, params->parser_result->full_raw_data->data);
}

void addToHistory(MyAppUI* my_app,ParserResult* parser_result){
    printf("=====================> my_app: %p | parser_result: %p \n", my_app, parser_result);
    printf("HI: %s\n\n\n", parser_result->full_raw_data);
    char* path = parser_result->parser->http_request->resource_path;
    int len = strlen(path);

    printf("[*] len: %d.\n", len);
    if(len > 30){
        path = malloc(sizeof(char) * 31);
        strncpy(path, parser_result->parser->http_request->resource_path, 27);
        path[27] = '.';
        path[28] = '.';
        path[29] = '.';
        path[30] = '\0';
    }
    // FIXME: Add the new label widget to a history list so we can use that later to clear the list and avoid memory leaks.
    GtkWidget* label = gtk_button_new_with_label(path);
    gtk_list_box_append(my_app->history->list_content, label);
    onClickParam* params = createOnClickParam(my_app, parser_result);
    g_signal_connect(label, "clicked", G_CALLBACK(onClick), params);
    printf("[*] addToHistory Done.\n");
}

void clearHistory(GtkWidget* list){
    // TODO: Implement a LinkedList for history items.
}

void setRequestRaw(GtkWidget* raw,char* data,int size){
    // FIXME: Use a TextView instead of label.
    gtk_label_set_label(raw, data);
}

void setResponseRaw(GtkWidget* raw,char* data,int size){
    // FIXME: Use a TextView instead of label.
    gtk_label_set_label(raw, data);
}

void setRequestBody(GtkWidget* raw,char* data,int size){
    // FIXME: Use a TextView instead of label.
    gtk_label_set_label(raw, data);
}

void setResponseBody(GtkWidget* raw,char* data,int size){
    // FIXME: Use a TextView instead of label.
    gtk_label_set_label(raw, data);
}

IPCParam* createIPCParam(MyAppUI* my_app,ParserResult* data){
    IPCParam* params = malloc(sizeof(IPCParam));
    
    params->my_app = my_app;
    params->data = data;
    
    return params;
}

IPCMessage* createIPCMessage(IPCCallback callback,IPCParam* params){
    IPCMessage* ipc_message = malloc(sizeof(IPCMessage));

    ipc_message->callback = callback;
    ipc_message->params = params;

    return ipc_message;
}