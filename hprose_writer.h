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
 * LastModified: Jan 5, 2016                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_WRITER_H
#define	HPROSE_WRITER_H

#include "hprose_common.h"
#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_class_manager.h"
#include "zend_interfaces.h"
#include "ext/spl/spl_observer.h"
#if PHP_API_VERSION >= 20090626
#include "ext/date/php_date.h"
#endif

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_writer_ce();

HPROSE_STARTUP_FUNCTION(writer);

typedef struct {
    zend_llist *ref;
    zval *sref;
    zval *oref;
    int32_t refcount;
} hprose_writer_refer;

static zend_always_inline void hprose_writer_refer_write_ref(hprose_bytes_io *stream, int32_t index) {
    hprose_bytes_io_putc(stream, HPROSE_TAG_REF);
    hprose_bytes_io_write_int(stream, index);
    hprose_bytes_io_putc(stream, HPROSE_TAG_SEMICOLON);
}

static void hprose_writer_refer_set(hprose_writer_refer *refer, zval *val) {
    switch (Z_TYPE_P(val)) {
        case IS_STRING:
            add_assoc_long_ex(refer->sref, Z_STRVAL_P(val), Z_STRLEN_P(val), refer->refcount);
            break;
        case IS_OBJECT: {
            ulong h;
#if PHP_MAJOR_VERSION < 7
            Z_ADDREF_P(val);
            SEPARATE_ZVAL(&val);
            zend_llist_add_element(refer->ref, &val);
            h = (ulong)Z_OBJ_HANDLE_P(val);
#else
            Z_ADDREF_P(val);
            zend_llist_add_element(refer->ref, &(Z_OBJ_P(val)));
            h = (ulong)Z_OBJ_P(val);
#endif
            add_index_long(refer->oref, h, refer->refcount);
            break;
        }
    }
    ++(refer->refcount);
}
static zend_bool hprose_writer_refer_write(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val) {
    long index;
    switch (Z_TYPE_P(val)) {
        case IS_STRING:
            if (php_assoc_array_get_long(refer->sref, Z_STRVAL_P(val), Z_STRLEN_P(val), &index)) {
                hprose_writer_refer_write_ref(stream, (int32_t)index);
                return 1;
            }
            break;
        case IS_OBJECT: {
#if PHP_MAJOR_VERSION < 7
            ulong h = (ulong)Z_OBJ_HANDLE_P(val);
#else
            ulong h = (ulong)Z_OBJ_P(val);
#endif
            if (php_array_get_long(refer->oref, h, &index)) {
                hprose_writer_refer_write_ref(stream, (int32_t)index);
                return 1;
            }
            break;
        }
    }
    return 0;
}

static zend_always_inline void hprose_writer_refer_reset(hprose_writer_refer *refer) {
    zend_llist_clean(refer->ref);
    zend_hash_clean(Z_ARRVAL_P(refer->sref));
    zend_hash_clean(Z_ARRVAL_P(refer->oref));
    refer->refcount = 0;
}

static void hprose_writer_refer_free(hprose_writer_refer *refer) {
    zend_llist_destroy(refer->ref);
    efree(refer->ref);
    refer->ref = NULL;
    hprose_zval_free(refer->sref);
    hprose_zval_free(refer->oref);
    refer->sref = NULL;
    refer->oref = NULL;
    efree(refer);
}

static void __hprose_writer_refer_dtor(void *data) {
#if PHP_MAJOR_VERSION < 7
        zval **val = (zval **)data;
        if (data) {
            zval_ptr_dtor(val);
        }
#else
        zend_object **obj = (zend_object **)data;
        if (data) {
            OBJ_RELEASE(*obj);
        }
#endif
}

