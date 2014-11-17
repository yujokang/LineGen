/*
 * Wrapper for assert function that only runs if DEBUG is defined
 * Convenient if only want one assert line.
 */
#ifndef DEBUG_ASSERT_H
#define DEBUG_ASSERT_H

#include <assert.h>

#ifdef DEBUG
#define debug_assert(a) assert(a)
#else
#define debug_assert(a)
#endif

#endif /* DEBUG_ASSERT_H */
