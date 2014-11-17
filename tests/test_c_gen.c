#define DEBUG
#include "c_gen_tests.h"
#include <logger.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* path to the output file */
#define TEST_PATH	"test_out.c"
#define EXPECTED_DIR	"expected/"

#define FILE_BLOCK_SIZE	1024

/*
 * Check if two files have the same contents.
 * in_0:	the first file to read
 * in_1:	the second file to read
 * returns	1 iff the files are equal, 0 otherwise
 */
static int files_equal(FILE *in_0, FILE *in_1)
{
	size_t total_read = 0;
	int have_bytes = 1;
	int ret = 1;

	while (have_bytes) {
		char buf_0[FILE_BLOCK_SIZE], buf_1[FILE_BLOCK_SIZE];
		size_t read_0, read_1;

		read_0 = fread(buf_0, 1, FILE_BLOCK_SIZE, in_0);
		read_1 = fread(buf_1, 1, FILE_BLOCK_SIZE, in_1);

		/* check if a file stopped early */
		if (read_0 < FILE_BLOCK_SIZE || read_1 < FILE_BLOCK_SIZE) {
			if (read_0 != read_1) {
				printlg(INFO_LEVEL,
					"Unequal length read: %u vs. %u.\n",
					(unsigned) (total_read + read_0),
					(unsigned) (total_read + read_1));
				ret = 0;
			}
			have_bytes = 0;
		}
		if (have_bytes) {
			size_t byte_i;

			for (byte_i = 0; byte_i < read_0; byte_i++) {
				if (buf_0[byte_i] != buf_1[byte_i]) {
					printlg(INFO_LEVEL,
						"Mismatch at %u: "
						"%c != %c.\n",
						(unsigned)
						(total_read + byte_i),
						buf_0[byte_i], buf_1[byte_i]);
					have_bytes = 0;
					ret = 0;
				}
			}

			total_read += read_0;
		}
	}

	return ret;
}

/*
 * Run a single c_gen test vector
 * c_gen_test:	the test vector containing the expected file and
 *		the testing function
 * returns	1 iff successful, else return 0
 */
static int test_c(struct c_gen_tv *c_gen_test)
{
	struct c_gen output;
	int ret = 1;

	/* open file to write to */
	if (open_c_gen(&output, TEST_PATH)) {
		printlg(ERROR_LEVEL,
			"Could not create temporay output file: %d.\n", errno);
		return 0;
	}

	/* write to file */
	ret = c_gen_test->tester(&output);
	close_c_gen(&output);
	if (ret) {
		size_t expected_dir_len = strlen(EXPECTED_DIR);
		size_t expected_file_len = strlen(c_gen_test->expected_file);
		char expected_path[expected_dir_len + expected_file_len + 1];
		FILE *expected_file, *output_file;

		/* open the expected file */
		strncpy(expected_path, EXPECTED_DIR, expected_dir_len + 1);
		strncat(expected_path, c_gen_test->expected_file,
			expected_file_len + 1);
		expected_file = fopen(expected_path, "r");
		if (expected_file == NULL) {
			printlg(ERROR_LEVEL,
				"Could not open expected file %s.\n",
				expected_path);
			ret = 0;
		} else {
			/* reopen the written-to file for reading */
			output_file = fopen(TEST_PATH, "r");
			if (output_file == NULL) {
				printlg(ERROR_LEVEL,
					"Could not open the output file.\n");
				ret = 0;
			} else {
				if (!files_equal(output_file, expected_file)) {
					printlg(ERROR_LEVEL,
						"Unexpected output.\n");
					ret = 0;
				}
				fclose(output_file);
			}
			fclose(expected_file);
		}

	} else {
		printlg(ERROR_LEVEL, "Premature error during test.\n");
		return 0;
	}

	return ret;
} 

static void test_cs()
{
	size_t test_i;

	for (test_i = 0; test_i < N_C_GEN_TESTS; test_i++) {
		printlg(INFO_LEVEL, "Running c_gen test %u...\n",
			(unsigned) test_i);
		if (test_c(c_gen_tvs[test_i])) {
			printlg(INFO_LEVEL, "Passed!\n");
		} else {
			printlg(ERROR_LEVEL, "Failed!\n");
		}
	}
}

int main()
{
	test_cs();

	return 0;
}
