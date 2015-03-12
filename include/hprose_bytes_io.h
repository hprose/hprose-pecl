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
 * LastModified: Mar 12, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_BYTES_IO_H
#define	HPROSE_BYTES_IO_H

#include "php.h"
#include "zend_exceptions.h"
#if PHP_MAJOR_VERSION < 7
#include "ext/standard/php_smart_str.h"
#else
#include "ext/standard/php_smart_string.h"
#endif

BEGIN_EXTERN_C()

extern zend_class_entry *hprose_bytes_io_ce;

HPROSE_STARTUP_FUNCTION(bytes_io);

typedef struct _hprose_bytes_io {
    smart_str buffer;
    int32_t pos;
    int32_t mark;
} hprose_bytes_io_t;

static inline hprose_bytes_io_t * hprose_bytes_io_create(const char *buf, int32_t len) {
    hprose_bytes_io_t * bytes = ecalloc(1, sizeof(hprose_bytes_io_t));
    bytes->buffer.c = NULL;
    bytes->buffer.len = 0;
    bytes->buffer.a = 0;
    bytes->pos = 0;
    bytes->mark = -1;
    if (buf) {
        // maybe here has a bomb, using smart_str_append is more safe, but now it fast.
        if (len == -1) {
            smart_str_sets(&bytes->buffer, buf);
        }
        else {
            smart_str_setl(&bytes->buffer, buf, len);
        }
    }
    return bytes;
}

static inline void hprose_bytes_io_close(hprose_bytes_io_t *_this) {
    smart_str_free(&_this->buffer);
    _this->pos = 0;
    _this->mark = -1;
}

static inline void hprose_bytes_io_free(hprose_bytes_io_t *_this) {
    hprose_bytes_io_close(_this);
    efree(_this);
}

static inline size_t hprose_bytes_io_length(hprose_bytes_io_t *_this) {
    return _this->buffer.len;
}

static inline char hprose_bytes_io_getc(hprose_bytes_io_t *_this) {
    return _this->buffer.c[_this->pos++];
}

static inline smart_str hprose_bytes_io_read(hprose_bytes_io_t *_this, int32_t n) {
    smart_str s = {0};
    smart_str_appendl(&s, _this->buffer.c + _this->pos, n);
    smart_str_0(&s);
    _this->pos += n;
    return s;
}

static inline smart_str hprose_bytes_io_readfull(hprose_bytes_io_t *_this) {
    smart_str s = {0};
    smart_str_appendl(&s, _this->buffer.c + _this->pos, _this->buffer.len - _this->pos);
    smart_str_0(&s);
    _this->pos = _this->buffer.len;
    return s;
}

static inline smart_str hprose_bytes_io_readuntil(hprose_bytes_io_t *_this, char tag, zend_bool skiptag) {
    int32_t i, n, p;
    smart_str s = {0};
    for (i = _this->pos, n = _this->buffer.len; i < n; ++i) {
        if (_this->buffer.c[i] == tag) {
            p = i;
            break;
        }
    }
    smart_str_appendl(&s, _this->buffer.c + _this->pos, p - _this->pos);
    smart_str_0(&s);
    _this->pos = p;
    if (_this->pos < _this->buffer.len && skiptag) {
        _this->pos++;
    }
    return s;
}

static inline smart_str hprose_bytes_io_read_string(hprose_bytes_io_t *_this, int32_t n TSRMLS_DC) {
    int32_t i, pos = _this->pos;
    uint8_t *buf = (uint8_t *)_this->buffer.c;
    for (i = 0; i < n; ++i) {
        switch (buf[pos] >> 4) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                // 0xxx xxxx
                ++pos;
                break;
            case 12:
            case 13:
                // 110x xxxx   10xx xxxx
                pos += 2;
                break;
            case 14:
                // 1110 xxxx  10xx xxxx  10xx xxxx
                pos += 3;
                break;
            case 15:
                // 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx
                pos += 4;
                ++i;
                if (i < n) break;
                /* fall through */
            default:
                zend_throw_exception(zend_exception_get_default(TSRMLS_C), "bad utf-8 encoding", 0 TSRMLS_CC);
                break;
        }
    }
    return hprose_bytes_io_read(_this, pos - _this->pos);
}

static inline void hprose_bytes_io_mark(hprose_bytes_io_t *_this) {
    _this->mark = _this->pos;
}

static inline void hprose_bytes_io_unmark(hprose_bytes_io_t *_this) {
    _this->mark = -1;
}

static inline void hprose_bytes_io_reset(hprose_bytes_io_t *_this) {
    if (_this->mark != -1) _this->pos = _this->mark;
}

static inline void hprose_bytes_io_skip(hprose_bytes_io_t *_this, int32_t n) {
    _this->pos += n;
}

static inline zend_bool hprose_bytes_io_eof(hprose_bytes_io_t *_this) {
    return (_this->pos >= _this->buffer.len);
}

static inline void hprose_bytes_io_write(hprose_bytes_io_t *_this, const char *str, int n) {
    if (n < 0) n = strlen(str);
    smart_str_appendl(&(_this->buffer), str, n);
    smart_str_0(&(_this->buffer));
}

END_EXTERN_C()

#endif	/* HPROSE_BYTES_IO_H */

