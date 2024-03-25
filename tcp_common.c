#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/altcp.h"

#include "tcp_common.h"

#define HEADER "GET /index.html HTTP/1.1\r\n" \
               "Host: example.com\r\n" \
               "Connection: close\r\n" \
               "\r\n"

err_t sent_cb(void *arg, struct altcp_pcb *pcb, u16_t len) {
    printf("send some data...\n");
    return ERR_OK;
}

err_t recv_cb(void *arg, struct altcp_pcb *tpcb, struct pbuf *p, err_t err) {
    if (p != NULL) {
        printf("Recieved %d\n", p->tot_len);
        char *buffer = malloc((p->tot_len + 1) * sizeof(char));
        buffer[p->tot_len] = 0;
        pbuf_copy_partial(p, buffer, p->tot_len, 0);
        printf(buffer);
        altcp_recved(tpcb, p->tot_len);
        pbuf_free(p);
        free(buffer);
    } else {
        altcp_close(tpcb);
    }
    return ERR_OK;
}

err_t connected_cb(void *arg, struct altcp_pcb *tpcb, err_t err) {
    err = altcp_write(tpcb, HEADER, strlen(HEADER), TCP_WRITE_FLAG_COPY);
    err = altcp_output(tpcb);
}

void err_cb (void *arg, err_t err) {
    return ERR_OK;
}

void tcp_setup() {
    struct altcp_pcb *pcb = altcp_new();

    altcp_sent(pcb, sent_cb);
    altcp_recv(pcb, recv_cb);
    altcp_err(pcb, err_cb);

    ip_addr_t ip;
    IP4_ADDR(&ip, 93, 184, 216, 34);
    cyw43_arch_lwip_begin();
    altcp_connect(pcb, &ip, 80, connected_cb);
    cyw43_arch_lwip_end();
}
