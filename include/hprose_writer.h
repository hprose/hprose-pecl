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
 * LastModified: Mar 14, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_WRITER_H
#define	HPROSE_WRITER_H

#include "hprose.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_writer_ce();

HPROSE_STARTUP_FUNCTION(writer);

typedef void hprose_writer_refer_set(void *_this, zval *val);
typedef zend_bool hprose_writer_refer_write(void *_this, hprose_bytes_io *stream, zval *val);
typedef void hprose_writer_refer_reset(void *_this);

typedef struct {
    hprose_writer_refer_set *set;
    hprose_writer_refer_write *write;
    hprose_writer_refer_reset *reset;
} hprose_writer_refer;

static void hprose_writer_fack_refer_set(void *_this, zval *val) {}
static zend_bool hprose_writer_fack_refer_write(void *_this, hprose_bytes_io *stream, zval *val) {
    return 0;
}
static void hprose_writer_fack_refer_reset(void *_this) {}

static zend_always_inline hprose_writer_refer *hprose_writer_fack_refer_new() {
    hprose_writer_refer *_this = emalloc(sizeof(hprose_writer_refer));
    _this->set = &hprose_writer_fack_refer_set;
    _this->write = &hprose_writer_fack_refer_write;
    _this->reset = &hprose_writer_fack_refer_reset;
    return _this;
}

typedef struct {
    hprose_writer_refer_set *set;
    hprose_writer_refer_write *write;
    hprose_writer_refer_reset *reset;
    zend_llist *ref;
    zval *sref;
    zval *oref;
    int refcount;
} hprose_writer_real_refer;

static zend_always_inline void hprose_writer_real_refer_write_ref(hprose_bytes_io *stream, int32_t index) {
    hprose_bytes_io_write_char(stream, HPROSE_TAG_REF);
    hprose_bytes_io_write_int(stream, index);
    hprose_bytes_io_write_char(stream, HPROSE_TAG_SEMICOLON);
}

static void hprose_writer_real_refer_set(void *_this, zval *val) {
    hprose_writer_real_refer *refer = (hprose_writer_real_refer *)_this;
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
static zend_bool hprose_writer_real_refer_write(void *_this, hprose_bytes_io *stream, zval *val) {
    hprose_writer_real_refer *refer = (hprose_writer_real_refer *)_this;
    long index;
    char *key;
    switch (Z_TYPE_P(val)) {
        case IS_STRING:
            if (php_assoc_array_get_long(refer->sref, Z_STRVAL_P(val), Z_STRLEN_P(val), &index)) {
                hprose_writer_real_refer_write_ref(stream, (int32_t)index);
                return 1;
            }
            break;
        case IS_OBJECT:
            key = object_hash(val);
            if (php_assoc_array_get_long(refer->oref, key, 32, &index)) {
                hprose_writer_real_refer_write_ref(stream, (int32_t)index);
                efree(key);
                return 1;
            }
            efree(key);
            break;
    }
    return 0;
}
static void hprose_writer_real_refer_reset(void *_this) {
    hprose_writer_real_refer *refer = (hprose_writer_real_refer *)_this;
    zend_llist_clean(refer->ref);

}

typedef struct {
    hprose_bytes_io *stream;
    HashTable *classref;
    HashTable *propsref;
    hprose_writer_refer *refer;
    zend_bool persistent;
} hprose_writer;

END_EXTERN_C()

#endif	/* HPROSE_WRITER_H */

