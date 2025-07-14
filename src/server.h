#ifndef SERVER_H
#define SERVER_H

// Includes

#include <uv.h>
#include "command.h"

// Macro

#define MAX_URL_LENGTH      1024
#define INACTIVITY_TIMEOUT (60 * 1000) // expressed in ms

// Data

typedef enum {
    PARSE_STATE_EXPECT_TYPE,   
    PARSE_STATE_EXPECT_LENGTH, 
    PARSE_STATE_EXPECT_DATA,   
} parser_state_t;

typedef struct client_context_t{
    uv_tcp_t client_handle;
    server_context_t* server_ctx;
    
    char* buffer;
    size_t buffer_used;
    size_t buffer_capacity;

    parser_state_t state;
    size_t args_total;
    size_t args_parsed;
    size_t data_to_read;

    char** temp_argv;         
    size_t* temp_arg_lengths;

    uv_timer_t inactivity_timer;
} client_context_t;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;


// Public API

    void parse_buffer(client_context_t* ctx);
    void reset_parser(client_context_t* ctx);
    void append_to_buffer(client_context_t* ctx, const char* data, size_t len);
    void on_new_connection(uv_stream_t *server, int status);
    void on_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf);
    void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    void on_close(uv_handle_t* handle);
    void on_close(uv_handle_t* handle);
    void on_write_complete(uv_write_t* req, int status);


#endif