static hprose_writer_refer * hprose_writer_refer_new() {
    hprose_writer_refer *_this = emalloc(sizeof(hprose_writer_refer));
    _this->ref = emalloc(sizeof(zend_llist));
#if PHP_MAJOR_VERSION < 7
    zend_llist_init(_this->ref, sizeof(zval *), (llist_dtor_func_t)__hprose_writer_refer_dtor, 0);
#else
    zend_llist_init(_this->ref, sizeof(zend_object *), (llist_dtor_func_t)__hprose_writer_refer_dtor, 0);
#endif
    hprose_zval_new(_this->sref);
    hprose_zval_new(_this->oref);
    array_init(_this->sref);
    array_init(_this->oref);
    _this->refcount = 0;
    return _this;
}

typedef struct {
    hprose_bytes_io *stream;
    zval *classref;
    zval *propsref;
    hprose_writer_refer *refer;
} hprose_writer;

PHP_HPROSE_API void _hprose_writer_serialize(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC);
#define hprose_writer_serialize(_this, val) _hprose_writer_serialize((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void hprose_writer_init(hprose_writer *_this, hprose_bytes_io *stream, zend_bool simple) {
    _this->stream = stream;
    hprose_zval_new(_this->classref);
    hprose_zval_new(_this->propsref);
    array_init(_this->classref);
    array_init(_this->propsref);
    _this->refer = simple ? NULL : hprose_writer_refer_new();
}

static zend_always_inline hprose_writer * hprose_writer_create(hprose_bytes_io *stream, zend_bool simple) {
    hprose_writer *_this = emalloc(sizeof(hprose_writer));
    hprose_writer_init(_this, stream, simple);
    return _this;
}

static zend_always_inline void hprose_writer_destroy(hprose_writer *_this) {
    _this->stream = NULL;
    hprose_zval_free(_this->classref);
    hprose_zval_free(_this->propsref);
    _this->classref = NULL;
    _this->propsref = NULL;
    if (_this->refer) {
        hprose_writer_refer_free(_this->refer);
        _this->refer = NULL;
    }
}

static zend_always_inline void hprose_writer_free(hprose_writer *_this) {
    hprose_writer_destroy(_this);
    efree(_this);
}

static zend_always_inline void hprose_writer_reset(hprose_writer *_this) {
    zend_hash_clean(Z_ARRVAL_P(_this->classref));
    zend_hash_clean(Z_ARRVAL_P(_this->propsref));
    if (_this->refer) {
        hprose_writer_refer_reset(_this->refer);
    }
}

#define _hprose_writer_write_nan(stream) hprose_bytes_io_putc((stream), HPROSE_TAG_NAN)
#define _hprose_writer_write_null(stream) hprose_bytes_io_putc((stream), HPROSE_TAG_NULL)
#define _hprose_writer_write_true(stream) hprose_bytes_io_putc((stream), HPROSE_TAG_TRUE)
#define _hprose_writer_write_false(stream) hprose_bytes_io_putc((stream), HPROSE_TAG_FALSE)
#define _hprose_writer_write_bool(stream, val) ((val) ? _hprose_writer_write_true(stream) : _hprose_writer_write_false(stream))
#define _hprose_writer_write_empty(stream) hprose_bytes_io_putc((stream), HPROSE_TAG_EMPTY)

#define hprose_writer_write_nan(_this) _hprose_writer_write_nan((_this)->stream)
#define hprose_writer_write_null(_this) _hprose_writer_write_null((_this)->stream)
#define hprose_writer_write_true(_this) _hprose_writer_write_true((_this)->stream)
#define hprose_writer_write_false(_this) _hprose_writer_write_false((_this)->stream)
#define hprose_writer_write_bool(_this, val) _hprose_writer_write_bool((_this)->stream, (val))
#define hprose_writer_write_empty(_this) _hprose_writer_write_empty((_this)->stream)

static zend_always_inline void _hprose_writer_write_ulong(hprose_bytes_io *stream, uint64_t i) {
    if (i <= 9) {
        hprose_bytes_io_putc(stream, (char)('0' + i));
    }
    else {
        hprose_bytes_io_putc(stream, (i > INT32_MAX) ? HPROSE_TAG_LONG : HPROSE_TAG_INTEGER);
        hprose_bytes_io_write_ulong(stream, i);
        hprose_bytes_io_putc(stream, HPROSE_TAG_SEMICOLON);
    }
}

