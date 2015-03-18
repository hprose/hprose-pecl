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
 * hprose_writer.h                                        *
 *                                                        *
 * hprose writer for pecl header file.                    *
 *                                                        *
 * LastModified: Mar 18, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_WRITER_H
#define	HPROSE_WRITER_H

#include "hprose.h"
#include "hprose_tags.h"
#include "hprose_bytes_io.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_writer_ce();

HPROSE_STARTUP_FUNCTION(writer);

typedef void hprose_writer_refer_set(void *_this, zval *val);
typedef zend_bool hprose_writer_refer_write(void *_this, hprose_bytes_io *stream, zval *val);
typedef void hprose_writer_refer_reset(void *_this);
typedef void hprose_writer_refer_free(void *_this);

typedef struct {
    hprose_writer_refer_set *set;
    hprose_writer_refer_write *write;
    hprose_writer_refer_reset *reset;
    hprose_writer_refer_free *free;
} hprose_writer_refer_handlers;

typedef struct {
    hprose_writer_refer_handlers *handlers;
} hprose_writer_refer;

static void hprose_fake_writer_refer_set(void *_this, zval *val) {}
static zend_bool hprose_fake_writer_refer_write(void *_this, hprose_bytes_io *stream, zval *val) { return 0; }
static void hprose_fake_writer_refer_reset(void *_this) {}
static void hprose_fake_writer_refer_free(void *_this) { efree(_this); }

static hprose_writer_refer_handlers __hprose_fake_writer_refer = {
    hprose_fake_writer_refer_set,
    hprose_fake_writer_refer_write,
    hprose_fake_writer_refer_reset,
    hprose_fake_writer_refer_free
};

static zend_always_inline hprose_writer_refer *hprose_fake_writer_refer_new() {
    hprose_writer_refer *_this = emalloc(sizeof(hprose_writer_refer));
    _this->handlers = &__hprose_fake_writer_refer;
    return _this;
}

typedef struct {
    hprose_writer_refer_handlers *handlers;
    zend_llist *ref;
    zval *sref;
    zval *oref;
    int refcount;
} hprose_real_writer_refer;

static zend_always_inline void hprose_real_writer_refer_write_ref(hprose_bytes_io *stream, int32_t index) {
    hprose_bytes_io_write_char(stream, HPROSE_TAG_REF);
    hprose_bytes_io_write_int(stream, index);
    hprose_bytes_io_write_char(stream, HPROSE_TAG_SEMICOLON);
}

static void hprose_real_writer_refer_set(void *_this, zval *val) {
    hprose_real_writer_refer *refer = (hprose_real_writer_refer *)_this;
    char *key;
    switch (Z_TYPE_P(val)) {
        case IS_STRING:
            add_assoc_long_ex(refer->sref, Z_STRVAL_P(val), Z_STRLEN_P(val), refer->refcount);
            break;
        case IS_OBJECT:
            Z_ADDREF_P(val);
            zend_llist_add_element(refer->ref, &val);
            key = object_hash(val);
            add_assoc_long_ex(refer->oref, key, 32, refer->refcount);
            efree(key);
            break;
    }
    ++(refer->refcount);
}
static zend_bool hprose_real_writer_refer_write(void *_this, hprose_bytes_io *stream, zval *val) {
    hprose_real_writer_refer *refer = (hprose_real_writer_refer *)_this;
    long index;
    char *key;
    switch (Z_TYPE_P(val)) {
        case IS_STRING:
            if (php_assoc_array_get_long(refer->sref, Z_STRVAL_P(val), Z_STRLEN_P(val), &index)) {
                hprose_real_writer_refer_write_ref(stream, (int32_t)index);
                return 1;
            }
            break;
        case IS_OBJECT:
            key = object_hash(val);
            if (php_assoc_array_get_long(refer->oref, key, 32, &index)) {
                hprose_real_writer_refer_write_ref(stream, (int32_t)index);
                efree(key);
                return 1;
            }
            efree(key);
            break;
    }
    return 0;
}

