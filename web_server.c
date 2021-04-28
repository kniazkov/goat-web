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

#include "goat_data.h"
#include "mongoose.h"
#include <memory.h>

static struct mg_serve_http_opts s_http_server_opts;

static void ev_handler(struct mg_connection *nc, int ev, void *p)
{
    if (ev == MG_EV_HTTP_REQUEST)
	{
        mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
    }
}

typedef struct
{
    struct mg_mgr manager;
    struct mg_connection *connection;
} web_server_data;

goat_value * create_server(const goat_shell *shell, const goat_allocator *allocator, int arg_count, goat_value **arg_list)
{
    if (arg_count < 1)
        return NULL;

    if (!is_goat_number(arg_list[0]))
        return NULL;

    int64_t port = goat_value_to_int64(arg_list[0]);
    if (port < 0 || port > 65536)
        return NULL;

    char port_str[16];
    sprintf(port_str, "%ld", port);

    web_server_data *data = calloc(1, sizeof(web_server_data));
    mg_mgr_init(&data->manager, NULL);
    printf("Starting web server on port %ld\n", port);
    data->connection = mg_bind(&data->manager, port_str, ev_handler);
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
        web_server_data *data = (web_server_data*)(arg->raw_data);
        mg_mgr_poll(&data->manager, 1000);
    }
    return NULL;
}

/*
int main(void)
{
    struct mg_mgr mgr;
    struct mg_connection *nc;

    mg_mgr_init(&mgr, NULL);
    printf("Starting web server on port %s\n", s_http_port);
    nc = mg_bind(&mgr, s_http_port, ev_handler);
    if (nc == NULL)
	{
        printf("Failed to create listener\n");
        return 1;
    }

    // Set up HTTP server parameters
    mg_set_protocol_http_websocket(nc);
    s_http_server_opts.document_root = ".";    // Serve current directory
    s_http_server_opts.enable_directory_listing = "yes";

    for (;;)
	{
        mg_mgr_poll(&mgr, 1000);
    }
    mg_mgr_free(&mgr);

    return 0;
}
*/