/*
 * A generic wrapper around the FILE stream to keep track of indentation,
 * which helps create properly-formatted code.
 */
#ifndef FORMAT_GEN_H
#define FORMAT_GEN_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>

#include <logger.h>

/*
 * the indentation tab character,
 * which will be printed once per indentation depth
 */
#define INDENT_CHAR	'\t'
/* the line break string */
#define LINE_BREAK_STR "\n"
/* the length of the line break string */
#define LINE_BREAK_LEN	strlen(LINE_BREAK_STR)

/*
 * the basic wrapper that keeps track of the FILE stream,
 * as well as the current indentation depth, up to a chosen limit,
 * and if the indentation still needs to be written for the current line
 */
struct line_gen {
	/* the FILE stream to write to */
	FILE *out_stream;
	/* current indentation depth. default of 0 */
	size_t indent;
	/* the maximum indentation depth */
	size_t max_indent;
	/*
	 * Have we not written to the line yet?
	 * If so we'll need to indent on the next write.
	 */
	int on_new_line;
};

/*
 * Initializes "struct line_gen" with the default values,
 * and sets the "out_stream" field
 * to_open:	the struct in which to write the initialized values
 * max_indent:	the desired "max_indent" field value
 * out_stream:	the desired "out_stream" field value
 */
static inline void init_line_gen(struct line_gen *to_open,
				 size_t max_indent, FILE *out_stream)
{
	to_open->out_stream = out_stream;
	to_open->indent = 0;
	to_open->max_indent = max_indent;
	to_open->on_new_line = 1;
}

/*
 * Initializes "struct line_gen" with the default values,
 * and opens the FILE stream
 * to_open:	the struct in which to write the initialized values
 * max_indent:	the desired "max_indent" field value
 * path:	the path of the file to open
 * returns	0 iff successful;
 *		-1 if opening the file in the path failed, which will set errno.
 */
static inline int open_line_gen(struct line_gen *to_open,
				size_t max_indent, const char *path)
{
	FILE *out_stream = fopen(path, "w");
	if (out_stream == NULL) {
		printlg(DEBUG_LEVEL, "Could not open stream.\n");
		return -1;
	}

	init_line_gen(to_open, max_indent, out_stream);

	return 0;
}

/*
 * Close the "struct line_gen",
 * which should be done before it is deallocated, or falls out of scope.
 * The "out_stream" field will be closed and set to NULL.
 * to_close:	the struct whose FILE stream to close
 * returns	0 iff successful;
 *		-1 if fclose failed, which will set errno
 */
static inline int close_line_gen(struct line_gen *to_close)
{
	FILE *stream_to_close = to_close->out_stream;

	to_close->out_stream = NULL;
	return fclose(stream_to_close);
}

/*
 * If the line is new, then write indentations, and mark it as not new.
 * Used when writing text.
 * to_write:	contains the stream to which to try to write the line
 * returns	0 iff successfuly wrote the bytes;
 *		-1 iff writing indentation characters failed.
 *		   Indentation will not count as written, ie. line is still new.
 */
static inline int try_start_line(struct line_gen *to_write)
{
	if (to_write->on_new_line) {
		if (to_write->indent) {
			char tab_buf[to_write->indent];
			size_t written;

			memset(tab_buf, INDENT_CHAR, to_write->indent);
			written = fwrite(tab_buf, to_write->indent, 1,
					 to_write->out_stream);
			if (written == 0) {
				printlg(DEBUG_LEVEL,
					"Could not indent: %d.\n", errno);
				return -1;
			}
		}
		to_write->on_new_line = 0;
	}

	return 0;
}

/*
 * Finish the current line with a line break, without starting the next line,
 * which will be marked as new.
 * Calling this operation multiple times in a row will result in blank lines
 * without indents.
 * to_write:	contains the stream to which to try to write the line break
 * returns	0 iff successfuly wrote the line break;
 *		-1 iff writing new line failed. Still keep old line.
 */
static inline int finish_line(struct line_gen *to_write)
{
	size_t written = fwrite(LINE_BREAK_STR, LINE_BREAK_LEN, 1,
				to_write->out_stream);
	if (written == 0) {
		printlg(DEBUG_LEVEL, "Could break line.\n");
		return -1;
	}
	to_write->on_new_line = 1;

	return 0;
}