static void hprose_real_writer_refer_reset(void *_this) {
    hprose_real_writer_refer *refer = (hprose_real_writer_refer *)_this;
    zend_llist_clean(refer->ref);
    zend_hash_clean(Z_ARRVAL_P(refer->sref));
    zend_hash_clean(Z_ARRVAL_P(refer->oref));
    refer->refcount = 0;
}

static void hprose_real_writer_refer_free(void *_this) {
    hprose_real_writer_refer *refer = (hprose_real_writer_refer *)_this;
    zend_llist_destroy(refer->ref);
    efree(refer->ref);
    refer->ref = NULL;
    hprose_zval_free(refer->sref);
    hprose_zval_free(refer->oref);
    refer->sref = NULL;
    refer->oref = NULL;
    efree(refer);
}

static hprose_writer_refer_handlers __hprose_real_writer_refer = {
    hprose_real_writer_refer_set,
    hprose_real_writer_refer_write,
    hprose_real_writer_refer_reset,
    hprose_real_writer_refer_free
};

static void __hprose_writer_refer_dtor(void *data) {
#if PHP_MAJOR_VERSION < 7
        zval **val = (zval **)data;
#else
        zval *val = (zval *)data;
#endif
    if (data) {
        zval_ptr_dtor(val);
    }
}

static zend_always_inline hprose_writer_refer * hprose_real_writer_refer_new() {
    hprose_real_writer_refer *_this = emalloc(sizeof(hprose_real_writer_refer));
    _this->handlers = &__hprose_real_writer_refer;
    _this->ref = emalloc(sizeof(zend_llist));
    zend_llist_init(_this->ref, sizeof(zval *), (llist_dtor_func_t)__hprose_writer_refer_dtor, 0);
    hprose_make_zval(_this->sref);
    hprose_make_zval(_this->oref);
    array_init(_this->sref);
    array_init(_this->oref);
    _this->refcount = 0;
    return (hprose_writer_refer *)(void *)_this;
}

typedef struct {
    hprose_bytes_io *stream;
    zval *classref;
    zval *propsref;
    hprose_writer_refer *refer;
} hprose_writer;

static inline void hprose_writer_serialize(hprose_writer *_this, zval *val TSRMLS_DC);

static zend_always_inline hprose_writer * hprose_writer_create(hprose_bytes_io *stream, zend_bool simple) {
    hprose_writer *_this = emalloc(sizeof(hprose_writer));
    _this->stream = stream;
    hprose_make_zval(_this->classref);
    hprose_make_zval(_this->propsref);
    array_init(_this->classref);
    array_init(_this->propsref);
    _this->refer = simple ? hprose_fake_writer_refer_new() : hprose_real_writer_refer_new();
    return _this;
}

static zend_always_inline void hprose_writer_free(hprose_writer *_this) {
    _this->stream = NULL;
    hprose_zval_free(_this->classref);
    hprose_zval_free(_this->propsref);
    _this->classref = NULL;
    _this->propsref = NULL;
    _this->refer->handlers->free(_this->refer);
    _this->refer = NULL;
    efree(_this);
}

#define hprose_writer_write_null(_this) hprose_bytes_io_write_char((_this)->stream, HPROSE_TAG_NULL)
#define hprose_writer_write_true(_this) hprose_bytes_io_write_char((_this)->stream, HPROSE_TAG_TRUE)
#define hprose_writer_write_false(_this) hprose_bytes_io_write_char((_this)->stream, HPROSE_TAG_FALSE)
#define hprose_writer_write_bool(_this, val) ((val) ? hprose_writer_write_true(_this) : hprose_writer_write_false(_this))
#define hprose_writer_write_empty(_this) hprose_bytes_io_write_char((_this)->stream, HPROSE_TAG_EMPTY)

static zend_always_inline void hprose_writer_write_int(hprose_writer *_this, int32_t i) {
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_INTEGER);
    hprose_bytes_io_write_int(_this->stream, i);
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_SEMICOLON);
}

