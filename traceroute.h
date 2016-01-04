#ifndef TRACEROUTE_H
# define TRACEROUTE_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define PACKETSIZE  64

// time to wait for the call back
#define WAIT_TO_RECEIVE 2

// number of packet to send to test the destination
#define NB_PACKET 20

#define BUFSIZE IP_MAXPACKET

#define DEFMAXTTL 64

typedef struct sockaddr_in s_sockaddr_in;
typedef struct sockaddr s_sockaddr;
typedef struct icmphdr s_icmphdr;
typedef struct addrinfo s_addrinfo;
typedef struct iphdr s_iphdr;
typedef struct icmp s_icmp;
typedef struct ip s_ip;
typedef struct timeval s_timeval;

// add timeval ?
typedef struct	packet
{
	s_icmphdr	hdr;
	char 		msg[PACKETSIZE - sizeof(s_icmphdr)];
}				s_packet;

typedef struct	option
{
	/*useless, to prevent memory errors*/
	char		blop[1024];

	/**
	 * Time To Live.
	 */
	int			ttl;
	s_addrinfo	*addr_info;
	const char	*ip;
	const char	*name;
}				s_option;

unsigned short	checksum(void *b, int len);
void			traceroute(s_option *opt);

#endif
