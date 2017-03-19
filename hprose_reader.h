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
 * hprose_reader.h                                        *
 *                                                        *
 * hprose reader for pecl header file.                    *
 *                                                        *
 * LastModified: Mar 19, 2017                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_READER_H
#define	HPROSE_READER_H

#include "hprose_common.h"
#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_class_manager.h"
#include "hprose_raw_reader.h"
#if PHP_VERSION_ID >= 50311
#include "ext/date/php_date.h"
#endif

/* Workaround for old gcc. */
#ifndef NAN
#define NAN (HUGE_VAL/HUGE_VAL)
#endif
#ifndef INFINITY
#define INFINITY HUGE_VAL
#endif

#ifndef Z_PHPDATE_P
#define Z_PHPDATE_P(zv)  zend_object_store_get_object(zv TSRMLS_CC)
#endif

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_reader_ce();

HPROSE_STARTUP_FUNCTION(reader);

typedef struct {
    hprose_bytes_io *stream;
    zval *classref;
    zval *propsref;
    zval *refer;
} hprose_reader;

static zend_always_inline void hprose_reader_refer_set(zval *refer, zval *val) {
    if (refer) {
#ifdef Z_TRY_ADDREF_P
        Z_TRY_ADDREF_P(val);
#else
        Z_ADDREF_P(val);
#endif
        add_next_index_zval(refer, val);
    }
}

PHP_HPROSE_API void hprose_reader_unserialize(hprose_reader *_this, zval *return_value TSRMLS_DC);

static zend_always_inline void hprose_reader_init(hprose_reader *_this, hprose_bytes_io *stream, zend_bool simple) {
    _this->stream = stream;
    hprose_zval_new(_this->classref);
    hprose_zval_new(_this->propsref);
    array_init(_this->classref);
    array_init(_this->propsref);
    if (simple) {
        _this->refer = NULL;
    }
    else {
        hprose_zval_new(_this->refer);
        array_init(_this->refer);
    }
}

static zend_always_inline hprose_reader * hprose_reader_create(hprose_bytes_io *stream, zend_bool simple) {
    hprose_reader *_this = emalloc(sizeof(hprose_reader));
    hprose_reader_init(_this, stream, simple);
    return _this;
}

static zend_always_inline void hprose_reader_destroy(hprose_reader *_this) {
    _this->stream = NULL;
    hprose_zval_free(_this->classref);
    hprose_zval_free(_this->propsref);
    _this->classref = NULL;
    _this->propsref = NULL;
    if (_this->refer) {
        hprose_zval_free(_this->refer);
        _this->refer = NULL;
    }
}

static zend_always_inline void hprose_reader_free(hprose_reader *_this) {
    hprose_reader_destroy(_this);
    efree(_this);
}

static zend_always_inline void hprose_reader_reset(hprose_reader *_this) {
    zend_hash_clean(Z_ARRVAL_P(_this->classref));
    zend_hash_clean(Z_ARRVAL_P(_this->propsref));
    if (_this->refer) {
        zend_hash_clean(Z_ARRVAL_P(_this->refer));
    }
}

static zend_always_inline long _hprose_reader_read_integer_without_tag(hprose_bytes_io *stream) {
    return hprose_bytes_io_read_int(stream, HPROSE_TAG_SEMICOLON);
}

#define hprose_reader_read_integer_without_tag(_this) _hprose_reader_read_integer_without_tag((_this)->stream)

static zend_always_inline long hprose_reader_read_integer(hprose_reader *_this TSRMLS_DC) {
    char expected_tags[] = {'0', '1', '2', '3', '4',
                            '5', '6', '7', '8', '9',
                            HPROSE_TAG_INTEGER, '\0'};
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
        default: unexpected_tag(tag, expected_tags TSRMLS_CC); return 0;
    }
}

#if PHP_MAJOR_VERSION < 7

static zend_always_inline char * _hprose_reader_read_long_without_tag(hprose_bytes_io *stream, int32_t *len_ptr) {
    return hprose_bytes_io_readuntil(stream, HPROSE_TAG_SEMICOLON, len_ptr);
}

#define hprose_reader_read_long_without_tag(_this, len_ptr) _hprose_reader_read_long_without_tag((_this)->stream, len_ptr)

