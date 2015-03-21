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
 * LastModified: Mar 21, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_BYTES_IO_H
#define	HPROSE_BYTES_IO_H

#include "hprose.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_bytes_io_ce();

HPROSE_STARTUP_FUNCTION(bytes_io);

typedef struct {
    char * buf;
    int32_t len;
    int32_t cap;
    int32_t pos;
    zend_bool persistent;
} hprose_bytes_io;

#ifndef HPROSE_BYTES_IO_PREALLOC
#define HPROSE_BYTES_IO_PREALLOC 1024
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
    register int32_t size = _hprose_pow2roundup(_this->len + n + 1);
    if (_this->buf) {
        size *= 2;
        if (size > _this->cap) {
            char *buf = pemalloc(size, _this->persistent);
            memcpy(buf, _this->buf, _this->len);
            buf[_this->len] = 0;
            pefree(_this->buf, _this->persistent);
            _this->buf = buf;
            _this->cap = size;
        }
    }
    else {
        _this->cap = (size > HPROSE_BYTES_IO_PREALLOC) ? size : HPROSE_BYTES_IO_PREALLOC;
        _this->buf = pemalloc(_this->cap, _this->persistent);
        assert(_this->len == 0);
        assert(_this->pos == 0);
        _this->buf[0] = '\0';
    }
}

static zend_always_inline hprose_bytes_io *hprose_bytes_io_pnew(zend_bool persistent) {
    hprose_bytes_io *_this = pecalloc(1, sizeof(hprose_bytes_io), persistent);
    _this->buf = NULL;
    _this->len = 0;
    _this->cap = 0;
    _this->pos = 0;
    _this->persistent = persistent;
    return _this;
}

#define hprose_bytes_io_new() hprose_bytes_io_pnew(0)

static zend_always_inline hprose_bytes_io *hprose_bytes_io_pcreate(const char *buf, int32_t len, zend_bool persistent) {
    hprose_bytes_io *_this = pecalloc(1, sizeof(hprose_bytes_io), persistent);
    _this->buf = pestrndup(buf, len, persistent);
    _this->len = len;
    _this->cap = len + 1;
    _this->pos = 0;
    _this->persistent = persistent;
    return _this;
}

#define hprose_bytes_io_create(buf, len) hprose_bytes_io_pcreate((buf), (len), 0)

static zend_always_inline void hprose_bytes_io_close(hprose_bytes_io *_this) {
    if (_this->buf) {
        pefree(_this->buf, _this->persistent);
        _this->buf = NULL;
    }
    _this->len = 0;
    _this->cap = 0;
    _this->pos = 0;
}

static zend_always_inline void hprose_bytes_io_free(hprose_bytes_io *_this) {
    hprose_bytes_io_close(_this);
    pefree(_this, _this->persistent);
}

static zend_always_inline char hprose_bytes_io_getc(hprose_bytes_io *_this) {
    assert(_this->buf != NULL);
    assert(_this->pos < _this->len);
    return _this->buf[_this->pos++];
}

static zend_always_inline char * hprose_bytes_io_pread(hprose_bytes_io *_this, int32_t n, zend_bool persistent) {
    char *s;
    assert(_this->buf != NULL);
    assert(_this->pos + n <= _this->len);
    s = pestrndup(_this->buf + _this->pos, n, persistent);
    _this->pos += n;
    return s;
}

#define hprose_bytes_io_read(_this, n) hprose_bytes_io_pread((_this), (n), 0);

static zend_always_inline char * hprose_bytes_io_preadfull(hprose_bytes_io *_this, int32_t *len_ptr, zend_bool persistent) {
    char *s;
    assert(_this->buf != NULL);
    assert(_this->pos <= _this->len);
    assert(len_ptr);
    *len_ptr = _this->len - _this->pos;
    s = pestrndup(_this->buf + _this->pos, *len_ptr, persistent);
    _this->pos = _this->len;
    return s;
}

#define hprose_bytes_io_readfull(_this, len_ptr) hprose_bytes_io_preadfull((_this), (len_ptr), 0);

