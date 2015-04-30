/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/

/**********************************************************\
 *                                                        *
 * hprose_bytes_io.h                                      *
 *                                                        *
 * hprose bytes io for pecl header file.                  *
 *                                                        *
 * LastModified: Apr 9, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_BYTES_IO_H
#define	HPROSE_BYTES_IO_H

#include "hprose_common.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_bytes_io_ce();

HPROSE_STARTUP_FUNCTION(bytes_io);

#if PHP_MAJOR_VERSION < 7

typedef struct {
    char * buf;
    int32_t len;
    int32_t cap;
    int32_t pos;
    zend_bool persistent;
} hprose_bytes_io;

#define HB_STR(b) ((b).buf)
#define HB_BUF(b) ((b).buf)
#define HB_LEN(b) ((b).len)

#define HB_STR_P(b) ((b)->buf)
#define HB_BUF_P(b) ((b)->buf)
#define HB_LEN_P(b) ((b)->len)

#else

typedef struct {
    zend_string *s;
    int32_t cap;
    int32_t pos;
    zend_bool persistent;
} hprose_bytes_io;

#define HB_STR(b) ((b).s)
#define HB_BUF(b) ((b).s->val)
#define HB_LEN(b) ((b).s->len)

#define HB_STR_P(b) ((b)->s)
#define HB_BUF_P(b) ((b)->s->val)
#define HB_LEN_P(b) ((b)->s->len)

#endif

#define HB_INITED(b) (HB_STR(b) != NULL)
#define HB_CAP(b) ((b).cap)
#define HB_POS(b) ((b).pos)
#define HB_PERSISTENT(b) ((b).persistent)

#define HB_INITED_P(b) (HB_STR_P(b) != NULL)
#define HB_CAP_P(b) ((b)->cap)
#define HB_POS_P(b) ((b)->pos)
#define HB_PERSISTENT_P(b) ((b)->persistent)

#ifndef HPROSE_BYTES_IO_PREALLOC
#define HPROSE_BYTES_IO_PREALLOC 64
#endif

#ifndef HPROSE_INT32_MIN_STR
#define HPROSE_INT32_MIN_STR "-2147483648"
#endif

#ifndef HPROSE_INT64_MIN_STR
#define HPROSE_INT64_MIN_STR "-9223372036854775808"
#endif

static zend_always_inline int32_t _hprose_pow2roundup(int32_t x) {
#if defined(__GNUC__)
    return 0x2 << (__builtin_clz(x - 1) ^ 0x1f);
#else
    x -= 1;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x + 1;
#endif
}

static zend_always_inline void _hprose_bytes_io_grow(hprose_bytes_io *_this, int32_t n) {
    register int32_t size;
    if (HB_INITED_P(_this)) {
        register int32_t len = HB_LEN_P(_this);
        size = len + n + 1;
        if (size > HB_CAP_P(_this)) {
            size = _hprose_pow2roundup(size);
#if PHP_MAJOR_VERSION < 7
            HB_STR_P(_this) = perealloc(HB_STR_P(_this), size, HB_PERSISTENT_P(_this));
#else
            HB_STR_P(_this) = zend_string_realloc(HB_STR_P(_this), size, HB_PERSISTENT_P(_this));
            HB_LEN_P(_this) = len;
#endif
            HB_BUF_P(_this)[len] = '\0';
            HB_CAP_P(_this) = size;
        }
    }
    else {
        size = _hprose_pow2roundup(n + 1);
        HB_CAP_P(_this) = (size > HPROSE_BYTES_IO_PREALLOC) ? size : HPROSE_BYTES_IO_PREALLOC;
#if PHP_MAJOR_VERSION < 7
        HB_STR_P(_this) = pemalloc(HB_CAP_P(_this), HB_PERSISTENT_P(_this));
#else
        HB_STR_P(_this) = zend_string_alloc(HB_CAP_P(_this), HB_PERSISTENT_P(_this));
#endif
        HB_LEN_P(_this) = 0;
        HB_POS_P(_this) = 0;
        HB_BUF_P(_this)[0] = '\0';
    }
}

static zend_always_inline void hprose_bytes_io_pinit(hprose_bytes_io *_this, const char *buf, int32_t len, zend_bool persistent) {
    if (buf) {
#if PHP_MAJOR_VERSION < 7
        HB_STR_P(_this) = pestrndup(buf, len, persistent);
        HB_LEN_P(_this) = len;
#else
        HB_STR_P(_this) = zend_string_init(buf, len, persistent);
#endif
        HB_CAP_P(_this) = len;
    }
    else {
#if PHP_MAJOR_VERSION < 7
        HB_STR_P(_this) = NULL;
        HB_LEN_P(_this) = 0;
#else
        HB_STR_P(_this) = NULL;
#endif
        HB_CAP_P(_this) = 0;
    }
    HB_POS_P(_this) = 0;
    HB_PERSISTENT_P(_this) = persistent;
}

#define hprose_bytes_io_init(_this, buf, len) hprose_bytes_io_pinit(_this, buf, len, 0)

static zend_always_inline hprose_bytes_io *hprose_bytes_io_pnew(zend_bool persistent) {
    hprose_bytes_io *_this = pecalloc(1, sizeof(hprose_bytes_io), persistent);
    hprose_bytes_io_pinit(_this, NULL, 0, persistent);
    return _this;
}

#define hprose_bytes_io_new() hprose_bytes_io_pnew(0)

static zend_always_inline hprose_bytes_io *hprose_bytes_io_pcreate(const char *buf, int32_t len, zend_bool persistent) {
    hprose_bytes_io *_this = pecalloc(1, sizeof(hprose_bytes_io), persistent);
    hprose_bytes_io_pinit(_this, buf, len, persistent);
    return _this;
}

#define hprose_bytes_io_create(buf, len) hprose_bytes_io_pcreate((buf), (len), 0)

#if PHP_MAJOR_VERSION < 7

static zend_always_inline void hprose_bytes_io_init_readonly(hprose_bytes_io *_this, const char *buf, int32_t len) {
    HB_STR_P(_this) = (char *)buf;
    HB_LEN_P(_this) = len;
    HB_CAP_P(_this) = len;
    HB_POS_P(_this) = 0;
    HB_PERSISTENT_P(_this) = 0;
}

/* only for read, don't call hprose_bytes_io_close or hprose_bytes_io_free on it, using efree to free it. */
static zend_always_inline hprose_bytes_io *hprose_bytes_io_create_readonly(const char *buf, int32_t len) {
    hprose_bytes_io *_this = emalloc(sizeof(hprose_bytes_io));
    hprose_bytes_io_init_readonly(_this, buf, len);
    return _this;
}

