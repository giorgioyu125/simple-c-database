#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "hashtable.h"
#include "string_functionality.h"
#include "server.h"

void on_timer_close(uv_handle_t* handle) {
    my_client_context_t* ctx = (my_client_context_t*)handle->data;
    free(ctx);
    printf("[DEBUG] on_timer_close: Timer closed. Server context freed.\n");
}

void on_client_close(uv_handle_t* handle) {
    my_client_context_t* ctx = handle->data;

    printf("[DEBUG] on_client_close: Client stream closed. Freeing tcp handle.\n");
    free(handle); 

    if (ctx) {
        if (!uv_is_closing((uv_handle_t*)&ctx->inactivity_timer)) {
            printf("[DEBUG] on_client_close: Starting timer closing.\n");
            uv_close((uv_handle_t*)&ctx->inactivity_timer, on_timer_close);
        }
    }
}

void on_client_timeout(uv_timer_t* timer) {
    my_client_context_t* ctx = timer->data;
    printf("[INFO] Inactive client. Closing connection.\n");
    uv_close((uv_handle_t*)ctx->client_stream, on_client_close);
}

void on_write_complete(uv_write_t* req, int status) {
    write_req_t* wr = (write_req_t*)req;
    free(wr->buf.base);
    free(wr);

    if (status < 0) {
        fprintf(stderr, "[ERROR] on_write_complete: '%s'.\n", uv_strerror(status));
        uv_close((uv_handle_t*)req->handle, on_client_close);
    }
}

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf) {
    (void)handle;
    buf->base = (char*)malloc(suggested_size);
    buf->len = (buf->base != NULL) ? suggested_size : 0;
}

void process_and_execute_command(my_client_context_t* ctx, char* command_line) {
    printf("[INFO] Execution with the following command_line: '%s'\n", command_line);

    char* argv[MAX_TOKENS];
    int argc_with_command = tokenize_string(" ", command_line, MAX_TOKENS, argv);

    if (argc_with_command == 0) {
        return;
    }

    char* command_name = argv[0];
    int argc = argc_with_command - 1;

    char** command_argv = &argv[1];
    if (argc == 0){
        command_argv = NULL;
    }

    size_t arg_lengths[MAX_TOKENS];
    for (int i = 0; i < argc; i++) {
        arg_lengths[i] = strlen(command_argv[i]);
    }

    execute_result_t result = execute_command(ctx->server_ctx, command_name, argc,
                                              command_argv, arg_lengths);

    const char* response_body = (const char*)result.body;
    size_t body_len = result.body_length;

    char* response_str = malloc(body_len + 2);
    if (response_str == NULL){
        free_execute_result(&result);
        uv_close((uv_handle_t*)ctx->client_stream, on_client_close);
        return;
    }

    memcpy(response_str, response_body, body_len);
    response_str[body_len] = '\n';
    response_str[body_len + 1] = '\0';

    write_req_t* req = (write_req_t*)malloc(sizeof(write_req_t));
    req->buf = uv_buf_init(response_str, (unsigned int)body_len + 1);
    req->req.data = ctx->client_stream;

    int write_status = uv_write((uv_write_t*)req, ctx->client_stream, &req->buf, 1, on_write_complete);
    if (write_status < 0) {
        fprintf(stderr, "[ERROR] process_and_execute_command: uv_write failed immediately: %s\n", uv_strerror(write_status));
        free(req->buf.base);
        free(req);
        uv_close((uv_handle_t*)ctx->client_stream, on_client_close);
    }

    free_execute_result(&result);
}

void on_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf){
    my_client_context_t* ctx = (my_client_context_t*)client->data;

    if (nread > 0){
        uv_timer_start(&ctx->inactivity_timer, on_client_timeout, INACTIVITY_TIMEOUT, 0);

        if (ctx->buffer_len + (size_t)nread > MAX_VALUE_SIZE) {
            fprintf(stderr, "[ERROR] on_read: Request too large. Closing connection.\n");
            uv_close((uv_handle_t*)client, on_client_close);
            goto cleanup;
        }

        memcpy(ctx->buffer + ctx->buffer_len, buf->base, (size_t)nread);
        ctx->buffer_len += (size_t)nread;

        char* newline_pos;
        while ((newline_pos = memchr(ctx->buffer, '\n', ctx->buffer_len)) != NULL) {
            *newline_pos = '\0';
            char* command_line = ctx->buffer;

            process_and_execute_command(ctx, command_line);

            size_t remaining_len = ctx->buffer_len - (size_t)(newline_pos - ctx->buffer + 1);
            memmove(ctx->buffer, newline_pos + 1, remaining_len);
            ctx->buffer_len = remaining_len;
        }
    } else if (nread < 0){
        uv_timer_stop(&ctx->inactivity_timer);
        if (nread == UV_EOF){
            if (ctx->buffer_len > 0){
                ctx->buffer[ctx->buffer_len] = '\0';
                process_and_execute_command(ctx, ctx->buffer);
            } else {
                uv_close((uv_handle_t*)client, on_client_close);
            }
        } else {
            fprintf(stderr, "[ERROR] on_read: '%s'\n", uv_strerror((int)nread));
            uv_close((uv_handle_t*)client, on_client_close);
        }
    }

cleanup:
    if (buf->base) {
        free(buf->base);
    }
}

void on_new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "[ERROR] on_new_connection: %s.\n", uv_strerror(status));
        return;
    }

    uv_tcp_t* client = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
    if (client == NULL) {
        return;
    }
    uv_tcp_init(uv_default_loop(), client);

    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        my_client_context_t* ctx = calloc(1, sizeof(my_client_context_t));
        if (ctx == NULL) {
            uv_close((uv_handle_t*)client, on_client_close);
            return;
        }

        ctx->client_stream = (uv_stream_t*)client;
        ctx->server_ctx = server->data;
        client->data = ctx;

        uv_timer_init(uv_default_loop(), &ctx->inactivity_timer);
        ctx->inactivity_timer.data = ctx;
        uv_timer_start(&ctx->inactivity_timer, on_client_timeout, INACTIVITY_TIMEOUT, 0);

        uv_read_start((uv_stream_t*)client, alloc_buffer, on_read);
    } else {
        uv_close((uv_handle_t*)client, on_client_close);
    }
}

int main(int argc, char** argv) {
    if (argc != 2){
        fprintf(stderr, "[ERROR] main: Wrong number of argument provided, only 1 is permitted. Example: simple_c_database <DB_SIZE>.\n");
        return -1;
    }    

    server_context_t g_server_ctx;
    g_server_ctx.reg = registry_create();
    g_server_ctx.db = table_create(stosizet(argv[1]));

    if (g_server_ctx.db == 0){
        fprintf(stderr, "[ERROR] main: Wrong argument provided, internal error. Example: simple_c_database <DB_SIZE>.\n");
        return -1;
    }

    fprintf(stderr, "[INFO] Global resources initialized.\n");

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

    fprintf(stderr, "[INFO] Server listening on port 7000.\n");

    int run_result = uv_run(loop, UV_RUN_DEFAULT);
    
    registry_destroy(&g_server_ctx.reg);
    table_destroy(g_server_ctx.db, destroy_data_entry);
    fprintf(stderr, "[INFO] Server terminated.\n");

    return run_result;
}
