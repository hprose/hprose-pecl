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
 * LastModified: Apr 2, 2015                              *
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

/* Workaround for old gcc. */
#ifndef NAN
#define NAN (0.0/0.0)
#endif
#ifndef INFINITY
#define INFINITY (1.0/0.0)
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

static inline void hprose_reader_unserialize(hprose_reader *_this, zval *return_value TSRMLS_DC);

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

static zend_always_inline long hprose_reader_read_integer_without_tag(hprose_reader *_this) {
    return hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_SEMICOLON);
}

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

static zend_always_inline char * hprose_reader_read_long_without_tag(hprose_reader *_this, int32_t *len_ptr) {
    return hprose_bytes_io_readuntil(_this->stream, HPROSE_TAG_SEMICOLON, len_ptr);
}

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

static zend_always_inline void hprose_reader_read_datetime_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char tag;
    hprose_bytes_io *tmp = hprose_bytes_io_new();
    hprose_bytes_io_read_to(_this->stream, tmp, 4);
    hprose_bytes_io_putc(tmp, '-');
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    hprose_bytes_io_putc(tmp, '-');
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_TIME) {
        hprose_bytes_io_putc(tmp, tag);
        hprose_bytes_io_read_to(_this->stream, tmp, 2);
        hprose_bytes_io_putc(tmp, ':');
        hprose_bytes_io_read_to(_this->stream, tmp, 2);
        hprose_bytes_io_putc(tmp, ':');
        hprose_bytes_io_read_to(_this->stream, tmp, 2);
        tag = hprose_bytes_io_getc(_this->stream);
        if (tag == HPROSE_TAG_POINT) {
            hprose_bytes_io_putc(tmp, tag);
            hprose_bytes_io_read_to(_this->stream, tmp, 3);
            tag = hprose_bytes_io_getc(_this->stream);
            if ((tag >= '0') && (tag <= '9')) {
                hprose_bytes_io_putc(tmp, tag);
                hprose_bytes_io_read_to(_this->stream, tmp, 2);
                tag = hprose_bytes_io_getc(_this->stream);
                if ((tag >= '0') && (tag <= '9')) {
                    hprose_bytes_io_skip(_this->stream, 2);
                    tag = hprose_bytes_io_getc(_this->stream);
                }
            }
        }
    }
    if (tag == HPROSE_TAG_UTC) {
        zval timezone;
        function_invoke(timezone_open, &timezone, "s", ZEND_STRL("UTC"));
        function_invoke(date_create, return_value, "sz", tmp->buf, tmp->len, &timezone);
    }
    else {
        function_invoke(date_create, return_value, "s", tmp->buf, tmp->len);
    }
    hprose_bytes_io_free(tmp);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void hprose_reader_read_time_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char tag;
    hprose_bytes_io *tmp = hprose_bytes_io_new();
    hprose_bytes_io_write(tmp, "1970-01-01", 10);
    hprose_bytes_io_putc(tmp, HPROSE_TAG_TIME);
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    hprose_bytes_io_putc(tmp, ':');
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    hprose_bytes_io_putc(tmp, ':');
    hprose_bytes_io_read_to(_this->stream, tmp, 2);
    tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_POINT) {
        hprose_bytes_io_putc(tmp, tag);
        hprose_bytes_io_read_to(_this->stream, tmp, 3);
        tag = hprose_bytes_io_getc(_this->stream);
        if ((tag >= '0') && (tag <= '9')) {
            hprose_bytes_io_putc(tmp, tag);
            hprose_bytes_io_read_to(_this->stream, tmp, 2);
            tag = hprose_bytes_io_getc(_this->stream);
            if ((tag >= '0') && (tag <= '9')) {
                hprose_bytes_io_skip(_this->stream, 2);
                tag = hprose_bytes_io_getc(_this->stream);
            }
        }
    }
    if (tag == HPROSE_TAG_UTC) {
        zval timezone;
        function_invoke(timezone_open, &timezone, "s", ZEND_STRL("UTC"));
        function_invoke(date_create, return_value, "sz", tmp->buf, tmp->len, &timezone);
    }
    else {
        function_invoke(date_create, return_value, "s", tmp->buf, tmp->len);
    }
    hprose_bytes_io_free(tmp);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void hprose_reader_read_bytes_without_tag(hprose_reader *_this, zval *return_value) {
    int32_t count = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_QUOTE);
    char *bytes = hprose_bytes_io_read(_this->stream, count);
    hprose_bytes_io_skip(_this->stream, 1);
    RETVAL_STRINGL_0(bytes, count);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void hprose_reader_read_utf8char_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    int32_t len;
    char *uc = hprose_bytes_io_read_string(_this->stream, 1, &len);
    RETVAL_STRINGL_0(uc, len);
}

