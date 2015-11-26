#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define DEFAULT_NB_PACKET 100
#define PACKET_SIZE 80

void usage()
{
	printf("Usage: ft_ping ip_adress\n");
}

// void get_ip_from_str(char const str[], void *dst)
// {

// 	int s = inet_pton(AF_INET, str, dst);
// 	if (s <= 0)	{
// 		if (s == 0)
// 			fprintf(stderr, "Not in presentation format");
// 		else
// 			perror("inet_pton");
// 		exit(EXIT_FAILURE);
// 	}
// }

// Computing the internet checksum (RFC 1071).
// Note that the internet checksum does not preclude collisions.
uint16_t checksum (uint16_t *addr, int len)
{
  int count = len;
  register uint32_t sum = 0;
  uint16_t answer = 0;

  // Sum up 2-byte values until none or only one byte left.
  while (count > 1) {
    sum += *(addr++);
    count -= 2;
  }

  // Add left-over byte, if any.
  if (count > 0) {
    sum += *(uint8_t *) addr;
  }

  // Fold 32-bit sum into 16 bits; we lose information by doing this,
  // increasing the chances of a collision.
  // sum = (lower 16 bits) + (upper 16 bits shifted right 16 bits)
  while (sum >> 16) {
    sum = (sum & 0xffff) + (sum >> 16);
  }

  // Checksum is one's compliment of sum.
  answer = ~sum;

  return (answer);
}

int main(int argc, char const *argv[])
{
	if (argc == 0){
		usage();
	}

	// create socket
	int fd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (fd == -1) {
	    printf("%s", strerror(errno));
	    exit(EXIT_FAILURE);
	}

	// Option so that the ip header will be constructed by the user.
	int hdrincl = 1;
	if (setsockopt(fd, IPPROTO_IP, IP_HDRINCL, &hdrincl, sizeof(hdrincl)) == -1) {
	    printf("%s", strerror(errno));
	    exit(EXIT_FAILURE);
	}

	// create icmp header for data
	const size_t req_size=8;
	struct icmphdr req;
	req.type= ICMP_ECHO;
	req.code = 0;
	req.checksum = 0;
	req.un.echo.id = htons(rand());
	req.un.echo.sequence = htons(1);
	req.checksum = checksum(&req, req_size);

	// create address
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */
	s = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (s != 0) {
		printf("getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	// send the data
	if (sendto(fd,
			&req,
			req_size,
			0,
	    	res->ai_addr,
	    	res->ai_addrlen) == -1) {
	    printf("%s",strerror(errno));
	exit(EXIT_FAILURE);
	}

	// receive the callback
	bytes = recvfrom(fd, inbuf, PACKET_SIZE, 0, (sockaddr *)&pingaddr, (socklen_t *)&addrlen);

	return EXIT_SUCCESS;
}