static zend_always_inline void hprose_bytes_io_close(hprose_bytes_io *_this) {
    if (HB_INITED_P(_this)) {
        pefree(HB_STR_P(_this), HB_PERSISTENT_P(_this));
        HB_STR_P(_this) = NULL;
    }
    HB_LEN_P(_this) = 0;
    HB_CAP_P(_this) = 0;
    HB_POS_P(_this) = 0;
}

#else

static zend_always_inline void hprose_bytes_io_init_readonly(hprose_bytes_io *_this, zend_string *s) {
    HB_STR_P(_this) = s;
    HB_CAP_P(_this) = HB_LEN_P(_this);
    HB_POS_P(_this) = 0;
    HB_PERSISTENT_P(_this) = 0;
}

/* only for read, don't call hprose_bytes_io_close or hprose_bytes_io_free on it, using efree to free it. */
static zend_always_inline hprose_bytes_io *hprose_bytes_io_create_readonly(zend_string *s) {
    hprose_bytes_io *_this = emalloc(sizeof(hprose_bytes_io));
    hprose_bytes_io_init_readonly(_this, s);
    return _this;
}

static zend_always_inline void hprose_bytes_io_close(hprose_bytes_io *_this) {
    if (HB_INITED_P(_this)) {
        zend_string_release(HB_STR_P(_this));
        HB_STR_P(_this) = NULL;
    }
    HB_CAP_P(_this) = 0;
    HB_POS_P(_this) = 0;
}