#define hprose_writer_write_ulong(_this, i) _hprose_writer_write_ulong((_this)->stream, (i))

static zend_always_inline void _hprose_writer_write_long(hprose_bytes_io *stream, int64_t i) {
    if (i >= 0 && i <= 9) {
        hprose_bytes_io_putc(stream, (char)('0' + i));
    }
    else {
        hprose_bytes_io_putc(stream, (i > INT32_MAX || i < INT32_MIN) ? HPROSE_TAG_LONG : HPROSE_TAG_INTEGER);
        hprose_bytes_io_write_long(stream, i);
        hprose_bytes_io_putc(stream, HPROSE_TAG_SEMICOLON);
    }
}

#define hprose_writer_write_long(_this, i) _hprose_writer_write_long((_this)->stream, (i))

static zend_always_inline void _hprose_writer_write_infinity(hprose_bytes_io *stream, zend_bool positive) {
    hprose_bytes_io_putc(stream, HPROSE_TAG_INFINITY);
    hprose_bytes_io_putc(stream, (positive ? HPROSE_TAG_POS : HPROSE_TAG_NEG));
}

#define hprose_writer_write_infinity(_this, positive) _hprose_writer_write_infinity((_this)->stream, (positive))

static zend_always_inline void _hprose_writer_write_double(hprose_bytes_io *stream, double d) {
    if (isnan(d)) {
        _hprose_writer_write_nan(stream);
    }
    else if (isinf(d)) {
        _hprose_writer_write_infinity(stream, d > 0);
    }
    else if (floor(d) == d && d <= INT64_MAX && d >= INT64_MIN) {
        _hprose_writer_write_long(stream, (int64_t)d);
    }
    else {
        hprose_bytes_io_putc(stream, HPROSE_TAG_DOUBLE);
        hprose_bytes_io_write_double(stream, d);
        hprose_bytes_io_putc(stream, HPROSE_TAG_SEMICOLON);
    }
}

#define hprose_writer_write_double(_this, d) _hprose_writer_write_double((_this)->stream, (d))

static zend_always_inline void _hprose_writer_write_utf8char(hprose_bytes_io *stream, zval *val) {
    hprose_bytes_io_putc(stream, HPROSE_TAG_UTF8CHAR);
    hprose_bytes_io_write(stream, Z_STRVAL_P(val), Z_STRLEN_P(val));
}

#define hprose_writer_write_utf8char(_this, val) _hprose_writer_write_utf8char((_this)->stream, (val))

static zend_always_inline void _hprose_writer_write_string(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val) {
    int32_t len = ustrlen(Z_STRVAL_P(val), Z_STRLEN_P(val));
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_STRING);
    if (len) {
        hprose_bytes_io_write_int(stream, len);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
    hprose_bytes_io_write(stream, Z_STRVAL_P(val), Z_STRLEN_P(val));
    hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
}

#define hprose_writer_write_string(_this, val) _hprose_writer_write_string((_this)->refer, (_this)->stream, (val))

static zend_always_inline void _hprose_writer_write_string_with_ref(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val) {
    if (refer == NULL || !hprose_writer_refer_write(refer, stream, val)) _hprose_writer_write_string(refer, stream, val);
}

#define hprose_writer_write_string_with_ref(_this, val) _hprose_writer_write_string_with_ref((_this)->refer, (_this)->stream, (val))

static zend_always_inline void _hprose_writer_write_bytes(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val) {
    int32_t len = Z_STRLEN_P(val);
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_BYTES);
    if (len) {
        hprose_bytes_io_write_int(stream, len);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
    hprose_bytes_io_write(stream, Z_STRVAL_P(val), len);
    hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
}

#define hprose_writer_write_bytes(_this, val) _hprose_writer_write_bytes((_this)->refer, (_this)->stream, (val))

static zend_always_inline void _hprose_writer_write_bytes_with_ref(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val) {
    if (refer == NULL || !hprose_writer_refer_write(refer, stream, val)) _hprose_writer_write_bytes(refer, stream, val);
}

