#ifndef STRLIB_H_
#define STRLIB_H_

#if !defined(NO_STR) && defined(NO_SV)
#error "STR requires SV. Do not define NO_SV when using STR."
#endif

#if !defined(NO_SB) && defined(NO_STR)
#error "SB requires STR. Do not define NO_STR when using SB."
#endif

#if !defined(NO_SB) && defined(NO_SV)
#error "SB requires SV. Do not define NO_SV when using SB."
#endif

#ifndef NO_SV
#include "sv.h"
#endif

#if !defined(NO_STR) && !defined(NO_SV)
#include "str.h"
#endif

#if !defined(NO_SB) && !defined(NO_STR) && !defined(NO_SV)
#include "sb.h"
#endif

#endif /* STRLIB_H_ */