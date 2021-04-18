#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <memory.h>

#define MC_PORT 1818
#define MC_GROUP "239.0.0.1"

/* This program sends data on a mulicast socket, then receives data on a unicast
   socket. It then replies on the unicast socket (because it can). It loops forever */

struct inpacket
{
  char version;
  short port;
};
int main(int argc, char *argv[])
{
  struct sockaddr_in addr, from_addr;
  struct sockaddr_in server;
  int sock, cnt;
  socklen_t addrlen;
  socklen_t serverLen;
  struct ip_mreq mreq;
  char message[50];
  struct inpacket packectIn;

  /* set up socket. I only NEED one socket on the client side */
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)
  {
    perror("socket");
    exit(1);
  }
  // This sets up the TO address. Note that I am setting up the TO address
  // to be a multicast address....
  bzero((char *)&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(MC_PORT);
  addr.sin_addr.s_addr = inet_addr(MC_GROUP);
  addrlen = sizeof(addr);

  while (1)
  {
    time_t t = time(0);
    sprintf(message, "time is %-24.24s", ctime(&t));

    printf("sending: %s\n", message);
    // sending to the MULTICAST address stored in addr
    cnt = sendto(sock, message, sizeof(message), 0,
                 (struct sockaddr *)&addr, addrlen);
    if (cnt < 0)
    {
      perror("sendto");
      exit(1);
    }
    memset(message, 0, 50);
    // I now do a recvfrom. NOTE: i get the 'from' address in the variable from_addr
    // this is NOT a MC address
    cnt = recvfrom(sock, &packectIn, sizeof(packectIn), 0,
                   (struct sockaddr *)&from_addr, &addrlen);
    //struct sockaddr_in *p = (struct sockaddr_in *)&from_addr;
    //printf("address: %08X (%s)\n", p->sin_addr.s_addr, inet_ntoa(p->sin_addr));
    int port = ntohs(packectIn.port);
    char address[16]; //Create an address array to store the server's ip address

    sprintf(address, inet_ntoa(from_addr.sin_addr)); //Copy the server ip address into the array

    printf("address: (%s)\n", inet_ntoa(from_addr.sin_addr));
    //Need a port number variable
    int portNumber[1]; //Create port number to store
    memcpy(portNumber, message, 4);
    portNumber[1] = ntohs(portNumber[1]);
    printf("received '%s'\n", message);
    memset(message, 0, 50);
    sprintf(message, "reply on regular socket");
    // just to show i can use the from address, i send a UNICAST response back
    cnt = sendto(sock, address, sizeof(address), 0,
                 (struct sockaddr *)&from_addr, addrlen);

    sleep(5); // sleep so you can see thing on the screen, then loop forever
  }
  return 0;
}
