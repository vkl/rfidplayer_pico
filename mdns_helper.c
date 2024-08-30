#include <string.h>
#include <stdio.h>
#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <lwip/pbuf.h>
#include <lwip/udp.h>
#include <lwip/dns.h>
#include <lwip/prot/dns.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>

#include "casts.h"
#include "mdns_helper.h"

static u16_t 
parse_dns_name(struct pbuf *p, u16_t query_idx, char *name); 

void mdns_send(ChromeCastDevices *devices, callback __func) {

    const char *hostname = GOOGLECAST_TCP;
    const char *hostname_part;

    struct dns_hdr hdr;
    
    hdr.id = 0x0000;
    hdr.flags1 = 0;
    hdr.flags2 = 0;
    hdr.numquestions = PP_HTONS(1);
    hdr.numanswers = 0;
    hdr.numauthrr = 0;
    hdr.numextrarr = 0;

    struct dns_query qry;

    qry.type = PP_HTONS(DNS_RRTYPE_PTR);
    qry.cls = PP_HTONS(DNS_RRCLASS_IN | MDNS_UNICAST_RESPONSE);
    
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT,
            (u16_t)(SIZEOF_DNS_HDR + strlen(hostname) + 2 + SIZEOF_DNS_QUERY),
            PBUF_RAM);
    pbuf_take(p, &hdr, SIZEOF_DNS_HDR);

    u16_t query_idx, copy_len;
    u8_t n;
    
    query_idx = SIZEOF_DNS_HDR;
    --hostname;
    do {
        ++hostname;
        hostname_part = hostname;
        for (n = 0; *hostname != '.' && *hostname != 0; ++hostname) {
            ++n;
        }
        copy_len = (u16_t)(hostname - hostname_part);
        if (query_idx + n + 1 > 0xFFFF) {
            /* u16_t overflow */
            return;
        }
        pbuf_put_at(p, query_idx, n);
        pbuf_take_at(p, hostname_part, copy_len, (u16_t)(query_idx + 1));
        query_idx = (u16_t)(query_idx + n + 1);
    } while (*hostname != 0);
    pbuf_put_at(p, query_idx, 0);
    query_idx++;

    pbuf_take_at(p, &qry, SIZEOF_DNS_QUERY, query_idx);

    struct udp_pcb* pcb = udp_new();

    int err_t;
    ip_addr_t addr = DNS_MQUERY_IPV4_GROUP_INIT;

    err_t = udp_sendto(pcb, p, &addr, DNS_MQUERY_PORT);
    pbuf_free(p);
    if (err_t != ERR_OK) {
        LWIP_DEBUGF(DNS_DEBUG, ("Failed to send UDP packet! error=%d\n", err_t));
        return;
    }

    udp_recv(pcb, mdns_recv, (void*)devices);

    __func((void*)pcb);
}

