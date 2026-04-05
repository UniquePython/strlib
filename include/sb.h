#ifndef SB_H_
#define SB_H_

#include <stddef.h>

typedef struct
{
    char *data;
    size_t len;
    size_t cap;
} SB;

#endif