/*
 * Break the old line if it wasn't already, ie. if the current line is not new,
 * and increment the indentation for next line
 * as long as the maximum has not already been reached.
 * This operation will therefore not leave any blank lines before the indent,
 * and can be called multiple times to increas the indent count
 * without starting a new line.
 * to_indent:	contains the stream to indent
 * returns	0 iff successfully broke the current line, and started the next;
 *		-1 if writing the new line was necessary, but failed.
 *		   Indentation stays the same.
 *		-2 if the indentation depth was at the maximum,
 *		   so that indentation cannot be increased.
 *		   New line is also not written.
 */
static inline int indent(struct line_gen *to_indent)
{
	int finish_line_ret;
	if (to_indent->indent >= to_indent->max_indent) {
		printlg(DEBUG_LEVEL, "Indentation would exceed maximum of "
				       "%u.\n",
			(unsigned) to_indent->max_indent);
		return -2;
	}
	if (!to_indent->on_new_line &&
	    (finish_line_ret = finish_line(to_indent))) {
		printlg(DEBUG_LEVEL,
			"Failed to start line before indentation.\n");
		return finish_line_ret;
	}
	to_indent->indent += 1;

	return 0;
}

/*
 * Break the current line if it wasn't already,
 * and decrease indentation by the desired amount for next line
 * as long as the indentation depth is
 * at least as much as the amount we want to decrease.
 * This function, which directly allows decreasing the indentation depth
 * by an amount greater than 1,
 * is available to support languages, such a Python,
 * in which undindentation could occur multiple times in one line.
 * to_unindent:	contains the stream to decrease the indentation count
 * less:	the amount by which we want to decrease indentation
 * returns	0 iff successfully broke the current line, and started the next;
 *		-1 if writing the new line was needed, but failed.
 *		   Indentation stays the same.
 *		-2 if the indentation depth is less than
 *		   what we want to decrease,
 *		   so that indentation cannot be decreased.
 *		   New line is also not written.
 */
static inline int less_indent(struct line_gen *to_unindent, size_t less)
{
	int finish_line_ret;
	if (to_unindent->indent < less) {
		printlg(DEBUG_LEVEL, "Current indentation of %u "
				       "is less than %u, "
				       "by which we want to decrease it.\n",
			(unsigned) to_unindent->indent, (unsigned) less);
		return -2;
	}
	if (!to_unindent->on_new_line &&
	    (finish_line_ret = finish_line(to_unindent))) {
		printlg(DEBUG_LEVEL,
			"Failed to start line before unindentation.\n");
		return finish_line_ret;
	}
	to_unindent->indent -= less;

	return 0;
}

/*
 * Break the current line if it wasn't already,
 * and decrease indentation by 1 for next line as long as it isn't 0.
 * to_unindent:	contains the stream to unindent
 * returns	0 iff successfully broke the current line, and started the next;
 *		-1 if writing the new line was needed, but failed.
 *		   Indentation stays the same.
 *		-2 if the indentation depth was 0,
 *		   so that indentation cannot be decreased.
 *		   New line is also not written.
 */
static inline int unindent(struct line_gen *to_unindent)
{
	return less_indent(to_unindent, 1);
}

/*
 * Write raw text to the current line.
 * text:	the string to write, up to the 0 character
 * to_write:	contains the stream to write the text to
 * returns	0 iff successfully wrote all bytes.
 *		-1 if failed to start a new line or write the text
 */
static inline int line_gen_write(const char *text, struct line_gen *to_write)
{
	if (try_start_line(to_write)) {
		printlg(DEBUG_LEVEL,
			"Failed to indent before writing raw text.\n");
		return -1;
	}
	if (fwrite(text, strlen(text), 1, to_write->out_stream) == 0) {
		printlg(DEBUG_LEVEL, "Failed to write raw text.\n");
		return -1;
	}
	return 0;
}

/*
 * Write formatted text to the current line.
 * to_write:	contains the stream to write the text to
 * fmt:		the format of the stream to write
 * ...:		the arguments to plug into the format
 * returns	the number of bytes written, or -1 on error,
 *		either while trying to start the new line,
 *		or while writing the line
 */
static inline int line_gen_printf(struct line_gen *to_write,
				  const char *fmt, ...)
{
	va_list args;
	int ret;

	if (try_start_line(to_write)) {
		printlg(DEBUG_LEVEL,
			"Failed to indent before writing formatted text.\n");
		return -1;
	}

	va_start(args, fmt);
	ret = vfprintf(to_write->out_stream, fmt, args);
	va_end(args);

	return ret;
}
#endif /* FORMAT_GEN_H */
