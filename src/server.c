#include <uv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "command.h"
#include "string_functionality.h"
#include "server.h"

void on_close_after_failure(uv_handle_t* handle) {
    free(handle->data);
    printf("[DEBUG] on_close_after_failure: Freed context after setup failure.\n");
}

void on_timer_close(uv_handle_t* handle){
    client_context_t* ctx = (client_context_t*)handle->data;
    if (ctx->buffer) {
        free(ctx->buffer);
    }
    free(ctx);
    printf("[DEBUG] on_timer_close: Timer closed. Client context freed.\n");
}

void on_client_close(uv_handle_t* handle){
    client_context_t* ctx = handle->data;
    printf("[DEBUG] on_client_close: Client stream closed.\n");

    if (!uv_is_closing((uv_handle_t*)&ctx->inactivity_timer)){
        printf("[DEBUG] on_client_close: Closing associated timer.\n");
        uv_close((uv_handle_t*)&ctx->inactivity_timer, on_timer_close);
    }
}

void on_client_timeout(uv_timer_t* timer){
    client_context_t* ctx = timer->data;
    printf("[INFO] on_client_timeout: Inactive client. Closing connection.\n");
    uv_close((uv_handle_t*)&(ctx->client_handle), on_client_close);
}

void on_write_complete(uv_write_t* req, int status){
    write_req_t* wr = (write_req_t*)req;
    free(wr->buf.base);
    free(wr);

    if (status < 0) {
        fprintf(stderr, "[ERROR] on_write_complete: '%s'.\n", uv_strerror(status));
        uv_close((uv_handle_t*)req->handle, on_client_close);
    }
}

void alloc_buffer(uv_handle_t* handle, size_t suggested_size, uv_buf_t* buf){
    (void)handle;
    buf->base = (char*)malloc(suggested_size);
    buf->len = (buf->base != NULL) ? suggested_size : 0;
}

void free_parser_resources(client_context_t* ctx) {
    if (ctx->temp_argv) {
        for (size_t i = 0; i < ctx->args_parsed; i++) {
            free(ctx->temp_argv[i]);
        }
        free(ctx->temp_argv);
        ctx->temp_argv = NULL;
    }
    if (ctx->temp_arg_lengths) {
        free(ctx->temp_arg_lengths);
        ctx->temp_arg_lengths = NULL;
    }
}

void reset_parser(client_context_t* ctx) {
    ctx->state = PARSE_STATE_EXPECT_TYPE;
    ctx->args_total = 0;
    ctx->args_parsed = 0;
    ctx->data_to_read = 0;
}