#endif

static zend_always_inline void hprose_bytes_io_free(hprose_bytes_io *_this) {
    hprose_bytes_io_close(_this);
    pefree(_this, HB_PERSISTENT_P(_this));
}

static zend_always_inline char hprose_bytes_io_getc(hprose_bytes_io *_this) {
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) < HB_LEN_P(_this));
    return HB_BUF_P(_this)[HB_POS_P(_this)++];
}

#if PHP_MAJOR_VERSION < 7

static zend_always_inline char *hprose_bytes_io_pread(hprose_bytes_io *_this, int32_t n, zend_bool persistent) {
    char *s;
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) + n <= HB_LEN_P(_this));
    s = pestrndup(HB_BUF_P(_this) + HB_POS_P(_this), n, persistent);
    HB_POS_P(_this) += n;
    return s;
}

static zend_always_inline char * hprose_bytes_io_preadfull(hprose_bytes_io *_this, int32_t *len_ptr, zend_bool persistent) {
    char *s;
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) <= HB_LEN_P(_this));
    assert(len_ptr);
    *len_ptr = HB_LEN_P(_this) - HB_POS_P(_this);
    s = pestrndup(HB_BUF_P(_this) + HB_POS_P(_this), *len_ptr, persistent);
    HB_POS_P(_this) = HB_LEN_P(_this);
    return s;
}

static zend_always_inline char * hprose_bytes_io_preaduntil_ex(hprose_bytes_io *_this, char tag, int32_t *len_ptr, zend_bool persistent, zend_bool skiptag) {
    char *s;
    int32_t i = HB_POS_P(_this), n = HB_LEN_P(_this), p = HB_LEN_P(_this);
    assert(HB_INITED_P(_this));
    for (; i < n; ++i) {
        if (HB_BUF_P(_this)[i] == tag) {
            p = i;
            break;
        }
    }
    *len_ptr = p - HB_POS_P(_this);
    s = pestrndup(HB_BUF_P(_this) + HB_POS_P(_this), *len_ptr, persistent);
    HB_POS_P(_this) = p;
    if (HB_POS_P(_this) < HB_LEN_P(_this) && skiptag) {
        HB_POS_P(_this)++;
    }
    return s;
}

#define hprose_bytes_io_readfull(_this, len_ptr) hprose_bytes_io_preadfull((_this), (len_ptr), 0)

#define hprose_bytes_io_readuntil_ex(_this, tag, len_ptr, skiptag) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), 0, (skiptag))
#define hprose_bytes_io_preaduntil(_this, tag, len_ptr, persistent) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), (persistent), 1)
#define hprose_bytes_io_readuntil(_this, tag, len_ptr) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), 0, 1)

#else

static zend_always_inline zend_string *hprose_bytes_io_pread(hprose_bytes_io *_this, int32_t n, zend_bool persistent) {
    zend_string *s;
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) + n <= HB_LEN_P(_this));
    s = zend_string_init(HB_BUF_P(_this) + HB_POS_P(_this), n, persistent);
    HB_POS_P(_this) += n;
    return s;
}

static zend_always_inline zend_string *hprose_bytes_io_preadfull(hprose_bytes_io *_this, zend_bool persistent) {
    zend_string *s;
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) <= HB_LEN_P(_this));
    s = zend_string_init(HB_BUF_P(_this) + HB_POS_P(_this), HB_LEN_P(_this) - HB_POS_P(_this), persistent);
    HB_POS_P(_this) = HB_LEN_P(_this);
    return s;
}

