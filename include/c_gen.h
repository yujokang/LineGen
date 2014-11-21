/*
 * A wrapper and macros for writing properly-formatted C code
 */
#ifndef C_GEN_H
#define C_GEN_H

#include <line_gen.h>
#include <logger.h>

#include <stdarg.h>

/* not enforced, but used to determine MAX_C_INDENTS */
#define MAX_C_CHARS_PER_LINE	80
/* maximum number of indents allowed in a line of C code */
#define MAX_C_INDENTS		(MAX_C_CHARS_PER_LINE / 8)

/* opening and closing characters */
/* for code blocks, unions and structs */
#define BLOCK_OPEN	"{"
#define BLOCK_CLOSE	"}"
/* for function arguments and expressions */
#define PAREN_OPEN	"("
#define PAREN_CLOSE	")"

/* delimiter for new argument */
#define NEW_ARG		", "
/* mark end of statement line */
#define END_STATEMENT	";"
/* field access marker for struct or union */
#define FIELD_ACCESS	"."
/* field access marker for pointer to struct or union */
#define POINTER_FIELD_ACCESS	"->"
/* derefence pointer */
#define DEREF		"*"
/* get refence pointer */
#define GET_REF		"&"

/* C keywords, denoted by KW suffix */
#define STATIC_KW	"static"
#define INLINE_KW	"inline"
#define VOLATILE_KW	"volatile"
#define RETURN_KW	"return"
#define BREAK_KW	"break"
#define DEFAULT_KW	"default:"
#define ELSE_KW		" else "
#define INCLUDE_KW	"#include "
#define STRUCT_KW	"struct"
#define UNION_KW	"union"

/* Format strings for C code, denoted by FMT suffix */
#define FOR_FMT			"for (%s; %s; %s) "
#define WHILE_FMT		"while (%s) "
#define IF_FMT			"if (%s) "
#define ELSE_IF_FMT		ELSE_KW IF_FMT
#define SWITCH_FMT		"switch (%s) "
#define CASE_FMT		"case %s:"
#define ARR_FMT			"%s[%s]"
#define ARR_INT_FMT		"%s[%d]"
#define RETURN_VAL_FMT		RETURN_KW " %s"
#define STRUCT_FMT		STRUCT_KW " %s "
#define UNION_FMT		UNION_KW " %s "
#define VAR_DEC_FMT		"%s %s"
#define VAR_DEF_FMT		VAR_DEC_FMT " = "
#define ASSIGN_FMT		"%s = "
#define FIELD_ASSIGN_FMT	FIELD_ACCESS ASSIGN_FMT
#define TYPEDEF_FMT		"typedef " VAR_DEC_FMT
#define MACRO_FMT		"#define %s %s"
#define INCLUDE_LOCAL_FMT	INCLUDE_KW "\"%s\""
#define INCLUDE_FMT		INCLUDE_KW "<%s>"
#define STRING_FMT		"\"%s\""

/* types, denoted by TP suffix */
#define VOID_TP		"void"
#define INT_TP		"int"
#define CHAR_TP		"char"
#define UNSIGNED_TP	"unsigned"
#define SHORT_TP	"short"
#define LONG_TP		"long"
#define POINTER_TP	"*"

/* common standard include files */
#define STDIO_H_PATH	"stdio.h"
#define STDLIB_H_PATH	"stdlib.h"

/* common function names */
#define MAIN_FUNC_NAME	"main"

/*
 * wrapper around the basic line formatter
 */
struct c_gen {
	/*
	 * the basic line formatter that keeps
	 * indentation state and the output stream
	 */
	struct line_gen base_gen;
};

/*
 * a variable with a type, used in function declarations
 */
struct typed_var {
	char *type; /* name of the type */
	char *name; /* name of the variable */
};

/*
 * Initializes "struct c_gen" with the specific values for proper C code,
 * and opens the FILE stream
 * to_open:	the struct in which to write the initialized values
 * path:	the path of the file to write to
 * returns	0 iff successful;
 *		-1 if fopen failed, which will set errno.
 */
static inline int open_c_gen(struct c_gen *to_open, const char *path)
{
	return open_line_gen(&to_open->base_gen, MAX_C_INDENTS, path);
}

