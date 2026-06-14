#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <lwip/udp.h>

#define UDP_PORT 3000


typedef int t_cmd_callback(const char *arg, char *response_buf);

//TODO: doc

bool is_udp_asking_pause(void);

struct udp_pcb *udp_server_init(void);
void udp_server_deinit(struct udp_pcb *upcb);

#endif
