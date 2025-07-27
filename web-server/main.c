#include "civetweb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "DEV_Config.h"
#include "EPD_7in3f.h"

// E-paper initialization
void init_epaper() {
    printf("E-paper initialized\n");
    if (DEV_Module_Init() != 0) {
        printf("DEV_Module_Init failed\n");
        return;
    }
    EPD_7IN3F_Init();
}

// Clearing e-paper
void clear_epaper() {
    printf("E-paper cleared\n");
    EPD_7IN3F_Clear(EPD_7IN3F_WHITE); 
}

// request handler
int handleHello(struct mg_connection *conn, void *cbdata) {
    mg_printf(conn,
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
    printf("hello pressed\n");
    return 200;
}

// form submissions handler (Post req.)
int handleForm(struct mg_connection *conn, void *cbdata) {
    char post_data[1024];
    int post_data_len = mg_read(conn, post_data, sizeof(post_data));
    post_data[post_data_len] = '\0';

    mg_printf(conn,
        "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"
        "<html><body><h1>Form Data Received:</h1><pre>%s</pre></body></html>",
        post_data);

    return 200;
}

void handleClear(struct mg_connection* conn, void* cbdata) {
        mg_printf(conn,
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n");
        printf("clearin e-paper\n");
        clear_epaper();
}

int main() {
    const char *options[] = {
        "document_root", "./templates",     // Serve static files from this dir
        "listening_ports", "8080",
        0
    };

    struct mg_callbacks callbacks;
    struct mg_context *ctx;

    memset(&callbacks, 0, sizeof(callbacks));
    ctx = mg_start(&callbacks, 0, options);

    // Register routes
    mg_set_request_handler(ctx, "/hello", handleHello, 0);
    mg_set_request_handler(ctx, "/submit", handleForm, 0);
    mg_set_request_handler(ctx, "/clear-epaper", handleClear, 0);

    printf("Server running on http://localhost:8080\n");

    printf("Initializing e-paper");
    init_epaper();
    printf("finished initializing e-paper");

    // Keep server running
    while (1) {
        sleep(1);
    }

    mg_stop(ctx);
    return 0;
}