#ifndef _MDNS_HELPER_H_
#define _MDNS_HELPER_H_

#include <lwip/udp.h>
#include "casts.h"

#define MDNS_PORT 5353
#define MDNS_ADDR 224.0.0.251
#define MDNS_UNICAST_RESPONSE 0x8000
#define SIZEOF_DNS_QUERY 4
#define SIZEOF_DNS_ANSWER 10
#define SIZEOF_DNS_SRV_RECORD 6
#define GOOGLECAST_TCP "_googlecast._tcp.local"
#define BUF_SIZE 512

PACK_STRUCT_BEGIN
struct dns_query {
    PACK_STRUCT_FIELD(u16_t type);
    PACK_STRUCT_FIELD(u16_t cls);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct dns_answer {
    PACK_STRUCT_FIELD(u16_t type);
    PACK_STRUCT_FIELD(u16_t cls);
    PACK_STRUCT_FIELD(u32_t ttl);
    PACK_STRUCT_FIELD(u16_t len);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

PACK_STRUCT_BEGIN
struct dns_srv_record {
    PACK_STRUCT_FIELD(u16_t priority);
    PACK_STRUCT_FIELD(u16_t weight);
    PACK_STRUCT_FIELD(u16_t port);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

typedef void (*callback) (void*arg);

void mdns_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
    const ip_addr_t *addr, u16_t port);
void mdns_send(ChromeCastDevices *devices, callback __func);

#endif