void mdns_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p,
        const ip_addr_t *addr, u16_t port) {
    u8_t i;
    u16_t txid;
    u16_t res_idx = 0;
    struct dns_hdr hdr;
    struct dns_answer ans;
    struct dns_query qry;
    struct dns_srv_record srvr;
    u16_t nquestions, nanswers, txtLen, currPos, dataLen;
    char name[255] = {0};
    char target[255] = {0};
    char *txtData = NULL; 
    ip4_addr_t ip4addr;
    ChromeCastDevices *devices = (ChromeCastDevices*)arg;

    if (p == NULL) return;

    LWIP_DEBUGF(DNS_DEBUG, ("Received %d bytes\n", p->tot_len));

    /* is the dns message big enough ? */
    if (p->tot_len < (SIZEOF_DNS_HDR + SIZEOF_DNS_QUERY)) {
        LWIP_DEBUGF(DNS_DEBUG, ("dns_recv: pbuf too small\n"));
        /* free pbuf and return */
        goto ignore_packet;
    }

    if (pbuf_copy_partial(p, &hdr, SIZEOF_DNS_HDR, res_idx) != SIZEOF_DNS_HDR) {
        goto ignore_packet;
    }
    
    nquestions = lwip_htons(hdr.numquestions);
    nanswers   = lwip_htons(hdr.numanswers);
    nanswers += lwip_htons(hdr.numauthrr);
    nanswers += lwip_htons(hdr.numextrarr);

    /* Check for correct response. */
    if ((hdr.flags1 & DNS_FLAG1_RESPONSE) == 0) {
        LWIP_DEBUGF(DNS_DEBUG, ("dns_recv: not a response\n"));
        goto ignore_packet; /* ignore this packet */
    }
    res_idx = (u16_t)(res_idx + SIZEOF_DNS_HDR);

    /* Question section */
    while ((nquestions > 0) && (res_idx < p->tot_len)) {
        res_idx = parse_dns_name(p, res_idx, NULL);
        if (pbuf_copy_partial(p, &qry, SIZEOF_DNS_QUERY, res_idx) != SIZEOF_DNS_QUERY) {
            goto ignore_packet;
        }
        nquestions--;
        res_idx = (u16_t)(res_idx + SIZEOF_DNS_QUERY);
    }
    
    while ((nanswers > 0) && (res_idx < p->tot_len)) {
        res_idx = parse_dns_name(p, res_idx, NULL);
        if (pbuf_copy_partial(p, &ans, SIZEOF_DNS_ANSWER, res_idx) != SIZEOF_DNS_ANSWER) {
            LWIP_DEBUGF(DNS_DEBUG, ("dns_recv: error copy of answer\n"));
            goto ignore_packet; /* ignore this packet */
        }
        if (res_idx + SIZEOF_DNS_ANSWER > 0xFFFF) {
            LWIP_DEBUGF(DNS_DEBUG, ("dns_recv: res_idx + SIZEOF_DNS_ANSWER > 0xFFFF\n"));
            goto ignore_packet;
        }
        res_idx = (u16_t)(res_idx + SIZEOF_DNS_ANSWER);

        if (ans.cls == PP_HTONS(DNS_RRCLASS_IN)) {
            
            switch (PP_NTOHS(ans.type)) {
                case DNS_RRTYPE_PTR:
                    res_idx = parse_dns_name(p, res_idx, name);
                    LWIP_DEBUGF(DNS_DEBUG, ("%s\n", name));
                    break;
                case DNS_RRTYPE_A:
                    /* read the IP address after answer resource record's header */
                    if (pbuf_copy_partial(p, &ip4addr, sizeof(ip4_addr_t), res_idx) != sizeof(ip4_addr_t)) {
                        goto ignore_packet; /* ignore this packet */
                    }
                    LWIP_DEBUGF(DNS_DEBUG, ("%s\n", ipaddr_ntoa(&ip4addr)));
                    res_idx = (u16_t)(res_idx + sizeof(ip4_addr_t));
                    break;
                case DNS_RRTYPE_SRV:
                    if (pbuf_copy_partial(p, &srvr, SIZEOF_DNS_SRV_RECORD, res_idx) 
                            != SIZEOF_DNS_SRV_RECORD) {
                        goto ignore_packet;
                    }
                    res_idx = (u16_t)(res_idx + SIZEOF_DNS_SRV_RECORD);
                    res_idx = parse_dns_name(p, res_idx, target);
                    LWIP_DEBUGF(DNS_DEBUG, ("%u, %u, %u, %s\n",
                            PP_NTOHS(srvr.priority), PP_NTOHS(srvr.weight), PP_NTOHS(srvr.port), target));
                    break;
                case DNS_RRTYPE_TXT:
                    txtLen = 0;
                    currPos = 0; 
                    dataLen = PP_NTOHS(ans.len);
                    txtData = calloc(1, dataLen + 1);
                    do {
                        txtLen = pbuf_get_at(p, res_idx++);
                        pbuf_copy_partial(p, &txtData[currPos], txtLen, res_idx);
                        dataLen -= (txtLen + 1);
                        res_idx += txtLen;
                        currPos += txtLen;
                    } while (dataLen > 0);
                    break;
                default:
                    goto ignore_packet;
            }

            nanswers--;
        }
    }
    addChromeCastDevice(devices, name, txtData, target, ip4addr, PP_NTOHS(srvr.port));
    free(txtData);
ignore_packet:
    /* deallocate memory and return */
    pbuf_free(p);
    return;
}

static u16_t 
parse_dns_name(struct pbuf *p, u16_t query_idx, char *name) {
    u8_t j = 0;
    u8_t label_len = 0;
    u16_t tmp_pos = query_idx;  // Temporary position to track compressed names
    u16_t tmp = 0;              // Store the original position when following a compression pointer

    while (pbuf_get_at(p, tmp_pos) != 0) {
        if ((pbuf_get_at(p, tmp_pos) & 0xC0) == 0xC0) {
            if (tmp == 0) {
                tmp = tmp_pos + 2;  // Save the current position to return after the compressed label
            }
            tmp_pos = ((pbuf_get_at(p, tmp_pos) & 0x3F) << 8) | pbuf_get_at(p, tmp_pos + 1);
        } else {
            label_len = pbuf_get_at(p, tmp_pos);
            tmp_pos++;
            if (name != NULL) {
                pbuf_copy_partial(p, &name[j], label_len, tmp_pos);
                j += label_len;
                name[j++] = '.';
            } else {
                j += label_len + 1;
            }
            tmp_pos += (u16_t)label_len;
        }
    }
    if (name != NULL) name[j - 1] = '\0';  // Replace the last '.' with a null terminator
    query_idx = tmp > 0 ? tmp : tmp_pos + 1;  // Restore position after name parsing
    return query_idx;
}

