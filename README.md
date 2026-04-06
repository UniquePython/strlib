# strlib

Strlib provides a small, C11-based toolkit for handling string views, heap-allocated strings, and string builders in a consistent and ergonomic way. It exists to replace scattered, error-prone string handling in C with a unified, safer, and more convenient abstraction layer that still stays close to low-level control.

## Design Philosophy

The library uses a **three-layer ownership model** which provides a way to separate string handling into increasing levels of ownership and responsibility, so you can move between performance, safety, and convenience without mixing concerns.

### Layer 1: String View (SV) - borrowed data

**A non-owning reference:**
- pointer + length
- does NOT allocate
- does NOT free

```C
typedef struct {
    const char *data;
    size_t len;
} SV;
```

**Why it exists**

This layer is for **zero-cost string handling.**

It allows you to:

- inspect strings without copying
- pass substrings efficiently
- avoid allocations entirely

### Layer 2: String (Str) - owned (effectively immutable after construction) heap string

A heap-allocated string that owns its memory.

```C
typedef struct {
    char *data;
    size_t len;
} Str;
```

**Why it exists**

This is the **safe middle ground:**

- owns memory
- can be passed around safely
- can be freed cleanly
- works with SV seamlessly, i.e., `str_sv()` borrows a Str as an SV for free, and most functions accept both via `NEW_SV` (more on this later).

**Key idea**

Str is what you use when you want:

- persistence
- safety
- no manual buffer management

### Layer 3: String Builder (SB) - mutable construction tool

A growable buffer used to build strings efficiently.

```C
typedef struct {
    char *data;
    size_t len;
    size_t cap;
} SB;
```

**Why it exists**

This solves a performance problem:

> repeated string concatenation is expensive in C

So SB gives you:

- amortized O(1) appends
- no repeated realloc + copy chains
- efficient formatting/building

**The construction pipeline**

SB is also the only way to dynamically construct a `Str`. Once you're done building, `sb_build()` transfers ownership from the SB to a new `Str` and leaves the SB in a valid empty state - no copy, just a handoff. This makes the construction pipeline explicit:

> SB (build) -> Str (own) -> SV (borrow)

Data flows in one direction. You never go backwards.

### UTF-8 stance

Strings are treated as raw byte sequences (UTF-8 encoded) unless explicitly stated otherwise, and any operation that understands Unicode codepoints will be clearly separated and named. **All current operations are byte-level safe with UTF-8 already.**

### General Philosophy

This library is designed around three core principles:

**Ergonomics first**

The API prioritizes ease of use and readability while staying close to low-level C. Common operations should feel natural, concise, and predictable without unnecessary boilerplate.

**Explicit ownership**

Memory ownership is always clear. Types and functions make it obvious whether data is borrowed, owned, or transferred, avoiding hidden lifetime assumptions and reducing the risk of misuse.

**No hidden allocations**

Allocations never happen implicitly. Any operation that allocates memory does so explicitly through clearly named functions, ensuring performance characteristics are always visible and under the user’s control.

## Requirements

### C Standard

