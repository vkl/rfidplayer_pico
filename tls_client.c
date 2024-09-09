#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp.h"
#include "lwip/altcp_tls.h"

#include "common.h"
#include "cast_control.h"
#include "tls_client.h"

#define MESSAGE_BUFFER 4096
#define POLL_TCP_INTERVAL 60

err_t tcp_recv_cb(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
    struct ConnectionState *cs = (struct ConnectionState *)arg;
    if (p != NULL) {
        DEBUG_PRINT("recv total %d this buffer %d %p err %d\n",  p->tot_len, p->len, p->next, err);
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
            processData(cs->castState, cs->recvData);
        }
        pbuf_free(p);
    } else {
        cs->state = CONNECTED;
    }
    return ERR_OK;
}

err_t tcp_sent_cb(void *arg, struct altcp_pcb *tpcb, u16_t len) {
    struct ConnectionState *cs = (struct ConnectionState *)arg;
    return ERR_OK;
}

void tcp_error_cb(void *arg, err_t err) {
    if (err != ERR_ABRT) {
        DEBUG_PRINT("client_err %d\n", err);
    }
}

err_t tcp_poll_cb(void *arg, struct altcp_pcb *tpcb) {
    DEBUG_PRINT("Connection Closed \n");
    struct ConnectionState *cs = (struct ConnectionState *)arg;
    cs->state = CONNECTION_CLOSE;
    return ERR_OK;
}

err_t connected(void *arg, struct altcp_pcb *tpcb, err_t err) {
    DEBUG_PRINT("connected\n");
    struct ConnectionState *cs = (struct ConnectionState *)arg;
    cs->state = CONNECTED;
    return ERR_OK;
}

int
pollConnection(struct ConnectionState **pcs,
        struct MessageQueueItem **msgQueueItem,
        struct MessageItem *msgItem) {
    
    if (*pcs==NULL) return 0;
    
    struct ConnectionState *cs = *pcs;
    
    switch (cs->state) {
        case NOT_CONNECTED:
        case CONNECTING:
        case PREPARE_DATA:
        case REQUEST_PENDING:
        case DATA_READY:
            break;
        case CONNECTED:
            if ((*msgQueueItem) != NULL) {
                cs->state = PREPARE_DATA;
                cs->start = 0;
                printf("prepare data\n");
            }
            break;
        case READY_TO_SEND:
            printf("ready to send\n");
            cyw43_arch_lwip_begin();
            err_t err = altcp_write(cs->pcb, msgItem->msg, msgItem->msgLen, TCP_WRITE_FLAG_COPY);
            err = altcp_output(cs->pcb);
            cyw43_arch_lwip_end();
            cs->state = CONNECTED;
            free(msgItem->msg);
            break;
        case INITIAL_DATA_PACKAGE:
            cs->state = WAITING_MORE_DATA;
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

struct ConnectionState *newConnection() {
    struct ConnectionState *cs = (struct ConnectionState*)
                                malloc(sizeof(struct ConnectionState));

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

struct ConnectionState *doConnect(ip_addr_t ip, const char *hostname, int port) {
    struct ConnectionState *cs = newConnection();
    mbedtls_ssl_set_hostname(altcp_tls_context(cs->pcb), hostname);
    cyw43_arch_lwip_begin();
    altcp_connect(cs->pcb, &ip, port, connected);
    cyw43_arch_lwip_end();
    cs->state = CONNECTING;
    return cs;
}

