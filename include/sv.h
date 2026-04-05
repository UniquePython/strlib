#ifndef SV_H_
#define SV_H_

#include <stddef.h>
#include <stdbool.h>

typedef struct
{
    const char *data;
    size_t len;
} SV;

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
bool sv_contains_sv(SV sv, SV needle);
size_t sv_find_sv(SV sv, SV needle);

SV sv_slice(SV sv, size_t start, size_t end);
SV sv_trim_left(SV sv);
SV sv_trim_right(SV sv);
SV sv_trim(SV sv);

SV sv_chop_by_delim(SV *sv, char delim);

#define sv_eq(a, b) sv_eq_sv(NEW_SV(a), NEW_SV(b))
#define sv_cmp(a, b) sv_cmp_sv(NEW_SV(a), NEW_SV(b))

#define sv_starts_with(a, b) sv_starts_with_sv(NEW_SV(a), NEW_SV(b))
#define sv_ends_with(a, b) sv_ends_with_sv(NEW_SV(a), NEW_SV(b))
#define sv_contains(a, b) sv_contains_sv(NEW_SV(a), NEW_SV(b))
#define sv_find(a, b) sv_find_sv(NEW_SV(a), NEW_SV(b))

#define SV_FMT "%.*s"
#define SV_ARGS(sv) (int)(sv).len, (sv).data

#endif /* SV_H_ */