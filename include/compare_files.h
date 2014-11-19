/*
 * function for comparing contents of files.
 * Used in testing of programs that use "line_gen.h".
 */
#ifndef COMPARE_FILES_H
#define COMPARE_FILES_H

#include <stdio.h>

/*
 * Check if two files have the same contents.
 * in_0:	the first file to read
 * in_1:	the second file to read
 * returns	1 iff the files are equal, 0 otherwise
 */
int files_equal(FILE *in_0, FILE *in_1);

#endif /* COMPARE_FILES_H */