static zend_always_inline char * hprose_reader_read_long(hprose_reader *_this, int32_t *len_ptr TSRMLS_DC) {
    char expected_tags[] = {'0', '1', '2', '3', '4',
                            '5', '6', '7', '8', '9',
                            HPROSE_TAG_INTEGER,
                            HPROSE_TAG_LONG, '\0'};
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
        default: unexpected_tag(tag, expected_tags TSRMLS_CC); return 0;
    }
}
#else
static zend_always_inline zend_string *_hprose_reader_read_long_without_tag(hprose_bytes_io *stream) {
    return hprose_bytes_io_readuntil(stream, HPROSE_TAG_SEMICOLON);
}

#define hprose_reader_read_long_without_tag(_this) _hprose_reader_read_long_without_tag((_this)->stream)

static zend_always_inline zend_string *hprose_reader_read_long(hprose_reader *_this TSRMLS_DC) {
    char expected_tags[] = {'0', '1', '2', '3', '4',
                            '5', '6', '7', '8', '9',
                            HPROSE_TAG_INTEGER,
                            HPROSE_TAG_LONG, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case '0': return zend_string_init("0", 1, 0);
        case '1': return zend_string_init("1", 1, 0);
        case '2': return zend_string_init("2", 1, 0);
        case '3': return zend_string_init("3", 1, 0);
        case '4': return zend_string_init("4", 1, 0);
        case '5': return zend_string_init("5", 1, 0);
        case '6': return zend_string_init("6", 1, 0);
        case '7': return zend_string_init("7", 1, 0);
        case '8': return zend_string_init("8", 1, 0);
        case '9': return zend_string_init("9", 1, 0);
        case HPROSE_TAG_INTEGER:
        case HPROSE_TAG_LONG:
            return hprose_reader_read_long_without_tag(_this);
        default: unexpected_tag(tag, expected_tags TSRMLS_CC); return 0;
    }
}

#endif

static zend_always_inline double _hprose_reader_read_double_without_tag(hprose_bytes_io *stream) {
#if PHP_MAJOR_VERSION < 7
    int32_t l;
    char *s = hprose_bytes_io_readuntil(stream, HPROSE_TAG_SEMICOLON, &l);
    double d = atof(s);
    efree(s);
#else
    zend_string *s = hprose_bytes_io_readuntil(stream, HPROSE_TAG_SEMICOLON);
    double d = atof(s->val);
    zend_string_release(s);
#endif
    return d;
}

#define hprose_reader_read_double_without_tag(_this) _hprose_reader_read_double_without_tag((_this)->stream)

static zend_always_inline double _hprose_reader_read_infinity_without_tag(hprose_bytes_io *stream) {
    return (hprose_bytes_io_getc(stream) == HPROSE_TAG_NEG) ? -INFINITY : INFINITY;
}

#define hprose_reader_read_infinity_without_tag(_this) _hprose_reader_read_infinity_without_tag((_this)->stream)

static zend_always_inline double hprose_reader_read_double(hprose_reader *_this TSRMLS_DC) {
    char expected_tags[] = {'0', '1', '2', '3', '4',
                            '5', '6', '7', '8', '9',
                            HPROSE_TAG_INTEGER,
                            HPROSE_TAG_LONG,
                            HPROSE_TAG_DOUBLE,
                            HPROSE_TAG_NAN,
                            HPROSE_TAG_INFINITY, '\0'};
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
        default: unexpected_tag(tag, expected_tags TSRMLS_CC); return 0;
    }
}

static zend_always_inline zend_bool hprose_reader_read_boolean(hprose_reader *_this TSRMLS_DC) {
    char expected_tags[] = {HPROSE_TAG_TRUE,
                            HPROSE_TAG_FALSE, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_TRUE: return 1;
        case HPROSE_TAG_FALSE: return 0;
        default: unexpected_tag(tag, expected_tags TSRMLS_CC); return 0;
    }
}

static zend_always_inline void hprose_reader_read_ref(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    if (_this->refer) {
        uint32_t index = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_SEMICOLON);
        zval *result = php_array_get(_this->refer, index);
        RETURN_ZVAL(result, 1, 0);
        return;
    }
    zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
            "Unexpected serialize tag '%c' in stream", HPROSE_TAG_REF);
}

