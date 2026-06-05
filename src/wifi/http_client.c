#include "http_client.h"

#include <pico/cyw43_arch.h>
#include <lwip/altcp_tls.h>
#include <lwip/netif.h>
#include <FreeRTOS.h>
#include <task.h>

#include "utils/log.h"
#include "http_client_util.h"


// Print body to stdout
static err_t http_client_receive_print_fn(__unused void *arg, __unused struct altcp_pcb *conn, struct pbuf *p, err_t err) {
    if (err) {
        LOG_ERROR("Error in http request response: %d", err);
    }
#ifndef NDEBUG
    LOG_DEBUG("Http request response:");
    u16_t offset = 0;
    while (offset < p->tot_len) {
        char c = (char)pbuf_get_at(p, offset++);
        LOG_CHR(c);
    }
#endif
    return ERR_OK;
}

#define ENCODE_BUF_SIZE 2048

static const char *encode_url(const char *host,
                              const char *url, const char *headers, const char *content)
{
    static char buf[ENCODE_BUF_SIZE];

    if (!content) {
        snprintf(buf, ENCODE_BUF_SIZE,
                 "GET %s HTTP/1.0" EOL EOL,
                 url);
    } else {
        snprintf(buf, ENCODE_BUF_SIZE,
                 "POST %s HTTP/1.0" EOL
                 "%s" EOL
                 "Host: %s" EOL
                 "User-Agent: sf3k" EOL
                 "Accept: */*" EOL
                 "Content-length: %d" EOL EOL
                 "%s" EOL EOL,
                 url, headers, host, strlen(content), content);

    }

    return buf;
}

int http_request(t_http_client_conf *conf,
                 const char *url, const char *headers, const char *content)
{
    static EXAMPLE_HTTP_REQUEST_T req = {0};
    req.hostname = conf->host;
    req.url = encode_url(conf->host, url, headers, content);
    req.recv_fn = http_client_receive_print_fn; //TODO: param?
    req.tls_config = altcp_tls_create_config_client(conf->tls_cert, conf->tls_len);

    int pass = http_client_request_sync(cyw43_arch_async_context(), &req);
    altcp_tls_free_config(req.tls_config);

    return pass;
}
