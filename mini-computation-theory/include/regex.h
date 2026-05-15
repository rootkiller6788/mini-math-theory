#ifndef REGEX_H
#define REGEX_H

#include <stdbool.h>

#define MAX_REGEX_LEN 256

/* Compile a regex pattern string into internal representation (an NFA compiled to DFA).
 * Supported operators: concatenation, | (alternation), * (Kleene star),
 * + (one-or-more), ? (zero-or-one), . (any character except newline),
 * ( ) for grouping, \\ for literal backslash.
 *
 * Returns true on success, false on parse error. `err_msg` is filled on failure.
 */
bool regex_compile(const char *pattern, char *err_msg, int err_size);

/* Match the entire string `text` against the compiled regex.
 * Returns true if the whole string matches. */
bool regex_match(const char *text);

/* Find the first match of the compiled regex in `text`.
 * Returns start index in *start and end index (exclusive) in *end.
 * Returns true if a match was found. */
bool regex_match_partial(const char *text, int *start, int *end);

#endif /* REGEX_H */
