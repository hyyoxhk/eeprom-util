#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

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

int main() {

	// char *aaa = "bc:be:cd:01:1c:d9";
	// int val;

	// char *bin = aaa;

	// strtoi_base(&bin, &val, 16);

	// printf("bin %s\n", bin);

	// printf("%x\n", (unsigned char)val);

	// bin++;

	// strtoi_base(&bin, &val, 16);

	// printf("%x\n", (unsigned char)val);

	// char str[30] = "2030300 This is test";
	// char *ptr;
	// int ret;

	// ret = strtol(str, &ptr, 10);
	// printf("数字（无符号长整数）是 %d\n", ret);
	// printf("字符串部分是 |%s|\n", ptr);


	// printf("%ld\n", sizeof(long int));
	// printf("%ld\n", sizeof(long));
	// printf("%ld\n", sizeof(int));

	char *bbb = "12a0b234a";
	char tmp[3] = { 0, 0, 0 };
	int i = 0;

	for (int k = 0; k < 2; k++) {
		tmp[k] = bbb[i + k];	
	}

	char *str = tmp;
	int byte = 0;
	strtoi_base(&str, &byte, 16);

	printf("%x\n", (unsigned char)byte);

	i = i + 2;

	for (int k = 0; k < 2; k++) {
		tmp[k] = bbb[i + k];	
	}

	char *str1 = tmp;
	int byte1 = 0;
	strtoi_base(&str1, &byte1, 16);

	strtoi_base(&str1, &byte1, 16);

	printf("%x\n", (unsigned char)byte1);






	return 0;
}