/*
 * Initializes "struct c_gen" with the specific values for proper C code,
 * and sets the FILE stream
 * to_open:	the struct in which to write the initialized values
 * out_stream:	the desired "out_stream" field value
 */
static inline void init_c_gen(struct c_gen *to_open, FILE *out_stream)
{
	init_line_gen(&to_open->base_gen, MAX_C_INDENTS, out_stream);
}

/*
 * Close the "struct c_gen",
 * which should be done before it is deallocated, or falls out of scope.
 * The "out_stream" field of the "base_gen" field
 * will be closed and set to NULL.
 * to_close:	contains the "base_gen" field to close
 * returns	0 iff successful;
 *		-1 if fclose failed, which will set errno
 */
static inline int close_c_gen(struct c_gen *to_close)
{
	return close_line_gen(&to_close->base_gen);
}

/*
 * Start a block with an open brace and indent.
 * to_start:	contains the stream in which to open the block
 * returns	0 iff successful
 *		-1 if adding open brace or starting new line failed.
 *		   errno will be set
 *		-2 if indenting failed because indentation depth
 *		   would exceed maximum. errno is not set
 */
static inline int open_block(struct c_gen *to_start)
{
	int ret;
	if ((ret = line_gen_write(BLOCK_OPEN, &to_start->base_gen))) {
		printlg(ERROR_LEVEL, "Could not write open brace.\n");
		return ret;
	}
	if ((ret = indent(&to_start->base_gen))) {
		printlg(ERROR_LEVEL, "Could not indent for new block.\n");
		return ret;
	}

	return 0;
}

/*
 * Close a block and with an unindent and closing brace,
 * but do not start the next line.
 * to_close:	contains the stream in which to close the block
 * returns	0 iff successful
 *		-1 if adding closing brace or starting new line failed,
 *		   with errno set
 *		-2 if unindenting failed because indentation depth
 *		   is already 0. errno is not set
 */
static inline int _close_block(struct c_gen *to_close)
{
	int ret;
	if ((ret = unindent(&to_close->base_gen))) {
		printlg(ERROR_LEVEL,
			"Could not unindent before closing block.\n");
		return ret;
	}
	if ((ret = line_gen_write(BLOCK_CLOSE, &to_close->base_gen))) {
		printlg(ERROR_LEVEL,
			"Could not write close brace.\n");
		return ret;
	}

	return 0;
}

/*
 * Close a block, and start the next line 
 * to_close:	contains the stream in which to close the block
 * returns	0 iff successful
 *		-1 if adding closing brace or starting new line failed,
 *		   with errno set
 *		-2 if unindenting failed because indentation depth
 *		   is already 0. errno is not set
 */
static inline int close_block(struct c_gen *to_close)
{
	int ret;
	if ((ret = _close_block(to_close))) {
		printlg(ERROR_LEVEL, "Could not close block.\n");
		return ret;
	}

	if ((ret = finish_line(&to_close->base_gen))) {
		printlg(ERROR_LEVEL,
			"Could not start new line after block.\n");
		return ret;
	}

	return 0;
}

/*
 * End statement line
 * to_end:	contains the stream in which to end the line
 * returns	0 iff successful
 *		-1 if writing the line end, or starting the next line failed
 */
static inline int end_statement(struct c_gen *to_end)
{
	int ret;
	if ((ret = line_gen_write(END_STATEMENT, &to_end->base_gen))) {
		printlg(ERROR_LEVEL, "Could not end statement.\n");
		return ret;
	}
	if ((ret = finish_line(&to_end->base_gen))) {
		printlg(ERROR_LEVEL,
			"Could not start line after the statement.\n");
		return ret;
	}

	return 0;
}

/*
 * Start if block
 * to_start:	contains the stream in which to start the block
 * condition:	the condition for the if statement
 * returns	0 iff successful
 *		-1 if writing the if line, or opening the block failed,
 *		   with errno set
 *		-2 if indenting failed because indentation depth
 *		   would exceed maximum. errno is not set
 */