static zend_always_inline zend_string *hprose_bytes_io_preaduntil_ex(hprose_bytes_io *_this, char tag, zend_bool persistent, zend_bool skiptag) {
    zend_string *s;
    int32_t i = HB_POS_P(_this), n = HB_LEN_P(_this), p = HB_LEN_P(_this);
    assert(HB_INITED_P(_this));
    for (; i < n; ++i) {
        if (HB_BUF_P(_this)[i] == tag) {
            p = i;
            break;
        }
    }
    s = zend_string_init(HB_BUF_P(_this) + HB_POS_P(_this), p - HB_POS_P(_this), persistent);
    HB_POS_P(_this) = p;
    if (HB_POS_P(_this) < HB_LEN_P(_this) && skiptag) {
        HB_POS_P(_this)++;
    }
    return s;
}

#define hprose_bytes_io_readfull(_this) hprose_bytes_io_preadfull((_this), 0)

#define hprose_bytes_io_readuntil_ex(_this, tag, skiptag) hprose_bytes_io_preaduntil_ex((_this), (tag), 0, (skiptag))
#define hprose_bytes_io_preaduntil(_this, tag, persistent) hprose_bytes_io_preaduntil_ex((_this), (tag), (persistent), 1)
#define hprose_bytes_io_readuntil(_this, tag) hprose_bytes_io_preaduntil_ex((_this), (tag), 0, 1)

#endif

#define hprose_bytes_io_read(_this, n) hprose_bytes_io_pread((_this), (n), 0)

static zend_always_inline int32_t hprose_bytes_io_read_int(hprose_bytes_io *_this, char tag) {
    int32_t result = 0, sign = 1;
    char c = hprose_bytes_io_getc(_this);
    if (c == tag) {
        return 0;
    }
    switch (c) {
        case '-': sign = -1; /* fallthrough */
        case '+': c = hprose_bytes_io_getc(_this); break;
    }
    while ((HB_POS_P(_this) < HB_LEN_P(_this)) && (c != tag)) {
        result *= 10;
        result += (c - '0') * sign;
        c = hprose_bytes_io_getc(_this);
    }
    return result;
}

static zend_always_inline int32_t _hprose_bytes_io_get_string_len(hprose_bytes_io *_this, int32_t n TSRMLS_DC) {
    int32_t i, p = HB_POS_P(_this), l = HB_LEN_P(_this);
    uint8_t *buf = (uint8_t *)HB_BUF_P(_this);
    assert(HB_INITED_P(_this));
    for (i = 0; i < n && p < l; ++i) {
        switch (buf[p] >> 4) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                /* 0xxx xxxx */
                ++p;
                break;
            case 12:
            case 13:
                /* 110x xxxx   10xx xxxx */
                p += 2;
                break;
            case 14:
                /* 1110 xxxx  10xx xxxx  10xx xxxx */
                p += 3;
                break;
            case 15:
                /* 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx */
                p += 4;
                ++i;
                if (i < n) break;
                /* fall through */
            default:
                zend_throw_exception(NULL, "bad utf-8 encoding", 0 TSRMLS_CC);
                break;
        }
    }
    return p - HB_POS_P(_this);
}

#if PHP_MAJOR_VERSION < 7

static zend_always_inline char * _hprose_bytes_io_read_pstring(hprose_bytes_io *_this, int32_t n, int32_t *len_ptr, zend_bool persistent TSRMLS_DC) {
    *len_ptr = _hprose_bytes_io_get_string_len(_this, n TSRMLS_CC);
    return hprose_bytes_io_pread(_this, *len_ptr, persistent);
}

#define hprose_bytes_io_read_pstring(_this, n, len_ptr, persistent) _hprose_bytes_io_read_pstring((_this), (n), (len_ptr), (persistent) TSRMLS_CC)
#define hprose_bytes_io_read_string(_this, n, len_ptr) hprose_bytes_io_read_pstring((_this), (n), (len_ptr), 0)

static zend_always_inline char * hprose_bytes_io_to_string(hprose_bytes_io *_this) {
    return estrndup(HB_BUF_P(_this), HB_LEN_P(_this));
}

#else

static zend_always_inline zend_string *_hprose_bytes_io_read_pstring(hprose_bytes_io *_this, int32_t n, zend_bool persistent TSRMLS_DC) {
    return hprose_bytes_io_pread(_this, _hprose_bytes_io_get_string_len(_this, n TSRMLS_CC), persistent);
}

