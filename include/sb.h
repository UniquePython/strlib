#ifndef SB_H_
#define SB_H_

#include <stddef.h>
#include "sv.h"
#include "str.h"

typedef struct
{
    char *data;
    size_t len;
    size_t cap;
} SB;

SB sb_new(void);
SB sb_with_cap(size_t initial_cap);
void sb_free(SB *sb);
void sb_reset(SB *sb);

void sb_append_char(SB *sb, char c);
void sb_append_cstr(SB *sb, const char *cstr);
void sb_append_sv(SB *sb, SV sv);
void sb_append_str(SB *sb, Str s);
void sb_append_fmt(SB *sb, const char *fmt, ...);

SV sb_sv(const SB *sb);

Str sb_build(SB *sb);

#define sb_append_one(sb, x) _Generic((x), \
    char: sb_append_char((sb), (x)),       \
    int: sb_append_char((sb), (char)(x)),  \
    SV: sb_append_sv((sb), (x)),           \
    Str: sb_append_str((sb), (x)),         \
    char *: sb_append_cstr((sb), (x)),     \
    const char *: sb_append_cstr((sb), (x)))

#define SB_A1(sb, a) sb_append_one(sb, a)
#define SB_A2(sb, a, ...) \
    sb_append_one(sb, a); \
    SB_A1(sb, __VA_ARGS__)
#define SB_A3(sb, a, ...) \
    sb_append_one(sb, a); \
    SB_A2(sb, __VA_ARGS__)
#define SB_A4(sb, a, ...) \
    sb_append_one(sb, a); \
    SB_A3(sb, __VA_ARGS__)
#define SB_A5(sb, a, ...) \
    sb_append_one(sb, a); \
    SB_A4(sb, __VA_ARGS__)
#define SB_A6(sb, a, ...) \
    sb_append_one(sb, a); \
    SB_A5(sb, __VA_ARGS__)
#define SB_A7(sb, a, ...) \
    sb_append_one(sb, a); \
    SB_A6(sb, __VA_ARGS__)
#define SB_A8(sb, a, ...) \
    sb_append_one(sb, a); \
    SB_A7(sb, __VA_ARGS__)

#define SB_NARGS_(_1, _2, _3, _4, _5, _6, _7, _8, N, ...) N
#define SB_NARGS(...) SB_NARGS_(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1)

#define SB_CONCAT_(a, b) a##b
#define SB_CONCAT(a, b) SB_CONCAT_(a, b)

#define sb_append(sb, ...)                                       \
    do                                                           \
    {                                                            \
        SB_CONCAT(SB_A, SB_NARGS(__VA_ARGS__))(sb, __VA_ARGS__); \
    } while (0)

#define SB_AUTO STRLIB_AUTO(sb_free)

#endif /* SB_H_ */