#include "sb.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define SB_INIT_CAP 64

static void *xmalloc(size_t n)
{
    void *p = malloc(n);
    if (!p)
    {
        fputs("strlib::sb: out of memory\n", stderr);
        abort();
    }
    return p;
}

static void *xrealloc(void *p, size_t n)
{
    p = realloc(p, n);
    if (!p)
    {
        fputs("strlib::sb: out of memory\n", stderr);
        abort();
    }
    return p;
}

static void sb_grow(SB *sb, size_t needed)
{
    if (sb->len + needed <= sb->cap)
        return;

    size_t new_cap = sb->cap ? sb->cap * 2 : SB_INIT_CAP;
    while (new_cap < sb->len + needed)
        new_cap *= 2;

    sb->data = xrealloc(sb->data, new_cap);
    sb->cap = new_cap;
}

SB sb_new(void)
{
    return (SB){.data = NULL, .len = 0, .cap = 0};
}

SB sb_with_cap(size_t initial_cap)
{
    SB sb = sb_new();
    sb.data = xmalloc(initial_cap);
    sb.cap = initial_cap;
    return sb;
}

void sb_free(SB *sb)
{
    if (!sb)
        return;
    free(sb->data);
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;
}

void sb_reset(SB *sb)
{
    sb->len = 0;
}

void sb_append_char(SB *sb, char c)
{
    sb_grow(sb, 1);
    sb->data[sb->len++] = c;
}

void sb_append_sv(SB *sb, SV sv)
{
    if (sv.len == 0)
        return;
    sb_grow(sb, sv.len);
    memcpy(sb->data + sb->len, sv.data, sv.len);
    sb->len += sv.len;
}

void sb_append_cstr(SB *sb, const char *cstr)
{
    sb_append_sv(sb, sv_from_cstr(cstr));
}

void sb_append_str(SB *sb, Str s)
{
    sb_append_sv(sb, str_sv(s));
}

void sb_append_fmt(SB *sb, const char *fmt, ...)
{
    va_list args;

    /* First pass: measure */
    va_start(args, fmt);
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed <= 0)
        return;

    sb_grow(sb, (size_t)needed + 1);

    /* Second pass: write */
    va_start(args, fmt);
    vsnprintf(sb->data + sb->len, (size_t)needed + 1, fmt, args);
    va_end(args);

    sb->len += (size_t)needed;
}

SV sb_sv(const SB *sb)
{
    return (SV){.data = sb->data, .len = sb->len};
}

Str sb_build(SB *sb)
{
    /* Null-terminate and hand ownership to Str */
    sb_grow(sb, 1);
    sb->data[sb->len] = '\0';

    Str s = {.data = sb->data, .len = sb->len};

    /* Leave sb in a valid-but-empty state */
    sb->data = NULL;
    sb->len = 0;
    sb->cap = 0;

    return s;
}