static zend_always_inline void _hprose_reader_read_datetime_without_tag(hprose_bytes_io *stream, zval *return_value TSRMLS_DC) {
    char tag;
    hprose_bytes_io *tmp = hprose_bytes_io_new();
    hprose_bytes_io_read_to(stream, tmp, 4);
    hprose_bytes_io_putc(tmp, '-');
    hprose_bytes_io_read_to(stream, tmp, 2);
    hprose_bytes_io_putc(tmp, '-');
    hprose_bytes_io_read_to(stream, tmp, 2);
    tag = hprose_bytes_io_getc(stream);
    if (tag == HPROSE_TAG_TIME) {
        hprose_bytes_io_putc(tmp, tag);
        hprose_bytes_io_read_to(stream, tmp, 2);
        hprose_bytes_io_putc(tmp, ':');
        hprose_bytes_io_read_to(stream, tmp, 2);
        hprose_bytes_io_putc(tmp, ':');
        hprose_bytes_io_read_to(stream, tmp, 2);
        tag = hprose_bytes_io_getc(stream);
        if (tag == HPROSE_TAG_POINT) {
            hprose_bytes_io_putc(tmp, tag);
            hprose_bytes_io_read_to(stream, tmp, 3);
            tag = hprose_bytes_io_getc(stream);
            if ((tag >= '0') && (tag <= '9')) {
                hprose_bytes_io_putc(tmp, tag);
                hprose_bytes_io_read_to(stream, tmp, 2);
                tag = hprose_bytes_io_getc(stream);
                if ((tag >= '0') && (tag <= '9')) {
                    hprose_bytes_io_skip(stream, 2);
                    tag = hprose_bytes_io_getc(stream);
                }
            }
        }
    }
#if PHP_VERSION_ID < 50311
    if (tag == HPROSE_TAG_UTC) {
        zval timezone;
        function_invoke(timezone_open, &timezone, "s", ZEND_STRL("UTC"));
        function_invoke(date_create, return_value, "sz", HB_BUF_P(tmp), HB_LEN_P(tmp), &timezone);
    }
    else {
        function_invoke(date_create, return_value, "s", HB_BUF_P(tmp), HB_LEN_P(tmp));
    }
#else
    if (tag == HPROSE_TAG_UTC) {
        zval timezone;
        function_invoke(timezone_open, &timezone, "s", ZEND_STRL("UTC"));
        php_date_instantiate(php_date_get_date_ce(), return_value TSRMLS_CC);
        php_date_initialize(Z_PHPDATE_P(return_value), HB_BUF_P(tmp), HB_LEN_P(tmp), NULL, &timezone, 0 TSRMLS_CC);
#if PHP_MAJOR_VERSION < 7
        zval_dtor(&timezone);
#else
        zval_ptr_dtor(&timezone);
#endif
    }
    else {
        php_date_instantiate(php_date_get_date_ce(), return_value TSRMLS_CC);
        php_date_initialize(Z_PHPDATE_P(return_value), HB_BUF_P(tmp), HB_LEN_P(tmp), NULL, NULL, 0 TSRMLS_CC);
    }
#endif
    hprose_bytes_io_free(tmp);
}

