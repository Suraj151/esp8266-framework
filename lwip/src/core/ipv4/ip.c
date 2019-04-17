/**
 * @file
 * This is the IPv4 layer implementation for incoming and outgoing IP traffic.
 *
 * @see ip_frag.c
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"
#include "lwip/ip.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/ip_frag.h"
#include "lwip/inet_chksum.h"
#include "lwip/netif.h"
#include "lwip/icmp.h"
#include "lwip/igmp.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "lwip/autoip.h"
#include "lwip/stats.h"
#include "lwip/lwip_napt.h"
#ifdef IP_ROUTING_TAB
#include "lwip/ip_route.h"
#endif
#include "arch/perf.h"

#include <string.h>

/** Set this to 0 in the rare case of wanting to call an extra function to
 * generate the IP checksum (in contrast to calculating it on-the-fly). */
#ifndef LWIP_INLINE_IP_CHKSUM
#define LWIP_INLINE_IP_CHKSUM   1
#endif
#if LWIP_INLINE_IP_CHKSUM && CHECKSUM_GEN_IP
#define CHECKSUM_GEN_IP_INLINE  1
#else
#define CHECKSUM_GEN_IP_INLINE  0
#endif

#if LWIP_DHCP || defined(LWIP_IP_ACCEPT_UDP_PORT)
#define IP_ACCEPT_LINK_LAYER_ADDRESSING 1

/** Some defines for DHCP to let link-layer-addressed packets through while the
 * netif is down.
 * To use this in your own application/protocol, define LWIP_IP_ACCEPT_UDP_PORT
 * to return 1 if the port is accepted and 0 if the port is not accepted.
 */
#if LWIP_DHCP && defined(LWIP_IP_ACCEPT_UDP_PORT)
/* accept DHCP client port and custom port */
#define IP_ACCEPT_LINK_LAYER_ADDRESSED_PORT(port) (((port) == PP_NTOHS(DHCP_CLIENT_PORT)) \
         || (LWIP_IP_ACCEPT_UDP_PORT(port)))
#elif defined(LWIP_IP_ACCEPT_UDP_PORT) /* LWIP_DHCP && defined(LWIP_IP_ACCEPT_UDP_PORT) */
/* accept custom port only */
#define IP_ACCEPT_LINK_LAYER_ADDRESSED_PORT(port) (LWIP_IP_ACCEPT_UDP_PORT(dst_port))
#else /* LWIP_DHCP && defined(LWIP_IP_ACCEPT_UDP_PORT) */
/* accept DHCP client port only */
#define IP_ACCEPT_LINK_LAYER_ADDRESSED_PORT(port) ((port) == PP_NTOHS(DHCP_CLIENT_PORT))
#endif /* LWIP_DHCP && defined(LWIP_IP_ACCEPT_UDP_PORT) */

#else /* LWIP_DHCP */
#define IP_ACCEPT_LINK_LAYER_ADDRESSING 0
#endif /* LWIP_DHCP */

/**
 * The interface that provided the packet for the current callback
 * invocation.
 */
struct netif *current_netif;

/**
 * Header of the input packet currently being processed.
 */
const struct ip_hdr *current_header;
/** Source IP address of current_header */
ip_addr_t current_iphdr_src;
/** Destination IP address of current_header */
ip_addr_t current_iphdr_dest;

/** The IP header ID of the next outgoing IP packet */
static u16_t ip_id;

#ifdef IP_ROUTING_TAB
/** Destination IP address of current_header after routing */
ip_addr_t current_ip_new_dest;
#endif /* IP_ROUTING_TAB */

/**
 * Finds the appropriate network interface for a given IP address. It
 * searches the list of network interfaces linearly. A match is found
 * if the masked IP address of the network interface equals the masked
 * IP address given to the function.
 *
 * @param dest the destination IP address for which to find the route
 * @return the netif on which to send to reach dest
 */
struct netif *
ip_route(ip_addr_t *dest)
{
  struct netif *netif;

  #ifdef IP_ROUTING_TAB
    ip_addr_copy(current_ip_new_dest, *dest);
  #endif

  #ifdef IP_ROUTING_TAB
    int i;
  //os_printf_plus("ip_route route to %d.%d.%d.%d\r\n",
  //          ip4_addr1_16(dest), ip4_addr2_16(dest), ip4_addr3_16(dest), ip4_addr4_16(dest));
    /* search route */
    struct route_entry *found_route = ip_find_route(*dest);
    if (found_route) {
      ip_addr_copy(current_ip_new_dest, found_route->gw);

      /* now go on and find the netif on which to forward the packet */
      dest = &current_ip_new_dest;
  //os_printf_plus("redirected to %d.%d.%d.%d\r\n",
  //          ip4_addr1_16(dest), ip4_addr2_16(dest), ip4_addr3_16(dest), ip4_addr4_16(dest));
    }
  #endif /* IP_ROUTING_TAB */

  /* iterate through netifs */
  for(netif = netif_list; netif != NULL; netif = netif->next) {
    /* network mask matches? */
    if (netif_is_up(netif)) {
      if (ip_addr_netcmp(dest, &(netif->ip_addr), &(netif->netmask))) {
        /* return netif on which to forward IP packet */
        return netif;
      }
    }
  }
  /* iterate through netifs */
  for(netif = netif_list; netif != NULL; netif = netif->next) {
    /* network mask matches? */
    if (netif_is_up(netif)) {
      if (!ip_addr_isbroadcast(dest, netif) && netif == (struct netif *)eagle_lwip_getif(0)) {
        return netif;
      }
    }
  }
  if ((netif_default == NULL) || (!netif_is_up(netif_default))) {
    LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("ip_route: No route to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
      ip4_addr1_16(dest), ip4_addr2_16(dest), ip4_addr3_16(dest), ip4_addr4_16(dest)));
    IP_STATS_INC(ip.rterr);
    snmp_inc_ipoutnoroutes();
    return NULL;
  }
  /* no matching netif found, use default netif */
  return netif_default;
}

/**
 * Finds the appropriate network interface for a source IP address. It
 * searches the list of network interfaces linearly. A match is found
 * if the masked IP address of the network interface equals the masked
 * IP address given to the function.
 *
 * @param source the sourcination IP address for which to find the route
 * @return the netif on which to send to reach source
 */

struct netif *ICACHE_FLASH_ATTR
ip_router(ip_addr_t *dest, ip_addr_t *source){
	struct netif *netif;
	/* iterate through netifs */
  	for(netif = netif_list; netif != NULL; netif = netif->next) {
	    /* network mask matches? */

		if (netif_is_up(netif)) {
	      if (ip_addr_netcmp(dest, &(netif->ip_addr), &(netif->netmask))) {
	        /* return netif on which to forward IP packet */
	        return netif;
	      }
	    }

		if (netif_is_up(netif)) {
	      if (ip_addr_netcmp(source, &(netif->ip_addr), &(netif->netmask))) {
	        /* return netif on which to forward IP packet */
	        return netif;
	      }
	    }
  	}

	if ((netif_default == NULL) || (!netif_is_up(netif_default))) {
	    LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("ip_route: No route to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
	      ip4_addr1_16(dest), ip4_addr2_16(dest), ip4_addr3_16(dest), ip4_addr4_16(dest)));
	    IP_STATS_INC(ip.rterr);
	    snmp_inc_ipoutnoroutes();
	    return NULL;
  	}
  	/* no matching netif found, use default netif */
  	os_printf("ip_router %d %p\n", __LINE__, netif_default);
  	return netif_default;
}

#if IP_FORWARD
#if IP_NAPT

#define NO_IDX ((u16_t)-1)
#define NT(x) ((x) == NO_IDX ? NULL : &ip_napt_table[x])

u16_t napt_list = NO_IDX, napt_list_last = NO_IDX, napt_free = 0;

static struct napt_table *ip_napt_table;
struct portmap_table *ip_portmap_table;

int nr_active_napt_tcp = 0, nr_active_napt_udp = 0, nr_active_napt_icmp = 0;
uint16_t ip_napt_max = 0;
uint8_t ip_portmap_max = 0;
uint32_t ip_napt_tcp_timeout = IP_NAPT_TIMEOUT_MS_TCP;
uint32_t ip_napt_udp_timeout = IP_NAPT_TIMEOUT_MS_UDP;

