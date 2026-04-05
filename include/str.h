#ifndef STR_H_
#define STR_H_

#include <stddef.h>
#include "sv.h"

typedef struct
{
    char *data;
    size_t len;
} Str;

Str str_from_cstr(const char *cstr);
Str str_from_parts(const char *data, size_t len);
Str str_from_sv(SV sv);
Str str_clone(Str s);

void str_free(Str *s);

static inline SV str_sv(Str s)
{
    return (SV){.data = s.data, .len = s.len};
}

const char *str_cstr(Str s);

Str str_concat_sv(SV a, SV b);

#undef NEW_SV
#define NEW_SV(s) _Generic((s), \
    SV: (s),                    \
    Str: str_sv(s),             \
    char *: sv_from_cstr(s),    \
    const char *: sv_from_cstr(s))

#define NEW_STR(s) _Generic((s), \
    Str: str_clone(s),           \
    SV: str_from_sv(s),          \
    char *: str_from_cstr(s),    \
    const char *: str_from_cstr(s))

#define str_concat(a, b) str_concat_sv(NEW_SV(a), NEW_SV(b))

#define STR_AUTO STRLIB_AUTO(str_free)

#endif /* STR_H_ */