#include "sv.h"

#include <string.h>
#include <ctype.h>

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

bool sv_eq_cstr(SV sv, const char *cstr)
{
    return sv_eq_sv(sv, sv_from_cstr(cstr));
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