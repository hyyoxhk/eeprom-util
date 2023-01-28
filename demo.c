#include <stdio.h>
#include <stdlib.h>
#include <net/if.h>

int main(int argc, char const *argv[])
{
	char ifname[16];

	int ifindex = atoi(argv[1]);

	printf("argv ===> %s\n", argv[1]);

	if_indextoname(ifindex, ifname);
	printf("ifname ===> %s\n", ifname);

	return 0;
}

