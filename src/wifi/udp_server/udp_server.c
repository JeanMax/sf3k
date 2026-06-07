#include "udp_server.h"

#include <string.h>

#define RESPONSE_LEN_MAX 512


static void help_fun(const char *arg, char *response_buf)
{

}

static void status_fun(const char *arg, char *response_buf)
{

}

static void goal_fun(const char *arg, char *response_buf)
{

}

static void cool_range_fun(const char *arg, char *response_buf)
{

}

static void hot_range_fun(const char *arg, char *response_buf)
{

}

static void reboot_fun(const char *arg, char *response_buf)
{

}

static void pause_fun(const char *arg, char *response_buf)
{

}

#define MAX_CMD 7
#define CMD_LEN_MAX 16

char cmd_list[MAX_CMD][CMD_LEN_MAX] =  {
    "help",
    "status",
    "goal",
    "cool_range",
    "hot_range",
    "reboot",
    "pause",
};

t_cmd_callback *cmd_fun[MAX_CMD] =  {
    help_fun,
    status_fun,
    goal_fun,
    cool_range_fun,
    hot_range_fun,
    reboot_fun,
    pause_fun,
};



static char *run_cmd(char *payload)
{
    static char response_buf[RESPONSE_LEN_MAX] = {0};
    char *cmd = payload;
    char *arg = NULL;

    while (*payload) {
        if (*payload == ' ' || *payload == '\t') {
            *payload = 0;
            arg = payload + 1;
        } else if (arg) {
            break;
        }
        payload++;
    }

    for (int i = 0; i < MAX_CMD; i++) {
        if (!strcmp(cmd, cmd_list[i])) {
            cmd_fun[i](arg, response_buf);
        }
    }

    return response_buf;
}

static void udp_receive_callback(__unused void *arg, struct udp_pcb *upcb,
                                 struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    struct pbuf *txBuf;
    char buf[100];
    int len = sprintf(buf, "Hello %s From UDP SERVER\n",
                      (char *)p->payload);

    txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    pbuf_take(txBuf, buf, len);

    udp_connect(upcb, addr, port);
    udp_send(upcb, txBuf);
    udp_disconnect(upcb);

    pbuf_free(txBuf);
    pbuf_free(p);
}


struct udp_pcb *udp_server_init(void)
{
    struct udp_pcb *upcb = udp_new();
    err_t err = udp_bind(upcb, NULL, UDP_PORT);

    if (err == ERR_OK) {
        udp_recv(upcb, udp_receive_callback, NULL);
        return upcb;
    }

    udp_remove(upcb);
    return NULL;
}

void udp_server_deinit(struct udp_pcb *upcb)
{
    udp_remove(upcb);
}