static zend_always_inline void hprose_writer_write_long(hprose_writer *_this, int64_t i) {
    if (i >= 0 && i <= 9) {
        hprose_bytes_io_write_char(_this->stream, '0' + i);
    }
    else {
        hprose_bytes_io_write_char(_this->stream, (i > INT32_MAX || i < INT32_MIN) ? HPROSE_TAG_LONG : HPROSE_TAG_INTEGER);
        hprose_bytes_io_write_long(_this->stream, i);
        hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_SEMICOLON);
    }
}

static zend_always_inline void hprose_writer_write_infinity(hprose_writer *_this, zend_bool positive) {
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_INFINITY);
    hprose_bytes_io_write_char(_this->stream, (positive ? HPROSE_TAG_POS : HPROSE_TAG_NEG));
}

static zend_always_inline void hprose_writer_write_double(hprose_writer *_this, double d) {
    if (isnan(d)) {
        hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_NAN);
    }
    else if (isinf(d)) {
        hprose_writer_write_infinity(_this, d > 0);
    }
    else {
        hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_DOUBLE);
        hprose_bytes_io_write_double(_this->stream, d);
        hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_SEMICOLON);
    }
}

static zend_always_inline void hprose_writer_write_utf8char(hprose_writer *_this, zval *val) {
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_UTF8CHAR);
    hprose_bytes_io_write(_this->stream, Z_STRVAL_P(val), Z_STRLEN_P(val));
}

static zend_always_inline void hprose_writer_write_string(hprose_writer *_this, zval *val) {
    int32_t len = ustrlen(Z_STRVAL_P(val), Z_STRLEN_P(val));
    _this->refer->handlers->set(_this->refer, val);
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_STRING);
    if (len) {
        hprose_bytes_io_write_int(_this->stream, len);
    }
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_QUOTE);
    hprose_bytes_io_write(_this->stream, Z_STRVAL_P(val), Z_STRLEN_P(val));
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_QUOTE);
}

static zend_always_inline void hprose_writer_write_string_with_ref(hprose_writer *_this, zval *val) {
    if (!(_this->refer->handlers->write(_this->refer, _this->stream, val))) hprose_writer_write_string(_this, val);
}

static zend_always_inline void hprose_writer_write_bytes(hprose_writer *_this, zval *val) {
    int32_t len = Z_STRLEN_P(val);
    _this->refer->handlers->set(_this->refer, val);
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_BYTES);
    if (len) {
        hprose_bytes_io_write_int(_this->stream, len);
    }
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_QUOTE);
    hprose_bytes_io_write(_this->stream, Z_STRVAL_P(val), len);
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_QUOTE);
}

static zend_always_inline void hprose_writer_write_bytes_with_ref(hprose_writer *_this, zval *val) {
    if (!(_this->refer->handlers->write(_this->refer, _this->stream, val))) hprose_writer_write_bytes(_this, val);
}

static zend_always_inline void hprose_writer_write_datetime(hprose_writer *_this, zval *val TSRMLS_DC) {
    zval tmp, fmt;
    zval *params[] = { &fmt, NULL };
    _this->refer->handlers->set(_this->refer, val);
    call_php_method(val, "getOffset", &tmp, 0, NULL);
    if (Z_LVAL(tmp) == 0) {
        ZVAL_LITERAL_STRINGL(&fmt, "\\DYmd\\THis.u\\Z");
    }
    else {
        ZVAL_LITERAL_STRINGL(&fmt, "\\DYmd\\THis.u;");
    }
    call_php_method(val, "format", &tmp, 1, &params[0]);
    hprose_bytes_io_write(_this->stream, Z_STRVAL(tmp), Z_STRLEN(tmp));
#if PHP_MAJOR_VERSION < 7
    STR_FREE(Z_STRVAL(tmp));
#else
    zval_ptr_dtor(&fmt);
    zval_ptr_dtor(&tmp);
#endif
}

static zend_always_inline void hprose_writer_write_datetime_with_ref(hprose_writer *_this, zval *val TSRMLS_DC) {
    if (!(_this->refer->handlers->write(_this->refer, _this->stream, val))) hprose_writer_write_datetime(_this, val TSRMLS_CC);
}

