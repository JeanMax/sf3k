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
    LOG_DEBUG("Http request response:");
    u16_t offset = 0;
    while (offset < p->tot_len) {
        char c = (char)pbuf_get_at(p, offset++);
        LOG_CHR(c);
    }
    return ERR_OK;
}

#define DEFAULT_HEADER                          \
    "User-Agent: sf3k" EOL                      \
    "Accept: */*" EOL                           \
    "Connection: Close" EOL

#define FORMAT_GET(url, headers, host)          \
    "GET %s HTTP/1.1" EOL                       \
    DEFAULT_HEADER                              \
    "%s"                                        \
    "Host: %s" EOL EOL,                         \
     url, headers ? headers : "", host

#define FORMAT_POST(url, headers, host, content)                    \
    "POST %s HTTP/1.1" EOL                                          \
    DEFAULT_HEADER                                                  \
    "%s"                                                            \
    "Host: %s" EOL                                                  \
    "Content-length: %d" EOL EOL                                    \
    "%s" EOL EOL,                                                   \
    url, headers ? headers : "", host, strlen(content), content

#define ENCODE_BUF_SIZE 1024

static const char *encode_url(const char *host,
                              const char *url, const char *headers, const char *content)
{
    static char buf[ENCODE_BUF_SIZE];

    if (!content) {
        snprintf(buf, ENCODE_BUF_SIZE, FORMAT_GET(url, headers, host));
    } else {
        snprintf(buf, ENCODE_BUF_SIZE, FORMAT_POST(url, headers, host, content));
    }

    LOG_DEBUG("to send:\n%s", buf);

    return buf;
}

int http_request(const char *host, const char *url,
                 const char *headers, const char *content)
{
    HTTP_REQUEST_T req = {0};
    req.hostname = host;
    req.url = encode_url(host, url, headers, content);
#ifndef NDEBUG
    req.recv_fn = http_client_receive_print_fn;
#endif
    return http_client_request_sync(cyw43_arch_async_context(), &req);;
}
