#include "traceroute.h"

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

// return the address struct corresponding to the ip
s_addrinfo *get_addr(const char *ip)
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
	return (rp);
}

// pretend to manage option -h -v
char *ip_arg(int argc, char *argv[])
{
	for (int i = 1; i < argc; ++i) {
		char *str = argv[i];
		if (strcmp("-v", str) != 0 && strcmp("-h", str) != 0) {
			return str;
		}
	}
	printf("error, no argument\n");
	exit(EXIT_FAILURE);
}

void init_opt(s_option *opt, const char *ip)
{
	opt->ttl = 0;
	opt->addr_info = get_addr(ip);
	opt->ip = ip;
}

int main(int argc, char *argv[])
{
	s_option opt;

	if (argc < 2) {
		printf("usage: %s <addr_info>\n", argv[0]);
		exit(0);
	}
	const char *ip = ip_arg(argc, argv);
	init_opt(&opt, ip);
	traceroute(&opt);
	return 0;
}