void ICACHE_FLASH_ATTR
ip_napt_init(uint16_t max_nat, uint8_t max_portmap)
{
  u16_t i;

  ip_napt_max = max_nat;
  ip_portmap_max = max_portmap;

  ip_napt_table = (struct napt_table*)os_zalloc(sizeof(struct napt_table[ip_napt_max]));
  ip_portmap_table = (struct portmap_table*)os_zalloc(sizeof(struct portmap_table[ip_portmap_max]));

  for (i = 0; i < ip_napt_max - 1; i++)
    ip_napt_table[i].next = i + 1;
  ip_napt_table[i].next = NO_IDX;
}

void ICACHE_FLASH_ATTR
ip_napt_enable(u32_t addr, int enable)
{
  struct netif *netif;
  for (netif = netif_list; netif; netif = netif->next) {
    if (netif_is_up(netif) && !ip_addr_isany(&netif->ip_addr) && netif->ip_addr.addr == addr) {
      netif->napt = !!enable;
      break;
    }
  }
}

void ICACHE_FLASH_ATTR
ip_napt_enable_no(u8_t number, int enable)
{
  struct netif *netif;
  for (netif = netif_list; netif; netif = netif->next) {
    if (netif->num == number) {
      netif->napt = !!enable;
      break;
    }
  }
}

void ICACHE_FLASH_ATTR checksumadjust(unsigned char *chksum, unsigned char *optr,
   int olen, unsigned char *nptr, int nlen)
   /* assuming: unsigned char is 8 bits, long is 32 bits.
     - chksum points to the chksum in the packet
     - optr points to the old data in the packet
     - nptr points to the new data in the packet
   */
   {
     long x, old, new;
     x=chksum[0]*256+chksum[1];
     x=~x & 0xFFFF;
     while (olen)
     {
         old=optr[0]*256+optr[1]; optr+=2;
         x-=old & 0xffff;
         if (x<=0) { x--; x&=0xffff; }
         olen-=2;
     }
     while (nlen)
     {
         new=nptr[0]*256+nptr[1]; nptr+=2;
         x+=new & 0xffff;
         if (x & 0x10000) { x++; x&=0xffff; }
         nlen-=2;
     }
     x=~x & 0xFFFF;
     chksum[0]=x/256; chksum[1]=x & 0xff;
   }


/* t must be indexed by napt_free */
static void ICACHE_FLASH_ATTR
ip_napt_insert(struct napt_table *t)
{
  u16_t ti = t - ip_napt_table;
  if (ti != napt_free) *((int*)1)=1; //DEBUG
  napt_free = t->next;
  t->prev = NO_IDX;
  t->next = napt_list;
  if (napt_list != NO_IDX)
    NT(napt_list)->prev = ti;
  napt_list = ti;
  if (napt_list_last == NO_IDX)
    napt_list_last = ti;

#if LWIP_TCP
  if (t->proto == IP_PROTO_TCP)
    nr_active_napt_tcp++;
#endif
#if LWIP_UDP
  if (t->proto == IP_PROTO_UDP)
    nr_active_napt_udp++;
#endif
#if LWIP_ICMP
  if (t->proto == IP_PROTO_ICMP)
    nr_active_napt_icmp++;
#endif
//os_printf("T: %d, U: %d, I: %d\r\n", nr_active_napt_tcp, nr_active_napt_udp, nr_active_napt_icmp);
}

static void ICACHE_FLASH_ATTR
ip_napt_free(struct napt_table *t)
{
  u16_t ti = t - ip_napt_table;
  if (ti == napt_list)
    napt_list = t->next;
  if (ti == napt_list_last)
    napt_list_last = t->prev;
  if (t->next != NO_IDX)
    NT(t->next)->prev = t->prev;
  if (t->prev != NO_IDX)
    NT(t->prev)->next = t->next;
  t->prev = NO_IDX;
  t->next = napt_free;
  napt_free = ti;

#if LWIP_TCP
  if (t->proto == IP_PROTO_TCP)
    nr_active_napt_tcp--;
#endif
#if LWIP_UDP
  if (t->proto == IP_PROTO_UDP)
    nr_active_napt_udp--;
#endif
#if LWIP_ICMP
  if (t->proto == IP_PROTO_ICMP)
    nr_active_napt_icmp--;
#endif
  LWIP_DEBUGF(NAPT_DEBUG, ("ip_napt_free\n"));
  // napt_debug_print();
}

#if LWIP_TCP
static u8_t ICACHE_FLASH_ATTR
ip_napt_find_port(u8_t proto, u16_t port)
{
  int i, next;
  for (i = napt_list; i != NO_IDX; i = next) {
    struct napt_table *t = &ip_napt_table[i];
    next = t->next;
    if (t->proto == proto && t->mport == port)
      return 1;
  }
  return 0;
}

static struct portmap_table * ICACHE_FLASH_ATTR
ip_portmap_find(u8_t proto, u16_t mport);

static u8_t ICACHE_FLASH_ATTR
tcp_listening(u16_t port)
{
  struct tcp_pcb_listen *t;
  for (t = tcp_listen_pcbs.listen_pcbs; t; t = t->next)
    if (t->local_port == port)
      return 1;
  if (ip_portmap_find(IP_PROTO_TCP, port))
    return 1;
  return 0;
}
#endif // LWIP_TCP

#if LWIP_UDP
static u8_t ICACHE_FLASH_ATTR
udp_listening(u16_t port)
{
  struct udp_pcb *pcb;
  for (pcb = udp_pcbs; pcb; pcb = pcb->next)
    if (pcb->local_port == port)
      return 1;
  if (ip_portmap_find(IP_PROTO_UDP, port))
    return 1;
  return 0;
}
#endif // LWIP_UDP

static u16_t ICACHE_FLASH_ATTR
ip_napt_new_port(u8_t proto, u16_t port)
{
  if (PP_NTOHS(port) >= IP_NAPT_PORT_RANGE_START && PP_NTOHS(port) <= IP_NAPT_PORT_RANGE_END)
    if (!ip_napt_find_port(proto, port) && !tcp_listening(port))
      return port;
  for (;;) {
    port = PP_HTONS(IP_NAPT_PORT_RANGE_START +
                    os_random() % (IP_NAPT_PORT_RANGE_END - IP_NAPT_PORT_RANGE_START + 1));
    if (ip_napt_find_port(proto, port))
      continue;
#if LWIP_TCP
    if (proto == IP_PROTO_TCP && tcp_listening(port))
      continue;
#endif // LWIP_TCP
#if LWIP_UDP
    if (proto == IP_PROTO_UDP && udp_listening(port))
      continue;
#endif // LWIP_UDP

    return port;
  }
}

static struct napt_table* ICACHE_FLASH_ATTR
ip_napt_find(u8_t proto, u32_t addr, u16_t port, u16_t mport, u8_t dest)
{
  u16_t i, next;
  struct napt_table *t;

  LWIP_DEBUGF(NAPT_DEBUG, ("ip_napt_find\n"));
  LWIP_DEBUGF(NAPT_DEBUG, ("looking up in table %s: %"U16_F".%"U16_F".%"U16_F".%"U16_F", port: %u, mport: %u\n",
					(dest ? "dest" : "src"),
                    ip4_addr1_16(&addr), ip4_addr2_16(&addr),
                    ip4_addr3_16(&addr), ip4_addr4_16(&addr),
                    PP_HTONS(port),
                    PP_HTONS(mport)));
  // napt_debug_print();

  u32_t now = sys_now();
  for (i = napt_list; i != NO_IDX; i = next) {
    t = NT(i);
    next = t->next;
#if LWIP_TCP
    if (t->proto == IP_PROTO_TCP &&
        (((t->finack1 && t->finack2 || !t->synack) &&
          now - t->last > IP_NAPT_TIMEOUT_MS_TCP_DISCON) ||
         now - t->last > ip_napt_tcp_timeout)) {
      ip_napt_free(t);
      continue;
    }
#endif
#if LWIP_UDP
    if (t->proto == IP_PROTO_UDP && now - t->last > ip_napt_udp_timeout) {
      ip_napt_free(t);
      continue;
    }
#endif
#if LWIP_ICMP
    if (t->proto == IP_PROTO_ICMP && now - t->last > IP_NAPT_TIMEOUT_MS_ICMP) {
      ip_napt_free(t);
      continue;
    }
#endif
    if (dest == 0 && t->proto == proto && t->src == addr && t->sport == port) {
      t->last = now;
      LWIP_DEBUGF(NAPT_DEBUG, ("found\n"));
      return t;
    }
    if (dest == 1 && t->proto == proto && t->dest == addr && t->dport == port
        && t->mport == mport) {
      t->last = now;
      LWIP_DEBUGF(NAPT_DEBUG, ("found\n"));
      return t;
    }
  }

  LWIP_DEBUGF(NAPT_DEBUG, ("not found\n"));
  return NULL;
}

