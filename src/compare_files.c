#include <compare_files.h>
#include <logger.h>

#define FILE_BLOCK_SIZE	1024

int files_equal(FILE *in_0, FILE *in_1)
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
