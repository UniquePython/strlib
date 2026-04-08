#include "sv.h"

#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>

#define SVA_INIT_CAP 8

SVArray sva_new(void)
{
    return (SVArray){.data = NULL, .len = 0, .cap = 0};
}

void sva_push(SVArray *arr, SV sv)
{
    if (arr->len >= arr->cap)
    {
        size_t new_cap = arr->cap ? arr->cap * 2 : SVA_INIT_CAP;
        arr->data = realloc(arr->data, new_cap * sizeof(SV));
        if (!arr->data)
        {
            fputs("strlib::sv: out of memory\n", stderr);
            abort();
        }
        arr->cap = new_cap;
    }
    arr->data[arr->len++] = sv;
}

SV sva_get(SVArray *arr, size_t i)
{
    if (i >= arr->len)
    {
        fprintf(stderr, "strlib::sv: index %zu out of bounds (len %zu)\n", i, arr->len);
        abort();
    }
    return arr->data[i];
}

void sva_free(SVArray *arr)
{
    if (!arr)
        return;
    free(arr->data);
    arr->data = NULL;
    arr->len = 0;
    arr->cap = 0;
}

// ---- SV ------------>

SV sv_from_cstr(const char *cstr)
{
    return (SV){.data = cstr, .len = strlen(cstr)};
}

SV sv_from_parts(const char *data, size_t len)
{
    return (SV){.data = data, .len = len};
}

bool sv_eq_sv(SV a, SV b)
{
    return a.len == b.len && memcmp(a.data, b.data, a.len) == 0;
}

int sv_cmp_sv(SV a, SV b)
{
    size_t min_len = a.len < b.len ? a.len : b.len;
    int r = memcmp(a.data, b.data, min_len);
    if (r != 0)
        return r;
    if (a.len < b.len)
        return -1;
    if (a.len > b.len)
        return 1;
    return 0;
}

bool sv_starts_with_sv(SV sv, SV prefix)
{
    if (prefix.len > sv.len)
        return false;
    return memcmp(sv.data, prefix.data, prefix.len) == 0;
}

bool sv_ends_with_sv(SV sv, SV suffix)
{
    if (suffix.len > sv.len)
        return false;
    return memcmp(sv.data + sv.len - suffix.len, suffix.data, suffix.len) == 0;
}

size_t sv_find_sv(SV sv, SV needle)
{
    if (needle.len == 0)
        return 0;
    if (needle.len > sv.len)
        return sv.len;

    for (size_t i = 0; i <= sv.len - needle.len; i++)
    {
        if (memcmp(sv.data + i, needle.data, needle.len) == 0)
            return i;
    }
    return sv.len;
}

bool sv_contains_sv(SV sv, SV needle)
{
    return sv_find_sv(sv, needle) < sv.len;
}

SV sv_slice(SV sv, size_t start, size_t end)
{
    if (start > sv.len)
        start = sv.len;
    if (end > sv.len)
        end = sv.len;
    if (end < start)
        end = start;
    return (SV){.data = sv.data + start, .len = end - start};
}

SV sv_trim_left(SV sv)
{
    while (sv.len > 0 && isspace((unsigned char)*sv.data))
    {
        sv.data++;
        sv.len--;
    }
    return sv;
}

SV sv_trim_right(SV sv)
{
    while (sv.len > 0 && isspace((unsigned char)sv.data[sv.len - 1]))
        sv.len--;
    return sv;
}

SV sv_trim(SV sv)
{
    return sv_trim_left(sv_trim_right(sv));
}

SV sv_chop_by_delim(SV *sv, char delim)
{
    size_t i = 0;
    while (i < sv->len && sv->data[i] != delim)
        i++;

    SV chopped = {.data = sv->data, .len = i};

    if (i < sv->len)
    {
        /* skip past the delimiter */
        sv->data += i + 1;
        sv->len -= i + 1;
    }
    else
    {
        sv->data += i;
        sv->len = 0;
    }

    return chopped;
}

bool sv_is_empty(SV sv)
{
    return sv.len == 0;
}

bool sv_is_whitespace(SV sv)
{
    for (size_t i = 0; i < sv.len; i++)
        if (!isspace((unsigned char)sv.data[i]))
            return false;
    return sv.len > 0;
}

bool sv_is_alpha(SV sv)
{
    if (sv.len == 0)
        return false;
    for (size_t i = 0; i < sv.len; i++)
        if (!isalpha((unsigned char)sv.data[i]))
            return false;
    return true;
}

bool sv_is_numeric(SV sv)
{
    if (sv.len == 0)
        return false;
    for (size_t i = 0; i < sv.len; i++)
        if (!isdigit((unsigned char)sv.data[i]))
            return false;
    return true;
}

bool sv_is_alphanumeric(SV sv)
{
    if (sv.len == 0)
        return false;
    for (size_t i = 0; i < sv.len; i++)
        if (!isalnum((unsigned char)sv.data[i]))
            return false;
    return true;
}

bool sv_is_upper(SV sv)
{
    if (sv.len == 0)
        return false;
    for (size_t i = 0; i < sv.len; i++)
        if (!isupper((unsigned char)sv.data[i]))
            return false;
    return true;
}

bool sv_is_lower(SV sv)
{
    if (sv.len == 0)
        return false;
    for (size_t i = 0; i < sv.len; i++)
        if (!islower((unsigned char)sv.data[i]))
            return false;
    return true;
}