static inline int start_if(struct c_gen *to_start, char *condition)
{
	int ret;
	if (line_gen_printf(&to_start->base_gen, IF_FMT, condition) <= 0) {
		printlg(ERROR_LEVEL, "Could not write \"if\" line.\n");
		return -1;
	}
	if ((ret = open_block(to_start))) {
		printlg(ERROR_LEVEL, "Could not open \"if\" block.\n");
		return ret;
	}

	return 0;
}

/*
 * Start while block
 * to_start:	contains the stream in which to start the block
 * condition:	the condition for the while statement
 * returns	0 iff successful
 *		-1 if writing the while line, or opening the block failed,
 *		   with errno set
 *		-2 if indenting failed because indentation depth
 *		   would exceed maximum. errno is not set
 */
static inline int start_while(struct c_gen *to_start, char *condition)
{
	int ret;
	if (line_gen_printf(&to_start->base_gen, WHILE_FMT, condition) <= 0) {
		printlg(ERROR_LEVEL, "Could not write \"while\" line.\n");
		return -1;
	}
	if ((ret = open_block(to_start))) {
		printlg(ERROR_LEVEL, "Could not open \"while\" block.\n");
		return ret;
	}

	return 0;
}

/*
 * Start switch block
 * to_start:	contains the stream in which to start the block
 * expr:	the expression whose value to check
 * returns	0 iff successful
 *		-1 if writing the switch line, or opening the block failed,
 *		   with errno set
 *		-2 if indenting failed because indentation depth
 *		   would exceed maximum. errno is not set
 */
static inline int start_switch(struct c_gen *to_start, char *expr)
{
	int ret;
	if (line_gen_printf(&to_start->base_gen, SWITCH_FMT, expr) <= 0) {
		printlg(ERROR_LEVEL, "Could not write \"switch\" line.\n");
		return -1;
	}
	if ((ret = open_block(to_start))) {
		printlg(ERROR_LEVEL, "Could not open \"switch\" block.\n");
		return ret;
	}

	return 0;
}

/*
 * Start a new case in a switch block
 * to_start:	contains the stream in which to start the case
 * value:	the expected value for this case
 * returns	0 iff successful
 *		-1 if writing a line failed, with errno set
 *		-2 if unindenting failed because indentation depth is already 0.
 *		   errno is not set
 */
static inline int add_case(struct c_gen *to_start, char *value)
{
	int ret;
	if ((ret = unindent(&to_start->base_gen))) {
		printlg(ERROR_LEVEL, "Could not unindent to add case.\n");
		return ret;
	}
	if (line_gen_printf(&to_start->base_gen, CASE_FMT, value) <= 0) {
		printlg(ERROR_LEVEL, "Could not write value for \"case\"\n");
		return -1;
	}

	if ((ret = indent(&to_start->base_gen))) {
		printlg(ERROR_LEVEL, "Could not indent after \"case\"\n");
		return -1;
	}

	return 0;
}

/*
 * Start the default case in a switch block
 * to_start:	contains the stream in which to start the default case
 * value:	the expected value for this case
 * returns	0 iff successful
 *		-1 if writing a line failed, with errno set
 *		-2 if unindenting failed because indentation depth
 *		   is already 0. errno is not set
 */
static inline int add_default(struct c_gen *to_start)
{
	int ret;
	if ((ret = unindent(&to_start->base_gen))) {
		printlg(ERROR_LEVEL,
			"Could not unindent to add default case.\n");
		return ret;
	}
	if ((ret = line_gen_write(DEFAULT_KW, &to_start->base_gen))) {
		printlg(ERROR_LEVEL, "Could not start default case\n");
		return ret;
	}

	if ((ret = indent(&to_start->base_gen))) {
		printlg(ERROR_LEVEL, "Could not indent after default case\n");
		return -1;
	}

	return 0;
}

/*
 * Start for block
 * to_start:	contains the stream in which to start the block
 * init:	the initialization statement
 * condition:	the statement to check if the body should still be run
 * progress:	the statement executed at the end of each iteration
 * returns	0 iff successful
 *		-1 if writing the for line, or opening the block failed,
 *		   with errno set
 *		-2 if indenting failed because indentation depth
 *		   would exceed maximum. errno is not set
 */