void append_to_buffer(client_context_t* ctx, const char* data, size_t len) {
    if (ctx->buffer_used + len > ctx->buffer_capacity) {
        size_t new_capacity = (ctx->buffer_capacity == 0) ? 1024 : ctx->buffer_capacity * 2;
        if (new_capacity < ctx->buffer_used + len) {
            new_capacity = ctx->buffer_used + len;
        }
        char* new_buffer = realloc(ctx->buffer, new_capacity);
        if (!new_buffer) {
            fprintf(stderr, "[ERROR] Failed to realloc client buffer\n");
            uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
            return;
        }
        ctx->buffer = new_buffer;
        ctx->buffer_capacity = new_capacity;
    }
    memcpy(ctx->buffer + ctx->buffer_used, data, len);
    ctx->buffer_used += len;
}

      
void parse_buffer(client_context_t* ctx) {
    bool err;

    while (1) {
        if (ctx->state == PARSE_STATE_EXPECT_TYPE) {
            if (ctx->buffer_used < 1) {
                break;
            }

            if (ctx->buffer[0] != '*') {
                fprintf(stderr, "[ERROR] parse_buffer: Expected '*' for array type.\n");
                uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
                return;
            }

            char* crlf = memchr(ctx->buffer, '\r', ctx->buffer_used);
            if (!crlf || (crlf + 1 >= ctx->buffer + ctx->buffer_used) || *(crlf + 1) != '\n') {
                break;
            }

            long n_args = strtol(ctx->buffer + 1, NULL, 10);
            if (n_args <= 0 || n_args > 1024) {
                fprintf(stderr, "[ERROR] parse_buffer: Invalid number of arguments: %ld\n", n_args);
                uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
                return;
            }

            ctx->args_total = long_to_sizet(n_args, &err);
            if (err) {
                fprintf(stderr, "[ERROR] parse_buffer: Argument count conversion failed.\n");
                uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
                return;
            }

            ctx->args_parsed = 0;
            ctx->state = PARSE_STATE_EXPECT_LENGTH;

            size_t consumed = (size_t)((crlf + 2) - ctx->buffer);
            memmove(ctx->buffer, ctx->buffer + consumed, ctx->buffer_used - consumed);
            ctx->buffer_used -= consumed;

        } else if (ctx->state == PARSE_STATE_EXPECT_LENGTH) {
            if (ctx->buffer_used < 1 || ctx->buffer[0] != '$') {
                if (ctx->buffer_used > 0) {
                     fprintf(stderr, "[ERROR] parse_buffer: Expected '$' for bulk string length.\n");
                     uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
                }
                break;
            }

            char* crlf = memchr(ctx->buffer, '\r', ctx->buffer_used);
            if (!crlf || (crlf + 1 >= ctx->buffer + ctx->buffer_used) || *(crlf + 1) != '\n') {
                break;
            }

            long len = strtol(ctx->buffer + 1, NULL, 10);
            if (len < 0 || len > 8192){
                fprintf(stderr, "[ERROR] parse_buffer: Invalid bulk string length: %ld\n", len);
                uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
                return;
            }

            ctx->data_to_read = long_to_sizet(len, &err);
            if (err) {
                fprintf(stderr, "[ERROR] parse_buffer: Bulk string length conversion failed.\n");
                uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
                return;
            }

            ctx->state = PARSE_STATE_EXPECT_DATA;

            size_t consumed = (size_t)((crlf + 2) - ctx->buffer);
            memmove(ctx->buffer, ctx->buffer + consumed, ctx->buffer_used - consumed);
            ctx->buffer_used -= consumed;

        } else if (ctx->state == PARSE_STATE_EXPECT_DATA) {
            if (ctx->buffer_used < ctx->data_to_read + 2) {
                break;
            }

            if (ctx->args_parsed == 0) {
                ctx->temp_argv = malloc(ctx->args_total * sizeof(char*));
                ctx->temp_arg_lengths = malloc(ctx->args_total * sizeof(size_t));
                if (!ctx->temp_argv || !ctx->temp_arg_lengths) {
                    fprintf(stderr, "[ERROR] parse_buffer: Failed to allocate memory for command arguments.\n");
                    free_parser_resources(ctx); // free whichever succeeded
                    uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
                    return;
                }
            }

            ctx->temp_argv[ctx->args_parsed] = malloc(ctx->data_to_read + 1);
            if (!ctx->temp_argv[ctx->args_parsed]) {
                 fprintf(stderr, "[ERROR] parse_buffer: Failed to allocate memory for argument string.\n");
                 free_parser_resources(ctx);
                 uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
                 return;
            }
            memcpy(ctx->temp_argv[ctx->args_parsed], ctx->buffer, ctx->data_to_read);
            ctx->temp_argv[ctx->args_parsed][ctx->data_to_read] = '\0';
            ctx->temp_arg_lengths[ctx->args_parsed] = ctx->data_to_read;

            ctx->args_parsed++;

            size_t consumed = ctx->data_to_read + 2;
            memmove(ctx->buffer, ctx->buffer + consumed, ctx->buffer_used - consumed);
            ctx->buffer_used -= consumed;

            if (ctx->args_parsed == ctx->args_total){
                char* command_name = ctx->temp_argv[0];
                int argc = sizet_to_int(ctx->args_total - 1, &err);

                char** command_argv = (argc > 0) ? &ctx->temp_argv[1] : NULL;
                const size_t* args_lengths = (argc > 0) ? &ctx->temp_arg_lengths[1] : NULL;

                execute_result_t result = execute_command(ctx->server_ctx, command_name, argc, command_argv, args_lengths);

                const char* response_body = (const char*)result.body;
                size_t body_len = result.body_length;
                char* response_str = malloc(body_len + 2);
                if (!response_str){
                    free_execute_result(&result); 
                    free_parser_resources(ctx); 
                    uv_close((uv_handle_t*)&ctx->client_handle, on_client_close); 
                    return; 
                }
                
                memcpy(response_str, response_body, body_len);
                response_str[body_len] = '\n';
                response_str[body_len + 1] = '\0';

                write_req_t* req = malloc(sizeof(write_req_t));
                if (!req){
                    free(response_str); 
                    free_execute_result(&result); 
                    free_parser_resources(ctx); 
                    uv_close((uv_handle_t*)&ctx->client_handle, on_client_close); 
                    return; 
                    }

                unsigned int write_len = sizet_to_uint(body_len + 1, &err);
                if (err){ 
                    free(response_str); 
                    free(req); 
                    free_execute_result(&result); 
                    free_parser_resources(ctx); 
                    uv_close((uv_handle_t*)&ctx->client_handle, on_client_close); 
                    return; 
                }

                req->buf = uv_buf_init(response_str, write_len);
                uv_write((uv_write_t*)req, (uv_stream_t*)&ctx->client_handle, &req->buf, 1, on_write_complete);

                free_execute_result(&result);
                free_parser_resources(ctx);
                reset_parser(ctx);

            } else{
                ctx->state = PARSE_STATE_EXPECT_LENGTH;
            }
        } else{
            break;
        }
    }
}

