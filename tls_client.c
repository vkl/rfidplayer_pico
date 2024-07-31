#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"
#include "lwip/dns.h"

#include "cast_channel.pb-c.h"
#include "cast_message.h"
#include "cast_controllers.h"

#include "tls_client.h"

#define MESSAGE_BUFFER 4096
#define POLL_TCP_INTERVAL 60

err_t tcp_recv_cb(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
    struct connectionState *cs = (struct connectionState *)arg;
    if (p != NULL) {
        // DEBUG_PRINT("recv total %d this buffer %d next %d err %d\n",  p->tot_len, p->len, p->next, err);
        if (p->tot_len > 2) {
            pbuf_copy_partial(p, (cs->recvData) + (cs->start), p->tot_len, 0);
            cs->start += p->tot_len;
            cs->recvData[cs->start] = 0;
            cs->state = INITIAL_DATA_PACKAGE;
            altcp_recved(tpcb, p->tot_len);
        }
        if (p->next == 0) {
            cs->state = DATA_READY;
            cs->start = 0;
        }
        pbuf_free(p);
    }
    return ERR_OK;
}

err_t tcp_sent_cb(void *arg, struct altcp_pcb *tpcb, u16_t len) {
    struct connectionState *cs = (struct connectionState *)arg;
    DEBUG_PRINT("len: %d, sent %d\n", len, cs->item->msgLen);
    return ERR_OK;
}

void tcp_error_cb(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        DEBUG_PRINT("client_err %d\n", err);
    }
}

err_t tcp_poll_cb(void *arg, struct altcp_pcb *tpcb) {
    DEBUG_PRINT("Connection Closed \n");
    struct connectionState *cs = (struct connectionState *)arg;
    cs->state = CONNECTION_CLOSE;
    return ERR_OK;
}

err_t connected(void *arg, struct altcp_pcb *tpcb, err_t err) {
    DEBUG_PRINT("connected\n");
    struct connectionState *cs = (struct connectionState *)arg;
    cs->state = CONNECTED;
    return ERR_OK;
}

int pollConnection(struct connectionState **pcs) {
    if (*pcs==NULL) return 0;
    struct connectionState *cs = *pcs;
    switch (cs->state) {
        case NOT_CONNECTED:
            break;
        case CONNECTING:
            break;
        case REQUEST_PENDING:
            // DEBUG_PRINT("request pending\n");
            break;
        case CONNECTED:
            // DEBUG_PRINT("connected\n");
            if (cs->item != NULL) {
                cs->state = READY_TO_SEND;
                cs->start = 0;
            }
            break;
        case READY_TO_SEND:
            DEBUG_PRINT("ready to send: %d\n", cs->item->msgLen);
            // DEBUG_PRINT("msg %s %d\n", cs->item->msg,cs->item->msgLen);
            // for (int i = 0; i < cs->item->msgLen; i++) {
            //     printf("%02x ", cs->item->msg[i]);
            // }
            // printf("\n");
            // sem_acquire_blocking(&semafore);
            cs->state = CONNECTED;
            cyw43_arch_lwip_begin();
            err_t err = altcp_write(cs->pcb, cs->item->msg, cs->item->msgLen, TCP_WRITE_FLAG_COPY);
            err = altcp_output(cs->pcb);
            cyw43_arch_lwip_end();
            if (cs->item->next != NULL) {
                struct MessageItem *tmp = cs->item->next;
                free(cs->item->msg);
                cs->item->msg = NULL;
                free(cs->item);
                cs->item = tmp;
            } else {
                free(cs->item->msg);
                cs->item->msg = NULL;
                free(cs->item);
                cs->item = NULL;
            }
            // sem_release(&semafore);
            break;
        case INITIAL_DATA_PACKAGE:
            cs->state = WAITING_MORE_DATA;
            break;
        case DATA_READY:
            // DEBUG_PRINT("data ready\n");
            break;
        case CONNECTION_CLOSE:
            DEBUG_PRINT("connection close\n");
            cyw43_arch_lwip_begin();
            altcp_close(cs->pcb);
            cyw43_arch_lwip_end();
            if (cs->recvData != NULL) {
                free(cs->recvData);
            }
            free(cs);
            *pcs = NULL;
            return 0;
    }
    return cs->state;
}

struct connectionState *newConnection() {
    struct connectionState *cs = (struct connectionState*)
                                malloc(sizeof(struct connectionState));

    struct altcp_tls_config *tls_config =  altcp_tls_create_config_client(NULL, 0);
    cs->pcb = altcp_tls_new(tls_config, IPADDR_TYPE_ANY); 

    altcp_recv(cs->pcb, tcp_recv_cb);
    altcp_sent(cs->pcb, tcp_sent_cb);
    altcp_err(cs->pcb, tcp_error_cb);
    altcp_poll(cs->pcb, tcp_poll_cb, POLL_TCP_INTERVAL);
    altcp_arg(cs->pcb, cs);
    cs->state = NOT_CONNECTED;
    cs->start = 0;
    cs->recvData = malloc(MESSAGE_BUFFER);
    return cs;
}

struct connectionState *doConnect(const char *ipaddr, int port) {
    ip_addr_t ip;
    ipaddr_aton(ipaddr, &ip);
    struct connectionState *cs = newConnection();
    mbedtls_ssl_set_hostname(altcp_tls_context(cs->pcb), ipaddr);
    cyw43_arch_lwip_begin();
    altcp_connect(cs->pcb, &ip, port, connected);
    cyw43_arch_lwip_end();
    cs->state = CONNECTING;
    cs->item = NULL;
    return cs;
}