static inline void hprose_writer_write_array(hprose_writer *_this, zval *val TSRMLS_DC) {
    HashTable *ht = Z_ARRVAL_P(val);
    int32_t i = zend_hash_num_elements(ht);
    _this->refer->handlers->set(_this->refer, val);
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_LIST);
    if (i) {
        hprose_bytes_io_write_int(_this->stream, i);
    }
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_OPENBRACE);
    if (i) {
        zend_hash_internal_pointer_reset(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **e;
            zend_hash_get_current_data(ht, (void **)&e);
            hprose_writer_serialize(_this, *e TSRMLS_CC);
#else
            zval *e = zend_hash_get_current_data(ht);
            hprose_writer_serialize(_this, e TSRMLS_CC);
#endif
            zend_hash_move_forward(ht);

        }
    }
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_CLOSEBRACE);
}

static inline void hprose_writer_write_assoc_array(hprose_writer *_this, zval *val TSRMLS_DC) {
    HashTable *ht = Z_ARRVAL_P(val);
    int32_t i = zend_hash_num_elements(ht);
    _this->refer->handlers->set(_this->refer, val);
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_MAP);
    if (i) {
        hprose_bytes_io_write_int(_this->stream, i);
    }
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_OPENBRACE);
    if (i) {
        zend_hash_internal_pointer_reset(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            char *str;
            uint len;
            ulong index;
            zval **value;
            if (zend_hash_get_current_key_ex(ht, &str, &len, &index, 0, NULL) == HASH_KEY_IS_STRING) {
                zval key;
                ZVAL_STRINGL(&key, str, len - 1, 0);
                hprose_writer_write_string_with_ref(_this, &key);
            }
            else {
                hprose_writer_write_int(_this, (int32_t)index);
            }
            zend_hash_get_current_data(ht, (void **)&value);
            hprose_writer_serialize(_this, *value TSRMLS_CC);
#else
            zval key, *value;
            zend_hash_get_current_key_zval(ht, &key);
            value = zend_hash_get_current_data(ht);
            hprose_writer_serialize(_this, &key TSRMLS_CC);
            hprose_writer_serialize(_this, value TSRMLS_CC);
            zval_ptr_dtor(&key);
#endif
            zend_hash_move_forward(ht);

        }
    }
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_CLOSEBRACE);
}

static inline void hprose_writer_write_map(hprose_writer *_this, zval *val TSRMLS_DC) {
    zval count;
    int32_t i;
    _this->refer->handlers->set(_this->refer, val);
    call_php_method(val, "count", &count, 0, NULL);
    i = Z_LVAL(count);
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_MAP);
    if (i) {
        hprose_bytes_io_write_int(_this->stream, i);
    }
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_OPENBRACE);
    if (i) {
        zval tmp;
        call_php_method(val, "rewind", &tmp, 0, NULL);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval *key, *value;
            hprose_make_zval(key);
            hprose_make_zval(value);
            call_php_method(val, "current", key, 0, NULL);
            call_php_method(val, "offsetGet", value, 1, &key);
            hprose_writer_serialize(_this, key TSRMLS_CC);
            hprose_writer_serialize(_this, value TSRMLS_CC);
            hprose_zval_free(key);
            hprose_zval_free(value);
#else /* PHP_MAJOR_VERSION < 7 */
/*
   The above code can also work well in PHP 7.
   The following code is only for the purpose of optimization.
 */
            zval key, value;
            zval *params[] = { &key, NULL };
            call_php_method(val, "current", &key, 0, NULL);
            call_php_method(val, "offsetGet", &value, 1, &params[0]);
            hprose_writer_serialize(_this, &key TSRMLS_CC);
            hprose_writer_serialize(_this, &value TSRMLS_CC);
            zval_ptr_dtor(&key);
            zval_ptr_dtor(&value);
#endif /* PHP_MAJOR_VERSION < 7 */
            call_php_method(val, "next", &tmp, 0, NULL);
        }
    }
    hprose_bytes_io_write_char(_this->stream, HPROSE_TAG_CLOSEBRACE);
}