static zend_always_inline void hprose_reader_read_datetime_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    _hprose_reader_read_datetime_without_tag(_this->stream, return_value TSRMLS_CC);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void _hprose_reader_read_time_without_tag(hprose_bytes_io *stream, zval *return_value TSRMLS_DC) {
    char tag;
    hprose_bytes_io *tmp = hprose_bytes_io_new();
    hprose_bytes_io_write(tmp, "1970-01-01", 10);
    hprose_bytes_io_putc(tmp, HPROSE_TAG_TIME);
    hprose_bytes_io_read_to(stream, tmp, 2);
    hprose_bytes_io_putc(tmp, ':');
    hprose_bytes_io_read_to(stream, tmp, 2);
    hprose_bytes_io_putc(tmp, ':');
    hprose_bytes_io_read_to(stream, tmp, 2);
    tag = hprose_bytes_io_getc(stream);
    if (tag == HPROSE_TAG_POINT) {
        hprose_bytes_io_putc(tmp, tag);
        hprose_bytes_io_read_to(stream, tmp, 3);
        tag = hprose_bytes_io_getc(stream);
        if ((tag >= '0') && (tag <= '9')) {
            hprose_bytes_io_putc(tmp, tag);
            hprose_bytes_io_read_to(stream, tmp, 2);
            tag = hprose_bytes_io_getc(stream);
            if ((tag >= '0') && (tag <= '9')) {
                hprose_bytes_io_skip(stream, 2);
                tag = hprose_bytes_io_getc(stream);
            }
        }
    }
#if PHP_VERSION_ID < 50311
    if (tag == HPROSE_TAG_UTC) {
        zval timezone;
        function_invoke(timezone_open, &timezone, "s", ZEND_STRL("UTC"));
        function_invoke(date_create, return_value, "sz", HB_BUF_P(tmp), HB_LEN_P(tmp), &timezone);
    }
    else {
        function_invoke(date_create, return_value, "s", HB_BUF_P(tmp), HB_LEN_P(tmp));
    }
#else
    if (tag == HPROSE_TAG_UTC) {
        zval timezone;
        function_invoke(timezone_open, &timezone, "s", ZEND_STRL("UTC"));
        php_date_instantiate(php_date_get_date_ce(), return_value TSRMLS_CC);
        php_date_initialize(Z_PHPDATE_P(return_value), HB_BUF_P(tmp), HB_LEN_P(tmp), NULL, &timezone, 0 TSRMLS_CC);
#if PHP_MAJOR_VERSION < 7
        zval_dtor(&timezone);
#else
        zval_ptr_dtor(&timezone);
#endif
    }
    else {
        php_date_instantiate(php_date_get_date_ce(), return_value TSRMLS_CC);
        php_date_initialize(Z_PHPDATE_P(return_value), HB_BUF_P(tmp), HB_LEN_P(tmp), NULL, NULL, 0 TSRMLS_CC);
    }
#endif
    hprose_bytes_io_free(tmp);
}

static zend_always_inline void hprose_reader_read_time_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    _hprose_reader_read_time_without_tag(_this->stream, return_value TSRMLS_CC);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void _hprose_reader_read_bytes_without_tag(hprose_bytes_io *stream, zval *return_value) {
    int32_t count = hprose_bytes_io_read_int(stream, HPROSE_TAG_QUOTE);
#if PHP_MAJOR_VERSION < 7
    RETVAL_STRINGL_0(hprose_bytes_io_read(stream, count), count);
#else
    RETVAL_STR(hprose_bytes_io_read(stream, count));
#endif
    hprose_bytes_io_skip(stream, 1);
}

static zend_always_inline void hprose_reader_read_bytes_without_tag(hprose_reader *_this, zval *return_value) {
    _hprose_reader_read_bytes_without_tag(_this->stream, return_value);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void _hprose_reader_read_utf8char_without_tag(hprose_bytes_io *stream, zval *return_value TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    int32_t len;
    char *uc = hprose_bytes_io_read_string(stream, 1, &len);
    RETVAL_STRINGL_0(uc, len);
#else
    RETVAL_STR(hprose_bytes_io_read_string(stream, 1));
#endif
}

#define hprose_reader_read_utf8char_without_tag(_this, return_value) _hprose_reader_read_utf8char_without_tag((_this)->stream, return_value TSRMLS_CC)

static zend_always_inline void _hprose_reader_read_string_without_tag(hprose_bytes_io *stream, zval *return_value TSRMLS_DC) {
    int32_t count = hprose_bytes_io_read_int(stream, HPROSE_TAG_QUOTE);
#if PHP_MAJOR_VERSION < 7
    int32_t len;
    char *str = hprose_bytes_io_read_string(stream, count, &len);
    RETVAL_STRINGL_0(str, len);
#else
    RETVAL_STR(hprose_bytes_io_read_string(stream, count));
#endif
    hprose_bytes_io_skip(stream, 1);
}

static zend_always_inline void hprose_reader_read_string_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    _hprose_reader_read_string_without_tag(_this->stream, return_value TSRMLS_CC);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void _hprose_reader_read_string(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char expected_tags[] = {HPROSE_TAG_UTF8CHAR,
                            HPROSE_TAG_STRING,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_UTF8CHAR: {
            hprose_reader_read_utf8char_without_tag(_this, return_value);
            return;
        }
        case HPROSE_TAG_STRING: {
            hprose_reader_read_string_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            convert_to_string(return_value);
            return;
        }
        default: unexpected_tag(tag, expected_tags TSRMLS_CC);
    }
}

static zend_always_inline void hprose_reader_read_string(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char expected_tags[] = {HPROSE_TAG_NULL,
                            HPROSE_TAG_EMPTY,
                            HPROSE_TAG_UTF8CHAR,
                            HPROSE_TAG_STRING,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_EMPTY: RETURN_EMPTY_STRING();
        case HPROSE_TAG_UTF8CHAR: {
            hprose_reader_read_utf8char_without_tag(_this, return_value);
            return;
        }
        case HPROSE_TAG_STRING: {
            hprose_reader_read_string_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            convert_to_string(return_value);
            return;
        }
        default: unexpected_tag(tag, expected_tags TSRMLS_CC);
    }
}

static zend_always_inline void _hprose_reader_read_guid_without_tag(hprose_bytes_io *stream, zval *return_value) {
    hprose_bytes_io_skip(stream, 1);
#if PHP_MAJOR_VERSION < 7
    RETVAL_STRINGL_0(hprose_bytes_io_read(stream, 36), 36);
#else
    RETVAL_STR(hprose_bytes_io_read(stream, 36));
#endif
    hprose_bytes_io_skip(stream, 1);
}

static zend_always_inline void hprose_reader_read_guid_without_tag(hprose_reader *_this, zval *return_value) {
    _hprose_reader_read_guid_without_tag(_this->stream, return_value);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void hprose_reader_read_list_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    int32_t i = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_OPENBRACE);
    array_init_size(return_value, i);
    hprose_reader_refer_set(_this->refer, return_value);
    for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
        zval *val;
        MAKE_STD_ZVAL(val);
        hprose_reader_unserialize(_this, val TSRMLS_CC);
        add_next_index_zval(return_value, val);
#else
        zval val;
        hprose_reader_unserialize(_this, &val TSRMLS_CC);
        add_next_index_zval(return_value, &val);
#endif
    }
    hprose_bytes_io_skip(_this->stream, 1);
}

