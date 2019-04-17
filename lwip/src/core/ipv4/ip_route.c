#include "lwip/opt.h"
#include "lwip/ip.h"

#ifdef IP_ROUTING_TAB
#include "lwip/ip_route.h"

struct route_entry ip_rt_table[MAX_ROUTES];
int ip_route_max = 0;

uint32_t ICACHE_FLASH_ATTR
mask2clidr (ip_addr_t *mask)
{
uint32_t clidr;

  uint32_t m = mask->addr;
  for (clidr = 0; m; m <<= 1,clidr++);
  return clidr;
}

bool ICACHE_FLASH_ATTR
ip_add_route(ip_addr_t ip, ip_addr_t mask, ip_addr_t gw)
{
  int add_pos, i, j;

  // Remove it if already existing
  ip_rm_route(ip, mask);

  if (ip_route_max >= MAX_ROUTES)
    return false;

  ip.addr &= mask.addr;

  add_pos = ip_route_max;
  for (i = 0; i<ip_route_max; i++) {
    // sort entries by mask length
    if (mask2clidr(&mask) > mask2clidr(&ip_rt_table[i].mask)) {
      add_pos = i;
      for (j = ip_route_max-1; j >= i; j--) {
	ip_rt_table[j+1] = ip_rt_table[j];    
      }
      break;
    }
  }

  ip_addr_copy(ip_rt_table[add_pos].ip, ip);
  ip_addr_copy(ip_rt_table[add_pos].mask, mask);
  ip_addr_copy(ip_rt_table[add_pos].gw, gw);
  ip_route_max++;
  return true;
}

bool ICACHE_FLASH_ATTR
ip_rm_route(ip_addr_t ip, ip_addr_t mask)
{
  int i;

  for (i = 0; i<ip_route_max; i++) {
    if (ip_addr_cmp(&ip, &ip_rt_table[i].ip) && ip_addr_cmp(&mask, &ip_rt_table[i].mask)) {
      for (i = i+1; i<ip_route_max; i++) {
	ip_rt_table[i-1] = ip_rt_table[i];    
      }
      ip_route_max--;
      return true;
    }
  }
  return false;
}

struct route_entry ICACHE_FLASH_ATTR *
ip_find_route(ip_addr_t ip)
{
  int i;

  for (i = 0; i<ip_route_max; i++) {
    if (ip_addr_netcmp(&ip, &ip_rt_table[i].ip, &ip_rt_table[i].mask)) {
      return &ip_rt_table[i];
    }
  }
  return NULL;
}

void ICACHE_FLASH_ATTR
ip_delete_routes(void)
{
  ip_route_max = 0;
}

bool ICACHE_FLASH_ATTR
ip_get_route(uint32_t no, ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw)
{
  if (no >= ip_route_max)
    return false;

  ip_addr_copy(*ip, ip_rt_table[no].ip);
  ip_addr_copy(*mask, ip_rt_table[no].mask);
  ip_addr_copy(*gw, ip_rt_table[no].gw);
  return true;
}

#endif /* IP_ROUTING_TAB */
