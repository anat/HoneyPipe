#ifndef HONEY_PIPE_H_
# define HONEY_PIPE_H_

# include <stdint.h>

typedef char bool;
# define false 0
# define true  1

typedef struct
{
  uint8_t  ip_hl:4;
  uint8_t  ip_v:4;
  uint8_t  ip_tos;
  uint16_t ip_len;
  uint16_t ip_id;
  uint16_t ip_off;
  uint8_t  ip_ttl;
  uint8_t  ip_p;
  uint16_t ip_sum;
  uint32_t ip_src;
  uint32_t ip_dst;
} t_ipheader;

typedef struct
{
  uint8_t  icmp_type;
  uint8_t  icmp_code;
  uint16_t icmp_cksum;
  uint16_t icmp_id;
  uint16_t icmp_seq;
} t_icmpheader;

void sigint_handler(int n);
int  iptolong(char *s);
void catch(int fd);
void push(int fd);

#endif
