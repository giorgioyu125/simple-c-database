#ifndef SERVER_H
#define SERVER_H

// Includes

#include <uv.h>
#include <llhttp.h>
#include "command.h"

// Macro

#define MAX_URL_LENGTH 1024

// Data

typedef struct {
    command_registry* reg;
    hashtable_t* db;
} server_context_t;

typedef struct {
    uv_stream_t* client_stream;
    llhttp_t parser;
    llhttp_settings_t parser_settings;
    server_context_t* server_ctx;

    char url_buffer[MAX_URL_LENGTH];
    size_t url_len;
    char body_buffer[MAX_VALUE_SIZE];
    size_t body_len;
} my_client_context_t;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

// Public API
    void on_new_connection(uv_stream_t *server, int status);
    void on_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
    void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    void on_close(uv_handle_t* handle);
    void on_close(uv_handle_t* handle);
    void on_write_complete(uv_write_t* req, int status);
    int on_url_cb(llhttp_t* parser, const char* at, size_t length);
    int on_body_cb(llhttp_t* parser, const char* at, size_t length);
    int on_message_complete_cb(llhttp_t* parser);


#endif