static inline void hprose_writer_write_map_with_ref(hprose_writer *_this, zval *val TSRMLS_DC) {
    if (!(_this->refer->handlers->write(_this->refer, _this->stream, val))) hprose_writer_write_map(_this, val TSRMLS_CC);
}
static inline void hprose_writer_write_list(hprose_writer *_this, zval *val TSRMLS_DC) {

}
static inline void hprose_writer_write_list_with_ref(hprose_writer *_this, zval *val TSRMLS_DC) {
    if (!(_this->refer->handlers->write(_this->refer, _this->stream, val))) hprose_writer_write_list(_this, val TSRMLS_CC);
}
static inline void hprose_writer_write_stdclass(hprose_writer *_this, zval *val TSRMLS_DC) {

}
static inline void hprose_writer_write_stdclass_with_ref(hprose_writer *_this, zval *val TSRMLS_DC) {
    if (!(_this->refer->handlers->write(_this->refer, _this->stream, val))) hprose_writer_write_stdclass(_this, val TSRMLS_CC);
}
static inline void hprose_writer_write_object(hprose_writer *_this, zval *val TSRMLS_DC) {

}
static inline void hprose_writer_write_object_with_ref(hprose_writer *_this, zval *val TSRMLS_DC) {
    if (!(_this->refer->handlers->write(_this->refer, _this->stream, val))) hprose_writer_write_object(_this, val TSRMLS_CC);
}
static inline void hprose_writer_serialize(hprose_writer *_this, zval *val TSRMLS_DC) {
    switch (Z_TYPE_P(val)) {
        case IS_NULL:
            hprose_writer_write_null(_this); break;
        case IS_LONG:
            hprose_writer_write_long(_this, Z_LVAL_P(val)); break;
        case IS_DOUBLE:
            hprose_writer_write_double(_this, Z_DVAL_P(val)); break;
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
            hprose_writer_write_bool(_this, Z_BVAL_P(val)); break;
#else /* PHP_MAJOR_VERSION < 7 */
        case IS_TRUE:
            hprose_writer_write_true(_this); break;
        case IS_FALSE:
            hprose_writer_write_false(_this); break;
#endif /* PHP_MAJOR_VERSION < 7 */
        case IS_ARRAY:
            if (is_list(val)) {
                hprose_writer_write_array(_this, val TSRMLS_CC);
            }
            else {
                hprose_writer_write_assoc_array(_this, val TSRMLS_CC);
            }
            break;
        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(val);
            if (instanceof(ce, "DateTime")) {
                hprose_writer_write_datetime_with_ref(_this, val TSRMLS_CC);
            }
            else if (instanceof(ce, "SplObjectStorage")) {
                hprose_writer_write_map_with_ref(_this, val TSRMLS_CC);
            }
            else if (instanceof(ce, "Traversable")) {
                hprose_writer_write_list_with_ref(_this, val TSRMLS_CC);
            }
            else if (instanceof(ce, "stdClass")) {
                hprose_writer_write_stdclass_with_ref(_this, val TSRMLS_CC);
            }
            else {
                hprose_writer_write_object_with_ref(_this, val TSRMLS_CC);
            }
            break;
        }
        case IS_STRING: {
            char * s = Z_STRVAL_P(val);
            int32_t l = Z_STRLEN_P(val);
            if (l == 0) {
                hprose_writer_write_empty(_this);
            }
            else if (is_utf8(s, l)) {
                if (l < 4 && ustrlen(s, l) == 1) {
                    hprose_writer_write_utf8char(_this, val);
                }
                else {
                    hprose_writer_write_string_with_ref(_this, val);
                }
            }
            else {
                hprose_writer_write_bytes_with_ref(_this, val);
            }
            break;
        }
        default:
            zend_throw_exception(zend_exception_get_default(TSRMLS_C),
                    "Not support to serialize this data", 0 TSRMLS_CC);
            break;
    }
}

HPROSE_CLASS_BEGIN(writer)
HPROSE_CLASS_END(writer)

END_EXTERN_C()

#endif	/* HPROSE_WRITER_H */