#define hprose_writer_write_bytes_with_ref(_this, val) _hprose_writer_write_bytes_with_ref((_this)->refer, (_this)->stream, (val))

static zend_always_inline void _hprose_writer_write_bytes_io(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    hprose_bytes_io *instream = HPROSE_GET_OBJECT_P(bytes_io, val)->_this;
    int32_t len = 0;
    if (HB_INITED_P(instream)) {
        len = HB_LEN_P(instream);
    }
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_BYTES);
    if (len) {
        int32_t pos = HB_POS_P(instream);
        HB_POS_P(instream) = 0;
        hprose_bytes_io_write_int(stream, len);
        hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
        hprose_bytes_io_read_to(instream, stream, len);
        HB_POS_P(instream) = pos;
    }
    else {
        hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
}

#define hprose_writer_write_bytes_io(_this, val) _hprose_writer_write_bytes_io((_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void _hprose_writer_write_bytes_io_with_ref(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    if (refer == NULL || !hprose_writer_refer_write(refer, stream, val)) _hprose_writer_write_bytes_io(refer, stream, val TSRMLS_CC);
}

#define hprose_writer_write_bytes_io_with_ref(_this, val) _hprose_writer_write_bytes_io_with_ref((_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void _hprose_writer_write_datetime(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    zval result;
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    method_invoke_no_args(val, getOffset, &result);
    if (Z_LVAL(result) == 0) {
        method_invoke(val, format, &result, "s", ZEND_STRL("\\DYmd\\THis.u\\Z"));
    }
    else {
        method_invoke(val, format, &result, "s", ZEND_STRL("\\DYmd\\THis.u;"));
    }
    hprose_bytes_io_write(stream, Z_STRVAL(result), Z_STRLEN(result));
#if PHP_MAJOR_VERSION < 7
    zval_dtor(&result);
#else
    zval_ptr_dtor(&result);
#endif
}

#define hprose_writer_write_datetime(_this, val) _hprose_writer_write_datetime((_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void _hprose_writer_write_datetime_with_ref(hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    if (refer == NULL || !hprose_writer_refer_write(refer, stream, val)) _hprose_writer_write_datetime(refer, stream, val TSRMLS_CC);
}

#define hprose_writer_write_datetime_with_ref(_this, val) _hprose_writer_write_datetime_with_ref((_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static void _hprose_writer_write_array(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    HashTable *ht = Z_ARRVAL_P(val);
    int32_t i = zend_hash_num_elements(ht);
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_LIST);
    if (i) {
        hprose_bytes_io_write_int(stream, i);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_OPENBRACE);
    if (i) {
        zend_hash_internal_pointer_reset(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **e;
            zend_hash_get_current_data(ht, (void **)&e);
            _hprose_writer_serialize(_this, refer, stream, *e TSRMLS_CC);
#else
            zval *e = zend_hash_get_current_data(ht);
            _hprose_writer_serialize(_this, refer, stream, e TSRMLS_CC);
#endif
            zend_hash_move_forward(ht);
        }
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_CLOSEBRACE);
}