static u16_t ICACHE_FLASH_ATTR
ip_napt_add(u8_t proto, u32_t src, u16_t sport, u32_t dest, u16_t dport)
{
  struct napt_table *t = ip_napt_find(proto, src, sport, 0, 0);
  if (t) {
    t->last = sys_now();
    t->dest = dest;
    t->dport = dport;
    /* move this entry to the top of napt_list */
    ip_napt_free(t);
    ip_napt_insert(t);

    LWIP_DEBUGF(NAPT_DEBUG, ("ip_napt_add\n"));
    // napt_debug_print();

    return t->mport;
  }
  t = NT(napt_free);
  if (t) {
    u16_t mport = sport;
#if LWIP_TCP
    if (proto == IP_PROTO_TCP)
      mport = ip_napt_new_port(IP_PROTO_TCP, sport);
#endif
#if LWIP_TCP
    if (proto == IP_PROTO_UDP)
      mport = ip_napt_new_port(IP_PROTO_UDP, sport);
#endif
    t->last = sys_now();
    t->src = src;
    t->dest = dest;
    t->sport = sport;
    t->dport = dport;
    t->mport = mport;
    t->proto = proto;
    t->fin1 = t->fin2 = t->finack1 = t->finack2 = t->synack = t->rst = 0;
    ip_napt_insert(t);

    LWIP_DEBUGF(NAPT_DEBUG, ("ip_napt_add\n"));
    // napt_debug_print();

    return mport;
  }
  os_printf("NAT table full\n");
  return 0;
}

u8_t ICACHE_FLASH_ATTR
ip_portmap_add(u8_t proto, u32_t maddr, u16_t mport, u32_t daddr, u16_t dport)
{
  mport = PP_HTONS(mport);
  dport = PP_HTONS(dport);
  int i;

  for (i = 0; i < ip_portmap_max; i++) {
    struct portmap_table *p = &ip_portmap_table[i];
    if (p->valid && p->proto == proto && p->mport == mport) {
      p->dport = dport;
      p->daddr = daddr;
    } else if (!p->valid) {
      p->maddr = maddr;
      p->daddr = daddr;
      p->mport = mport;
      p->dport = dport;
      p->proto = proto;
      p->valid = 1;
      return 1;
    }
  }
  return 0;
}

static struct portmap_table * ICACHE_FLASH_ATTR
ip_portmap_find(u8_t proto, u16_t mport)
{
  int i;
  for (i = 0; i < ip_portmap_max; i++) {
    struct portmap_table *p = &ip_portmap_table[i];
    if (!p->valid)
      return 0;
    if (p->proto == proto && p->mport == mport)
      return p;
  }
  return NULL;
}

static struct portmap_table * ICACHE_FLASH_ATTR
ip_portmap_find_dest(u8_t proto, u16_t dport, u32_t daddr)
{
  int i;
  for (i = 0; i < ip_portmap_max; i++) {
    struct portmap_table *p = &ip_portmap_table[i];
    if (!p->valid)
      return 0;
    if (p->proto == proto && p->dport == dport && p->daddr == daddr)
      return p;
  }
  return NULL;
}


u8_t ICACHE_FLASH_ATTR
ip_portmap_remove(u8_t proto, u16_t mport)
{
  mport = PP_HTONS(mport);
  struct portmap_table *last = &ip_portmap_table[ip_portmap_max - 1];
  struct portmap_table *m = ip_portmap_find(proto, mport);
  if (!m)
    return 0;
  for (; m != last; m++)
    memcpy(m, m + 1, sizeof(*m));
  last->valid = 0;
  return 1;
}


#if LWIP_TCP
void ICACHE_FLASH_ATTR
ip_napt_set_tcp_timeout(u32_t secs)
{
  ip_napt_tcp_timeout = secs * 1000;
}


void ICACHE_FLASH_ATTR
ip_napt_modify_port_tcp(struct tcp_hdr *tcphdr, u8_t dest, u16_t newval)
{
  if (dest) {
    checksumadjust((char *)&tcphdr->chksum, (char *)&tcphdr->dest, 2, (char *)&newval, 2);
    tcphdr->dest = newval;
  } else {
    checksumadjust((char *)&tcphdr->chksum, (char *)&tcphdr->src, 2, (char *)&newval, 2);
    tcphdr->src = newval;
  }
}


void ICACHE_FLASH_ATTR
ip_napt_modify_addr_tcp(struct tcp_hdr *tcphdr, ip_addr_p_t *oldval, u32_t newval)
{
  checksumadjust((char *)&tcphdr->chksum, (char *)&oldval->addr, 4, (char *)&newval, 4);
}
#endif // LWIP_TCP

#if LWIP_UDP
void ICACHE_FLASH_ATTR
ip_napt_set_udp_timeout(u32_t secs)
{
  ip_napt_udp_timeout = secs * 1000;
}


void ICACHE_FLASH_ATTR
ip_napt_modify_port_udp(struct udp_hdr *udphdr, u8_t dest, u16_t newval)
{
  if (dest) {
    checksumadjust((char *)&udphdr->chksum, (char *)&udphdr->dest, 2, (char *)&newval, 2);
    udphdr->dest = newval;
  } else {
    checksumadjust((char *)&udphdr->chksum, (char *)&udphdr->src, 2, (char *)&newval, 2);
    udphdr->src = newval;
  }
}

void ICACHE_FLASH_ATTR
ip_napt_modify_addr_udp(struct udp_hdr *udphdr, ip_addr_p_t *oldval, u32_t newval)
{
  checksumadjust((char *)&udphdr->chksum, (char *)&oldval->addr, 4, (char *)&newval, 4);
}
#endif // LWIP_UDP

void ICACHE_FLASH_ATTR
ip_napt_modify_addr(struct ip_hdr *iphdr, ip_addr_p_t *field, u32_t newval)
{
  checksumadjust((char *)&IPH_CHKSUM(iphdr), (char *)&field->addr, 4, (char *)&newval, 4);
  field->addr = newval;
}


/**
 * NAPT for an input packet. It checks weather the destination is on NAPT
 * table and modifythe packet destination address and port if needed.
 *
 * @param p the packet to forward (p->payload points to IP header)
 * @param iphdr the IP header of the input packet
 * @param inp the netif on which this packet was received
 */
