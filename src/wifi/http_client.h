#ifndef _HTTP_CLIENT_H
#define _HTTP_CLIENT_H

#include <stdint.h>

#define EOL "\r\n"


typedef struct http_client_conf  t_http_client_conf;
struct http_client_conf {
    const char *host;
    const uint8_t *tls_cert;
    unsigned long tls_len;
};
//TODO: init / free conf


/** @brief  TODO
 *
 * @param TODO
 *
 * @return TODO
 *
 * @note: TODO
 */
int http_request(t_http_client_conf *conf,
             const char *url, const char *headers, const char *content);

/* #define URL_REQUEST "/net_install/boot.sig" */

#endif
