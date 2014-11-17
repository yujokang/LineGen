/*
 * test vectors for using "struct c_gen"
 */
#include <c_gen.h>

struct c_gen_tv {
	/* the location of the expected output inside the "expected/" folder */
	char *expected_file;
	/*
	 * the test case
	 * out:		contains the stream to which the text will be written
	 * returns	1 if it the test function caught no errors yet,
	 *		or 0 if it caught a failure without reading the output
	 */
	int (*tester)(struct c_gen *out);
};

#define N_C_GEN_TESTS	4
/* the tests over which test_cs will run */
extern struct c_gen_tv *c_gen_tvs[N_C_GEN_TESTS];