static zend_always_inline void hprose_reader_read_list(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char expected_tags[] = {HPROSE_TAG_NULL,
                            HPROSE_TAG_LIST,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_LIST: {
            hprose_reader_read_list_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            return;
        }
        default: unexpected_tag(tag, expected_tags TSRMLS_CC);
    }
}

static zend_always_inline void hprose_reader_read_map_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    int32_t i = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_OPENBRACE);
    array_init_size(return_value, i);
    hprose_reader_refer_set(_this->refer, return_value);
    for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
        zval *key, *value;
        MAKE_STD_ZVAL(key);
        MAKE_STD_ZVAL(value);
        hprose_reader_unserialize(_this, key TSRMLS_CC);
        hprose_reader_unserialize(_this, value TSRMLS_CC);
        if (Z_TYPE_P(key) == IS_LONG) {
            add_index_zval(return_value, Z_LVAL_P(key), value);
        }
        else {
            convert_to_string(key);
            add_assoc_zval_ex(return_value, Z_STRVAL_P(key), Z_STRLEN_P(key) + 1, value);
        }
        zval_ptr_dtor(&key);
#else
        zval key, value;
        hprose_reader_unserialize(_this, &key TSRMLS_CC);
        hprose_reader_unserialize(_this, &value TSRMLS_CC);
        array_set_zval_key(Z_ARRVAL_P(return_value), &key, &value);
        zval_ptr_dtor(&key);
        zval_ptr_dtor(&value);
#endif
    }
    hprose_bytes_io_skip(_this->stream, 1);
}

static zend_always_inline void hprose_reader_read_map(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char expected_tags[] = {HPROSE_TAG_NULL,
                            HPROSE_TAG_MAP,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_MAP: {
            hprose_reader_read_map_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            return;
        }
        default: unexpected_tag(tag, expected_tags TSRMLS_CC);
    }
}

static zend_always_inline void hprose_reader_read_class(hprose_reader *_this TSRMLS_DC) {
    int32_t i = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_QUOTE);
#if PHP_MAJOR_VERSION < 7
    int32_t alen, nlen;
    char *alias = hprose_bytes_io_read_string(_this->stream, i, &alen);
    char *name = hprose_class_manager_get_class(alias, alen, &nlen);
    efree(alias);
#else
    zend_string *alias = hprose_bytes_io_read_string(_this->stream, i);
    zend_string *name = hprose_class_manager_get_class(alias->val, alias->len);
    zend_string_release(alias);
