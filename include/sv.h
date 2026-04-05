#ifndef SV_H_
#define SV_H_

#include <stddef.h>

typedef struct
{
    const char *data;
    size_t len;
} SV;

#endif