static void ICACHE_FLASH_ATTR
ip_napt_recv(struct pbuf *p, struct ip_hdr *iphdr, struct netif *inp)
{
  struct portmap_table *m;
  struct napt_table *t;

#if LWIP_ICMP
  /* NAPT for ICMP Echo Request using identifier */
  if (IPH_PROTO(iphdr) == IP_PROTO_ICMP) {
    struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
    if (iecho->type == ICMP_ER) {
      t = ip_napt_find(IP_PROTO_ICMP, iphdr->src.addr, iecho->id, iecho->id, 1);
      if (!t)
        return;
      ip_napt_modify_addr(iphdr, &iphdr->dest, t->src);
      return;
    }

    return;
  }
#endif // LWIP_ICMP

#if LWIP_TCP
  if (IPH_PROTO(iphdr) == IP_PROTO_TCP) {
    struct tcp_hdr *tcphdr = (struct tcp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);

    LWIP_DEBUGF(NAPT_DEBUG, ("ip_napt_recv\n"));
    LWIP_DEBUGF(NAPT_DEBUG, ("src: %"U16_F".%"U16_F".%"U16_F".%"U16_F", dest: %"U16_F".%"U16_F".%"U16_F".%"U16_F", ",
      ip4_addr1_16(&iphdr->src), ip4_addr2_16(&iphdr->src),
      ip4_addr3_16(&iphdr->src), ip4_addr4_16(&iphdr->src),
      ip4_addr1_16(&iphdr->dest), ip4_addr2_16(&iphdr->dest),
      ip4_addr3_16(&iphdr->dest), ip4_addr4_16(&iphdr->dest)));

      LWIP_DEBUGF(NAPT_DEBUG, ("sport %u, dport: %u\n",
                        PP_HTONS(tcphdr->src),
                        PP_HTONS(tcphdr->dest)));

    m = ip_portmap_find(IP_PROTO_TCP, tcphdr->dest);
    if (m) {
      /* packet to mapped port: rewrite destination */
      if (m->dport != tcphdr->dest)
        ip_napt_modify_port_tcp(tcphdr, 1, m->dport);
      ip_napt_modify_addr_tcp(tcphdr, &iphdr->dest, m->daddr);
      ip_napt_modify_addr(iphdr, &iphdr->dest, m->daddr);
      return;
    }
    t = ip_napt_find(IP_PROTO_TCP, iphdr->src.addr, tcphdr->src, tcphdr->dest, 1);
      if (!t)
        return; /* Unknown TCP session; do nothing */

      if (t->sport != tcphdr->dest)
        ip_napt_modify_port_tcp(tcphdr, 1, t->sport);
      ip_napt_modify_addr_tcp(tcphdr, &iphdr->dest, t->src);
      ip_napt_modify_addr(iphdr, &iphdr->dest, t->src);

      if ((TCPH_FLAGS(tcphdr) & (TCP_SYN|TCP_ACK)) == (TCP_SYN|TCP_ACK))
        t->synack = 1;
      if ((TCPH_FLAGS(tcphdr) & TCP_FIN))
        t->fin1 = 1;
      if (t->fin2 && (TCPH_FLAGS(tcphdr) & TCP_ACK))
        t->finack2 = 1; /* FIXME: Currently ignoring ACK seq... */
      if (TCPH_FLAGS(tcphdr) & TCP_RST)
        t->rst = 1;
      return;
  }
#endif // LWIP_TCP

#if LWIP_UDP
  if (IPH_PROTO(iphdr) == IP_PROTO_UDP) {
    struct udp_hdr *udphdr = (struct udp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
    m = ip_portmap_find(IP_PROTO_UDP, udphdr->dest);
    if (m) {
      /* packet to mapped port: rewrite destination */
      if (m->dport != udphdr->dest)
        ip_napt_modify_port_udp(udphdr, 1, m->dport);
      ip_napt_modify_addr_udp(udphdr, &iphdr->dest, m->daddr);
      ip_napt_modify_addr(iphdr, &iphdr->dest, m->daddr);
      return;
    }
    t = ip_napt_find(IP_PROTO_UDP, iphdr->src.addr, udphdr->src, udphdr->dest, 1);
      if (!t)
        return; /* Unknown session; do nothing */

      if (t->sport != udphdr->dest)
        ip_napt_modify_port_udp(udphdr, 1, t->sport);
      ip_napt_modify_addr_udp(udphdr, &iphdr->dest, t->src);
      ip_napt_modify_addr(iphdr, &iphdr->dest, t->src);
      return;
  }
#endif // LWIP_UDP
}

/**
 * NAPT for a forwarded packet. It checks weather we need NAPT and modify
 * the packet source address and port if needed.
 *
 * @param p the packet to forward (p->payload points to IP header)
 * @param iphdr the IP header of the input packet
 * @param inp the netif on which this packet was received
 * @param outp the netif on which this packet will be sent
 * @return ERR_OK if packet should be sent, or ERR_RTE if it should be dropped
 */
static err_t ICACHE_FLASH_ATTR
ip_napt_forward(struct pbuf *p, struct ip_hdr *iphdr, struct netif *inp, struct netif *outp)
{
  if (!inp->napt)
    return ERR_OK;

#if LWIP_ICMP
  /* NAPT for ICMP Echo Request using identifier */
  if (IPH_PROTO(iphdr) == IP_PROTO_ICMP) {
    struct icmp_echo_hdr *iecho = (struct icmp_echo_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
    if (iecho->type == ICMP_ECHO) {
      /* register src addr and iecho->id and dest info */
      ip_napt_add(IP_PROTO_ICMP, iphdr->src.addr, iecho->id, iphdr->dest.addr, iecho->id);

      ip_napt_modify_addr(iphdr, &iphdr->src, outp->ip_addr.addr);
    }
    return ERR_OK;
  }
#endif

#if LWIP_TCP
  if (IPH_PROTO(iphdr) == IP_PROTO_TCP) {
    struct tcp_hdr *tcphdr = (struct tcp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
    u16_t mport;

    struct portmap_table *m = ip_portmap_find_dest(IP_PROTO_TCP, tcphdr->src, iphdr->src.addr);
    if (m) {
      /* packet from port-mapped dest addr/port: rewrite source to this node */
      if (m->mport != tcphdr->src)
        ip_napt_modify_port_tcp(tcphdr, 0, m->mport);
      ip_napt_modify_addr_tcp(tcphdr, &iphdr->src, m->maddr);
      ip_napt_modify_addr(iphdr, &iphdr->src, m->maddr);
      return ERR_OK;
    }
    if ((TCPH_FLAGS(tcphdr) & (TCP_SYN|TCP_ACK)) == TCP_SYN &&
        PP_NTOHS(tcphdr->src) >= 1024) {
      /* Register new TCP session to NAPT */
      mport = ip_napt_add(IP_PROTO_TCP, iphdr->src.addr, tcphdr->src,
                          iphdr->dest.addr, tcphdr->dest);
    } else {
      struct napt_table *t = ip_napt_find(IP_PROTO_TCP, iphdr->src.addr, tcphdr->src, 0, 0);
      if (!t || t->dest != iphdr->dest.addr || t->dport != tcphdr->dest) {
#if LWIP_ICMP
        icmp_dest_unreach(p, ICMP_DUR_PORT);
#endif
        return ERR_RTE; /* Drop unknown TCP session */
      }
      mport = t->mport;
      if ((TCPH_FLAGS(tcphdr) & TCP_FIN))
        t->fin2 = 1;
      if (t->fin1 && (TCPH_FLAGS(tcphdr) & TCP_ACK))
        t->finack1 = 1; /* FIXME: Currently ignoring ACK seq... */
      if (TCPH_FLAGS(tcphdr) & TCP_RST)
        t->rst = 1;
    }

    if (mport != tcphdr->src)
      ip_napt_modify_port_tcp(tcphdr, 0, mport);
    ip_napt_modify_addr_tcp(tcphdr, &iphdr->src, outp->ip_addr.addr);
    ip_napt_modify_addr(iphdr, &iphdr->src, outp->ip_addr.addr);
    return ERR_OK;
  }
#endif

#if LWIP_UDP
  if (IPH_PROTO(iphdr) == IP_PROTO_UDP) {
    struct udp_hdr *udphdr = (struct udp_hdr *)((u8_t *)p->payload + IPH_HL(iphdr) * 4);
    u16_t mport;

    struct portmap_table *m = ip_portmap_find_dest(IP_PROTO_UDP, udphdr->src, iphdr->src.addr);
    if (m) {
      /* packet from port-mapped dest addr/port: rewrite source to this node */
      if (m->mport != udphdr->src)
        ip_napt_modify_port_udp(udphdr, 0, m->mport);
      ip_napt_modify_addr_udp(udphdr, &iphdr->src, m->maddr);
      ip_napt_modify_addr(iphdr, &iphdr->src, m->maddr);
      return ERR_OK;
    }
    if (PP_NTOHS(udphdr->src) >= 1024) {
      /* Register new UDP session */
      mport = ip_napt_add(IP_PROTO_UDP, iphdr->src.addr, udphdr->src,
                          iphdr->dest.addr, udphdr->dest);
    } else {
      struct napt_table *t = ip_napt_find(IP_PROTO_UDP, iphdr->src.addr, udphdr->src, 0, 0);
      if (!t || t->dest != iphdr->dest.addr || t->dport != udphdr->dest) {
#if LWIP_ICMP
        icmp_dest_unreach(p, ICMP_DUR_PORT);
#endif
        return ERR_RTE; /* Drop unknown UDP session */
      }
      mport = t->mport;
    }

    if (mport != udphdr->src)
      ip_napt_modify_port_udp(udphdr, 0, mport);
    ip_napt_modify_addr_udp(udphdr, &iphdr->src, outp->ip_addr.addr);
    ip_napt_modify_addr(iphdr, &iphdr->src, outp->ip_addr.addr);
    return ERR_OK;
  }
#endif

  return ERR_OK;
}
#endif // IP_NAPT

/**
 * Forwards an IP packet. It finds an appropriate route for the
 * packet, decrements the TTL value of the packet, adjusts the
 * checksum and outputs the packet on the appropriate interface.
 *
 * @param p the packet to forward (p->payload points to IP header)
 * @param iphdr the IP header of the input packet
 * @param inp the netif on which this packet was received
 */
static void ICACHE_FLASH_ATTR
ip_forward(struct pbuf *p, struct ip_hdr *iphdr, struct netif *inp)
{
  struct netif *netif;

  PERF_START;

  /* RFC3927 2.7: do not forward link-local addresses */
  if (ip_addr_islinklocal(&current_iphdr_dest)) {
    LWIP_DEBUGF(IP_DEBUG, ("ip_forward: not forwarding LLA %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
      ip4_addr1_16(&current_iphdr_dest), ip4_addr2_16(&current_iphdr_dest),
      ip4_addr3_16(&current_iphdr_dest), ip4_addr4_16(&current_iphdr_dest)));
    goto return_noroute;
  }

  /* Find network interface where to forward this IP packet to. */
  netif = ip_route(&current_iphdr_dest);
  if (netif == NULL) {
    LWIP_DEBUGF(IP_DEBUG, ("ip_forward: no forwarding route for %"U16_F".%"U16_F".%"U16_F".%"U16_F" found\n",
      ip4_addr1_16(&current_iphdr_dest), ip4_addr2_16(&current_iphdr_dest),
      ip4_addr3_16(&current_iphdr_dest), ip4_addr4_16(&current_iphdr_dest)));
    goto return_noroute;
  }
  /* Do not forward packets onto the same network interface on which
   * they arrived. */
  if (netif == inp
    #ifdef IP_ROUTING_TAB
          /* ... except if it had been routed to another gw */
          && ip_addr_cmp(&current_ip_new_dest, &current_iphdr_dest)
    #endif
  ) {
    LWIP_DEBUGF(IP_DEBUG, ("ip_forward: not bouncing packets back on incoming interface.\n"));
    goto return_noroute;
  }

  #ifdef IP_ROUTING_TAB
    /* copy it - just in case there is a new dest after routing */
    ip_addr_copy(current_iphdr_dest, current_ip_new_dest);
  #endif

  /* decrement TTL */
  IPH_TTL_SET(iphdr, IPH_TTL(iphdr) - 1);
  /* send ICMP if TTL == 0 */
  if (IPH_TTL(iphdr) == 0) {
    snmp_inc_ipinhdrerrors();
#if LWIP_ICMP
    /* Don't send ICMP messages in response to ICMP messages */
    if (IPH_PROTO(iphdr) != IP_PROTO_ICMP) {
      icmp_time_exceeded(p, ICMP_TE_TTL);
    }
#endif /* LWIP_ICMP */
    return;
  }

  #if IP_NAPT
    if (ip_napt_forward(p, iphdr, inp, netif) != ERR_OK)
      return;
  #endif

  /* Incrementally update the IP checksum. */
  if (IPH_CHKSUM(iphdr) >= PP_HTONS(0xffff - 0x100)) {
    IPH_CHKSUM_SET(iphdr, IPH_CHKSUM(iphdr) + PP_HTONS(0x100) + 1);
  } else {
    IPH_CHKSUM_SET(iphdr, IPH_CHKSUM(iphdr) + PP_HTONS(0x100));
  }

  LWIP_DEBUGF(IP_DEBUG, ("ip_forward: forwarding packet to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
    ip4_addr1_16(&current_iphdr_dest), ip4_addr2_16(&current_iphdr_dest),
    ip4_addr3_16(&current_iphdr_dest), ip4_addr4_16(&current_iphdr_dest)));

  IP_STATS_INC(ip.fw);
  IP_STATS_INC(ip.xmit);
  snmp_inc_ipforwdatagrams();

  PERF_STOP("ip_forward");
  u16_t dif = netif->mtu;
  //os_printf("ip_forward: checking mtu %c%c %d < %d\r\n", netif->name[0], netif->name[1], dif, p->tot_len);
  if(dif < p->tot_len) {
          //os_printf("ip_forward: datagram too big for %c%c %d, %d -> %d\r\n", netif->name[0], netif->name[1], netif->mtu, p->tot_len, dif);
          icmp_datagram_too_big(p, dif);
          return;
  }
  /* transmit pbuf on chosen interface */
  netif->output(netif, p, &current_iphdr_dest);
  return;
return_noroute:
  snmp_inc_ipoutnoroutes();
}
#endif /* IP_FORWARD */

/**
 * This function is called by the network interface device driver when
 * an IP packet is received. The function does the basic checks of the
 * IP header such as packet size being at least larger than the header
 * size etc. If the packet was not destined for us, the packet is
 * forwarded (using ip_forward). The IP checksum is always checked.
 *
 * Finally, the packet is sent to the upper layer protocol input function.
 *
 * @param p the received IP packet (p->payload points to IP header)
 * @param inp the netif on which this packet was received
 * @return ERR_OK if the packet was processed (could return ERR_* if it wasn't
 *         processed, but currently always returns ERR_OK)
 */
err_t
ip_input(struct pbuf *p, struct netif *inp)
{
  struct ip_hdr *iphdr;
  struct netif *netif;
  u16_t iphdr_hlen;
  u16_t iphdr_len;
#if IP_ACCEPT_LINK_LAYER_ADDRESSING
  int check_ip_src=1;
#endif /* IP_ACCEPT_LINK_LAYER_ADDRESSING */

  IP_STATS_INC(ip.recv);
  snmp_inc_ipinreceives();

  /* identify the IP header */
  iphdr = (struct ip_hdr *)p->payload;
  if (IPH_V(iphdr) != 4) {
    LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_WARNING, ("IP packet dropped due to bad version number %"U16_F"\n", IPH_V(iphdr)));
    ip_debug_print(p);
    pbuf_free(p);
    IP_STATS_INC(ip.err);
    IP_STATS_INC(ip.drop);
    snmp_inc_ipinhdrerrors();
    return ERR_OK;
  }

  /* obtain IP header length in number of 32-bit words */
  iphdr_hlen = IPH_HL(iphdr);
  /* calculate IP header length in bytes */
  iphdr_hlen *= 4;
  /* obtain ip length in bytes */
  iphdr_len = ntohs(IPH_LEN(iphdr));

  /* header length exceeds first pbuf length, or ip length exceeds total pbuf length? */
  if ((iphdr_hlen > p->len) || (iphdr_len > p->tot_len)) {
    if (iphdr_hlen > p->len) {
      LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
        ("IP header (len %"U16_F") does not fit in first pbuf (len %"U16_F"), IP packet dropped.\n",
        iphdr_hlen, p->len));
    }
    if (iphdr_len > p->tot_len) {
      LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
        ("IP (len %"U16_F") is longer than pbuf (len %"U16_F"), IP packet dropped.\n",
        iphdr_len, p->tot_len));
    }
    /* free (drop) packet pbufs */
    pbuf_free(p);
    IP_STATS_INC(ip.lenerr);
    IP_STATS_INC(ip.drop);
    snmp_inc_ipindiscards();
    return ERR_OK;
  }

  /* verify checksum */
#if CHECKSUM_CHECK_IP
  if (inet_chksum(iphdr, iphdr_hlen) != 0) {

    LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS,
      ("Checksum (0x%"X16_F") failed, IP packet dropped.\n", inet_chksum(iphdr, iphdr_hlen)));
    ip_debug_print(p);
    pbuf_free(p);
    IP_STATS_INC(ip.chkerr);
    IP_STATS_INC(ip.drop);
    snmp_inc_ipinhdrerrors();
    return ERR_OK;
  }