void on_read(uv_stream_t* client, ssize_t nread, const uv_buf_t* buf){
    client_context_t* ctx = (client_context_t*)client->data;
    bool err;

    if (nread > 0) {
        uv_timer_start(&ctx->inactivity_timer, on_client_timeout, INACTIVITY_TIMEOUT, 0);

        append_to_buffer(ctx, buf->base, ssizet_to_sizet(nread, &err));
        if (err == true) return;
        
        parse_buffer(ctx);
    } else if (nread < 0) {
        uv_timer_stop(&ctx->inactivity_timer);
        if (nread != UV_EOF) {
            fprintf(stderr, "[ERROR] on_read: '%s'\n", uv_strerror((int)nread));
        }
        uv_close((uv_handle_t*)client, on_client_close);
    }

    if (buf->base) {
        free(buf->base);
    }
}


void on_new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        fprintf(stderr, "[ERROR] on_new_connection: %s.\n", uv_strerror(status));
        return;
    }

    client_context_t* ctx = calloc(1, sizeof(client_context_t));
    if (ctx == NULL) {
        fprintf(stderr, "[ERROR] on_new_connection: Failed to allocate memory for new client context.\n");
        return; 
    }

    uv_tcp_init(uv_default_loop(), &ctx->client_handle);

    ctx->client_handle.data = ctx;
    ctx->server_ctx = server->data; 

    if (uv_accept(server, (uv_stream_t*)&ctx->client_handle) == 0) {
        reset_parser(ctx);

        uv_timer_init(uv_default_loop(), &ctx->inactivity_timer);
        ctx->inactivity_timer.data = ctx; 
        uv_timer_start(&ctx->inactivity_timer, on_client_timeout, INACTIVITY_TIMEOUT, 0);

        uv_read_start((uv_stream_t*)&ctx->client_handle, alloc_buffer, on_read);
    } else {
        fprintf(stderr, "[ERROR] on_new_connection: uv_accept failed.\n");

        uv_close((uv_handle_t*)&ctx->client_handle, on_client_close);
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "[ERROR] main: Wrong number of argument provided, only 1 is permitted. Example: simple_c_database <DB_SIZE>.\n");
        return -1;
    }

    server_context_t g_server_ctx;
    g_server_ctx.reg = registry_create();
    size_t db_size = strtoul(argv[1], NULL, 10);
    if (db_size == 0) {
        fprintf(stderr, "[ERROR] main: Invalid DB_SIZE provided.\n");
        return -1;
    }
    g_server_ctx.db = table_create(db_size);

    if (g_server_ctx.db == NULL) {
        fprintf(stderr, "[ERROR] main: Failed to create database table.\n");
        return -1;
    }

    fprintf(stderr, "[INFO] main: Global resources initialized.\n");

    uv_loop_t* loop = uv_default_loop();

    uv_tcp_t server_socket;
    uv_tcp_init(loop, &server_socket);
    server_socket.data = &g_server_ctx;

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 7000, &addr);

    int err = uv_tcp_bind(&server_socket, (const struct sockaddr*)&addr, 0);
    if (err) {
        fprintf(stderr, "[ERROR] main: Bind error: %s\n", uv_strerror(err));
        return 1;
    }

    uv_os_fd_t fd;
    err = uv_fileno((uv_handle_t*)&server_socket, &fd);
    if (err) {
        fprintf(stderr, "[ERROR] main: uv_fileno failed: %s\n", uv_strerror(err));
        return 1;
    }

    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0) {
        perror("[WARN] main: setsockopt(SO_REUSEADDR) failed");
    } else {
        fprintf(stderr, "[INFO] main: SO_REUSEADDR set successfully.\n");
    }

    err = uv_tcp_nodelay(&server_socket, 1);
    if (err) {
        fprintf(stderr, "[WARN] main: Failed to set TCP_NODELAY: %s\n", uv_strerror(err));
    } else {
        fprintf(stderr, "[INFO] main: TCP_NODELAY enabled.\n");
    }

    err = uv_listen((uv_stream_t*)&server_socket, 128, on_new_connection);
    if (err) {
        fprintf(stderr, "[ERROR] main: Listen error: %s\n", uv_strerror(err));
        return 1;
    }

    fprintf(stderr, "[INFO] main: Server listening on port 7000.\n");

    int run_result = uv_run(loop, UV_RUN_DEFAULT);

    registry_destroy(&g_server_ctx.reg);
    table_destroy(g_server_ctx.db, destroy_value_wrapper);
    fprintf(stderr, "[INFO] main: Server terminated.\n");

    return run_result;
}