static zend_always_inline void _hprose_reader_read_string_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    int32_t count = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_QUOTE);
    int32_t len;
    char *str = hprose_bytes_io_read_string(_this->stream, count, &len);
    hprose_bytes_io_skip(_this->stream, 1);
    RETVAL_STRINGL_0(str, len);
}

static zend_always_inline void hprose_reader_read_string_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    _hprose_reader_read_string_without_tag(_this, return_value TSRMLS_CC);
    hprose_reader_refer_set(_this->refer, return_value);
}

static zend_always_inline void _hprose_reader_read_string(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char expected_tags[] = {HPROSE_TAG_UTF8CHAR,
                            HPROSE_TAG_STRING,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_UTF8CHAR: {
            hprose_reader_read_utf8char_without_tag(_this, return_value TSRMLS_CC);
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
            hprose_reader_read_utf8char_without_tag(_this, return_value TSRMLS_CC);
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

static zend_always_inline void hprose_reader_read_guid_without_tag(hprose_reader *_this, zval *return_value) {
    hprose_bytes_io_skip(_this->stream, 1);
    char *s = hprose_bytes_io_read(_this->stream, 36);
    hprose_bytes_io_skip(_this->stream, 1);
    RETVAL_STRINGL_0(s, 36);
    hprose_reader_refer_set(_this->refer, return_value);
}

static inline void hprose_reader_read_list_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
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

static inline void hprose_reader_read_map_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
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
    int32_t alen, nlen;
    char *alias = hprose_bytes_io_read_string(_this->stream, i, &alen);
    char *name = hprose_class_manager_get_class(alias, alen, &nlen);
    efree(alias);
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
        add_next_index_stringl(_this->classref, name, nlen);
        efree(name);
        add_next_index_zval(_this->propsref, &props);
#endif
    } while(0);
    hprose_bytes_io_skip(_this->stream, 1);
}

static inline void hprose_reader_read_object_without_tag(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    int32_t index = hprose_bytes_io_read_int(_this->stream, HPROSE_TAG_OPENBRACE);
    zval *class_name = php_array_get(_this->classref, index);
    zval *props = php_array_get(_this->propsref, index);
    HashTable *props_ht = Z_ARRVAL_P(props);
    int32_t i = zend_hash_num_elements(props_ht);
    zend_class_entry *entry = __create_php_object(Z_STRVAL_P(class_name), Z_STRLEN_P(class_name), return_value TSRMLS_CC, "");
    hprose_reader_refer_set(_this->refer, return_value);
    if (i) {
        zend_hash_internal_pointer_reset(props_ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **e, *val;
            zend_hash_get_current_data(props_ht, (void **)&e);
            MAKE_STD_ZVAL(val);
            hprose_reader_unserialize(_this, val TSRMLS_CC);
            zend_update_property(entry, return_value, Z_STRVAL_PP(e), Z_STRLEN_PP(e), val TSRMLS_CC);
#else
            zval *e = zend_hash_get_current_data(props_ht), val;
            hprose_reader_unserialize(_this, &val TSRMLS_CC);
            zend_update_property(entry, return_value, Z_STRVAL_P(e), Z_STRLEN_P(e), &val TSRMLS_CC);
#endif
            zval_ptr_dtor(&val);
            zend_hash_move_forward(props_ht);
        }
    }
    hprose_bytes_io_skip(_this->stream, 1);
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
            hprose_reader_read_list_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_MAP: {
            hprose_reader_read_map_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_CLASS: {
            hprose_reader_read_class(_this TSRMLS_CC);
            hprose_reader_unserialize(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_OBJECT: {
            hprose_reader_read_object_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_ERROR: {
            _hprose_reader_read_string(_this, return_value TSRMLS_CC);
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                                    "%s", Z_STRVAL_P(return_value));
            return;
        }
        default: unexpected_tag(tag, NULL TSRMLS_CC);
    }
}

HPROSE_CLASS_BEGIN(reader)
HPROSE_CLASS_END(reader)

END_EXTERN_C()

#endif	/* HPROSE_READER_H */