#endif

#if IP_NAPT
  /* for unicast packet, check NAPT table and modify dest if needed */
  if (!inp->napt && ip_addr_cmp(&iphdr->dest, &(inp->ip_addr)))
    ip_napt_recv(p, iphdr, netif);
#endif

  /* Trim pbuf. This should have been done at the netif layer,
   * but we'll do it anyway just to be sure that its done. */
  pbuf_realloc(p, iphdr_len);

  /* copy IP addresses to aligned ip_addr_t */
  ip_addr_copy(current_iphdr_dest, iphdr->dest);
  ip_addr_copy(current_iphdr_src, iphdr->src);

  /* match packet against an interface, i.e. is this packet for us? */
#if LWIP_IGMP
  if (ip_addr_ismulticast(&current_iphdr_dest)) {
    if ((inp->flags & NETIF_FLAG_IGMP) && (igmp_lookfor_group(inp, &current_iphdr_dest))) {
      netif = inp;
    } else {
      netif = NULL;
    }
  } else
#endif /* LWIP_IGMP */
  {
    /* start trying with inp. if that's not acceptable, start walking the
       list of configured netifs.
       'first' is used as a boolean to mark whether we started walking the list */
    int first = 1;
    netif = inp;
    do {
      LWIP_DEBUGF(IP_DEBUG, ("ip_input: iphdr->dest 0x%"X32_F" netif->ip_addr 0x%"X32_F" (0x%"X32_F", 0x%"X32_F", 0x%"X32_F")\n",
          ip4_addr_get_u32(&iphdr->dest), ip4_addr_get_u32(&netif->ip_addr),
          ip4_addr_get_u32(&iphdr->dest) & ip4_addr_get_u32(&netif->netmask),
          ip4_addr_get_u32(&netif->ip_addr) & ip4_addr_get_u32(&netif->netmask),
          ip4_addr_get_u32(&iphdr->dest) & ~ip4_addr_get_u32(&netif->netmask)));

      /* interface is up and configured? */
      if ((netif_is_up(netif)) && (!ip_addr_isany(&(netif->ip_addr)))) {
        /* unicast to this interface address? */
        if (ip_addr_cmp(&current_iphdr_dest, &(netif->ip_addr)) ||
            /* or broadcast on this interface network address? */
            ip_addr_isbroadcast(&current_iphdr_dest, netif)) {
          LWIP_DEBUGF(IP_DEBUG, ("ip_input: packet accepted on interface %c%c\n",
              netif->name[0], netif->name[1]));
          /* break out of for loop */
          break;
        }
#if LWIP_AUTOIP
        /* connections to link-local addresses must persist after changing
           the netif's address (RFC3927 ch. 1.9) */
        if ((netif->autoip != NULL) &&
            ip_addr_cmp(&current_iphdr_dest, &(netif->autoip->llipaddr))) {
          LWIP_DEBUGF(IP_DEBUG, ("ip_input: LLA packet accepted on interface %c%c\n",
              netif->name[0], netif->name[1]));
          /* break out of for loop */
          break;
        }
#endif /* LWIP_AUTOIP */
      }
      if (first) {
        first = 0;
        netif = netif_list;
      } else {
        netif = netif->next;
      }
      if (netif == inp) {
        netif = netif->next;
      }
    } while(netif != NULL);
  }