static zend_always_inline char * hprose_bytes_io_preaduntil_ex(hprose_bytes_io *_this, char tag, int32_t *len_ptr, zend_bool persistent, zend_bool skiptag) {
    char *s;
    int32_t i = _this->pos, n = _this->len, p = _this->len;
    assert(_this->buf != NULL);
    for (; i < n; ++i) {
        if (_this->buf[i] == tag) {
            p = i;
            break;
        }
    }
    *len_ptr = p - _this->pos;
    s = pestrndup(_this->buf + _this->pos, *len_ptr, persistent);
    _this->pos = p;
    if (_this->pos < _this->len && skiptag) {
        _this->pos++;
    }
    return s;
}

#define hprose_bytes_io_readuntil_ex(_this, tag, len_ptr, skiptag) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), 0, (skiptag))
#define hprose_bytes_io_preaduntil(_this, tag, len_ptr, persistent) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), (persistent), 1)
#define hprose_bytes_io_readuntil(_this, tag, len_ptr) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), 0, 1)

static zend_always_inline int32_t hprose_bytes_io_read_int(hprose_bytes_io *_this, char tag) {
    int32_t result = 0, p = _this->pos;
    char c = hprose_bytes_io_getc(_this);
    if (c == tag) {
        return 0;
    }
    int32_t sign = 1;
    switch (c) {
        case '-': sign = -1; // fallthrough
        case '+': c = hprose_bytes_io_getc(_this); break;
    }
    while ((_this->pos < _this->len) && (c != tag)) {
        result *= 10;
        result += (c - '0') * sign;
        c = hprose_bytes_io_getc(_this);
    }
    return result;
}

static zend_always_inline char * _hprose_bytes_io_read_pstring(hprose_bytes_io *_this, int32_t n, int32_t *len_ptr, zend_bool persistent TSRMLS_DC) {
    int32_t i, p = _this->pos, l = _this->len;
    uint8_t *buf = (uint8_t *)_this->buf;
    assert(_this->buf != NULL);
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
                // 0xxx xxxx
                ++p;
                break;
            case 12:
            case 13:
                // 110x xxxx   10xx xxxx
                p += 2;
                break;
            case 14:
                // 1110 xxxx  10xx xxxx  10xx xxxx
                p += 3;
                break;
            case 15:
                // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                p += 4;
                ++i;
                if (i < n) break;
                /* fall through */
            default:
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), "bad utf-8 encoding", 0 TSRMLS_CC);
                break;
        }
    }
    *len_ptr = p - _this->pos;
    return hprose_bytes_io_pread(_this, *len_ptr, persistent);
}

#define hprose_bytes_io_read_pstring(_this, n, len_ptr, persistent) _hprose_bytes_io_read_pstring((_this), (n), (len_ptr), (persistent) TSRMLS_CC)
#define hprose_bytes_io_read_string(_this, n, len_ptr) hprose_bytes_io_read_pstring((_this), (n), (len_ptr), 0)

static zend_always_inline void hprose_bytes_io_skip(hprose_bytes_io *_this, int32_t n) {
    assert(_this->buf != NULL);
    assert(n >= 0);
    _this->pos += n;
    assert(_this->pos <= _this->len);
}

static zend_always_inline zend_bool hprose_bytes_io_eof(hprose_bytes_io *_this) {
    return (_this->pos >= _this->len);
}

static zend_always_inline void hprose_bytes_io_write(hprose_bytes_io *_this, const char *str, int32_t n) {
    if (n < 0) n = strlen(str);
    _hprose_bytes_io_grow(_this, n);
    memcpy(_this->buf + _this->len, str, n);
    _this->len += n;
    _this->buf[_this->len] = '\0';
}

static zend_always_inline void hprose_bytes_io_write_char(hprose_bytes_io *_this, char c) {
    if (_this->len + 1 >= _this->cap) {
        _hprose_bytes_io_grow(_this, HPROSE_BYTES_IO_PREALLOC);
    }
    _this->buf[_this->len] = c;
    _this->buf[++_this->len] = '\0';
}

