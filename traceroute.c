#include "traceroute.h"

int sockopt_error()
{
	if (errno == EBADF)
	    printf("setsockopt: The argument socket is not a valid file descriptor.\n");
	else if (errno == EFAULT)
	    printf("setsockopt: The address pointed to by option_value is not in a valid part of the process address space.\n");
	else if (errno == EINVAL)
	    printf("setsockopt: The option is invalid at the level indicated.\n");
	else if (errno == ENOBUFS)
	    printf("setsockopt: Insufficient system resources available for the call to complete.\n");
	else if (errno == ENOMEM)
	    printf("setsockopt: Insufficient memory available for the system call to complete.\n");
	else if (errno == ENOPROTOOPT)
	    printf("setsockopt: The optption is unknown at the level indicated.\n");
	else if (errno == ENOTSOCK)
	    printf("setsockopt: The argument socket is not a socket (e.g., a plain file).\n");
	else if (errno == EDOM)
	    printf("setsockopt: The argument option_value is out of bounds.\n");
	else if (errno == EISCONN)
	    printf("setsockopt: ocket is already connected and a specified option cannot be set while this is the case.\n");
	else if (errno == EINVAL)
	    printf("setsockopt: The socket has been shut down.\n");
	else
		printf("setsockopt: unknow error\n");
	return (-1);
}

int	create_socket(int ttl)
{
	int				sock;
	struct timeval	to;
	int				errnum;

	sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock == -1)
	{
		perror("socket: ");
		return (-1);
	}

	//set timeout
	to.tv_sec = WAIT_TO_RECEIVE;
	to.tv_usec = 0;
	errnum = setsockopt(sock, IPPROTO_ICMP, SO_RCVTIMEO, (const void*)&to, sizeof(to));
	if (errnum < 0)
		return (sockopt_error());

	// set ttl
	errnum = setsockopt(sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));
	if (errnum < 0)
		return (sockopt_error());

	return (sock);
}

int test_is_end(s_option *opt, s_ip *ip)
{
	char			ip_buf[512];

	inet_ntop(AF_INET, (void*)&(ip->ip_src.s_addr), ip_buf, BUFSIZE);
	return (strcmp(opt->ip, ip_buf) == 0);
}

void display(int succeeded, s_option *opt, double timedif, s_ip *ip)
{
	struct hostent	*client;
	char			ip_buf[512];
	const char		*client_name;

	if (succeeded)
	{
		inet_ntop(AF_INET, (void*)&(ip->ip_src.s_addr), ip_buf, BUFSIZE);
		client = gethostbyaddr((void*)&(ip->ip_src.s_addr), sizeof(ip->ip_src.s_addr), AF_INET);
		if (client == NULL)
			client_name = ip_buf;
		else
			client_name = client->h_name;
		printf("%4d %s (%s) %.3f ms\n",
				opt->ttl, client_name, ip_buf, timedif);
	}
	else
		printf("%4d * * * *\n", opt->ttl);
	timedif = 0.;
}

double calculate_interval(s_timeval *begin, s_timeval *end)
{
	double			interval;

	interval = (double)((end->tv_sec - begin->tv_sec) * 1000.0f);
	interval += (double)((end->tv_usec - begin->tv_usec) / 1000.0f);
	return (interval);
}

// ping - Create message and send it.
void traceroute(s_option *opt)
{
	int sd = 0;
	s_packet packet;
	s_sockaddr_in r_addr;
	int pid = getpid();
	int len = sizeof(r_addr);

	// loop for one packet
	while (opt->ttl < DEFMAXTTL) {
		struct timeval	begin, end;
		++opt->ttl;

		// we change ttl each iteration, so we create one socket each iteration
		sd = create_socket(opt->ttl);
		if (sd == -1)
			exit(EXIT_FAILURE);

		// send message
		bzero(&packet, sizeof(packet));
		packet.hdr.type = ICMP_ECHO;
		packet.hdr.un.echo.id = pid;
		packet.hdr.un.echo.sequence = opt->ttl + 1;
		packet.hdr.checksum = checksum(&packet, sizeof(packet));
		gettimeofday(&begin, NULL);
		if (sendto(sd, &packet, sizeof(packet), 0, opt->addr_info->ai_addr, sizeof(*opt->addr_info->ai_addr)) <= 0)
		{
			perror("sendto");
			close(sd);
			exit(EXIT_FAILURE);
		}

		// receive message
		int recv_result = recvfrom(sd, &packet, sizeof(packet), 0, (s_sockaddr*)&r_addr, (socklen_t *)&len);

		// time management
		gettimeofday(&end, NULL);
		double timedif = calculate_interval(&begin, &end);

		// test message
		if (recv_result < 0) {
			display(0, opt, 0.0, NULL);
		} else {
			s_icmp *pkt;
			s_iphdr *iphdr = (s_iphdr*) &packet;
			pkt = (s_icmp*) (&packet + (iphdr->ihl << 2));
			display(1, opt, timedif, (s_ip*)&packet);
			if (test_is_end(opt, (s_ip*)&packet))
				exit(EXIT_SUCCESS);
		}
		close(sd);
	}
}