#if IP_ACCEPT_LINK_LAYER_ADDRESSING
  /* Pass DHCP messages regardless of destination address. DHCP traffic is addressed
   * using link layer addressing (such as Ethernet MAC) so we must not filter on IP.
   * According to RFC 1542 section 3.1.1, referred by RFC 2131).
   *
   * If you want to accept private broadcast communication while a netif is down,
   * define LWIP_IP_ACCEPT_UDP_PORT(dst_port), e.g.:
   *
   * #define LWIP_IP_ACCEPT_UDP_PORT(dst_port) ((dst_port) == PP_NTOHS(12345))
   */
  if (netif == NULL) {
    /* remote port is DHCP server? */
    if (IPH_PROTO(iphdr) == IP_PROTO_UDP) {
      struct udp_hdr *udphdr = (struct udp_hdr *)((u8_t *)iphdr + iphdr_hlen);
      LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_TRACE, ("ip_input: UDP packet to DHCP client port %"U16_F"\n",
        ntohs(udphdr->dest)));
      if (IP_ACCEPT_LINK_LAYER_ADDRESSED_PORT(udphdr->dest)) {
        LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_TRACE, ("ip_input: DHCP packet accepted.\n"));
        netif = inp;
        check_ip_src = 0;
      }
    }
  }
#endif /* IP_ACCEPT_LINK_LAYER_ADDRESSING */

  /* broadcast or multicast packet source address? Compliant with RFC 1122: 3.2.1.3 */
#if IP_ACCEPT_LINK_LAYER_ADDRESSING
  /* DHCP servers need 0.0.0.0 to be allowed as source address (RFC 1.1.2.2: 3.2.1.3/a) */
  if (check_ip_src && !ip_addr_isany(&current_iphdr_src))
#endif /* IP_ACCEPT_LINK_LAYER_ADDRESSING */
  {  if ((ip_addr_isbroadcast(&current_iphdr_src, inp)) ||
         (ip_addr_ismulticast(&current_iphdr_src))) {
      /* packet source is not valid */
      LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING, ("ip_input: packet source is not valid.\n"));
      /* free (drop) packet pbufs */
      pbuf_free(p);
      IP_STATS_INC(ip.drop);
      snmp_inc_ipinaddrerrors();
      snmp_inc_ipindiscards();
      return ERR_OK;
    }
  }

  /* packet not for us? */
  if (netif == NULL) {
    /* packet not for us, route or discard */
    LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_TRACE, ("ip_input: packet not for us.\n"));
#if IP_FORWARD
    /* non-broadcast packet? */
    if (!ip_addr_isbroadcast(&current_iphdr_dest, inp)) {
      /* try to forward IP packet on (other) interfaces */
      ip_forward(p, iphdr, inp);
    } else
#endif /* IP_FORWARD */
    {
      snmp_inc_ipinaddrerrors();
      snmp_inc_ipindiscards();
    }
    pbuf_free(p);
    return ERR_OK;
  }
  /* packet consists of multiple fragments? */
  if ((IPH_OFFSET(iphdr) & PP_HTONS(IP_OFFMASK | IP_MF)) != 0) {
#if IP_REASSEMBLY /* packet fragment reassembly code present? */
    LWIP_DEBUGF(IP_DEBUG, ("IP packet is a fragment (id=0x%04"X16_F" tot_len=%"U16_F" len=%"U16_F" MF=%"U16_F" offset=%"U16_F"), calling ip_reass()\n",
      ntohs(IPH_ID(iphdr)), p->tot_len, ntohs(IPH_LEN(iphdr)), !!(IPH_OFFSET(iphdr) & PP_HTONS(IP_MF)), (ntohs(IPH_OFFSET(iphdr)) & IP_OFFMASK)*8));
    /* reassemble the packet*/
    p = ip_reass(p);
    /* packet not fully reassembled yet? */
    if (p == NULL) {
      return ERR_OK;
    }
    iphdr = (struct ip_hdr *)p->payload;
#else /* IP_REASSEMBLY == 0, no packet fragment reassembly code present */
    pbuf_free(p);
    LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("IP packet dropped since it was fragmented (0x%"X16_F") (while IP_REASSEMBLY == 0).\n",
      ntohs(IPH_OFFSET(iphdr))));
    IP_STATS_INC(ip.opterr);
    IP_STATS_INC(ip.drop);
    /* unsupported protocol feature */
    snmp_inc_ipinunknownprotos();
    return ERR_OK;
#endif /* IP_REASSEMBLY */
  }

#if IP_OPTIONS_ALLOWED == 0 /* no support for IP options in the IP header? */

#if LWIP_IGMP
  /* there is an extra "router alert" option in IGMP messages which we allow for but do not police */
  if((iphdr_hlen > IP_HLEN) &&  (IPH_PROTO(iphdr) != IP_PROTO_IGMP)) {
#else
  if (iphdr_hlen > IP_HLEN) {
#endif /* LWIP_IGMP */
    LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("IP packet dropped since there were IP options (while IP_OPTIONS_ALLOWED == 0).\n"));
    pbuf_free(p);
    IP_STATS_INC(ip.opterr);
    IP_STATS_INC(ip.drop);
    /* unsupported protocol feature */
    snmp_inc_ipinunknownprotos();
    return ERR_OK;
  }
#endif /* IP_OPTIONS_ALLOWED == 0 */

  /* send to upper layers */
  LWIP_DEBUGF(IP_DEBUG, ("ip_input: \n"));
  ip_debug_print(p);
  LWIP_DEBUGF(IP_DEBUG, ("ip_input: p->len %"U16_F" p->tot_len %"U16_F"\n", p->len, p->tot_len));

  current_netif = inp;
  current_header = iphdr;

