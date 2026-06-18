#include "udp_server.h"

#include <pico/bootrom.h>
#include <string.h>

// for ADC read (pi temp)
#include "driver/photor.h"
#include "temp_ctrl/hysteresis.h"
#include "utils/persist.h"
#include "utils/log.h"
#include "utils/datetime.h"
#include "shared.h"
#include "PinConfig.h"

#define RESPONSE_LEN_MAX 512

volatile bool g_is_paused = false;

static int help_fun(__unused const char *arg, char *response_buf) {
    return snprintf(response_buf, RESPONSE_LEN_MAX,
                    "SF3K commands:\n\n"
                    "help -- you found this already\n"
                    "status -- show values for various sensors / relays\n"
                    "config -- show values for various settings\n"
                    "goal N -- set the goal temperature to N (int)\n"
                    "cool_range N -- set the cool_range value to N (float)\n"
                    "hot_range N -- set the hot_range value to N (float)\n"
                    "reboot -- reboot the pi to app\n"
                    "bootsel -- reboot the pi to bootsel\n"
                    "pause -- stop toggling relays until this command is issued again\n\n");
}

static int status_fun(__unused const char *arg, char *response_buf) {
    return snprintf(response_buf, RESPONSE_LEN_MAX,
                    "temp=%.1f, "
                    "room=%.1f, "
                    "goal=%.1f, "
                    "state=%s, "
                    "pause=%d, "
                    "uptime='%s'\n",
                    shared__current_temp,
                    ROOM_TEMP(read_onboard_temperature(INTERNAL_TEMP_ADC_CHANNEL)),
                    shared__goal_temp,
                    shared__state == WAIT ? "off" :
                        (shared__state == HEAT ? "heating" : "cooling"),
                    g_is_paused,
                    get_timestamp_str());
}

static int config_fun(__unused const char *arg, char *response_buf) {
    t_ctrl_temp_conf *conf = get_ctrl_temp_conf();

    return snprintf(response_buf, RESPONSE_LEN_MAX,
                    "goal=%d, "
                    "hot_range=%.1f, "
                    "cool_range=%.1f, "
                    "cooling_start=%.1f, "
                    "cooling_stop=%.1f, "
                    "heating_start=%.1f, "
                    "heating_stop=%.1f\n",
                    shared__goal_temp,
                    shared__hot_range,
                    shared__cool_range,
                    conf->cooling_start,
                    conf->cooling_stop,
                    conf->heating_start,
                    conf->heating_stop);
}

static int goal_fun(const char *arg, char *response_buf) {
    int old = shared__goal_temp;
    int new;

    int ret = sscanf(arg, "%d", &new);
    if (ret > 0) {
        shared__goal_temp = new;
        save_persistent_config();
        return sprintf(response_buf, "goal=%d (prev=%d)\n", new, old);
    }
    return sprintf(response_buf, "goal: invalid arg '%s'\n", arg);
}

static int cool_range_fun(const char *arg, char *response_buf) {
    float old = shared__cool_range;
    float new;

    int ret = sscanf(arg, "%f", &new);
    if (ret > 0) {
        shared__cool_range = new;
        save_persistent_config();
        return sprintf(response_buf, "cool_range=%.1f (prev=%.1f)\n", new, old);
    }
    return sprintf(response_buf, "cool_range: invalid arg '%s'\n", arg);
}

static int hot_range_fun(const char *arg, char *response_buf) {
    float old = shared__hot_range;
    float new;

    int ret = sscanf(arg, "%f", &new);
    if (ret > 0) {
        shared__hot_range = new;
        save_persistent_config();
        return sprintf(response_buf, "hot_range=%.1f (prev=%.1f)\n", new, old);
    }
    return sprintf(response_buf, "hot_range: invalid arg '%s'\n", arg);
}

static int reboot_fun(__unused const char *arg, __unused char *response_buf) {
    LOG_INFO("Waiting to be rebooted by watchdog");
    while(42) {}
    return 0;
}

static int bootsel_fun(__unused const char *arg, __unused char *response_buf) {
    LOG_INFO("Bootsel");
    reset_usb_boot(0, 0);
    return 0;
}

static int pause_fun(__unused const char *arg, char *response_buf) {
    g_is_paused ^= 1;
    return sprintf(response_buf, "pause=%d (prev=%d)\n", g_is_paused, g_is_paused ^ 1);
}

bool is_udp_asking_pause() {
    return g_is_paused;
}

#define MAX_CMD 9
#define CMD_LEN_MAX 16

char cmd_list[MAX_CMD][CMD_LEN_MAX] =  {
    "help",
    "status",
    "config",
    "goal",
    "cool_range",
    "hot_range",
    "reboot",
    "bootsel",
    "pause",
};

t_cmd_callback *cmd_fun[MAX_CMD] =  {
    help_fun,
    status_fun,
    config_fun,
    goal_fun,
    cool_range_fun,
    hot_range_fun,
    reboot_fun,
    bootsel_fun,
    pause_fun,
};


#define IS_SPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\n')

static int run_cmd(const char *payload, int payload_len, char *response_buf) {
    char *s = (char *)payload;
    while (*s && !IS_SPACE(*s)
           && (s - payload) < CMD_LEN_MAX
           && (s - payload) < payload_len) {
        s++;
    }

    char cmd[CMD_LEN_MAX] = {0};
    memcpy(cmd, payload, s - payload);

    if (!*cmd) {
        return 0;
    }

    while (*s && IS_SPACE(*s)
           && (s - payload) < payload_len) {
        s++;
    }

    char *arg_start = s;
    while (*s && !IS_SPACE(*s)
           && (s - arg_start) < CMD_LEN_MAX
           && (s - payload) < payload_len) {
        s++;
    }

    char arg[CMD_LEN_MAX] = {0};
    memcpy(arg, arg_start, s - arg_start);

    LOG_DEBUG("UDP: received [%s] [%s]", cmd, arg);

    for (int i = 0; i < MAX_CMD; i++) {
        if (!strcmp(cmd, cmd_list[i])) {
            return cmd_fun[i](arg, response_buf);
        }
    }

    return sprintf(response_buf, "sf3k: command not found: '%s' (try 'help'?)\n", cmd);
}

static void udp_receive_callback(__unused void *arg, struct udp_pcb *upcb,
                                 struct pbuf *p, const ip_addr_t *addr, u16_t port) {
    static char response_buf[RESPONSE_LEN_MAX] = {0};

    int len = run_cmd((char *)p->payload, p->len, response_buf);
    if (len > 0) {
        struct pbuf *txBuf = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
        pbuf_take(txBuf, response_buf, len);

        udp_connect(upcb, addr, port);
        udp_send(upcb, txBuf);
        udp_disconnect(upcb);

        pbuf_free(txBuf);
    }
    pbuf_free(p);
}


struct udp_pcb *udp_server_init(void) {
    struct udp_pcb *upcb = udp_new();
    err_t err = udp_bind(upcb, NULL, UDP_PORT);

    if (err == ERR_OK) {
        udp_recv(upcb, udp_receive_callback, NULL);
        return upcb;
    }

    udp_remove(upcb);
    return NULL;
}

void udp_server_deinit(struct udp_pcb *upcb) {
    udp_remove(upcb);
}
