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
#define VERSION 6
#define RESUME 3

/*********************************/
/*     Setup Struct to receive   */
/*        multicast packet       */
/*********************************/
struct info
{
    char version;
    char command;
};
struct multicastPacket
{
    char version;
    short portNumber;
};

int main(int argc, char *argv[])
{
    int multicastSocket, tcpSocket, rc, portNum;
    struct sockaddr_in addr, from_addr;
    socklen_t addrlen;
    socklen_t serverlen;
    struct ip_mreq mreq;
    struct multicastPacket packetIn;
    struct info packetOut;
    char ipAddr[16];

    /*Sets up the multicast socket*/
    multicastSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (multicastSocket < 0)
    {
        perror("multicast socket");
        exit(1);
    }
    /*Sets up the TCP socket*/
    tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (tcpSocket < 0)
    {
        perror("tcpSocket");
        exit(1);
    }

    // This sets up the TO address. Note that I am setting up the TO address
    // to be a multicast address....
    bzero((char *)&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(MC_PORT);
    addr.sin_addr.s_addr = inet_addr(MC_GROUP);
    addrlen = sizeof(addr);

    packetOut.version = VERSION;
    packetOut.command = RESUME;

    /*********************************/
    /*          Send / Receive       */
    /*        multicast packet       */
    /*********************************/

    rc = sendto(multicastSocket, &packetOut, sizeof(packetOut), 0, (struct sockaddr *)&addr, addrlen);
    if (rc < 0)
    {
        perror("SEND TO");
        exit(1);
    }

    rc = recvfrom(multicastSocket, &packetIn, sizeof(packetIn), 0, (struct sockaddr *)&from_addr, &addrlen);
    if (rc < 0)
    {
        perror("RECV From");
        exit(1);
    }
    /***************************************************************************/

    portNum = ntohs(packetIn.portNumber);
    sprintf(ipAddr, inet_ntoa(from_addr.sin_addr));

    from_addr.sin_port = htons(portNum);
    inet_pton(AF_INET, ipAddr, &(from_addr.sin_addr));
    rc = connect(tcpSocket, (struct sockaddr *)&from_addr, &addrlen);
    char buffer[50];
    sprintf(buffer, "Test packet sent on TCP connection");

    rc = write(tcpSocket, &buffer, sizeof(buffer));
}