size_t sv_count_sv(SV sv, SV needle, bool overlapping)
{
    if (needle.len == 0 || needle.len > sv.len)
        return 0;

    size_t count = 0;
    size_t i = 0;
    size_t step = overlapping ? 1 : needle.len;

    while (i <= sv.len - needle.len)
    {
        if (memcmp(sv.data + i, needle.data, needle.len) == 0)
        {
            count++;
            i += step;
        }
        else
            i++;
    }

    return count;
}

static bool parse_longlong(SV sv, long long *out)
{
    if (sv.len == 0 || sv.len >= 32)
        return false;

    char buf[32];
    memcpy(buf, sv.data, sv.len);
    buf[sv.len] = '\0';

    char *end;
    errno = 0;
    long long val = strtoll(buf, &end, 10);

    if (end != buf + sv.len || errno != 0)
        return false;

    if (out)
        *out = val;
    return true;
}

static bool parse_ulonglong(SV sv, unsigned long long *out)
{
    if (sv.len == 0 || sv.len >= 32)
        return false;

    char buf[32];
    memcpy(buf, sv.data, sv.len);
    buf[sv.len] = '\0';

    char *end;
    errno = 0;
    unsigned long long val = strtoull(buf, &end, 10);

    if (end != buf + sv.len || errno != 0)
        return false;

    if (out)
        *out = val;
    return true;
}

static bool parse_double(SV sv, double *out)
{
    if (sv.len == 0 || sv.len >= 64)
        return false;

    char buf[64];
    memcpy(buf, sv.data, sv.len);
    buf[sv.len] = '\0';

    char *end;
    errno = 0;
    double val = strtod(buf, &end);

    if (end != buf + sv.len || errno != 0)
        return false;

    if (out)
        *out = val;
    return true;
}

#define PARSE_SIGNED(sv, out, min, max)      \
    long long val;                           \
    if (!parse_longlong(sv, &val))           \
        return false;                        \
    if (val < (min) || val > (max))          \
        return false;                        \
    if (out)                                 \
        *(out) = (STRLIB_TYPEOF(*(out)))val; \
    return true;

#define PARSE_UNSIGNED(sv, out, max)         \
    unsigned long long val;                  \
    if (!parse_ulonglong(sv, &val))          \
        return false;                        \
    if (val > (max))                         \
        return false;                        \
    if (out)                                 \
        *(out) = (STRLIB_TYPEOF(*(out)))val; \
    return true;

bool sv_parse_int(SV sv, int *out) { PARSE_SIGNED(sv, out, INT_MIN, INT_MAX) }
bool sv_parse_long(SV sv, long *out) { PARSE_SIGNED(sv, out, LONG_MIN, LONG_MAX) }
bool sv_parse_longlong(SV sv, long long *out) { PARSE_SIGNED(sv, out, LLONG_MIN, LLONG_MAX) }
bool sv_parse_int8(SV sv, int8_t *out) { PARSE_SIGNED(sv, out, INT8_MIN, INT8_MAX) }
bool sv_parse_int16(SV sv, int16_t *out) { PARSE_SIGNED(sv, out, INT16_MIN, INT16_MAX) }
bool sv_parse_int32(SV sv, int32_t *out) { PARSE_SIGNED(sv, out, INT32_MIN, INT32_MAX) }
bool sv_parse_int64(SV sv, int64_t *out) { PARSE_SIGNED(sv, out, INT64_MIN, INT64_MAX) }

bool sv_parse_uint8(SV sv, uint8_t *out) { PARSE_UNSIGNED(sv, out, UINT8_MAX) }
bool sv_parse_uint16(SV sv, uint16_t *out) { PARSE_UNSIGNED(sv, out, UINT16_MAX) }
bool sv_parse_uint32(SV sv, uint32_t *out) { PARSE_UNSIGNED(sv, out, UINT32_MAX) }
bool sv_parse_uint64(SV sv, uint64_t *out) { PARSE_UNSIGNED(sv, out, UINT64_MAX) }

bool sv_parse_float(SV sv, float *out)
{
    double val;
    if (!parse_double(sv, &val))
        return false;
    if (val < -FLT_MAX || val > FLT_MAX)
        return false;
    if (out)
        *out = (float)val;
    return true;
}

bool sv_parse_double(SV sv, double *out) { return parse_double(sv, out); }

SVArray sv_split_char(SV sv, char delim, size_t maxsplit)
{
    SVArray arr = sva_new();
    size_t splits = 0;

    while (sv.len > 0)
    {
        if (maxsplit != 0 && splits >= maxsplit)
        {
            sva_push(&arr, sv);
            return arr;
        }

        size_t i = 0;
        while (i < sv.len && sv.data[i] != delim)
            i++;

        sva_push(&arr, sv_from_parts(sv.data, i));
        splits++;

        if (i < sv.len)
        {
            sv.data += i + 1;
            sv.len -= i + 1;
        }
        else
            break;
    }

    return arr;
}

SVArray sv_split_sv(SV sv, SV delim, size_t maxsplit)
{
    SVArray arr = sva_new();

    if (delim.len == 0)
    {
        sva_push(&arr, sv);
        return arr;
    }

    size_t splits = 0;

    while (sv.len > 0)
    {
        if (maxsplit != 0 && splits >= maxsplit)
        {
            sva_push(&arr, sv);
            return arr;
        }

        size_t i = sv_find_sv(sv, delim);

        sva_push(&arr, sv_from_parts(sv.data, i));
        splits++;

        if (i < sv.len)
        {
            sv.data += i + delim.len;
            sv.len -= i + delim.len;
        }
        else
            break;
    }

    return arr;
}