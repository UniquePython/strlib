#include "str.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static void *xmalloc(size_t n)
{
    void *p = malloc(n);
    if (!p)
    {
        fputs("strlib::str: out of memory\n", stderr);
        abort();
    }
    return p;
}

Str str_from_parts(const char *data, size_t len)
{
    char *buf = xmalloc(len + 1);
    memcpy(buf, data, len);
    buf[len] = '\0';
    return (Str){.data = buf, .len = len};
}

Str str_from_cstr(const char *cstr)
{
    return str_from_parts(cstr, strlen(cstr));
}

Str str_from_sv(SV sv)
{
    return str_from_parts(sv.data, sv.len);
}

Str str_clone(Str s)
{
    return str_from_parts(s.data, s.len);
}

void str_free(Str *s)
{
    if (!s)
        return;
    free(s->data);
    s->data = NULL;
    s->len = 0;
}

const char *str_cstr(Str s)
{
    return s.data; /* always null-terminated */
}

Str str_concat_sv(SV a, SV b)
{
    size_t total = a.len + b.len;
    char *buf = xmalloc(total + 1);
    memcpy(buf, a.data, a.len);
    memcpy(buf + a.len, b.data, b.len);
    buf[total] = '\0';
    return (Str){.data = buf, .len = total};
}