#define hprose_bytes_io_read_pstring(_this, n, persistent) _hprose_bytes_io_read_pstring((_this), (n), (persistent) TSRMLS_CC)
#define hprose_bytes_io_read_string(_this, n) hprose_bytes_io_read_pstring((_this), (n), 0)

static zend_always_inline zend_string *hprose_bytes_io_to_string(hprose_bytes_io *_this) {
    return zend_string_init(HB_BUF_P(_this), HB_LEN_P(_this), 0);
}

#endif

static zend_always_inline void hprose_bytes_io_skip(hprose_bytes_io *_this, int32_t n) {
    assert(HB_INITED_P(_this));
    assert(n >= 0);
    HB_POS_P(_this) += n;
    assert(HB_POS_P(_this) <= HB_LEN_P(_this));
}

static zend_always_inline zend_bool hprose_bytes_io_eof(hprose_bytes_io *_this) {
    return (HB_POS_P(_this) >= HB_LEN_P(_this));
}

static zend_always_inline void hprose_bytes_io_write(hprose_bytes_io *_this, const char *str, int32_t n) {
    if (n < 0) n = strlen(str);
    if (n == 0) return;
    _hprose_bytes_io_grow(_this, n);
    memcpy(HB_BUF_P(_this) + HB_LEN_P(_this), str, n);
    HB_LEN_P(_this) += n;
    HB_BUF_P(_this)[HB_LEN_P(_this)] = '\0';
}

static zend_always_inline void hprose_bytes_io_putc(hprose_bytes_io *_this, char c) {
    if (!HB_INITED_P(_this) || HB_LEN_P(_this) + 1 >= HB_CAP_P(_this)) {
        _hprose_bytes_io_grow(_this, HPROSE_BYTES_IO_PREALLOC);
    }
    HB_BUF_P(_this)[HB_LEN_P(_this)] = c;
    HB_BUF_P(_this)[++HB_LEN_P(_this)] = '\0';
}

static zend_always_inline void hprose_bytes_io_write_int(hprose_bytes_io *_this, int32_t num) {
    if (num >= 0 && num <= 9) {
        hprose_bytes_io_putc(_this, (char)('0' + num));
    }
    else if (num == INT32_MIN) {
        hprose_bytes_io_write(_this, HPROSE_INT32_MIN_STR, sizeof(HPROSE_INT32_MIN_STR) - 1);
    }
    else {
        char buf[32];
        char *p = buf + 31;
        zend_bool neg = (num < 0);
        if (neg) {
            num = -num;
        }
        *p = '\0';
        while (num > 0) {
            *(--p) = (char)(num % 10) + '0';
            num /= 10;
        }
        if (neg) {
            *(--p) = '-';
        }
        hprose_bytes_io_write(_this, p, buf + 31 - p);
    }
}

static zend_always_inline void hprose_bytes_io_write_long(hprose_bytes_io *_this, int64_t num) {
    if (num >= 0 && num <= 9) {
        hprose_bytes_io_putc(_this, (char)('0' + num));
    }
    else if (num == INT64_MIN) {
        hprose_bytes_io_write(_this, HPROSE_INT64_MIN_STR, sizeof(HPROSE_INT64_MIN_STR) - 1);
    }
    else {
        char buf[32];
        char *p = buf + 31;
        zend_bool neg = (num < 0);
        if (neg) {
            num = -num;
        }
        *p = '\0';
        while (num > 0) {
            *(--p) = (char)(num % 10) + '0';
            num /= 10;
        }
        if (neg) {
            *(--p) = '-';
        }
        hprose_bytes_io_write(_this, p, buf + 31 - p);
    }
}

static zend_always_inline void hprose_bytes_io_write_uint(hprose_bytes_io *_this, uint32_t num) {
    if (num <= 9) {
        hprose_bytes_io_putc(_this, (char)('0' + num));
    }
    else {
        char buf[32];
        char *p = buf + 31;
        *p = '\0';
        while (num > 0) {
            *(--p) = (char)(num % 10) + '0';
            num /= 10;
        }
        hprose_bytes_io_write(_this, p, buf + 31 - p);
    }
}

