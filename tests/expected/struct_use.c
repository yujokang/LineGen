#include <stdio.h>

struct test_struct {
	char test_member;
};

static struct test_struct test_value = {
	.test_member = 'h',
};

int main()
{
	printf("Member of struct is %c\n", test_value.test_member);
	printf("Member of pointer is %c\n", (&test_value)->test_member);

	return 0;
}
