#ifndef STR_H_
#define STR_H_

#include <stddef.h>

typedef struct
{
    char *data;
    size_t len;
} Str;

#endif