The library requires **C11 or above** to work. It is enforced at compile time by [compat.h](include/compat.h#L4-L8).

### Compiler

**GCC** and **Clang** fully supported. MSVC is NOT supported and errors at compile. [See Code](include/compat.h#L24-L27)

### Platform

[Makefile](Makefile) assumes Linux (hardcoded linux commands and directories). The library itself is platform-agnostic C11.

### Dependencies

None. Only libc.

## Building and Installing

### Requirements

- GCC or Clang
- `make`
- `ar` (binutils)

### Build
```sh
make
```

### Install
```sh
sudo make install VERSION=0.1.0
```

Installs headers to `/usr/local/include/strlib-<version>/`, the static library to `/usr/local/lib/libstrlib-<version>.a`, and a pkg-config file to `/usr/local/lib/pkgconfig/strlib.pc`. Stable symlinks at `/usr/local/include/strlib` and `/usr/local/lib/libstrlib.a` always point to the latest installed version.

Override the install prefix if needed:
```sh
sudo make install VERSION=0.1.0 PREFIX=/usr
```

### Uninstall
```sh
sudo make uninstall VERSION=0.1.0
```

Only removes the specified version. Symlinks are removed only if they point to that version.

### Release
```sh
make release VERSION=0.1.0
```

Checks for a clean working tree, installs, tags the commit as `v<version>`, and pushes to origin.

### Linking

With pkg-config:
```sh
gcc main.c -o app $(pkg-config --cflags --libs strlib)
```

Without pkg-config:
```sh
gcc main.c -o app -I/usr/local/include/strlib -L/usr/local/lib -lstrlib
```

## Quick Overview

| Module | Header | Purpose                                                                                                  |
| ------ | ------ | -------------------------------------------------------------------------------------------------------- |
| SV     | sv.h   | Non-owning pointer + length view into existing string data. No allocation, no lifetime.                  |
| Str    | str.h  | Heap-allocated, owned string. Use when data needs to outlive its source or be passed around safely.      |
| SB     | sb.h   | Growable buffer for constructing strings incrementally. Transfers ownership to a `Str` via `sb_build()`. |

`strlib.h` is a convenience umbrella header that conditionally includes `sv.h`, `str.h`, and `sb.h` in dependency order, allowing selective exclusion via `NO_SV`, `NO_STR`, and `NO_SB` compile-time flags.

## Reference

### SV

**Functions:**

`SV sv_from_cstr(const char *cstr)` — Creates a view into an existing C string. Does not copy. `cstr` must outlive the view.

`SV sv_from_parts(const char *data, size_t len)` — Creates a view from a raw pointer and explicit length. No validation, no null-termination required.

`bool sv_eq_sv(SV a, SV b)` — Byte-exact equality check. Not locale or encoding aware.

`int sv_cmp_sv(SV a, SV b)` — Lexicographical comparison. Returns negative, zero, or positive. Shorter string loses on equal prefix.

`bool sv_starts_with_sv(SV sv, SV prefix)` — Returns true if `sv` begins with `prefix`.

`bool sv_ends_with_sv(SV sv, SV suffix)` — Returns true if `sv` ends with `suffix`.

`size_t sv_find_sv(SV sv, SV needle)` — Returns index of first match, or `sv.len` if not found. Empty needle matches at 0.

`bool sv_contains_sv(SV sv, SV needle)` — Returns true if `needle` occurs anywhere in `sv`.

`SV sv_slice(SV sv, size_t start, size_t end)` — Returns a sub-view. Out-of-range or inverted indices are clamped silently.

`SV sv_trim_left(SV sv)` — Returns view with leading whitespace removed.

`SV sv_trim_right(SV sv)` — Returns view with trailing whitespace removed.

`SV sv_trim(SV sv)` — Returns view with both leading and trailing whitespace removed.

`SV sv_chop_by_delim(SV *sv, char delim)` — Returns everything before the first `delim` and advances `sv` past it. If not found, returns the full view and leaves `sv` empty.

**Macros:**

`NEW_SV(s)` — Converts `SV`, `char *`, or `const char *` into an `SV` at compile time via `_Generic`. No copy.

`sv_eq(a, b)`, `sv_cmp(a, b)`, `sv_starts_with(a, b)`, `sv_ends_with(a, b)`, `sv_contains(a, b)`, `sv_find(a, b)` — Type-coercing wrappers around their `_sv` counterparts. Accept any mix of `SV`, `char *`, or `const char *` as arguments.

`SV_FMT`, `SV_ARGS(sv)` — Use with printf-style functions to print an SV without null-termination:
```c
printf(SV_FMT "\n", SV_ARGS(my_sv));
```

### Str

**Functions:**

`Str str_from_cstr(const char *cstr)` — Allocates and copies a C string into a new `Str`. Caller owns the result.

`Str str_from_parts(const char *data, size_t len)` — Allocates and copies `len` bytes into a new `Str`. Always null-terminates.

`Str str_from_sv(SV sv)` — Allocates and copies an SV into a new `Str`.

`Str str_clone(Str s)` — Deep copies a `Str`. Caller owns the result.

`void str_free(Str *s)` — Frees the underlying buffer and zeroes the struct. Safe to call on an already-freed `Str`.

`SV str_sv(Str s)` — Borrows a `Str` as an `SV`. Free. The view is only valid for the lifetime of the `Str`.

`const char *str_cstr(Str s)` — Returns the underlying buffer as a null-terminated C string. Valid for the lifetime of the `Str`.

`Str str_concat_sv(SV a, SV b)` — Allocates a new `Str` containing `a` followed by `b`.

**Macros:**

`NEW_STR(s)` — Converts `Str`, `SV`, `char *`, or `const char *` into an owned `Str` via `_Generic`. Always allocates.

`str_concat(a, b)` — Concatenates any two string-like values into a new `Str`. Accepts any mix of `SV`, `Str`, `char *`, or `const char *`. Watch out for temporary `Str` arguments — they won't be freed automatically.

`STR_AUTO` — Marks a `Str` for automatic cleanup when it goes out of scope. **GCC/Clang only.**

### SB

**Functions:**

`SB sb_new(void)` — Creates an empty builder. Does not allocate until first append.

`SB sb_with_cap(size_t initial_cap)` — Creates a builder with a pre-allocated buffer. Use when final size is roughly known.

`void sb_free(SB *sb)` — Frees the buffer and zeroes the struct. Safe to call on an already-freed `SB`.

`void sb_reset(SB *sb)` — Resets length to zero without freeing. Use to reuse the allocation for a new string.

`void sb_append_char(SB *sb, char c)` — Appends a single character.

`void sb_append_cstr(SB *sb, const char *cstr)` — Appends a null-terminated C string.

`void sb_append_sv(SB *sb, SV sv)` — Appends a string view.

`void sb_append_str(SB *sb, Str s)` — Appends an owned string. Does not consume or free `s`.

`void sb_append_fmt(SB *sb, const char *fmt, ...)` — Appends a printf-style formatted string. Uses two-pass `vsnprintf` to avoid intermediate buffers.

`SV sb_sv(const SB *sb)` — Borrows the current contents as an SV. The view is invalidated by any subsequent append that triggers a realloc.

`Str sb_build(SB *sb)` — Transfers ownership of the buffer to a new `Str`, null-terminating it. Leaves the SB in a valid empty state. No copy.

**Macros:**

`sb_append(sb, ...)` — Appends up to 8 arguments of mixed types in one call. Accepts any combination of `char`, `int`, `SV`, `Str`, `char *`, and `const char *`. Dispatches per argument via `_Generic` at compile time. Maximum 8 arguments per call.

`SB_AUTO` — Marks an `SB` for automatic cleanup when it goes out of scope. **GCC/Clang only.**

## Compatibility

`compat.h` is included automatically via `sv.h` and handles all compiler and standard detection. It is not intended to be included directly.

### C Standard

C11 is required and enforced at compile time. Compiling with `-std=c99` or older will produce a hard error.

### Compiler Support

| Compiler | Status          | Notes                                 |
| -------- | --------------- | ------------------------------------- |
| GCC      | Fully supported |                                       |
| Clang    | Fully supported |                                       |
| MSVC     | Not supported   | Hard error at compile time            |
| Unknown  | Partial         | Compiles but loses cleanup and typeof |

### Feature Flags

| Flag                 | What it enables                                            | Lost on unknown compiler                 |
| -------------------- | ---------------------------------------------------------- | ---------------------------------------- |
| `STRLIB_HAS_CLEANUP` | `SB_AUTO`, `STR_AUTO`                                      | Both become no-ops, no automatic cleanup |
| `STRLIB_HAS_TYPEOF`  | Future use                                                 | Nothing currently                        |
| `STRLIB_HAS_GENERIC` | `sb_append`, `NEW_SV`, `NEW_STR`, all type-coercing macros | Guaranteed by C11, cannot be lost        |

### Portability Macros

`STRLIB_AUTO(fn)` — Expands to `__attribute__((cleanup(fn)))` on GCC/Clang, no-op otherwise. Emits a compile-time warning when unavailable.

`STRLIB_UNUSED` — Expands to `__attribute__((unused))` on GCC/Clang to suppress unused variable warnings.

`STRLIB_WARN(msg)` — Emits a compile-time warning via `_Pragma`. No-op on unknown compilers.

## Footguns

### `sb_sv` dangling pointer

`sb_sv()` returns a view into the builder's internal buffer. Any append that triggers a realloc invalidates it:
```c
SV view = sb_sv(&sb);
sb_append_cstr(&sb, "more");  // may realloc
// view.data is now potentially dangling
```

Only use `sb_sv()` when you are done appending, or when you can guarantee no realloc will occur.

### `str_concat` temporary leak

`str_concat` does not free its arguments. If you pass a temporary `Str` returned from a function, it leaks:
```c
Str s = str_concat(make_str(), make_str());  // both temporaries leak
```

Assign temporaries first and free them manually:
```c
Str a = make_str();
Str b = make_str();
Str s = str_concat(a, b);
str_free(&a);
str_free(&b);
```

### `SB_AUTO` / `STR_AUTO` are no-ops on unknown compilers

On compilers without `__attribute__((cleanup))`, both macros expand to nothing and no warning is emitted at the point of use — only at the point `compat.h` is processed. Variables marked `SB_AUTO` or `STR_AUTO` will not be freed automatically. You will leak memory silently.

## Versioning

strlib follows [Semantic Versioning](https://semver.org). Version numbers are `MAJOR.MINOR.PATCH`:

- `MAJOR` — breaking API changes
- `MINOR` — new features, backwards compatible
- `PATCH` — bug fixes, backwards compatible

However, strlib does not strictly abide by the above convention and may change version in non-standard ways.

The current version is defined in `strlib.h` as `STRLIB_VERSION`.

## Roadmap

strlib is currently at v0.1.0 and is not feature complete. The API will expand significantly before v1.0.0.

Planned areas (no fixed order or timeline):

- **Allocator abstraction** — pluggable allocator interface for arena, pool, and stack allocation
- **Expanded SV API** — split, count, parse primitives, ...
- **Expanded Str API** — slice, find, trim, case conversion, reverse, ...
- **Expanded SB API** — repeat, padding, ...
- **Number conversions** — string to integer/float and vice versa
- **UTF-8 awareness** — codepoint-aware variants of operations that require it, clearly separated from byte-level defaults
- **Parser primitives** — cursor-based API built on SV for writing hand-rolled parsers
- **String interning** — deduplication table for identifier-heavy use cases
- **Tests** — proper test suite before v1.0.0
- **`compat.h` expansion** — broader compiler support as needed

No guarantees on any of this until v1.0.0 is close.