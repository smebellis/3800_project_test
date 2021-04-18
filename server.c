#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
/*DMO */

#define MC_PORT 1818
#define MC_GROUP "239.0.0.1"
/* In this code, I create a MC socket to receive messages on, AND i create a non-multicast
   socket that I will send responses to */

struct outpacket
{
  char version;
  short port;
};

struct multicast
{
  char version;
  char command;
};

struct gamePacket
{
  char version;
  char command;
  char move;
  char game;
  char sequence;
};

int main(int argc, char *argv[])
{
  struct sockaddr_in addr, tcp_addr;
  int MC_sock, cnt, regularSock, tcpSocket;
  socklen_t addrlen;
  struct ip_mreq mreq;
  char message[50];
  int counter = 0;
  struct outpacket packetout, packetin;
  struct multicast multiPacketin;
  struct gamePacket gamePacket;

  regularSock = socket(AF_INET, SOCK_DGRAM, 0); // This is the non-MC socket
  if (regularSock < 0)
  {
    perror("socket");
    exit(1);
  }
  // Since I am a server, I will bind to a port/IP for the NON multi-cast socket
  bzero((char *)&addr, sizeof(addr));
  tcp_addr.sin_family = AF_INET;
  tcp_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  tcp_addr.sin_port = htons(10005);
  addrlen = sizeof(addr);

  if (bind(regularSock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    exit(1);
  }

  /* set up MulitCast socket */
  MC_sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (MC_sock < 0)
  {
    perror("socket");
    exit(1);
  }
  bzero((char *)&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(MC_PORT);
  addrlen = sizeof(addr);

  if (bind(MC_sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    perror("bind");
    exit(1);
  }
  mreq.imr_multiaddr.s_addr = inet_addr(MC_GROUP);
  mreq.imr_interface.s_addr = htonl(INADDR_ANY);
  if (setsockopt(MC_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                 &mreq, sizeof(mreq)) < 0)
  {
    perror("setsockopt mreq");
    exit(1);
  }

  cnt = recvfrom(MC_sock, &multiPacketin, sizeof(multiPacketin), 0, (struct sockaddr *)&addr, &addrlen);
  if (cnt < 0)
  {
    perror("recvfrom");
    exit(1);
  }

  packetout.port = htons(10005);
  packetout.version = 6;

  cnt = sendto(regularSock, &packetout, sizeof(packetout), 0, (struct sockaddr *)&addr, addrlen);
  if (cnt < 0)
  {
    perror("sendto");
    exit(1);
  }

  tcpSocket = socket(AF_INET, SOCK_STREAM, 0);

  addr.sin_port = htons(10005);
  cnt = bind(tcpSocket, (struct sockaddr *)&addr, addrlen);
  cnt = listen(tcpSocket, 5);

  cnt = accept(tcpSocket, (struct sockaddr *)&addr, &addrlen);

  cnt = recv(tcpSocket, &gamePacket, sizeof(gamePacket), 0);

  printf("[TCP SOCKET]: \n %x", gamePacket.version);

  // Now i loop forever, getting a message on the MC socket, then unicast a respons
  // and just because I can, i then receive something on the regula

  /*
 while (1)
  {
    memset(message, 0, 50);
    // Receive something from the multicast socket
    cnt = recvfrom(MC_sock, message, sizeof(message), 0,
                   (struct sockaddr *)&addr, &addrlen);
    if (cnt < 0)
    {
      perror("recvfrom");
      exit(1);
    }
    else if (cnt == 0)
    {
      break;
    }
    printf("%s: message = \"%s\"\n", inet_ntoa(addr.sin_addr), message);
    packetout.port = htons(10004);
    packetout.version = 6;
    //memset(message, 0, 50);
    //sprintf(message, "I am the server %d", 10004); //counter++);
    // Send something to the regular socket. NOTE: the 'to' address here
    // is set above in the recvfrom. I am sending a unicast back to whomever
    // send me the multicast
    cnt = sendto(regularSock, &packetout, sizeof(packetout), 0,
                 (struct sockaddr *)&addr, addrlen);
    if (cnt < 0)
    {
      perror("sendto");
      exit(1);
    }
    memset(message, 0, 50);
    // this is just to show that I can also receive stuff back on the unicast socket
    cnt = recvfrom(regularSock, message, sizeof(message), 0,
                   (struct sockaddr *)&addr, &addrlen);
    if (cnt > 0)
      printf("received the following on my normal socket '%s'\n", message);
  }*/
  return 0;
}