static zend_always_inline void hprose_bytes_io_write_ulong(hprose_bytes_io *_this, uint64_t num) {
    if (num <= 9) {
        hprose_bytes_io_putc(_this, (char)('0' + num));
    }
    else {
        char buf[32];
        char *p = buf + 31;
        *p = '\0';
        while (num > 0) {
            *(--p) = (char)(num % 10) + '0';
            num /= 10;
        }
        hprose_bytes_io_write(_this, p, buf + 31 - p);
    }
}

static zend_always_inline void hprose_bytes_io_write_double(hprose_bytes_io *_this, double num) {
    char buf[32];
    int n = sprintf(buf, "%.16g", num);
    hprose_bytes_io_write(_this, buf, n);
}

static zend_always_inline void hprose_bytes_io_getc_to(hprose_bytes_io *from, hprose_bytes_io *to) {
    hprose_bytes_io_putc(to, HB_BUF_P(from)[HB_POS_P(from)++]);
}

static zend_always_inline void hprose_bytes_io_read_to(hprose_bytes_io *from, hprose_bytes_io *to, int32_t n) {
    assert(HB_INITED_P(from));
    assert(HB_POS_P(from) + n <= HB_LEN_P(from));
    hprose_bytes_io_write(to, HB_BUF_P(from) + HB_POS_P(from), n);
    HB_POS_P(from) += n;
}

static zend_always_inline void hprose_bytes_io_readuntil_to(hprose_bytes_io *from, hprose_bytes_io *to, char tag, zend_bool include_tag) {
    int32_t i = HB_POS_P(from), n = HB_LEN_P(from), p = HB_LEN_P(from);
    assert(HB_INITED_P(from));
    for (; i < n; ++i) {
        if (HB_BUF_P(from)[i] == tag) {
            p = include_tag ? i + 1 : i;
            break;
        }
    }
    hprose_bytes_io_write(to, HB_BUF_P(from) + HB_POS_P(from), p - HB_POS_P(from));
    HB_POS_P(from) = p;
    if (HB_POS_P(from) < HB_LEN_P(from) && !include_tag) {
        HB_POS_P(from)++;
    }
}

static zend_always_inline int32_t hprose_bytes_io_read_int_to(hprose_bytes_io *from, hprose_bytes_io *to, char tag, zend_bool include_tag) {
    int32_t result = 0, sign = 1, p = HB_POS_P(from);
    char c = hprose_bytes_io_getc(from);
    if (c == tag) {
        if (include_tag) {
            hprose_bytes_io_putc(to, c);
        }
        return 0;
    }
    switch (c) {
        case '-': sign = -1; /* fallthrough */
        case '+': c = hprose_bytes_io_getc(from); break;
    }
    while ((HB_POS_P(from) < HB_LEN_P(from)) && (c != tag)) {
        result *= 10;
        result += (c - '0') * sign;
        c = hprose_bytes_io_getc(from);
    }
    hprose_bytes_io_write(to, HB_BUF_P(from) + p, HB_POS_P(from) - p - 1 + ((int32_t)include_tag & 1));
    return result;
}

static zend_always_inline void _hprose_bytes_io_read_string_to(hprose_bytes_io *from, hprose_bytes_io *to, int32_t n TSRMLS_DC) {
    int32_t len = _hprose_bytes_io_get_string_len(from, n TSRMLS_CC);
    hprose_bytes_io_write(to, HB_BUF_P(from) + HB_POS_P(from), len);
    HB_POS_P(from) += len;
}

#define hprose_bytes_io_read_string_to(from, to, n) _hprose_bytes_io_read_string_to((from), (to), (n) TSRMLS_CC)

HPROSE_CLASS_BEGIN(bytes_io)
    int32_t mark;
HPROSE_CLASS_END(bytes_io)

END_EXTERN_C()

#endif	/* HPROSE_BYTES_IO_H */
