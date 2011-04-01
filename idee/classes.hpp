struct eth
{
  int mac1;
  int mac2;
};

struct ip : public eth
{
  int ip1;
  int ip2;
};

struct tcp : public ip
{
  int ack;
};
