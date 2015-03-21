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
 * hprose_raw_reader.h                                    *
 *                                                        *
 * hprose raw reader for pecl header file.                *
 *                                                        *
 * LastModified: Mar 21, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_READER_H
#define	HPROSE_READER_H

#include "hprose.h"
#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_class_manager.h"
#include "hprose_raw_reader.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_reader_ce();

HPROSE_STARTUP_FUNCTION(reader);

typedef void hprose_reader_refer_set(void *_this, zval *val);
typedef zval * hprose_reader_refer_read(void *_this, int32_t index);
typedef void hprose_reader_refer_reset(void *_this);
typedef void hprose_reader_refer_free(void *_this);

typedef struct {
    hprose_reader_refer_set *set;
    hprose_reader_refer_read *read;
    hprose_reader_refer_reset *reset;
    hprose_reader_refer_free *free;
} hprose_reader_refer_handlers;

typedef struct {
    hprose_reader_refer_handlers *handlers;
} hprose_reader_refer;

static void hprose_fake_reader_refer_set(void *_this, zval *val) {}
static zval * hprose_fake_reader_refer_read(void *_this, int32_t index) {
    TSRMLS_FETCH();
    zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
            "Unexpected serialize tag '%c' in stream", HPROSE_TAG_REF);
    return NULL;
}
static void hprose_fake_reader_refer_reset(void *_this) {}
static void hprose_fake_reader_refer_free(void *_this) { efree(_this); }

static hprose_reader_refer_handlers __hprose_fake_reader_refer = {
    hprose_fake_reader_refer_set,
    hprose_fake_reader_refer_read,
    hprose_fake_reader_refer_reset,
    hprose_fake_reader_refer_free
};

static zend_always_inline hprose_reader_refer *hprose_fake_reader_refer_new() {
    hprose_reader_refer *_this = emalloc(sizeof(hprose_reader_refer));
    _this->handlers = &__hprose_fake_reader_refer;
    return _this;
}

typedef struct {
    hprose_reader_refer_handlers *handlers;
    zval *ref;
} hprose_real_reader_refer;

static void hprose_real_reader_refer_set(void *_this, zval *val) {
    hprose_real_reader_refer *refer = (hprose_real_reader_refer *)_this;
    Z_ADDREF_P(val);
    add_next_index_zval(refer->ref, val);
}
static zval * hprose_real_reader_refer_read(void *_this, int32_t index) {
    hprose_real_reader_refer *refer = (hprose_real_reader_refer *)_this;
    zval *result = php_array_get(refer->ref, index);
    return result;
}

static void hprose_real_reader_refer_reset(void *_this) {
    hprose_real_reader_refer *refer = (hprose_real_reader_refer *)_this;
    zend_hash_clean(Z_ARRVAL_P(refer->ref));
}

static void hprose_real_reader_refer_free(void *_this) {
    hprose_real_reader_refer *refer = (hprose_real_reader_refer *)_this;
    hprose_zval_free(refer->ref);
    refer->ref = NULL;
    efree(refer);
}

static hprose_reader_refer_handlers __hprose_real_reader_refer = {
    hprose_real_reader_refer_set,
    hprose_real_reader_refer_read,
    hprose_real_reader_refer_reset,
    hprose_real_reader_refer_free
};

static zend_always_inline hprose_reader_refer * hprose_real_reader_refer_new() {
    hprose_real_reader_refer *_this = emalloc(sizeof(hprose_real_reader_refer));
    _this->handlers = &__hprose_real_reader_refer;
    hprose_make_zval(_this->ref);
    array_init(_this->ref);
    return (hprose_reader_refer *)(void *)_this;
}

typedef struct {
    hprose_bytes_io *stream;
    zval *classref;
    hprose_reader_refer *refer;
} hprose_reader;

static inline void hprose_reader_unserialize(hprose_reader *_this, zval *return_value TSRMLS_DC);

static zend_always_inline hprose_reader * hprose_reader_create(hprose_bytes_io *stream, zend_bool simple) {
    hprose_reader *_this = emalloc(sizeof(hprose_reader));
    _this->stream = stream;
    hprose_make_zval(_this->classref);
    array_init(_this->classref);
    _this->refer = simple ? hprose_fake_reader_refer_new() : hprose_real_reader_refer_new();
    return _this;
}

static zend_always_inline void hprose_reader_free(hprose_reader *_this) {
    _this->stream = NULL;
    hprose_zval_free(_this->classref);
    _this->classref = NULL;
    _this->refer->handlers->free(_this->refer);
    _this->refer = NULL;
    efree(_this);
}

