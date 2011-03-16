#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

typedef char bool;
# define false 0
# define true  1

# define QUIT 1

void sigint_handler(int n);

int main()
{
  int fd;
  char buff[4096];
  int cc, i = 1;

  if ((fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1)
    {
      fprintf(stderr, "socket(): error: %s\n", strerror(errno));
      exit(EXIT_FAILURE);
    }

  signal(SIGINT, sigint_handler);
  while ((cc = read(fd, buff, 4095)) != -1)
    {
      printf("%d: %d bytes received.\n", i, cc);
    }
  return (EXIT_SUCCESS);
}

void sigint_handler(int n)
{
  (void) n;
  exit(EXIT_SUCCESS);
}
