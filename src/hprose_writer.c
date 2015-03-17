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
 * hprose_writer.c                                        *
 *                                                        *
 * hprose writer for pecl source file.                    *
 *                                                        *
 * LastModified: Mar 16, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_writer.h"

ZEND_METHOD(hprose_writer, __construct) {
    zval *obj = NULL;
    zend_bool simple = 0;
    HPROSE_OBJECT_INTERN(writer);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &obj, &simple) == SUCCESS) {
        intern->writer = hprose_writer_create(HPROSE_GET_OBJECT_P(bytes_io, obj)->bytes, simple);
    }
}

ZEND_METHOD(hprose_writer, __destruct) {
    HPROSE_OBJECT_INTERN(writer);
    if (intern->writer) {
        hprose_writer_free(intern->writer);
        intern->writer = NULL;
    }
}

ZEND_METHOD(hprose_writer, serialize) {
    zval *val = NULL;
    HPROSE_OBJECT(writer, writer);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &val) == SUCCESS) {
        hprose_writer_serialize(writer, val TSRMLS_CC);
    }
}

static zend_function_entry hprose_writer_methods[] = {
    ZEND_ME(hprose_writer, __construct, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_writer, __destruct, hprose_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_ME(hprose_writer, serialize, NULL, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(writer)

HPROSE_OBJECT_FREE_BEGIN(writer)
    if (intern->writer) {
        hprose_writer_free(intern->writer);
        intern->writer = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_NEW_BEGIN(writer)
HPROSE_OBJECT_NEW_END(writer)

HPROSE_CLASS_ENTRY(writer)

HPROSE_STARTUP_FUNCTION(writer) {
    HPROSE_REGISTER_CLASS("Hprose", "Writer", writer);
    HPROSE_REGISTER_CLASS_HANDLERS(writer);
    return SUCCESS;
}