static zend_always_inline void hprose_reader_reset(hprose_reader *_this) {
    zend_hash_clean(Z_ARRVAL_P(_this->classref));
    _this->refer->handlers->reset(_this->refer);
}

static zend_always_inline long hprose_reader_read_integer_without_tag(hprose_reader *_this) {
    return hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_SEMICOLON);
}

static zend_always_inline long hprose_reader_read_integer(hprose_reader *_this TSRMLS_DC) {
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case HPROSE_TAG_INTEGER:
            return hprose_reader_read_integer_without_tag(_this);
        default: unexpected_tag(tag, NULL TSRMLS_CC); return 0;
    }
}

static zend_always_inline char * hprose_reader_read_long_without_tag(hprose_reader *_this, int32_t *len_ptr) {
    return hprose_bytes_io_readuntil(_this->stream, HPROSE_TAG_SEMICOLON, len_ptr);
}

static zend_always_inline char * hprose_reader_read_long(hprose_reader *_this, int32_t *len_ptr TSRMLS_DC) {
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case '0': *len_ptr = 1; return estrndup("0", 1);
        case '1': *len_ptr = 1; return estrndup("1", 1);
        case '2': *len_ptr = 1; return estrndup("2", 1);
        case '3': *len_ptr = 1; return estrndup("3", 1);
        case '4': *len_ptr = 1; return estrndup("4", 1);
        case '5': *len_ptr = 1; return estrndup("5", 1);
        case '6': *len_ptr = 1; return estrndup("6", 1);
        case '7': *len_ptr = 1; return estrndup("7", 1);
        case '8': *len_ptr = 1; return estrndup("8", 1);
        case '9': *len_ptr = 1; return estrndup("9", 1);
        case HPROSE_TAG_INTEGER:
        case HPROSE_TAG_LONG:
            return hprose_reader_read_long_without_tag(_this, len_ptr);
        default: unexpected_tag(tag, NULL TSRMLS_CC); return 0;
    }
}

static zend_always_inline double hprose_reader_read_double_without_tag(hprose_reader *_this) {
    int32_t l;
    char *s = hprose_bytes_io_readuntil(_this->stream, HPROSE_TAG_SEMICOLON, &l);
    double d = atof(s);
    efree(s);
    return d;
}

static zend_always_inline double hprose_reader_read_infinity_without_tag(hprose_reader *_this) {
    return (hprose_bytes_io_getc(_this->stream) == HPROSE_TAG_NEG) ? -INFINITY : INFINITY;
}

static zend_always_inline double hprose_reader_read_double(hprose_reader *_this TSRMLS_DC) {
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        case HPROSE_TAG_INTEGER:
            return hprose_reader_read_integer_without_tag(_this);
        case HPROSE_TAG_LONG:
        case HPROSE_TAG_DOUBLE:
            return hprose_reader_read_double_without_tag(_this);
        case HPROSE_TAG_NAN:
            return NAN;
        case HPROSE_TAG_INFINITY:
            return hprose_reader_read_infinity_without_tag(_this);
        default: unexpected_tag(tag, NULL TSRMLS_CC); return 0;
    }
}

static zend_always_inline void hprose_reader_read_ref(hprose_reader *_this, zval *return_value) {
    zval *val = _this->refer->handlers->read(_this->refer, hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_SEMICOLON));
    RETURN_ZVAL(val, 1, 1);
}

static zend_always_inline void hprose_reader_read_datetime_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char tag;
#if PHP_MAJOR_VERSION < 7
    zval *t;
    hprose_make_zval(t);
#else
    zval t;
#endif
    hprose_bytes_io *tmp = hprose_bytes_io_new();
    hprose_bytes_io_read_to(_this->stream, tmp, 4);
    hprose_bytes_io_write_char(tmp, '-');
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    hprose_bytes_io_write_char(tmp, '-');
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_TIME) {
        hprose_bytes_io_write_char(tmp, tag);
        hprose_bytes_io_read_to(_this->stream, tmp, 2);
        hprose_bytes_io_write_char(tmp, ':');
        hprose_bytes_io_read_to(_this->stream, tmp, 2);
        hprose_bytes_io_write_char(tmp, ':');
        hprose_bytes_io_read_to(_this->stream, tmp, 2);
        tag = hprose_bytes_io_getc(_this->stream);
        if (tag == HPROSE_TAG_POINT) {
            hprose_bytes_io_write_char(tmp, tag);
            hprose_bytes_io_read_to(_this->stream, tmp, 3);
            tag = hprose_bytes_io_getc(_this->stream);
            if ((tag >= '0') && (tag <= '9')) {
                hprose_bytes_io_write_char(tmp, tag);
                hprose_bytes_io_read_to(_this->stream, tmp, 2);
                tag = hprose_bytes_io_getc(_this->stream);
                if ((tag >= '0') && (tag <= '9')) {
                    hprose_bytes_io_skip(_this->stream, 2);
                    tag = hprose_bytes_io_getc(_this->stream);
                }
            }
        }
    }
