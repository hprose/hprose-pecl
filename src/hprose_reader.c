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
 * hprose_reader.c                                        *
 *                                                        *
 * hprose reader for pecl source file.                    *
 *                                                        *
 * LastModified: Mar 21, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_reader.h"

ZEND_METHOD(hprose_reader, __construct) {
    zval *obj = NULL;
    zend_bool simple = 0;
    HPROSE_OBJECT_INTERN(reader);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &obj, &simple) == SUCCESS) {
        intern->_this = hprose_reader_create(HPROSE_GET_OBJECT_P(bytes_io, obj)->_this, simple);
    }
}

ZEND_METHOD(hprose_reader, __destruct) {
    HPROSE_OBJECT_INTERN(reader);
    if (intern->_this) {
        hprose_reader_free(intern->_this);
        intern->_this = NULL;
    }
}

ZEND_METHOD(hprose_reader, unserialize) {
    HPROSE_THIS(reader);
    hprose_reader_unserialize(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, reset) {
    HPROSE_THIS(reader);
    hprose_reader_reset(_this);
}

ZEND_BEGIN_ARG_INFO_EX(hprose_reader_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_reader_construct_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, stream, HproseBytesIO, 0)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_reader_methods[] = {
    ZEND_ME(hprose_reader, __construct, hprose_reader_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_reader, __destruct, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_ME(hprose_reader, unserialize, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, reset, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(reader)

HPROSE_OBJECT_FREE_BEGIN(reader)
    if (intern->_this) {
        hprose_reader_free(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_NEW_BEGIN(reader)
HPROSE_OBJECT_NEW_END(reader)

HPROSE_CLASS_ENTRY(reader)

HPROSE_STARTUP_FUNCTION(reader) {
    HPROSE_REGISTER_CLASS_EX("Hprose", "Reader", reader, get_hprose_raw_reader_ce(), "HproseRawReader");
    HPROSE_REGISTER_CLASS_HANDLERS(reader);
    return SUCCESS;
}
