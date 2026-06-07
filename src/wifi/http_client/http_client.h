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


/** @brief  Perform a http request synchronously
 *
 * @param host  the name of the host, e.g. rmrf.fr
 * @param url  the url to request, e.g. /favicon.ico
 * @param headers  extra headers to add, e.g. "Content-Type: application/json" EOL (can be NULL)
 * @param content  extra content to pass as a POST request (can be NULL, then GET request)
 *
 * @return 0 if success (otherwise the error from `http_client_request_sync`)
 */
int http_request(const char *host, const char *url,
                 const char *headers, const char *content);

/* #define URL_REQUEST "/net_install/boot.sig" */

#endif