#define hprose_writer_write_array(_this, val) _hprose_writer_write_array((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static void _hprose_writer_write_hashtable(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, HashTable *ht TSRMLS_DC) {
    int32_t i = zend_hash_num_elements(ht);
    hprose_bytes_io_putc(stream, HPROSE_TAG_MAP);
    if (i) {
        hprose_bytes_io_write_int(stream, i);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_OPENBRACE);
    if (i) {
        HashPosition *position = emalloc(sizeof(HashPosition));
        memset(position, 0, sizeof(HashPosition));
        zend_hash_internal_pointer_reset_ex(ht, position);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            char *str;
            uint len;
            ulong index;
            zval **value;
            if (zend_hash_get_current_key_ex(ht, &str, &len, &index, 0, position) == HASH_KEY_IS_STRING) {
                zval key;
                ZVAL_STRINGL(&key, str, len - 1, 0);
                _hprose_writer_serialize(_this, refer, stream, &key TSRMLS_CC);
            }
            else {
                _hprose_writer_write_ulong(stream, (uint64_t)index);
            }
            zend_hash_get_current_data_ex(ht, (void **)&value, position);
            _hprose_writer_serialize(_this, refer, stream, *value TSRMLS_CC);
#else
            zval key, *value;
            zend_hash_get_current_key_zval_ex(ht, &key, position);
            value = zend_hash_get_current_data_ex(ht, position);
            _hprose_writer_serialize(_this, refer, stream, &key TSRMLS_CC);
            _hprose_writer_serialize(_this, refer, stream, value TSRMLS_CC);
            zval_ptr_dtor(&key);
#endif
            zend_hash_move_forward_ex(ht, position);

        }
        efree(position);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_CLOSEBRACE);
}

static zend_always_inline void _hprose_writer_write_assoc_array(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    _hprose_writer_write_hashtable(_this, refer, stream, Z_ARRVAL_P(val) TSRMLS_CC);
}

#define hprose_writer_write_assoc_array(_this, val) _hprose_writer_write_assoc_array((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static void _hprose_writer_write_map(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    zval count;
    int32_t i;
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    method_invoke_no_args(val, count, &count);
    i = Z_LVAL(count);
    hprose_bytes_io_putc(stream, HPROSE_TAG_MAP);
    if (i) {
        hprose_bytes_io_write_int(stream, i);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_OPENBRACE);
    if (i) {
        method_invoke_no_args(val, rewind, NULL);
        for (; i > 0; --i) {
            zval key, value;
            method_invoke_no_args(val, current, &key);
            method_invoke(val, offsetGet, &value, "z", &key);
            _hprose_writer_serialize(_this, refer, stream, &key TSRMLS_CC);
            _hprose_writer_serialize(_this, refer, stream, &value TSRMLS_CC);
#if PHP_MAJOR_VERSION < 7
            zval_dtor(&key);
            zval_dtor(&value);
#else
            zval_ptr_dtor(&key);
            zval_ptr_dtor(&value);
#endif
            method_invoke_no_args(val, next, NULL);
        }
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_CLOSEBRACE);
}

#define hprose_writer_write_map(_this, val) _hprose_writer_write_map((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void _hprose_writer_write_map_with_ref(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    if (refer == NULL || !hprose_writer_refer_write(refer, stream, val)) _hprose_writer_write_map(_this, refer, stream, val TSRMLS_CC);
}

#define hprose_writer_write_map_with_ref(_this, val) _hprose_writer_write_map_with_ref((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static void _hprose_writer_write_iterator(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    zval count;
    int32_t i;
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    method_invoke_no_args(val, count, &count);
    i = Z_LVAL(count);
    hprose_bytes_io_putc(stream, HPROSE_TAG_LIST);
    if (i) {
        hprose_bytes_io_write_int(stream, i);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_OPENBRACE);
    if (i) {
        method_invoke_no_args(val, rewind, NULL);
        for (; i > 0; --i) {
            zval e;
            method_invoke_no_args(val, current, &e);
            _hprose_writer_serialize(_this, refer, stream, &e TSRMLS_CC);
#if PHP_MAJOR_VERSION < 7
            zval_dtor(&e);
#else
            zval_ptr_dtor(&e);
#endif
            method_invoke_no_args(val, next, NULL);
        }
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_CLOSEBRACE);
}

#define hprose_writer_write_iterator(_this, val) _hprose_writer_write_iterator((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void _hprose_writer_write_iterator_with_ref(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    if (refer == NULL || !hprose_writer_refer_write(refer, stream, val)) _hprose_writer_write_iterator(_this, refer, stream, val TSRMLS_CC);
}

