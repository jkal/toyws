#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <bstring.h>

#include "protocol.h"
#include "http_parser.h"

static http_parser *parser;

int request_path_cb (http_parser *p, const char *buf, size_t len)
{
    assert(p == parser);

    Request *req = (Request*)p->data;
    req->path = blk2bstr(buf, len);

    return 0;
}

int request_url_cb (http_parser *p, const char *buf, size_t len)
{
    assert(p == parser);

    Request *req = (Request*)p->data;
    req->uri = blk2bstr(buf, len);

    return 0;
}

int query_string_cb (http_parser *p, const char *buf, size_t len)
{
    assert(p == parser);

    Request *req = (Request*)p->data;
    req->query_string = blk2bstr(buf, len);

    return 0;
}

int fragment_cb (http_parser *p, const char *buf, size_t len)
{
    assert(p == parser);

    Request *req = (Request*)p->data;
    req->fragment = blk2bstr(buf, len);

    return 0;
}

int header_field_cb (http_parser *p, const char *buf, size_t len)
{
    assert(p == parser);
    // TODO:
    return 0;
}

int header_value_cb (http_parser *p, const char *buf, size_t len)
{
    assert(p == parser);
    // TODO:
    return 0;
}

int body_cb (http_parser *p, const char *buf, size_t len)
{
    assert(p == parser);
    return 0;
}


int message_begin_cb (http_parser *p)
{
    assert(p == parser);
    return 0;
}

int headers_complete_cb (http_parser *p)
{
    assert(p == parser);
    Request *req = (Request*)p->data;

    req->request_method = bfromcstr(http_method_str(p->method));
    req->version = bformat("%d.%d", p->http_major, p->http_minor);

    return 0;
}

int message_complete_cb (http_parser *p)
{
    assert(p == parser);
    return 0;
}

Request *requestNew(char *rbuf, size_t count)
{
    Request *req = calloc(sizeof(Request), 1);
    int nparsed;

    static http_parser_settings settings =
        {.on_message_begin = message_begin_cb
        ,.on_header_field = header_field_cb
        ,.on_header_value = header_value_cb
        ,.on_path = request_path_cb
        ,.on_url = request_url_cb
        ,.on_fragment = fragment_cb
        ,.on_query_string = query_string_cb
        ,.on_body = body_cb
        ,.on_headers_complete = headers_complete_cb
        ,.on_message_complete = message_complete_cb
        };

    parser = NULL;
    parser = malloc(sizeof(http_parser));
    if (!parser) {
        fprintf(stderr, "FATAL: Failed to allocate parser.");
        exit(-1);
    }

    http_parser_init(parser, HTTP_REQUEST);

    parser->data = req;

    nparsed = http_parser_execute(parser, &settings, rbuf, count);

    return req;
}

#ifdef TEST_HTTP_PARSER
int main (int argc, char const *argv[])
{
    Request *req;
    char buf[] = "GET /path/file.html HTTP/1.1\r\nFrom: user@example.com\r\nUser-Agent: HTTPTool/1.0\r\n\r\n";

    req = reqCreate(buf, sizeof(buf));
    puts(bdata(req->request_method));
    puts(bdata(req->version));
    puts(bdata(req->path));
    puts(bdata(req->uri));
    puts(bdata(req->query_string));

    return 0;
}
#endif