#if LWIP_RAW
  /* raw input did not eat the packet? */
  if (raw_input(p, inp) == 0)
#endif /* LWIP_RAW */
  {

    switch (IPH_PROTO(iphdr)) {
#if LWIP_UDP
    case IP_PROTO_UDP:
#if LWIP_UDPLITE
    case IP_PROTO_UDPLITE:
#endif /* LWIP_UDPLITE */
      snmp_inc_ipindelivers();
      udp_input(p, inp);
      break;
#endif /* LWIP_UDP */
#if LWIP_TCP
    case IP_PROTO_TCP:
      snmp_inc_ipindelivers();
      tcp_input(p, inp);
      break;
#endif /* LWIP_TCP */
#if LWIP_ICMP
    case IP_PROTO_ICMP:
      snmp_inc_ipindelivers();
      icmp_input(p, inp);
      break;
#endif /* LWIP_ICMP */
#if LWIP_IGMP
    case IP_PROTO_IGMP:
      igmp_input(p, inp, &current_iphdr_dest);
      break;
#endif /* LWIP_IGMP */
    default:
#if LWIP_ICMP
      /* send ICMP destination protocol unreachable unless is was a broadcast */
      if (!ip_addr_isbroadcast(&current_iphdr_dest, inp) &&
          !ip_addr_ismulticast(&current_iphdr_dest)) {
        p->payload = iphdr;
        icmp_dest_unreach(p, ICMP_DUR_PROTO);
      }
#endif /* LWIP_ICMP */
      pbuf_free(p);

      LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("Unsupported transport protocol %"U16_F"\n", IPH_PROTO(iphdr)));

      IP_STATS_INC(ip.proterr);
      IP_STATS_INC(ip.drop);
      snmp_inc_ipinunknownprotos();
    }
  }

  current_netif = NULL;
  current_header = NULL;
  ip_addr_set_any(&current_iphdr_src);
  ip_addr_set_any(&current_iphdr_dest);

  return ERR_OK;
}

/**
 * Sends an IP packet on a network interface. This function constructs
 * the IP header and calculates the IP header checksum. If the source
 * IP address is NULL, the IP address of the outgoing network
 * interface is filled in as source address.
 * If the destination IP address is IP_HDRINCL, p is assumed to already
 * include an IP header and p->payload points to it instead of the data.
 *
 * @param p the packet to send (p->payload points to the data, e.g. next
            protocol header; if dest == IP_HDRINCL, p already includes an IP
            header and p->payload points to that IP header)
 * @param src the source IP address to send from (if src == IP_ADDR_ANY, the
 *         IP  address of the netif used to send is used as source address)
 * @param dest the destination IP address to send the packet to
 * @param ttl the TTL value to be set in the IP header
 * @param tos the TOS value to be set in the IP header
 * @param proto the PROTOCOL to be set in the IP header
 * @param netif the netif on which to send this packet
 * @return ERR_OK if the packet was sent OK
 *         ERR_BUF if p doesn't have enough space for IP/LINK headers
 *         returns errors returned by netif->output
 *
 * @note ip_id: RFC791 "some host may be able to simply use
 *  unique identifiers independent of destination"
 */
err_t
ip_output_if(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
             u8_t ttl, u8_t tos,
             u8_t proto, struct netif *netif)
{
#if IP_OPTIONS_SEND
  return ip_output_if_opt(p, src, dest, ttl, tos, proto, netif, NULL, 0);
}

/**
 * Same as ip_output_if() but with the possibility to include IP options:
 *
 * @ param ip_options pointer to the IP options, copied into the IP header
 * @ param optlen length of ip_options
 */
err_t ip_output_if_opt(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
       u8_t ttl, u8_t tos, u8_t proto, struct netif *netif, void *ip_options,
       u16_t optlen)
{
#endif /* IP_OPTIONS_SEND */
  struct ip_hdr *iphdr;
  ip_addr_t dest_addr;
#if CHECKSUM_GEN_IP_INLINE
  u32_t chk_sum = 0;
#endif /* CHECKSUM_GEN_IP_INLINE */

  /* pbufs passed to IP must have a ref-count of 1 as their payload pointer
     gets altered as the packet is passed down the stack */
  LWIP_ASSERT("p->ref == 1", p->ref == 1);

  snmp_inc_ipoutrequests();

  /* Should the IP header be generated or is it already included in p? */
  if (dest != IP_HDRINCL) {
    u16_t ip_hlen = IP_HLEN;
#if IP_OPTIONS_SEND
    u16_t optlen_aligned = 0;
    if (optlen != 0) {
#if CHECKSUM_GEN_IP_INLINE
      int i;
#endif /* CHECKSUM_GEN_IP_INLINE */
      /* round up to a multiple of 4 */
      optlen_aligned = ((optlen + 3) & ~3);
      ip_hlen += optlen_aligned;
      /* First write in the IP options */
      if (pbuf_header(p, optlen_aligned)) {
        LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("ip_output_if_opt: not enough room for IP options in pbuf\n"));
        IP_STATS_INC(ip.err);
        snmp_inc_ipoutdiscards();
        return ERR_BUF;
      }
      MEMCPY(p->payload, ip_options, optlen);
      if (optlen < optlen_aligned) {
        /* zero the remaining bytes */
        os_memset(((char*)p->payload) + optlen, 0, optlen_aligned - optlen);
      }
#if CHECKSUM_GEN_IP_INLINE
      for (i = 0; i < optlen_aligned/2; i++) {
        chk_sum += ((u16_t*)p->payload)[i];
      }
#endif /* CHECKSUM_GEN_IP_INLINE */
    }
#endif /* IP_OPTIONS_SEND */
    /* generate IP header */
    if (pbuf_header(p, IP_HLEN)) {
      LWIP_DEBUGF(IP_DEBUG | LWIP_DBG_LEVEL_SERIOUS, ("ip_output: not enough room for IP header in pbuf\n"));

      IP_STATS_INC(ip.err);
      snmp_inc_ipoutdiscards();
      return ERR_BUF;
    }

    iphdr = (struct ip_hdr *)p->payload;
    LWIP_ASSERT("check that first pbuf can hold struct ip_hdr",
               (p->len >= sizeof(struct ip_hdr)));

    IPH_TTL_SET(iphdr, ttl);
    IPH_PROTO_SET(iphdr, proto);
#if CHECKSUM_GEN_IP_INLINE
    chk_sum += LWIP_MAKE_U16(proto, ttl);
#endif /* CHECKSUM_GEN_IP_INLINE */

    /* dest cannot be NULL here */
    ip_addr_copy(iphdr->dest, *dest);
#if CHECKSUM_GEN_IP_INLINE
    chk_sum += ip4_addr_get_u32(&iphdr->dest) & 0xFFFF;
    chk_sum += ip4_addr_get_u32(&iphdr->dest) >> 16;
#endif /* CHECKSUM_GEN_IP_INLINE */

    IPH_VHLTOS_SET(iphdr, 4, ip_hlen / 4, tos);
#if CHECKSUM_GEN_IP_INLINE
    chk_sum += iphdr->_v_hl_tos;
#endif /* CHECKSUM_GEN_IP_INLINE */
    IPH_LEN_SET(iphdr, htons(p->tot_len));
#if CHECKSUM_GEN_IP_INLINE
    chk_sum += iphdr->_len;
#endif /* CHECKSUM_GEN_IP_INLINE */
    IPH_OFFSET_SET(iphdr, 0);
    IPH_ID_SET(iphdr, htons(ip_id));
#if CHECKSUM_GEN_IP_INLINE
    chk_sum += iphdr->_id;
#endif /* CHECKSUM_GEN_IP_INLINE */
    ++ip_id;

    if (ip_addr_isany(src)) {
      ip_addr_copy(iphdr->src, netif->ip_addr);
    } else {
      /* src cannot be NULL here */
      ip_addr_copy(iphdr->src, *src);
    }

#if CHECKSUM_GEN_IP_INLINE
    chk_sum += ip4_addr_get_u32(&iphdr->src) & 0xFFFF;
    chk_sum += ip4_addr_get_u32(&iphdr->src) >> 16;
    chk_sum = (chk_sum >> 16) + (chk_sum & 0xFFFF);
    chk_sum = (chk_sum >> 16) + chk_sum;
    chk_sum = ~chk_sum;
    iphdr->_chksum = chk_sum; /* network order */
#else /* CHECKSUM_GEN_IP_INLINE */
    IPH_CHKSUM_SET(iphdr, 0);
#if CHECKSUM_GEN_IP
    IPH_CHKSUM_SET(iphdr, inet_chksum(iphdr, ip_hlen));
#endif
#endif /* CHECKSUM_GEN_IP_INLINE */
  } else {
    /* IP header already included in p */
    iphdr = (struct ip_hdr *)p->payload;
    ip_addr_copy(dest_addr, iphdr->dest);
    dest = &dest_addr;
  }

  #ifdef IP_ROUTING_TAB
    struct netif *new_netif = ip_route(dest);
    if (!ip_addr_cmp(dest, &current_ip_new_dest)) {
      //os_printf_plus("We changeed the routing in ip_output_if_opt\r\n");
      ip_addr_copy(*dest, current_ip_new_dest);
      netif = new_netif;
    }
  #endif

  IP_STATS_INC(ip.xmit);

  LWIP_DEBUGF(IP_DEBUG, ("ip_output_if: %c%c%"U16_F"\n", netif->name[0], netif->name[1], netif->num));
  ip_debug_print(p);