#define hprose_writer_write_iterator_with_ref(_this, val) _hprose_writer_write_iterator_with_ref((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static void _hprose_writer_write_list(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    zend_class_entry *ce = Z_OBJCE_P(val);
    if (instanceof_function(ce, zend_ce_aggregate TSRMLS_CC)) {
        zval iterator;
        method_invoke_no_args(val, getIterator, &iterator);
        _hprose_writer_write_list(_this, refer, stream, &iterator TSRMLS_CC);
    }
    else {
        _hprose_writer_write_iterator(_this, refer, stream, val TSRMLS_CC);
    }
}

#define hprose_writer_write_list(_this, val) _hprose_writer_write_list((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static void _hprose_writer_write_list_with_ref(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    zend_class_entry *ce = Z_OBJCE_P(val);
    if (instanceof_function(ce, zend_ce_aggregate TSRMLS_CC)) {
        zval iterator;
        method_invoke_no_args(val, getIterator, &iterator);
        _hprose_writer_write_list_with_ref(_this, refer, stream, &iterator TSRMLS_CC);
#if PHP_MAJOR_VERSION < 7
        zval_dtor(&iterator);
#else
        zval_ptr_dtor(&iterator);
#endif
    }
    else {
        _hprose_writer_write_iterator_with_ref(_this, refer, stream, val TSRMLS_CC);
    }
}

#define hprose_writer_write_list_with_ref(_this, val) _hprose_writer_write_list_with_ref((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void _hprose_writer_write_stdclass(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    _hprose_writer_write_hashtable(_this, refer, stream, Z_OBJPROP_P(val) TSRMLS_CC);
}

#define hprose_writer_write_stdclass(_this, val) _hprose_writer_write_stdclass((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void _hprose_writer_write_stdclass_with_ref(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    if (refer == NULL || !hprose_writer_refer_write(refer, stream, val)) _hprose_writer_write_stdclass(_this, refer, stream, val TSRMLS_CC);
}

#define hprose_writer_write_stdclass_with_ref(_this, val) _hprose_writer_write_stdclass_with_ref((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static int32_t _hprose_writer_write_class(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, char *alias, int32_t len, HashTable *ht) {
    int32_t i = zend_hash_num_elements(ht);
    int32_t index = Z_ARRLEN_P(_this->propsref);
#if PHP_MAJOR_VERSION < 7
    zval *props;
    hprose_zval_new(props);
    array_init(props);
#else
    zval props;
    array_init(&props);
#endif
    hprose_bytes_io_putc(stream, HPROSE_TAG_CLASS);
    hprose_bytes_io_write_int(stream, len);
    hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
    hprose_bytes_io_write(stream, alias, len);
    hprose_bytes_io_putc(stream, HPROSE_TAG_QUOTE);
    if (i) {
        HashPosition *position = emalloc(sizeof(HashPosition));
        memset(position, 0, sizeof(HashPosition));
        zend_hash_internal_pointer_reset_ex(ht, position);
        hprose_bytes_io_write_int(stream, i);
        hprose_bytes_io_putc(stream, HPROSE_TAG_OPENBRACE);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            char *str;
            uint len;
            ulong index;
            if (zend_hash_get_current_key_ex(ht, &str, &len, &index, 0, position) == HASH_KEY_IS_STRING) {
                zval prop;
                size_t pos = 0;
                add_next_index_stringl(props, str, len, 1);
                if (str[0] == 0) {
                    pos = strlen(str + 1) + 2;
                }
                ZVAL_STRINGL(&prop, str + pos, len - pos - 1, 1);
                str = Z_STRVAL(prop);
                if ((str[0] >= 'A') && (str[0] <= 'Z')) {
                    str[0] = str[0] + 32;
                }
                _hprose_writer_write_string_with_ref(refer, stream, &prop);
                zval_dtor(&prop);
            }
#else
            zval prop;
            char *str;
            uint len;
            size_t pos = 0;
            zend_hash_get_current_key_zval_ex(ht, &prop, position);
            assert(Z_TYPE(prop) == IS_STRING);
            add_next_index_zval(&props, &prop);
            str = Z_STRVAL(prop);
            len = Z_STRLEN(prop);
            if (str[0] == 0) {
                pos = strlen(str + 1) + 2;
            }
            ZVAL_STRINGL(&prop, str + pos, len - pos);
            str = Z_STRVAL(prop);
            if ((str[0] >= 'A') && (str[0] <= 'Z')) {
                str[0] = str[0] + 32;
            }
            _hprose_writer_write_string_with_ref(refer, stream, &prop);
            zval_ptr_dtor(&prop);
#endif
            zend_hash_move_forward_ex(ht, position);
        }
        efree(position);
        hprose_bytes_io_putc(stream, HPROSE_TAG_CLOSEBRACE);
    }
    else {
        hprose_bytes_io_putc(stream, HPROSE_TAG_OPENBRACE);
        hprose_bytes_io_putc(stream, HPROSE_TAG_CLOSEBRACE);
    }
