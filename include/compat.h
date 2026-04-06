#ifndef STRLIB_COMPAT_H_
#define STRLIB_COMPAT_H_

/* ── Standard detection ── */
#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "strlib requires C11 or later (-std=c11)"
#endif

/* ── Compiler detection ── */
#if defined(__clang__)
#define STRLIB_COMPILER_CLANG
#define STRLIB_COMPILER_NAME "clang"
#elif defined(__GNUC__)
#define STRLIB_COMPILER_GCC
#define STRLIB_COMPILER_NAME "gcc"
#elif defined(_MSC_VER)
#define STRLIB_COMPILER_MSVC
#define STRLIB_COMPILER_NAME "msvc"
#else
#define STRLIB_COMPILER_UNKNOWN
#define STRLIB_COMPILER_NAME "unknown"
#endif

#ifdef STRLIB_COMPILER_MSVC
#error "MSVC not supported yet"
#endif

/* ── Compile-time warnings ── */
#define STRLIB_STRINGIFY_(x) #x
#define STRLIB_STRINGIFY(x) STRLIB_STRINGIFY_(x)

#if defined(STRLIB_COMPILER_CLANG)
#define STRLIB_WARN(msg) _Pragma(STRLIB_STRINGIFY(clang warning msg))
#elif defined(STRLIB_COMPILER_GCC)
#define STRLIB_WARN(msg) _Pragma(STRLIB_STRINGIFY(GCC warning msg))
#elif defined(STRLIB_COMPILER_MSVC)
#define STRLIB_WARN(msg) __pragma(message(msg))
#else
#define STRLIB_WARN(msg)
#endif

/* ── Feature flags ── */
#if defined(STRLIB_COMPILER_GCC) || defined(STRLIB_COMPILER_CLANG)
#define STRLIB_HAS_CLEANUP
#define STRLIB_HAS_TYPEOF
#endif

/* _Generic is C11, already guaranteed by standard check above */
#define STRLIB_HAS_GENERIC

/* ── Portability macros ── */
#if defined(STRLIB_HAS_CLEANUP)
#define STRLIB_AUTO(fn) __attribute__((cleanup(fn)))
#else
STRLIB_WARN("SB_AUTO/STR_AUTO unavailable: no cleanup attribute")
#define STRLIB_AUTO(fn)
#endif

#if defined(STRLIB_COMPILER_GCC) || defined(STRLIB_COMPILER_CLANG)
#define STRLIB_UNUSED __attribute__((unused))
#elif defined(STRLIB_COMPILER_MSVC)
#define STRLIB_UNUSED
#else
#define STRLIB_UNUSED
#endif

#endif /* STRLIB_COMPAT_H_ */