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
 * LastModified: Mar 13, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_BYTES_IO_H
#define	HPROSE_BYTES_IO_H

#include "hprose_common.h"

BEGIN_EXTERN_C()

HPROSE_STARTUP_FUNCTION(bytes_io);

typedef struct {
    char * buf;
    int32_t len;
    int32_t cap;
    int32_t pos;
    zend_bool persistent;
} hprose_bytes_io_t;

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
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    return x + 1;
}

static zend_always_inline void _hprose_bytes_io_grow(hprose_bytes_io_t *_this, int32_t n) {
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

static zend_always_inline hprose_bytes_io_t *hprose_bytes_io_pnew(zend_bool persistent) {
    hprose_bytes_io_t *_this = pecalloc(1, sizeof(hprose_bytes_io_t), persistent);
    _this->buf = NULL;
    _this->len = 0;
    _this->cap = 0;
    _this->pos = 0;
    _this->persistent = persistent;
    return _this;
}

#define hprose_bytes_io_new() hprose_bytes_io_pnew(0)

static zend_always_inline hprose_bytes_io_t *hprose_bytes_io_pcreate(const char *buf, int32_t len, zend_bool persistent) {
    hprose_bytes_io_t *_this = pecalloc(1, sizeof(hprose_bytes_io_t), persistent);
    _this->buf = pestrndup(buf, len, persistent);
    _this->len = len;
    _this->cap = len + 1;
    _this->pos = 0;
    _this->persistent = persistent;
    return _this;
}

#define hprose_bytes_io_create(buf, len) hprose_bytes_io_pcreate((buf), (len), 0)

static zend_always_inline void hprose_bytes_io_close(hprose_bytes_io_t *_this) {
    if (_this->buf) {
        pefree(_this->buf, _this->persistent);
        _this->buf = NULL;
    }
    _this->len = 0;
    _this->cap = 0;
    _this->pos = 0;
}

static zend_always_inline void hprose_bytes_io_free(hprose_bytes_io_t *_this) {
    hprose_bytes_io_close(_this);
    pefree(_this, _this->persistent);
}

static zend_always_inline char hprose_bytes_io_getc(hprose_bytes_io_t *_this) {
    assert(_this->buf != NULL);
    assert(_this->pos < _this->len);
    return _this->buf[_this->pos++];
}

static zend_always_inline char * hprose_bytes_io_pread(hprose_bytes_io_t *_this, int32_t n, zend_bool persistent) {
    char *s;
    assert(_this->buf != NULL);
    assert(_this->pos + n <= _this->len);
    s = pestrndup(_this->buf + _this->pos, n, persistent);
    _this->pos += n;
    return s;
}

#define hprose_bytes_io_read(_this, n) hprose_bytes_io_pread((_this), (n), 0);

static zend_always_inline char * hprose_bytes_io_preadfull(hprose_bytes_io_t *_this, int32_t *p_len, zend_bool persistent) {
    char *s;
    assert(_this->buf != NULL);
    assert(_this->pos <= _this->len);
    assert(p_len);
    *p_len = _this->len - _this->pos;
    s = pestrndup(_this->buf + _this->pos, *p_len, persistent);
    _this->pos = _this->len;
    return s;
}

#define hprose_bytes_io_readfull(_this, p_len) hprose_bytes_io_preadfull((_this), (p_len), 0);

static zend_always_inline char * hprose_bytes_io_preaduntil_ex(hprose_bytes_io_t *_this, char tag, int32_t *p_len, zend_bool persistent, zend_bool skiptag) {
    char *s;
    int32_t i = _this->pos, n = _this->len, p = _this->len;
    assert(_this->buf != NULL);
    for (; i < n; ++i) {
        if (_this->buf[i] == tag) {
            p = i;
            break;
        }
    }
    *p_len = p - _this->pos;
    s = pestrndup(_this->buf + _this->pos, *p_len, persistent);
    _this->pos = p;
    if (_this->pos < _this->len && skiptag) {
        _this->pos++;
    }
    return s;
}

#define hprose_bytes_io_readuntil_ex(_this, tag, p_len, skiptag) hprose_bytes_io_preaduntil_ex((_this), (tag), (p_len), 0, (skiptag))
#define hprose_bytes_io_preaduntil(_this, tag, p_len, persistent) hprose_bytes_io_preaduntil_ex((_this), (tag), (p_len), (persistent), 1)
#define hprose_bytes_io_readuntil(_this, tag, p_len) hprose_bytes_io_preaduntil_ex((_this), (tag), (p_len), 0, 1)

static zend_always_inline char * _hprose_bytes_io_read_pstring(hprose_bytes_io_t *_this, int32_t n, int32_t *p_len, zend_bool persistent TSRMLS_DC) {
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
    *p_len = p - _this->pos;
    return hprose_bytes_io_pread(_this, *p_len, persistent);
}

#define hprose_bytes_io_read_pstring(_this, n, p_len, persistent) _hprose_bytes_io_read_pstring((_this), (n), (p_len), (persistent) TSRMLS_CC)
#define hprose_bytes_io_read_string(_this, n, p_len) hprose_bytes_io_read_pstring((_this), (n), (p_len), 0)

static zend_always_inline void hprose_bytes_io_skip(hprose_bytes_io_t *_this, int32_t n) {
    assert(_this->buf != NULL);
    assert(n >= 0);
    _this->pos += n;
    assert(_this->pos <= _this->len);
}

static zend_always_inline zend_bool hprose_bytes_io_eof(hprose_bytes_io_t *_this) {
    return (_this->pos >= _this->len);
}

static zend_always_inline void hprose_bytes_io_write(hprose_bytes_io_t *_this, const char *str, int32_t n) {
    if (n < 0) n = strlen(str);
    _hprose_bytes_io_grow(_this, n);
    memcpy(_this->buf + _this->len, str, n);
    _this->len += n;
    _this->buf[_this->len] = '\0';
}

static zend_always_inline void hprose_bytes_io_write_int(hprose_bytes_io_t *_this, int32_t num) {
    if (num == INT32_MIN) {
        hprose_bytes_io_write(_this, HPROSE_INT32_MIN_STR, sizeof(HPROSE_INT32_MIN_STR));
    }
    else {
        char nb[32];
        char *p = nb + 31;
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
        hprose_bytes_io_write(_this, p, nb + 31 - p);
    }
}

static zend_always_inline void hprose_bytes_io_write_long(hprose_bytes_io_t *_this, int64_t num) {
    if (num == INT64_MIN) {
        hprose_bytes_io_write(_this, HPROSE_INT64_MIN_STR, sizeof(HPROSE_INT64_MIN_STR));
    }
    else {
        char nb[32];
        char *p = nb + 31;
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
        hprose_bytes_io_write(_this, p, nb + 31 - p);
    }
}

static zend_always_inline void hprose_bytes_io_write_uint(hprose_bytes_io_t *_this, uint32_t num) {
    char nb[32];
    char *p = nb + 31;
    *p = '\0';
    while (num > 0) {
        *(--p) = (char)(num % 10) + '0';
        num /= 10;
    }
    hprose_bytes_io_write(_this, p, nb + 31 - p);
}

static zend_always_inline void hprose_bytes_io_write_ulong(hprose_bytes_io_t *_this, uint64_t num) {
    char nb[32];
    char *p = nb + 31;
    *p = '\0';
    while (num > 0) {
        *(--p) = (char)(num % 10) + '0';
        num /= 10;
    }
    hprose_bytes_io_write(_this, p, nb + 31 - p);
}

zend_class_entry *get_hprose_bytes_io_ce();

END_EXTERN_C()

#endif	/* HPROSE_BYTES_IO_H */