#endif
    hprose_bytes_io_skip(_this->stream, 1);
    i = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_OPENBRACE);
    do {
#if PHP_MAJOR_VERSION < 7
        zval *props;
        MAKE_STD_ZVAL(props);
        array_init_size(props, i);
        for (; i > 0; --i) {
            zval *prop;
            MAKE_STD_ZVAL(prop);
            _hprose_reader_read_string(_this, prop TSRMLS_CC);
            add_next_index_zval(props, prop);
        }
        add_next_index_stringl(_this->classref, name, nlen, 0);
        add_next_index_zval(_this->propsref, props);
#else
        zval props;
        array_init_size(&props, i);
        for (; i > 0; --i) {
            zval prop;
            _hprose_reader_read_string(_this, &prop TSRMLS_CC);
            add_next_index_zval(&props, &prop);
        }
        add_next_index_str(_this->classref, name);
        add_next_index_zval(_this->propsref, &props);
#endif
    } while(0);
    hprose_bytes_io_skip(_this->stream, 1);
}

static zend_always_inline void hprose_reader_read_object_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    int32_t index = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_OPENBRACE);
    zval *class_name = php_array_get(_this->classref, index);
    zval *props = php_array_get(_this->propsref, index);
    HashTable *props_ht = Z_ARRVAL_P(props);
    int32_t i = zend_hash_num_elements(props_ht);
    zend_class_entry *scope = __create_php_object(Z_STRVAL_P(class_name), Z_STRLEN_P(class_name), return_value TSRMLS_CC, "");
    hprose_reader_refer_set(_this->refer, return_value);
    if (i) {
#if PHP_VERSION_ID < 70100
	zend_class_entry *old_scope = EG(scope);
	EG(scope) = scope;
#else
	zend_class_entry *old_scope = EG(fake_scope);
	EG(fake_scope) = scope;
#endif

        zend_hash_internal_pointer_reset(props_ht);

	if (!Z_OBJ_HT_P(return_value)->write_property) {
            zend_error(E_CORE_ERROR, "Properties of class %s cannot be updated", Z_STRVAL_P(class_name));
	}

        for (; i > 0; --i) {
            char *name;
#if PHP_MAJOR_VERSION < 7
            zval **e, *val;
            zend_hash_get_current_data(props_ht, (void **)&e);
            MAKE_STD_ZVAL(val);
            hprose_reader_unserialize(_this, val TSRMLS_CC);
            name = Z_STRVAL_PP(e);
            name[0] = name[0] - 32;
            if (has_property(scope, return_value, *e)) {
#if PHP_API_VERSION < 20100412
                Z_OBJ_HT_P(return_value)->write_property(return_value, *e, val TSRMLS_CC);
#else
                Z_OBJ_HT_P(return_value)->write_property(return_value, *e, val, NULL TSRMLS_CC);
#endif
                name[0] = name[0] + 32;
            }
            else {
                name[0] = name[0] + 32;
#if PHP_API_VERSION < 20100412
                Z_OBJ_HT_P(return_value)->write_property(return_value, *e, val TSRMLS_CC);
#else
                Z_OBJ_HT_P(return_value)->write_property(return_value, *e, val, NULL TSRMLS_CC);
#endif
            }
#else
            zval *e = zend_hash_get_current_data(props_ht), val, prop;
            hprose_reader_unserialize(_this, &val TSRMLS_CC);
            name = Z_STRVAL_P(e);
            name[0] = name[0] - 32;
            ZVAL_STRINGL(&prop, name, Z_STRLEN_P(e));
            name[0] = name[0] + 32;
            if (has_property(scope, return_value, &prop)) {
                Z_OBJ_HT_P(return_value)->write_property(return_value, &prop, &val, NULL);
            }
            else {
                Z_OBJ_HT_P(return_value)->write_property(return_value, e, &val, NULL);
            }
            zval_ptr_dtor(&prop);
#endif
            zval_ptr_dtor(&val);
            zend_hash_move_forward(props_ht);
        }
#if PHP_VERSION_ID < 70100
	EG(scope) = old_scope;
#else
	EG(fake_scope) = old_scope;
#endif
    }
    hprose_bytes_io_skip(_this->stream, 1);
}

HPROSE_CLASS_BEGIN(reader)
HPROSE_CLASS_END(reader)

END_EXTERN_C()

#endif	/* HPROSE_READER_H */

