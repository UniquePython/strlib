#ifndef SV_H_
#define SV_H_

#include "compat.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    const char *data;
    size_t len;
} SV;

typedef struct
{
    SV *data;
    size_t len;
    size_t cap;
} SVArray;

SVArray sva_new(void);
void sva_push(SVArray *arr, SV sv);
SV sva_get(SVArray *arr, size_t i);
void sva_free(SVArray *arr);

#define NEW_SV(s) _Generic((s), \
    SV: (s),                    \
    char *: sv_from_cstr(s),    \
    const char *: sv_from_cstr(s))

SV sv_from_cstr(const char *cstr);
SV sv_from_parts(const char *data, size_t len);

bool sv_eq_sv(SV a, SV b);
int sv_cmp_sv(SV a, SV b);

bool sv_starts_with_sv(SV sv, SV prefix);
bool sv_ends_with_sv(SV sv, SV suffix);
size_t sv_find_sv(SV sv, SV needle);
bool sv_contains_sv(SV sv, SV needle);

SV sv_slice(SV sv, size_t start, size_t end);
SV sv_trim_left(SV sv);
SV sv_trim_right(SV sv);
SV sv_trim(SV sv);

SV sv_chop_by_delim(SV *sv, char delim);

bool sv_is_empty(SV sv);
bool sv_is_whitespace(SV sv);
bool sv_is_alpha(SV sv);
bool sv_is_numeric(SV sv);
bool sv_is_alphanumeric(SV sv);
bool sv_is_upper(SV sv);
bool sv_is_lower(SV sv);

size_t sv_count_sv(SV sv, SV needle, bool overlapping);

bool sv_parse_int(SV sv, int *out);
bool sv_parse_long(SV sv, long *out);
bool sv_parse_longlong(SV sv, long long *out);
bool sv_parse_int8(SV sv, int8_t *out);
bool sv_parse_int16(SV sv, int16_t *out);
bool sv_parse_int32(SV sv, int32_t *out);
bool sv_parse_int64(SV sv, int64_t *out);
bool sv_parse_uint8(SV sv, uint8_t *out);
bool sv_parse_uint16(SV sv, uint16_t *out);
bool sv_parse_uint32(SV sv, uint32_t *out);
bool sv_parse_uint64(SV sv, uint64_t *out);

SVArray sv_split_char(SV sv, char delim, size_t maxsplit);
SVArray sv_split_sv(SV sv, SV delim, size_t maxsplit);

#define sv_eq(a, b) sv_eq_sv(NEW_SV(a), NEW_SV(b))
#define sv_cmp(a, b) sv_cmp_sv(NEW_SV(a), NEW_SV(b))

#define sv_starts_with(a, b) sv_starts_with_sv(NEW_SV(a), NEW_SV(b))
#define sv_ends_with(a, b) sv_ends_with_sv(NEW_SV(a), NEW_SV(b))
#define sv_contains(a, b) sv_contains_sv(NEW_SV(a), NEW_SV(b))
#define sv_find(a, b) sv_find_sv(NEW_SV(a), NEW_SV(b))

#define sv_count(a, b) sv_count_sv(NEW_SV(a), NEW_SV(b), false)
#define sv_count_overlapping(a, b) sv_count_sv(NEW_SV(a), NEW_SV(b), true)

#define sv_split(sv, delim, maxsplit) _Generic((delim),   \
    char: sv_split_char((sv), (delim), (maxsplit)),       \
    SV: sv_split_sv((sv), (delim), (maxsplit)),           \
    char *: sv_split_sv((sv), NEW_SV(delim), (maxsplit)), \
    const char *: sv_split_sv((sv), NEW_SV(delim), (maxsplit)))

#define SV_FMT "%.*s"
#define SV_ARGS(sv) (int)(sv).len, (sv).data

#endif /* SV_H_ */