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
 * LastModified: Mar 16, 2015                             *
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
} hprose_writer_refer;

static void hprose_fake_writer_refer_set(void *_this, zval *val) {}
static zend_bool hprose_fake_writer_refer_write(void *_this, hprose_bytes_io *stream, zval *val) { return 0; }
static void hprose_fake_writer_refer_reset(void *_this) {}
static void hprose_fake_writer_refer_free(void *_this) { efree(_this); }

static zend_always_inline hprose_writer_refer *hprose_fake_writer_refer_new() {
    hprose_writer_refer *_this = emalloc(sizeof(hprose_writer_refer));
    _this->set = &hprose_fake_writer_refer_set;
    _this->write = &hprose_fake_writer_refer_write;
    _this->reset = &hprose_fake_writer_refer_reset;
    _this->free = &hprose_fake_writer_refer_free;
    return _this;
}

typedef struct {
    hprose_writer_refer_set *set;
    hprose_writer_refer_write *write;
    hprose_writer_refer_reset *reset;
    hprose_writer_refer_free *free;
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
            zend_llist_add_element(refer->ref, val);
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
#if PHP_MAJOR_VERSION < 7
    zval_ptr_dtor(&(refer->sref));
    zval_ptr_dtor(&(refer->oref));
#else /* PHP_MAJOR_VERSION < 7 */
    zval_ptr_dtor(refer->sref);
    zval_ptr_dtor(refer->oref);
#endif /* PHP_MAJOR_VERSION < 7 */
    refer->sref = NULL;
    refer->oref = NULL;
    efree(refer);
}

