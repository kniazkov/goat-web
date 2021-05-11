/*

Copyright (C) 2021 Ivan Kniazkov

This file is part of standard library for programming language
codenamed "Goat" ("Goat standard library").

Goat standard library is free software: you can redistribute it and/or
modify it under the terms of the GNU General Public License as published
by the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Goat standard library is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with Goat standard library.  If not, see <http://www.gnu.org/licenses/>.

*/

#define MG_ENABLE_CALLBACK_USERDATA 1

#include "goat_data.h"
#include "mongoose.h"
#include <memory.h>
#include <assert.h>
#include <ctype.h>

static struct mg_serve_http_opts s_http_server_opts;

typedef struct
{
    struct mg_mgr manager;
    struct mg_connection *connection;
    goat_function callback;
    const goat_function_caller *function_caller;
    const goat_allocator *allocator;
} web_server_data;

static void event_handler(struct mg_connection *connection, int event, void *event_data, void *user_data)
{
    web_server_data *me = (web_server_data*)user_data;
    assert(me != NULL);
    if (event == MG_EV_HTTP_REQUEST)
	{
        size_t i;
        struct http_message *http_data = (struct http_message *)event_data;
        goat_object *obj = create_goat_object(me->allocator);
        goat_object_add_record(me->allocator, obj, L"uri",
            create_goat_string_from_c_string_ext(me->allocator, http_data->uri.p, http_data->uri.len));
        wchar_t method[16];
        for (i = 0; i < http_data->method.len; i++)
            method[i] = (wchar_t)tolower(http_data->method.p[i]);
        goat_object_add_record(me->allocator, obj, L"method",
            create_goat_string_ext(me->allocator, method, http_data->method.len));
        goat_object_add_record(me->allocator, obj, L"query",
            create_goat_string_from_c_string_ext(me->allocator, http_data->query_string.p, http_data->query_string.len));
        goat_value * args[] = 
        {
            (goat_value*)obj
        };
        goat_value *response = call_goat_function(me->function_caller, &me->callback, me->allocator, 1, args);
        if (response->type == goat_type_null)
        {
            mg_serve_http(connection, http_data, s_http_server_opts);
        }
        else
        {
            assert(response->type == goat_type_byte_array);
            goat_byte_array *array = (goat_byte_array*)response;
            mg_printf(connection, "HTTP/1.1 200 OK\r\n");
            mg_printf(connection, "Content-Type: text/html, charset=utf-8\r\n");
            mg_printf(connection, "Content-Length: %d\r\n", array->length);
            mg_printf(connection, "\r\n");
            for (size_t i = 0; i < array->length; i++)
            {
                mg_printf(connection, "%c", array->data[i]);
            }
        }
    }
}

goat_value * create_server(const goat_shell *shell, const goat_allocator *allocator, int arg_count, goat_value **arg_list)
{
    if (arg_count < 2)
        return NULL;

    if (!is_goat_number(arg_list[0]) || arg_list[1]->type != goat_type_function)
        return NULL;

    int64_t port = goat_value_to_int64(arg_list[0]);
    if (port < 0 || port > 65536)
        return NULL;

    char port_str[16];
    sprintf(port_str, "%ld", port);

    web_server_data *data = calloc(1, sizeof(web_server_data));
    data->callback = *((goat_function*)arg_list[1]);
    mg_mgr_init(&data->manager, data);
    printf("Starting web server on port %ld\n", port);
    data->connection = mg_bind(&data->manager, port_str, event_handler, data);
    if (data->connection == NULL)
	{
        printf("Failed to create listener\n");
        return NULL;
    }
    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(data->connection);
    s_http_server_opts.document_root = ".";    // Serve current directory
    s_http_server_opts.enable_directory_listing = "yes";

    return create_goat_raw_data(allocator, data, "Web server");
}

goat_value * tick(const goat_shell *shell, const goat_allocator *allocator, int arg_count, goat_value **arg_list)
{
    if (arg_count > 0 && arg_list[0]->type == goat_type_raw_data)
    {
        goat_raw_data *arg = (goat_raw_data*)(arg_list[0]);
        web_server_data *me = (web_server_data*)(arg->raw_data);
        me->function_caller = shell->function_caller;
        me->allocator = allocator;
        mg_mgr_poll(&me->manager, 1000);
    }
    return NULL;
}