#if 0 && ENABLE_LOOPBACK
  if (ip_addr_cmp(dest, &netif->ip_addr)) {
    /* Packet to self, enqueue it for loopback */
    LWIP_DEBUGF(IP_DEBUG, ("netif_loop_output()"));
    return netif_loop_output(netif, p, dest);
  }
#if LWIP_IGMP
  if ((p->flags & PBUF_FLAG_MCASTLOOP) != 0) {
    netif_loop_output(netif, p, dest);
  }
#endif /* LWIP_IGMP */
#endif /* ENABLE_LOOPBACK */
#if IP_FRAG
  /* don't fragment if interface has mtu set to 0 [loopif] */
  if (netif->mtu && (p->tot_len > netif->mtu)) {
    return ip_frag(p, netif, dest);
  }
#endif /* IP_FRAG */

  LWIP_DEBUGF(IP_DEBUG, ("netif->output()\n"));
  return netif->output(netif, p, dest);
}

/**
 * Simple interface to ip_output_if. It finds the outgoing network
 * interface and calls upon ip_output_if to do the actual work.
 *
 * @param p the packet to send (p->payload points to the data, e.g. next
            protocol header; if dest == IP_HDRINCL, p already includes an IP
            header and p->payload points to that IP header)
 * @param src the source IP address to send from (if src == IP_ADDR_ANY, the
 *         IP  address of the netif used to send is used as source address)
 * @param dest the destination IP address to send the packet to
 * @param ttl the TTL value to be set in the IP header
 * @param tos the TOS value to be set in the IP header
 * @param proto the PROTOCOL to be set in the IP header
 *
 * @return ERR_RTE if no route is found
 *         see ip_output_if() for more return values
 */
err_t
ip_output(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
          u8_t ttl, u8_t tos, u8_t proto)
{
  struct netif *netif;

  /* pbufs passed to IP must have a ref-count of 1 as their payload pointer
     gets altered as the packet is passed down the stack */
  LWIP_ASSERT("p->ref == 1", p->ref == 1);

  if ((netif = ip_route(dest)) == NULL) {
    LWIP_DEBUGF(IP_DEBUG, ("ip_output: No route to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
      ip4_addr1_16(dest), ip4_addr2_16(dest), ip4_addr3_16(dest), ip4_addr4_16(dest)));
    IP_STATS_INC(ip.rterr);
    return ERR_RTE;
  }

  return ip_output_if(p, src, dest, ttl, tos, proto, netif);
}

#if LWIP_NETIF_HWADDRHINT
/** Like ip_output, but takes and addr_hint pointer that is passed on to netif->addr_hint
 *  before calling ip_output_if.
 *
 * @param p the packet to send (p->payload points to the data, e.g. next
            protocol header; if dest == IP_HDRINCL, p already includes an IP
            header and p->payload points to that IP header)
 * @param src the source IP address to send from (if src == IP_ADDR_ANY, the
 *         IP  address of the netif used to send is used as source address)
 * @param dest the destination IP address to send the packet to
 * @param ttl the TTL value to be set in the IP header
 * @param tos the TOS value to be set in the IP header
 * @param proto the PROTOCOL to be set in the IP header
 * @param addr_hint address hint pointer set to netif->addr_hint before
 *        calling ip_output_if()
 *
 * @return ERR_RTE if no route is found
 *         see ip_output_if() for more return values
 */
err_t
ip_output_hinted(struct pbuf *p, ip_addr_t *src, ip_addr_t *dest,
          u8_t ttl, u8_t tos, u8_t proto, u8_t *addr_hint)
{
  struct netif *netif;
  err_t err;

  /* pbufs passed to IP must have a ref-count of 1 as their payload pointer
     gets altered as the packet is passed down the stack */
  LWIP_ASSERT("p->ref == 1", p->ref == 1);

  if ((netif = ip_route(dest)) == NULL) {
    LWIP_DEBUGF(IP_DEBUG, ("ip_output: No route to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
      ip4_addr1_16(dest), ip4_addr2_16(dest), ip4_addr3_16(dest), ip4_addr4_16(dest)));
    IP_STATS_INC(ip.rterr);
    return ERR_RTE;
  }

  netif->addr_hint = addr_hint;
  err = ip_output_if(p, src, dest, ttl, tos, proto, netif);
  netif->addr_hint = NULL;

  return err;
}
#endif /* LWIP_NETIF_HWADDRHINT*/

#if IP_DEBUG
/* Print an IP header by using LWIP_DEBUGF
 * @param p an IP packet, p->payload pointing to the IP header
 */
void
ip_debug_print(struct pbuf *p)
{
  struct ip_hdr *iphdr = (struct ip_hdr *)p->payload;
  u8_t *payload;

  payload = (u8_t *)iphdr + IP_HLEN;

  LWIP_DEBUGF(IP_DEBUG, ("IP header:\n"));
  LWIP_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  LWIP_DEBUGF(IP_DEBUG, ("|%2"S16_F" |%2"S16_F" |  0x%02"X16_F" |     %5"U16_F"     | (v, hl, tos, len)\n",
                    IPH_V(iphdr),
                    IPH_HL(iphdr),
                    IPH_TOS(iphdr),
                    ntohs(IPH_LEN(iphdr))));
  LWIP_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  LWIP_DEBUGF(IP_DEBUG, ("|    %5"U16_F"      |%"U16_F"%"U16_F"%"U16_F"|    %4"U16_F"   | (id, flags, offset)\n",
                    ntohs(IPH_ID(iphdr)),
                    ntohs(IPH_OFFSET(iphdr)) >> 15 & 1,
                    ntohs(IPH_OFFSET(iphdr)) >> 14 & 1,
                    ntohs(IPH_OFFSET(iphdr)) >> 13 & 1,
                    ntohs(IPH_OFFSET(iphdr)) & IP_OFFMASK));
  LWIP_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  LWIP_DEBUGF(IP_DEBUG, ("|  %3"U16_F"  |  %3"U16_F"  |    0x%04"X16_F"     | (ttl, proto, chksum)\n",
                    IPH_TTL(iphdr),
                    IPH_PROTO(iphdr),
                    ntohs(IPH_CHKSUM(iphdr))));
  LWIP_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  LWIP_DEBUGF(IP_DEBUG, ("|  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  | (src)\n",
                    ip4_addr1_16(&iphdr->src),
                    ip4_addr2_16(&iphdr->src),
                    ip4_addr3_16(&iphdr->src),
                    ip4_addr4_16(&iphdr->src)));
  LWIP_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
  LWIP_DEBUGF(IP_DEBUG, ("|  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  |  %3"U16_F"  | (dest)\n",
                    ip4_addr1_16(&iphdr->dest),
                    ip4_addr2_16(&iphdr->dest),
                    ip4_addr3_16(&iphdr->dest),
                    ip4_addr4_16(&iphdr->dest)));
  LWIP_DEBUGF(IP_DEBUG, ("+-------------------------------+\n"));
}
#endif /* IP_DEBUG */
