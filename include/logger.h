/*
 * Logging and debugging utilities
 */
#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdarg.h>
#include <debug_assert.h>

/*
 * logging levels, as represented numerically.
 * Higher values are more severe.
 */
enum log_level {
	FATAL_LEVEL = 5, /* unrecoverable error, and program will stop */
	ERROR_LEVEL = 4, /* undesired execution, but try to continue */
	WARNING_LEVEL = 3, /* unsafe execution */
	INFO_LEVEL = 2, /* execution information */
	/*
	 * failure that should be impossible
	 * --should not appear in production
	 */
	ASSERT_LEVEL = 1,
	/* debug information --should not appear in production */
	DEBUG_LEVEL = 0
};
/* total number of levels */
#define N_LEVELS		(FATAL_LEVEL + 1)

/*
 * Set the default logging level at the production level,
 * ie. above ASSERT_LEVEL, unless DEBUG is defined,
 * in which case, set DISP_LEVEL to DEBUG_LEVEL
 */
#ifndef DISP_LEVEL
#ifdef DEBUG
#define DISP_LEVEL	DEBUG_LEVEL
#else
#define DISP_LEVEL	INFO_LEVEL
#endif /* DEBUG */
#endif /* DISP_LEVEL */

/* Default output is stderr */
#ifndef LOG_FILE
#define LOG_FILE	stderr
#endif

/*
 * Human-readable representation of the logging levels,
 * which will appear in the log output.
 * Texts are indexed by severity level.
 */
static const char *tags[N_LEVELS] = {[DEBUG_LEVEL] = "DEBUG",
				     [ASSERT_LEVEL] = "ASSERT",
				     [INFO_LEVEL] = "INFO",
				     [WARNING_LEVEL] = "WARNING",
				     [ERROR_LEVEL] = "ERROR",
				     [FATAL_LEVEL] = "FATAL"};
/*
 * Map the warning level to the string tag
 * level:	the numerical log level that indexes the tag
 * returns	the corresponding log string tag
 */
static inline const char *getTag(size_t level)
{
	debug_assert(level < N_LEVELS);
	return tags[level];
}

/*
 * Print debug tag and current file name and line number,
 * if the level meets the minimum DISP_LEVEL
 */
#define TAG_LOCATION(level) do { \
	if (level >= DISP_LEVEL) { \
		fprintf(LOG_FILE, "[%s] File %s, Line %d\n", getTag(level), \
			__FILE__, __LINE__); \
	} \
} while (0)

/*
 * Writes to log if specified level meets the minimum DISP_LEVEL
 * level:	the log level,
 *		which determines if the message should even be logged,
 *		and how to tag the message
 * fmt:		the format of the message to output,
 *		following printf semantics
 * ...:	the values to plug into the format
 */
__attribute__((format(printf, 2, 3)))
static inline void printlg(enum log_level level, char *fmt, ...)
{
#pragma GCC diagnostic ignored "-Wtype-limits"
	if (level >= DISP_LEVEL) {
#pragma GCC diagnostic warning "-Wtype-limits"
		va_list args;

		fprintf(LOG_FILE, "[%s]: ", getTag(level));

		va_start(args, fmt);
		vfprintf(LOG_FILE, fmt, args);
		va_end(args);
	}
}

#endif /* LOGGER_H */
