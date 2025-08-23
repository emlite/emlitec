#include <emcore/emcore.h>
#include <stdint.h>

EMLITE_USED int emlite_target(void) { return 1033; }

#if __has_include(<errno.h>)
#include <errno.h>
#else
extern _Thread_local int __errno_tls;
#define errno __errno_tls
#define ENOMEM 12
#endif

#if __has_include(<string.h>)
#include <string.h>
#else
size_t strlen(const char *s) {
    const char *p = s;
    while (*p)
        ++p;
    return (size_t)(p - s);
}
// needed for dlmalloc
void *memset(void *dest, int ch, size_t count) { return __builtin_memset(dest, ch, count); }
void *memcpy(void *dest, const void *src, size_t n) { return __builtin_memcpy(dest, src, n); }
#endif

#if __has_include(<stdlib.h>)
#include <stdlib.h>
EMLITE_USED void *emlite_malloc(size_t sz) { return malloc(sz); }
EMLITE_USED void *emlite_realloc(void *ptr, size_t sz) { return realloc(ptr, sz); }
EMLITE_USED void emlite_free(void *ptr) { free(ptr); }
#else
void abort(void) { __builtin_unreachable(); }

// If we don't have the above, we most likely don't have
// sbrk. If we link dlmalloc, it expects sbrk:
#define WASM_PAGESIZE 65536u

extern unsigned char __heap_base;
static uintptr_t heap_top = (uintptr_t)&__heap_base;

/* Round p up to the next multiple of n (n must be a
 * power-of-two). */
static inline uintptr_t align_up(uintptr_t p, uintptr_t n) { return (p + n - 1u) & ~(n - 1u); }
// sbrk code copied from wasi-libc implementation:
// https://github.com/WebAssembly/wasi-libc/blob/main/libc-bottom-half/sources/sbrk.c
// Licensed by wasi-libc under MIT, Apache and Apache-LLVM
void *sbrk(intptr_t increment) {
    // sbrk(0) returns the current memory size.
    if (increment == 0) {
        // The wasm spec doesn't guarantee that memory.grow
        // of 0 always succeeds.
        return (void *)(__builtin_wasm_memory_size(0) * WASM_PAGESIZE);
    }

    // We only support page-size increments.
    if (increment % WASM_PAGESIZE != 0) {
        abort();
    }

    // WebAssembly doesn't support shrinking linear memory.
    if (increment < 0) {
        abort();
    }

    uintptr_t old = __builtin_wasm_memory_grow(0, (uintptr_t)increment / WASM_PAGESIZE);

    if (old == SIZE_MAX) {
        errno = ENOMEM;
        return (void *)-1;
    }

    return (void *)(old * WASM_PAGESIZE);
}

void *emlite_malloc(size_t size) {
    uintptr_t aligned_top = align_up(heap_top, 8u);
    uintptr_t new_top     = aligned_top + (uintptr_t)size;

    uintptr_t cur_brk = (uintptr_t)sbrk(0);

    if (new_top > cur_brk) {
        uintptr_t diff      = new_top - cur_brk;
        uintptr_t increment = align_up(diff, WASM_PAGESIZE);
        if (sbrk((intptr_t)increment) == (void *)-1)
            return NULL;
    }
    heap_top = new_top;
    return (void *)aligned_top;
}
void emlite_free(void *ptr) { (void)ptr; }
void *emlite_realloc(void *old, size_t sz) {
    (void)old;
    return emlite_malloc(sz);
}

#ifndef HAVE_DLMALLOC
void *malloc(size_t s) { return emlite_malloc(s); }
void free(void *ptr) { emlite_free(ptr); }
void *realloc(void *ptr, size_t s) { return emlite_realloc(ptr, s); }
#endif
#endif

#if __has_include(<stdio.h>)
#include <stdio.h>
#else
size_t emlite_utoa(char *buf, unsigned long long value, int base, int upper) {
    static const char digits_low[] = "0123456789abcdef";
    static const char digits_up[]  = "0123456789ABCDEF";
    const char *digits             = upper ? digits_up : digits_low;

    size_t i = 0;
    if (value == 0) {
        buf[i++] = '0';
    } else {
        while (value) {
            buf[i++] = digits[value % base];
            value /= base;
        }

        for (size_t j = 0; j < i / 2; ++j) {
            char t         = buf[j];
            buf[j]         = buf[i - 1 - j];
            buf[i - 1 - j] = t;
        }
    }
    return i;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
    size_t pos = 0;

    while (*fmt) {
        if (*fmt != '%') {
            if (pos + 1 < n)
                out[pos] = *fmt;
            ++pos;
            ++fmt;
            continue;
        }

        ++fmt;

        if (*fmt == '%') {
            if (pos + 1 < n)
                out[pos] = '%';
            ++pos;
            ++fmt;
            continue;
        }

        int longflag = 0;
        while (*fmt == 'l') {
            ++longflag;
            ++fmt;
        }

        char tmp[32];
        const char *chunk = tmp;
        size_t chunk_len  = 0;
        int negative      = 0;

        switch (*fmt) {
        case 's': {
            const char *s = va_arg(ap, const char *);
            if (!s)
                s = "(null)";
            chunk     = s;
            chunk_len = strlen(s);
            break;
        }
        case 'c': {
            tmp[0]    = (char)va_arg(ap, int);
            chunk     = tmp;
            chunk_len = 1;
            break;
        }
        case 'd':
        case 'i': {
            long long v = longflag ? va_arg(ap, long long) : va_arg(ap, int);
            if (v < 0) {
                negative = 1;
                v        = -v;
            }
            chunk_len = emlite_utoa(tmp, (unsigned long long)v, 10, 0);
            break;
        }
        case 'u': {
            unsigned long long v =
                longflag ? va_arg(ap, unsigned long long) : va_arg(ap, unsigned int);
            chunk_len = emlite_utoa(tmp, v, 10, 0);
            break;
        }
        case 'x':
        case 'X': {
            unsigned long long v =
                longflag ? va_arg(ap, unsigned long long) : va_arg(ap, unsigned int);
            chunk_len = emlite_utoa(tmp, v, 16, (*fmt == 'X'));
            break;
        }
        default:
            tmp[0]    = '%';
            tmp[1]    = *fmt;
            chunk     = tmp;
            chunk_len = 2;
            break;
        }

        if (negative) {
            if (pos + 1 < n)
                out[pos] = '-';
            ++pos;
        }

        for (size_t i = 0; i < chunk_len; ++i) {
            if (pos + 1 < n)
                out[pos] = chunk[i];
            ++pos;
        }

        ++fmt;
    }

    if (n)
        out[(pos < n) ? pos : (n - 1)] = '\0';

    return (int)pos;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int r = vsnprintf(out, n, fmt, ap);
    va_end(ap);
    return r;
}
#endif