#if PHP_MAJOR_VERSION < 7
    ZVAL_STRINGL(t, tmp->buf, tmp->len, 0);
    if (tag == HPROSE_TAG_UTC) {
        zval *params[] = { t, NULL, NULL };
        zval *timezone, *utc;
        hprose_make_zval(timezone);
        hprose_make_zval(utc);
        ZVAL_LITERAL_STRINGL(utc, "UTC");
        call_php_function("timezone_open", timezone, 1, &utc);
        zval_ptr_dtor(&utc);
        params[1] = timezone;
        call_php_function("date_create", return_value, 2, &params[0]);
        zval_ptr_dtor(&timezone);
    }
    else {
        zval *params[] = { t, NULL };
        call_php_function("date_create", return_value, 1, &params[0]);
    }
    efree(t);
#else
    ZVAL_STRINGL(&t, tmp->buf, tmp->len);
    if (tag == HPROSE_TAG_UTC) {
        zval *params[] = { NULL, NULL, NULL };
        zval timezone, utc;
        ZVAL_LITERAL_STRINGL(&utc, "UTC");
        params[0] = &utc;
        call_php_function("timezone_open", &timezone, 1, &params[0]);
        zval_ptr_dtor(&utc);
        params[0] = &t;
        params[1] = &timezone;
        call_php_function("date_create", return_value, 2, &params[0]);
        zval_ptr_dtor(&timezone);
    }
    else {
        zval *params[] = { &t, NULL };
        call_php_function("date_create", return_value, 1, &params[0]);
    }
    zval_ptr_dtor(&t);
#endif
    hprose_bytes_io_free(tmp);
    _this->refer->handlers->set(_this->refer, return_value);
}

static zend_always_inline void hprose_reader_read_time_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char tag;
#if PHP_MAJOR_VERSION < 7
    zval *t;
    hprose_make_zval(t);
#else
    zval t;
#endif
    hprose_bytes_io *tmp = hprose_bytes_io_new();
    hprose_bytes_io_write(tmp, "1970-01-01", 10);
    hprose_bytes_io_write_char(tmp, HPROSE_TAG_TIME);
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    hprose_bytes_io_write_char(tmp, ':');
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    hprose_bytes_io_write_char(tmp, ':');
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_POINT) {
        hprose_bytes_io_write_char(tmp, tag);
        hprose_bytes_io_read_to(_this->stream, tmp, 3);
        tag = hprose_bytes_io_getc(_this->stream);
        if ((tag >= '0') && (tag <= '9')) {
            hprose_bytes_io_write_char(tmp, tag);
            hprose_bytes_io_read_to(_this->stream, tmp, 2);
            tag = hprose_bytes_io_getc(_this->stream);
            if ((tag >= '0') && (tag <= '9')) {
                hprose_bytes_io_skip(_this->stream, 2);
                tag = hprose_bytes_io_getc(_this->stream);
            }
        }
    }
#if PHP_MAJOR_VERSION < 7
    ZVAL_STRINGL(t, tmp->buf, tmp->len, 0);
    if (tag == HPROSE_TAG_UTC) {
        zval *params[] = { t, NULL, NULL };
        zval *timezone, *utc;
        hprose_make_zval(timezone);
        hprose_make_zval(utc);
        ZVAL_LITERAL_STRINGL(utc, "UTC");
        call_php_function("timezone_open", timezone, 1, &utc);
        zval_ptr_dtor(&utc);
        params[1] = timezone;
        call_php_function("date_create", return_value, 2, &params[0]);
        zval_ptr_dtor(&timezone);
    }
    else {
        zval *params[] = { t, NULL };
        call_php_function("date_create", return_value, 1, &params[0]);
    }
    efree(t);