static zend_always_inline hprose_writer_refer * hprose_real_writer_refer_new() {
    hprose_real_writer_refer *_this = emalloc(sizeof(hprose_real_writer_refer));
    _this->set = &hprose_real_writer_refer_set;
    _this->write = &hprose_real_writer_refer_write;
    _this->reset = &hprose_real_writer_refer_reset;
    _this->free = &hprose_real_writer_refer_free;
    _this->ref = emalloc(sizeof(zend_llist));
    // I don't know if this can work on PHP 7
    zend_llist_init(_this->ref, 32, (void (*)(void *))ZVAL_PTR_DTOR, 0);
#if PHP_MAJOR_VERSION < 7
    MAKE_STD_ZVAL(_this->sref);
    MAKE_STD_ZVAL(_this->oref);
#else /* PHP_MAJOR_VERSION < 7 */
    _this->sref = emalloc(sizeof(zval));
    _this->oref = emalloc(sizeof(zval));
#endif /* PHP_MAJOR_VERSION < 7 */
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

static zend_always_inline hprose_writer * hprose_writer_create(hprose_bytes_io *stream, zend_bool simple) {
    hprose_writer *_this = emalloc(sizeof(hprose_writer));
    _this->stream = stream;
#if PHP_MAJOR_VERSION < 7
    MAKE_STD_ZVAL(_this->classref);
    MAKE_STD_ZVAL(_this->propsref);
#else /* PHP_MAJOR_VERSION < 7 */
    _this->classref = emalloc(sizeof(zval));
    _this->propsref = emalloc(sizeof(zval));
#endif /* PHP_MAJOR_VERSION < 7 */
    array_init(_this->classref);
    array_init(_this->propsref);
    _this->refer = simple ? hprose_fake_writer_refer_new() : hprose_real_writer_refer_new();
    return _this;
}

static zend_always_inline void hprose_writer_free(hprose_writer *_this) {
    _this->stream = NULL;
#if PHP_MAJOR_VERSION < 7
    zval_ptr_dtor(&(_this->classref));
    zval_ptr_dtor(&(_this->propsref));
#else /* PHP_MAJOR_VERSION < 7 */
    zval_ptr_dtor(_this->classref);
    zval_ptr_dtor(_this->propsref);
#endif /* PHP_MAJOR_VERSION < 7 */
    _this->classref = NULL;
    _this->propsref = NULL;
    _this->refer->free(_this->refer);
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

static zend_always_inline void hprose_writer_write_array(hprose_writer *_this, zval *a) {

}

static zend_always_inline void hprose_writer_write_assoc_array(hprose_writer *_this, zval *aa) {

}
static zend_always_inline void hprose_writer_write_datetime(hprose_writer *_this, zval *dt TSRMLS_DC) {
    _this->refer->set(_this->refer, dt);
    zval tmp, fmt;
    zval *params = &fmt;
    call_php_function(dt, "getOffset", &tmp, 0, NULL);
    if (Z_LVAL(tmp) == 0) {
        ZVAL_LITERAL_STRINGL(&fmt, "\\DYmd\\THis.u\\Z");
    }
    else {
        ZVAL_LITERAL_STRINGL(&fmt, "\\DYmd\\THis.u;");
    }
    call_php_function(dt, "format", &tmp, 1, &params);
    hprose_bytes_io_write(_this->stream, Z_STRVAL(tmp), Z_STRLEN(tmp));
}
static zend_always_inline void hprose_writer_write_datetime_with_ref(hprose_writer *_this, zval *dt TSRMLS_DC) {
    if (!(_this->refer->write(_this->refer, _this->stream, dt))) hprose_writer_write_datetime(_this, dt TSRMLS_CC);
}
static zend_always_inline void hprose_writer_write_map(hprose_writer *_this, zval *obj) {

}
static zend_always_inline void hprose_writer_write_map_with_ref(hprose_writer *_this, zval *obj) {

}
static zend_always_inline void hprose_writer_write_list(hprose_writer *_this, zval *obj) {

}
static zend_always_inline void hprose_writer_write_list_with_ref(hprose_writer *_this, zval *obj) {

}
static zend_always_inline void hprose_writer_write_stdclass(hprose_writer *_this, zval *obj) {

}
static zend_always_inline void hprose_writer_write_stdclass_with_ref(hprose_writer *_this, zval *obj) {

}
static zend_always_inline void hprose_writer_write_object(hprose_writer *_this, zval *obj) {

}
static zend_always_inline void hprose_writer_write_object_with_ref(hprose_writer *_this, zval *obj) {

}
static zend_always_inline void hprose_writer_write_utf8char(hprose_writer *_this, const char *str, int32_t len) {

}
static zend_always_inline void hprose_writer_write_string(hprose_writer *_this, const char *str, int32_t len) {

}
static zend_always_inline void hprose_writer_write_string_with_ref(hprose_writer *_this, const char *str, int32_t len) {

}
static zend_always_inline void hprose_writer_write_bytes(hprose_writer *_this, const char *str, int32_t len) {

}
static zend_always_inline void hprose_writer_write_bytes_with_ref(hprose_writer *_this, const char *str, int32_t len) {

}
static zend_always_inline void hprose_writer_serialize(hprose_writer *_this, zval *val TSRMLS_DC) {
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
                hprose_writer_write_array(_this, val);
            }
            else {
                hprose_writer_write_assoc_array(_this, val);
            }
            break;
        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(val);
            if (instanceof(ce, "DateTime")) {
                hprose_writer_write_datetime_with_ref(_this, val TSRMLS_CC);
            }
            else if (instanceof(ce, "SplObjectStorage")) {
                hprose_writer_write_map_with_ref(_this, val);
            }
            else if (instanceof(ce, "Traversable")) {
                hprose_writer_write_list_with_ref(_this, val);
            }
            else if (instanceof(ce, "stdClass")) {
                hprose_writer_write_stdclass_with_ref(_this, val);
            }
            else {
                hprose_writer_write_object_with_ref(_this, val);
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
                    hprose_writer_write_utf8char(_this, s, l);
                }
                else {
                    hprose_writer_write_string_with_ref(_this, s, l);
                }
            }
            else {
                hprose_writer_write_bytes_with_ref(_this, s, l);
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

