#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DEFAULT_NB_PACKET 100

void usage()
{
	printf("Usage: ft_ping ip_adress\n");
}

void get_ip_from_str(char const str[], void *dst)
{

	int s = inet_pton(AF_INET, str, dst);
	if (s <= 0)	{
		if (s == 0)
			fprintf(stderr, "Not in presentation format");
		else
			perror("inet_pton");
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char const *argv[])
{
	if (argc == 0){
		usage();
	}
	unsigned char ip_adress[sizeof(struct in6_addr)];

	get_ip_from_str(argv[2], ip_adress);
	int i = 0;
	while (i < DEFAULT_NB_PACKET)
	{

		++i;
	}
	return EXIT_SUCCESS;
}
