#include <iostream>
#include "classes.hpp"

int main(int ac, char **av)
{
  int buffer[5];

  buffer[0] = 12345;
  buffer[1] = 88888;
  buffer[2] = 36627;
  buffer[3] = 99810;
  buffer[4] = 2;
  
  tcp *test = (tcp*)buffer;
  std::cout << "Mac 1 = " << test->mac1 << std::endl;
  std::cout << "Mac 2 = " << test->mac2 << std::endl;
  std::cout << "IP 1 = " << test->ip1 << std::endl;
  std::cout << "IP 2 = " << test->ip2 << std::endl;
  std::cout << "ack = " << test->ack << std::endl;
}
