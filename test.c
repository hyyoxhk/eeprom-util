#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>

int strtoi_base(char **str, int *dest, int base)
{
	if (**str == '\0')
		return -EINVAL;

	char *endptr;
	errno = 0;
	int num = strtol(*str, &endptr, base);

	if (errno != 0)
		return -errno;

	if (*str == endptr)
		return -EINVAL;

	*dest = num;
	*str = endptr;

	if (*endptr == 0)
		return 2;

	return 1;
}

static int __read_bin(unsigned char *data, int data_size, char *delimiter, bool reverse,
			char *str, size_t size)
{
	int i, n, len = 0;
	char *str1 = str;
	int from = reverse ? data_size - 1 : 0;
	int to = reverse ? 0 : data_size - 1;

	for (i = from; i != to; reverse ? i-- : i++) {
		n = snprintf(str1, size, "%02x%s", data[i], delimiter);
		str1 += n;
		len += n;
		printf("n = %d\n", n);
	}

	len += snprintf(str1, size, "%02x\n", data[i]);

	return len;
}

static int read_mac(unsigned char *data, int data_size, char *str, size_t size)
{
	return __read_bin(data, data_size, ":", false, str, size);
}

static void __print_bin(unsigned char *data, int data_size, char *delimiter, bool reverse)
{
	int i;
	int from = reverse ? data_size - 1 : 0;
	int to = reverse ? 0 : data_size - 1;
	for (i = from; i != to; reverse ? i-- : i++)
		printf("%02x%s", data[i], delimiter);

	printf("%02x\n", data[i]);
}

static void print_mac(unsigned char *data, int data_size)
{
	__print_bin(data, data_size, ":", false);
}

/* ARP hardware address length */
#define ARP_HLEN 6
/*
 * The size of a MAC address in string form, each digit requires two chars
 * and five separator characters to form '00:00:00:00:00:00'.
 */
#define ARP_HLEN_ASCII (ARP_HLEN * 2) + (ARP_HLEN - 1)
// bc:be:cd:01:1c:d9
int main()
{/*
	unsigned char aaa = 0xbc;
	unsigned char data[6]={0xbc, 0xbe, 0xcd, 0x01, 0x1c, 0xd9};
	int lenth;
	
	char bbb[20];

	print_mac(data, 6);
	printf("%02x\n", aaa);
	
	lenth = read_mac(data, sizeof(data), bbb, sizeof(bbb));
	printf("bbb    ===> %s\n", bbb);
	printf("lenth  ===> %d\n", lenth);
	printf("strlen ===> %ld\n", strlen(bbb));
	printf("sizeof ===> %ld\n", sizeof(bbb));
*/

	char aaa[] = "hello";
	char *bbb = "world";

	char *white0 = "\033[48;5;231m";
	char white1[] = "\033[48;5;231m";






	printf("%ld\n", sizeof(aaa));
	printf("%ld\n", sizeof(bbb));
	printf("%ld\n", sizeof(*white0));
	printf("%ld\n", sizeof(white1));
	return 0;
}






/*


std::string dir = "Documents";
char *_tempname = static_cast <char*> (malloc( dir.length() + 14));

strncpy(_tempname, dir.c_str(), dir.length()+1 );
strncat(_tempname, "/hellooXXXXXX", 13);


strncat(_tempname, "/hellooXXXXXX", 13);
strncat(_tempname, "/hellooXXXXXX", dir.length() + 14 - strlen(_tempname) - 1);


*/


