static zend_always_inline void hprose_bytes_io_write_int(hprose_bytes_io *_this, int32_t num) {
    if (num >= 0 && num <= 9) {
        hprose_bytes_io_write_char(_this, '0' + num);
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
        hprose_bytes_io_write_char(_this, '0' + num);
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
        hprose_bytes_io_write_char(_this, '0' + num);
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
        hprose_bytes_io_write_char(_this, '0' + num);
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

static zend_always_inline char * hprose_bytes_io_to_string(hprose_bytes_io *_this) {
    return estrndup(_this->buf, _this->len);
}

static zend_always_inline void hprose_bytes_io_getc_to(hprose_bytes_io *from, hprose_bytes_io *to) {
    hprose_bytes_io_write_char(to, from->buf[from->pos++]);
}

static zend_always_inline void hprose_bytes_io_read_to(hprose_bytes_io *from, hprose_bytes_io *to, int32_t n) {
    assert(from->buf != NULL);
    assert(from->pos + n <= from->len);
    hprose_bytes_io_write(to, from->buf + from->pos, n);
    from->pos += n;
}

static zend_always_inline void hprose_bytes_io_readuntil_to(hprose_bytes_io *from, hprose_bytes_io *to, char tag, zend_bool include_tag) {
    int32_t i = from->pos, n = from->len, p = from->len;
    assert(from->buf != NULL);
    for (; i < n; ++i) {
        if (from->buf[i] == tag) {
            p = include_tag ? i + 1 : i;
            break;
        }
    }
    hprose_bytes_io_write(to, from->buf + from->pos, p - from->pos);
    from->pos = p;
    if (from->pos < from->len && !include_tag) {
        from->pos++;
    }
}

static zend_always_inline int32_t hprose_bytes_io_read_int_to(hprose_bytes_io *from, hprose_bytes_io *to, char tag, zend_bool include_tag) {
    int32_t result = 0, p = from->pos;
    char c = hprose_bytes_io_getc(from);
    if (c == tag) {
        if (include_tag) {
            hprose_bytes_io_write_char(to, c);
        }
        return 0;
    }
    int32_t sign = 1;
    switch (c) {
        case '-': sign = -1; // fallthrough
        case '+': c = hprose_bytes_io_getc(from); break;
    }
    while ((from->pos < from->len) && (c != tag)) {
        result *= 10;
        result += (c - '0') * sign;
        c = hprose_bytes_io_getc(from);
    }
    hprose_bytes_io_write(to, from->buf + p, from->pos - p - 1 + (int32_t)include_tag);
    return result;
}

static zend_always_inline void _hprose_bytes_io_read_string_to(hprose_bytes_io *from, hprose_bytes_io *to, int32_t n TSRMLS_DC) {
    int32_t i, p = from->pos, l = from->len;
    uint8_t *buf = (uint8_t *)from->buf;
    assert(from->buf != NULL);
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
                // 0xxx xxxx
                ++p;
                break;
            case 12:
            case 13:
                // 110x xxxx   10xx xxxx
                p += 2;
                break;
            case 14:
                // 1110 xxxx  10xx xxxx  10xx xxxx
                p += 3;
                break;
            case 15:
                // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                p += 4;
                ++i;
                if (i < n) break;
                /* fall through */
            default:
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), "bad utf-8 encoding", 0 TSRMLS_CC);
                break;
        }
    }
    hprose_bytes_io_write(to, from->buf + from->pos, p - from->pos);
    from->pos = p;
}

#define hprose_bytes_io_read_string_to(from, to, n) _hprose_bytes_io_read_string_to((from), (to), (n) TSRMLS_CC)

HPROSE_CLASS_BEGIN(bytes_io)
    int32_t mark;
HPROSE_CLASS_END(bytes_io)

END_EXTERN_C()

#endif	/* HPROSE_BYTES_IO_H */
