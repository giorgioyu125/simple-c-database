// src/server.c

#include <uv.h>
#include <llhttp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "string_functionality.h"
#include "server.h"

void on_close(uv_handle_t* handle) {
    my_client_context_t* ctx = handle->data;
    if (ctx) {
        free(ctx);
    }
    free(handle);
    printf("[INFO] on_close: Connection and context freed.\n");
}

void on_write_complete(uv_write_t* req, int status) {
    write_req_t* wr = (write_req_t*)req;
    free(wr->buf.base);
    free(wr);
    if (status < 0) {
        fprintf(stderr, "[ERROR] on_write_complete: '%s'.\n", uv_strerror(status));
    }
    uv_close((uv_handle_t*)req->handle, on_close);
}

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    (void)handle;
    buf->base = (char*)malloc(suggested_size);
    buf->len = (buf->base != NULL) ? suggested_size : 0;
}

void on_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf) {
    my_client_context_t* ctx = (my_client_context_t*)client->data;

    if (nread > 0) {
        enum llhttp_errno err = llhttp_execute(&ctx->parser, buf->base, (size_t)nread);
        if (err != HPE_OK) {
            fprintf(stderr, "[ERROR] Parser error: %s %s\n", llhttp_errno_name(err), ctx->parser.reason);
            uv_close((uv_handle_t*)client, on_close);
        }
    } else if (nread < 0) {
        if (nread != UV_EOF) {
            fprintf(stderr, "[ERROR] Read error: %s\n", uv_strerror((int)nread));
        }
        uv_close((uv_handle_t*)client, on_close);
    }

    if (buf->base) {
        free(buf->base);
    }
}

int on_url_cb(llhttp_t* parser, const char* at, size_t length) {
    my_client_context_t* ctx = parser->data;
    if (ctx->url_len + length < MAX_URL_LENGTH) {
        memcpy(ctx->url_buffer + ctx->url_len, at, length);
        ctx->url_len += length;
    }
    return 0;
}

int on_body_cb(llhttp_t* parser, const char* at, size_t length) {
    my_client_context_t* ctx = parser->data;
    if (ctx->body_len + length < MAX_VALUE_SIZE) {
        memcpy(ctx->body_buffer + ctx->body_len, at, length);
        ctx->body_len += length;
    }
    return 0;
}

int on_message_complete_cb(llhttp_t* parser) {
    my_client_context_t* ctx = parser->data;
    uv_read_stop(ctx->client_stream);

    ctx->url_buffer[ctx->url_len] = '\0';
    char* command_name = strtok(ctx->url_buffer, "/");
    if (!command_name) { command_name = ""; }

    char* argv[MAX_TOKENS];
    ctx->body_buffer[ctx->body_len] = '\0';
    int argc = tokenize_string(" ", ctx->body_buffer, MAX_TOKENS, argv);

    size_t arg_lengths[MAX_TOKENS];
    for (int i = 0; i < argc; i++) {
        arg_lengths[i] = strlen(argv[i]);
    }

    printf("[INFO] Executing: %s (argc=%d)\n", command_name, argc);
    execute_result_t result = execute_command(ctx->server_ctx->reg, ctx->server_ctx->db,
                                              command_name, argc, argv, arg_lengths);

    const char* reason_phrase = llhttp_status_name(result.status_code);
    const char* content_type_str = (result.content_type == DATA_TYPE_STRING)
                                   ? "text/plain" : "application/octet-stream";

    size_t response_buffer_size = 512 + result.body_length;
    char* response_str = malloc(response_buffer_size);
    if (!response_str) {
        goto cleanup_and_close;
    }
    
    int header_len = snprintf(response_str, response_buffer_size,
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n\r\n",
        result.status_code,
        reason_phrase,
        content_type_str,
        result.body_length);

    if (header_len < 0) {
        free(response_str);
        goto cleanup_and_close;
    }

    if (result.body_length > 0) {
        memcpy(response_str + header_len, result.body, result.body_length);
    }
    
    write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
    if (!req) {
        free(response_str);
        goto cleanup_and_close;
    }

    size_t total_len = (size_t)header_len + result.body_length;
    req->buf = uv_buf_init(response_str, (unsigned int)total_len);
    
    int write_status = uv_write((uv_write_t*)req, ctx->client_stream, &req->buf, 1, on_write_complete);
    
    if (write_status < 0) {
        free(req->buf.base);
        free(req);
    }
    
    free_execute_result(&result);
    return HPE_OK;

cleanup_and_close:
    fprintf(stderr, "[FATAL] Failed to prepare response. Closing connection.\n");
    free_execute_result(&result);
    uv_close((uv_handle_t*)ctx->client_stream, on_close);
    return HPE_INTERNAL;
}

void on_new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "[ERROR] New connection error: %s\n", uv_strerror(status));
        return;
    }

    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    if (!client) return;
    uv_tcp_init(uv_default_loop(), client);

    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        my_client_context_t* ctx = calloc(1, sizeof(my_client_context_t));
        if (!ctx) {
            uv_close((uv_handle_t*)client, on_close);
            return;
        }
        ctx->client_stream = (uv_stream_t*)client;
        ctx->server_ctx = server->data;

        llhttp_settings_init(&ctx->parser_settings);
        ctx->parser_settings.on_url = on_url_cb;
        ctx->parser_settings.on_body = on_body_cb;
        ctx->parser_settings.on_message_complete = on_message_complete_cb;

        llhttp_init(&ctx->parser, HTTP_REQUEST, &ctx->parser_settings);
        ctx->parser.data = ctx;
        client->data = ctx;

        uv_read_start((uv_stream_t*)client, alloc_buffer, on_read);
    } else {
        uv_close((uv_handle_t*)client, on_close);
    }
}

int main(void) {
    server_context_t g_server_ctx;
    g_server_ctx.reg = registry_create();
    g_server_ctx.db = table_create(4096);
    printf("[INFO] Global resources initialized.\n");

    uv_loop_t* loop = uv_default_loop();

    uv_tcp_t server_socket;
    uv_tcp_init(loop, &server_socket);
    server_socket.data = &g_server_ctx;

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 7000, &addr);

    uv_tcp_bind(&server_socket, (const struct sockaddr*)&addr, 0);

    int r = uv_listen((uv_stream_t*)&server_socket, 128, on_new_connection);
    if (r){
        fprintf(stderr, "Listen error: %s\n", uv_strerror(r));
        return 1;
    }

    printf("[INFO] Server listening on http://0.0.0.0:7000\n");
    printf("[INFO] Test example: curl -X POST http://localhost:7000/CREATE -d \"1000\"\n");

    int run_result = uv_run(loop, UV_RUN_DEFAULT);

    registry_destroy(&g_server_ctx.reg);
    table_destroy(g_server_ctx.db, destroy_data_entry);
    printf("[INFO] Server terminated.\n");

    return run_result;
}