#else
    ZVAL_STRINGL(&t, tmp->buf, tmp->len);
    if (tag == HPROSE_TAG_UTC) {
        zval *params[] = { NULL, NULL, NULL };
        zval timezone, utc;
        ZVAL_LITERAL_STRINGL(&utc, "UTC");
        params[0] = &utc;
        call_php_function("timezone_open", &timezone, 1, &params[0]);
        zval_ptr_dtor(&utc);
        params[0] = &t;
        params[1] = &timezone;
        call_php_function("date_create", return_value, 2, &params[0]);
        zval_ptr_dtor(&timezone);
    }
    else {
        zval *params[] = { &t, NULL };
        call_php_function("date_create", return_value, 1, &params[0]);
    }
    zval_ptr_dtor(&t);
#endif
    hprose_bytes_io_free(tmp);
    _this->refer->handlers->set(_this->refer, return_value);
}

static inline void hprose_reader_read_bytes_without_tag(hprose_reader *_this, zval *return_value) {
    int32_t count = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_QUOTE);
    char *bytes = hprose_bytes_io_read(_this->stream, count);
    hprose_bytes_io_skip(_this->stream, 1);
    RETVAL_STRINGL_0(bytes, count);
    _this->refer->handlers->set(_this->refer, return_value);
}

static inline void hprose_reader_read_utf8char_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    int32_t len;
    char *uc = hprose_bytes_io_read_string(_this->stream, 1, &len);
    RETVAL_STRINGL_0(uc, len);
}

static inline void _hprose_reader_read_string_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    int32_t count = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_QUOTE);
    int32_t len;
    char *str = hprose_bytes_io_read_string(_this->stream, count, &len);
    hprose_bytes_io_skip(_this->stream, 1);
    RETVAL_STRINGL_0(str, len);
}

static inline void hprose_reader_read_string_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    _hprose_reader_read_string_without_tag(_this, return_value TSRMLS_CC);
    _this->refer->handlers->set(_this->refer, return_value);
}

static inline void hprose_reader_read_guid_without_tag(hprose_reader *_this, zval *return_value) {
    hprose_bytes_io_skip(_this->stream, 1);
    char *s = hprose_bytes_io_read(_this->stream, 36);
    hprose_bytes_io_skip(_this->stream, 1);
    RETVAL_STRINGL_0(s, 36);
    _this->refer->handlers->set(_this->refer, return_value);
}

static inline void hprose_reader_unserialize(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case '0': RETURN_LONG(0);
        case '1': RETURN_LONG(1);
        case '2': RETURN_LONG(2);
        case '3': RETURN_LONG(3);
        case '4': RETURN_LONG(4);
        case '5': RETURN_LONG(5);
        case '6': RETURN_LONG(6);
        case '7': RETURN_LONG(7);
        case '8': RETURN_LONG(8);
        case '9': RETURN_LONG(9);
        case HPROSE_TAG_INTEGER:
            RETURN_LONG(hprose_reader_read_integer_without_tag(_this));
        case HPROSE_TAG_LONG: {
            int32_t len;
            char *num = hprose_reader_read_long_without_tag(_this, &len);
            RETURN_STRINGL_0(num, len);
        }
        case HPROSE_TAG_DOUBLE: {
            RETURN_DOUBLE(hprose_reader_read_double_without_tag(_this));
        }
        case HPROSE_TAG_NAN:
            RETURN_DOUBLE(NAN);
        case HPROSE_TAG_INFINITY:
            RETURN_DOUBLE(hprose_reader_read_infinity_without_tag(_this));
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_EMPTY: RETURN_EMPTY_STRING();
        case HPROSE_TAG_TRUE: RETURN_TRUE;
        case HPROSE_TAG_FALSE: RETURN_FALSE;
        case HPROSE_TAG_DATE: {
            hprose_reader_read_datetime_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_TIME: {
            hprose_reader_read_time_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_BYTES: {
            hprose_reader_read_bytes_without_tag(_this, return_value);
            return;
        }
        case HPROSE_TAG_UTF8CHAR: {
            hprose_reader_read_utf8char_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_STRING: {
            hprose_reader_read_string_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_GUID: {
            hprose_reader_read_guid_without_tag(_this, return_value);
            return;
        }
        case HPROSE_TAG_LIST: {

        }
        case HPROSE_TAG_MAP: {

        }
        case HPROSE_TAG_CLASS: {

        }
        case HPROSE_TAG_OBJECT: {

        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value);
            return;
        }
        case HPROSE_TAG_ERROR: {

        }
        default: unexpected_tag(tag, NULL TSRMLS_CC);
    }
}

HPROSE_CLASS_BEGIN(reader)
HPROSE_CLASS_END(reader)

END_EXTERN_C()

#endif	/* HPROSE_READER_H */

