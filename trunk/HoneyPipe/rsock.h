#ifndef RSOCK_H_
# define RSOCK_H_

int  create_raw_socket();
int  get_interface_id(int fd, char *name);
void get_interface_addr(int fd, char *name, uint8_t *hwaddr);
void bind_interface(int fd, int ifidx);
void promisc_mode(int fd, int ifidx, int s);
uint32_t iptolong(char *s);
int mactoa(char *s, uint8_t *mac);

#endif
