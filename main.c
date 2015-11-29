#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/wait.h>

#include <fcntl.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#define PACKETSIZE  64

// time to wait for the call back
#define WAIT_TO_RECEIVE 2

// number of packet to send to test the destination
#define NB_PACKET 20

typedef struct sockaddr_in s_sockaddr_in;
typedef struct sockaddr s_sockaddr;
typedef struct icmphdr s_icmphdr;
typedef struct addrinfo s_addrinfo;

// add timeval ?
typedef struct	packet
{
	s_icmphdr	hdr;
	char 		msg[PACKETSIZE - sizeof(s_icmphdr)];
}				s_packet;

/*--------------------------------------------------------------------*/
/*--- checksum - standard 1s complement checksum                   ---*/
/*--------------------------------------------------------------------*/
unsigned short checksum(void *b, int len)
{
	unsigned short *buf = b;
	unsigned int sum=0;
	unsigned short result;

	for ( sum = 0; len > 1; len -= 2 )
		sum += *buf++;
	if ( len == 1 )
		sum += *(unsigned char*)buf;
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	result = ~sum;
	return result;
}

s_addrinfo get_addr(const char *ip)
{
	s_addrinfo hints;
	s_addrinfo *result, *rp;

	memset(&hints, 0, sizeof(s_addrinfo));
	hints.ai_family = AF_UNSPEC;      /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM;   /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0; /* Any protocol */

	int s = getaddrinfo(ip, NULL, &hints, &result);
	if (s != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	Try each address until we successfully create a socket. */
	for (rp = result; rp != NULL; rp = rp->ai_next) {
		int sfd = socket(rp->ai_family, rp->ai_socktype,
				rp->ai_protocol);
		if (sfd == -1)
			continue;

		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			close(sfd);
			break; /* Success */
		}

		close(sfd);
	}

	if (rp == NULL) { /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}
	return (*rp);
}

/*--------------------------------------------------------------------*/
/*--- ping - Create message and send it.                           ---*/
/*--------------------------------------------------------------------*/
void ping(s_addrinfo *addr_info)
{
	const int val=255;
	int sd, cnt=1;
	s_packet packet;
	s_sockaddr_in r_addr;
	int pid = getpid();

	// create socket
	sd = socket(PF_INET, SOCK_RAW, IPPROTO_ICMP);
	if ( sd < 0 ) {
		perror("socket");
		return;
	}
	if ( setsockopt(sd, SOL_IP, IP_TTL, &val, sizeof(val)) != 0)
		perror("Set TTL option");

	// loop for one packet
	while (42) {
		int len = sizeof(r_addr);

		// send message
		printf("Msg #%d\n", cnt);
		bzero(&packet, sizeof(packet));
		packet.hdr.type = ICMP_ECHO;
		packet.hdr.un.echo.id = pid;
		packet.hdr.un.echo.sequence = cnt++;
		packet.hdr.checksum = checksum(&packet, sizeof(packet));
		if (sendto(sd, &packet, sizeof(packet), 0, addr_info->ai_addr, sizeof(*addr_info->ai_addr)) <= 0)
			perror("sendto");

		//set timeout for recvfrom
		struct timeval tv;
		tv.tv_sec = WAIT_TO_RECEIVE;
		tv.tv_usec = 0;
		setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

		// receive message and test if it is a response to the echo message
		if (recvfrom(sd, &packet, sizeof(packet), 0, (s_sockaddr*)&r_addr, (socklen_t *)&len) > 0 ) {
			struct icmp *pkt;
			struct iphdr *iphdr = (struct iphdr *) &packet;
			pkt = (struct icmp *) (&packet + (iphdr->ihl << 2));
			if (pkt->icmp_type == ICMP_ECHOREPLY){
				printf("***Got message!***\n");
			} else {
				printf("Message is not an icmp echo response\n");
			}
		}
		else
			printf("can't receive response\n");
		sleep(1);
	}
}

/*--------------------------------------------------------------------*/
/*--- main - look up host and start ping processes.                ---*/
/*--------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("usage: %s <addr_info>\n", argv[0]);
		exit(0);
	}

	s_addrinfo addr_info = get_addr(argv[1]);
	ping(&addr_info);
	return 0;
}