static inline int start_for(struct c_gen *to_start,
			    char *init, char *condition, char *progress)
{
	int ret;
	if (line_gen_printf(&to_start->base_gen, FOR_FMT,
			    init, condition, progress) <= 0) {
		printlg(ERROR_LEVEL, "Could not write \"for\" line.\n");
		return -1;
	}
	if ((ret = open_block(to_start))) {
		printlg(ERROR_LEVEL, "Could not open \"for\" block.\n");
		return ret;
	}

	return 0;
}

/*
 * End the previous block, and start an else block
 * to_start:	contains the stream in which to start the block
 * returns	0 iff successful
 *		-1 if writing a line failed, with errno set
 *		-2 if indenting failed because indentation depth
 *		   would exceed maximum. errno is not set
 */
static inline int start_else(struct c_gen *to_start)
{
	int ret;
	if ((ret = _close_block(to_start))) {
		printlg(ERROR_LEVEL,
			"Could not end block before \"else\".\n");
		return ret;
	}
	if ((ret = line_gen_write(ELSE_KW, &to_start->base_gen))) {
		printlg(ERROR_LEVEL, "Could not write \"else\" statement\n");
		return ret;
	}

	if ((ret = open_block(to_start))) {
		printlg(ERROR_LEVEL, "Could not open \"else\" block.\n");
		return ret;
	}

	return 0;
}

/*
 * End the previous block, and start an else-if block
 * to_start:	contains the stream in which to start the block
 * condition:	the condition for the if part
 * returns	0 iff successful
 *		-1 if writing a line failed, with errno set
 *		-2 if indenting failed because indentation depth
 *		   would exceed maximum. errno is not set
 */
static inline int start_else_if(struct c_gen *to_start, char *condition)
{
	int ret;
	if ((ret = _close_block(to_start))) {
		printlg(ERROR_LEVEL,
			"Could not end block before else-if.\n");
		return ret;
	}
	if (line_gen_printf(&to_start->base_gen, ELSE_IF_FMT, condition) <= 0) {
		printlg(ERROR_LEVEL, "Could not write else-if statement\n");
		return -1;
	}

	if ((ret = open_block(to_start))) {
		printlg(ERROR_LEVEL, "Could not open else-if block.\n");
		return ret;
	}

	return 0;
}

/*
 * Print a line to include a local header file.
 * to_include_in:	contains the stream to which to write the include line
 * local_header:	the path of the local header file
 * returns		0 on success
 *			-1 if writing or ending the line failed, with errno set
 */
inline static int include_local(struct c_gen *to_include_in,
				const char *local_header)
{
	int ret;

	if (line_gen_printf(&to_include_in->base_gen, INCLUDE_LOCAL_FMT,
			    local_header) <= 0) {
		printlg(ERROR_LEVEL, "Could not write local include line.\n");
		return -1;
	}
	if ((ret = finish_line(&to_include_in->base_gen))) {
		printlg(ERROR_LEVEL,
			"Could not finish local include line.\n");
		return ret;
	}

	return 0;
}

/*
 * Print a line to include a standard header file,
 * or one in a directory specified by the "-I" to the compiler.
 * to_include_in:	contains the stream to which to write the include line
 * header:		the path of the header file
 * returns		0 on success
 *			-1 if writing or ending the line failed, with errno set
 */
inline static int include(struct c_gen *to_include_in, const char *header)
{
	int ret;

	if (line_gen_printf(&to_include_in->base_gen, INCLUDE_FMT, header)
	    <= 0) {
		printlg(ERROR_LEVEL, "Could not write include line.\n");
		return -1;
	}
	if ((ret = finish_line(&to_include_in->base_gen))) {
		printlg(ERROR_LEVEL,
			"Could not finish include line.\n");
		return ret;
	}

	return 0;
}

/*
 * Begin a function declaration.
 * Can be used to start a definition, with a new block,
 * or just a declaration, by ending the line as a statement.
 * to_declare:	contains the stream for writing the declaration line
 * type:	the return type
 * name:	the function name
 * n_args:	number of function arguments to follow
 * ...:		"struct typed_var *" instances that determine the arguments
 *		of the new function
 * returns	0 iff successful
 *		-1 if writing a line failed, with errno set
 */
int declare_function(struct c_gen *to_declare, const char *type,
		     const char *name, size_t n_args, ...);
#endif /* C_GEN_H */
