#ifndef STRV_H_
#define STRV_H_

#include <stddef.h>

typedef struct
{
    const char *data;
    size_t len;
} SV;

#endif