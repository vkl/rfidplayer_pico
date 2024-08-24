#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/dns.h>

#include "common.h"
#include "mdns_helper.h"


void found_hostname(const char *name, const ip_addr_t *ipaddr, void *arg) {
    DEBUG_PRINT("hostname: %s\n", name);
    char *ipaddress = ipaddr_ntoa(ipaddr);
    DEBUG_PRINT("ip addr: %s\n", ipaddress);
}

void sendMDNSRequest() {
    //struct udp_pcb *pcb = udp_new();
    //ip_addr_t dnssrv;
    //ipaddr_aton("224.0.0.251", &dnssrv);
    //dns_setserver(0, &dnssrv);
    err_t err;
    ip_addr_t addr;
    err = dns_gethostbyname_addrtype("48743c35-ea1a-bf8f-41ee-39096071fa7d.local",
            &addr, found_hostname, NULL, LWIP_DNS_ADDRTYPE_IPV4);
}