#if PHP_MAJOR_VERSION < 7
    add_next_index_zval(_this->propsref, props);
#else
    add_next_index_zval(_this->propsref, &props);
#endif
    add_assoc_long_ex(_this->classref, alias, len, index);
    return index;
}

static void _hprose_writer_write_object(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    HashTable *ht = Z_OBJPROP_P(val), *props_ht;
    zval *props;
    long index;
    int32_t i;
#if PHP_MAJOR_VERSION < 7
    char *classname = (char *)Z_OBJCE_P(val)->name;
    int32_t nlen = strlen(classname);
    int32_t alen;
    char *alias = hprose_class_manager_get_alias(classname, nlen, &alen);
    if (!php_assoc_array_get_long(_this->classref, alias, alen, &index)) {
        index = _hprose_writer_write_class(_this, refer, stream, alias, alen, ht);
    }
    efree(alias);
#else
    zend_string *_classname = Z_OBJ_HT_P(val)->get_class_name(Z_OBJ_P(val));
    zend_string *alias = hprose_class_manager_get_alias(_classname->val, _classname->len);
    if (!php_assoc_array_get_long(_this->classref, alias->val, alias->len, &index)) {
        index = _hprose_writer_write_class(_this, refer, stream, alias->val, alias->len, ht);
    }
    zend_string_release(_classname);
    zend_string_release(alias);
#endif
    props = php_array_get(_this->propsref, index);
    props_ht = Z_ARRVAL_P(props);
    i = zend_hash_num_elements(props_ht);
    if (refer) {
        hprose_writer_refer_set(refer, val);
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_OBJECT);
    hprose_bytes_io_write_int(stream, index);
    hprose_bytes_io_putc(stream, HPROSE_TAG_OPENBRACE);
    if (i) {
        zend_hash_internal_pointer_reset(props_ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **e, *value;
            zend_hash_get_current_data(props_ht, (void **)&e);
            value = zend_hash_str_find_ptr(ht, Z_STRVAL_PP(e), Z_STRLEN_PP(e));
            _hprose_writer_serialize(_this, refer, stream, value TSRMLS_CC);
#else
            zval *e = zend_hash_get_current_data(props_ht);
            zval *value = zend_hash_str_find_ptr(ht, Z_STRVAL_P(e), Z_STRLEN_P(e));
            _hprose_writer_serialize(_this, refer, stream, value TSRMLS_CC);
#endif
            zend_hash_move_forward(props_ht);
        }
    }
    hprose_bytes_io_putc(stream, HPROSE_TAG_CLOSEBRACE);
}

#define hprose_writer_write_object(_this, val) _hprose_writer_write_object((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

static zend_always_inline void _hprose_writer_write_object_with_ref(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    if (refer == NULL || !hprose_writer_refer_write(refer, stream, val)) _hprose_writer_write_object(_this, refer, stream, val TSRMLS_CC);
}

#define hprose_writer_write_object_with_ref(_this, val) _hprose_writer_write_object_with_ref((_this), (_this)->refer, (_this)->stream, (val) TSRMLS_CC)

HPROSE_CLASS_BEGIN(writer)
HPROSE_CLASS_END(writer)

END_EXTERN_C()

#endif	/* HPROSE_WRITER_H */

