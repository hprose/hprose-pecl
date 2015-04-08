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
 * hprose_raw_reader.c                                    *
 *                                                        *
 * hprose raw reader for pecl source file.                *
 *                                                        *
 * LastModified: Apr 8, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_raw_reader.h"

ZEND_METHOD(hprose_raw_reader, __construct) {
    zval *obj = NULL;
    HPROSE_OBJECT_INTERN(raw_reader);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &obj) == SUCCESS) {
        intern->_this = hprose_raw_reader_create(HPROSE_GET_OBJECT_P(bytes_io, obj)->_this);
    }
}

ZEND_METHOD(hprose_raw_reader, readRaw) {
    HPROSE_THIS(raw_reader);
    object_init_ex(return_value, get_hprose_bytes_io_ce());
    HPROSE_GET_OBJECT_P(bytes_io, return_value)->_this = hprose_raw_reader_read_raw(_this TSRMLS_CC);
    HPROSE_GET_OBJECT_P(bytes_io, return_value)->mark = -1;
}

ZEND_BEGIN_ARG_INFO_EX(hprose_raw_reader_construct_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_raw_reader_read_raw_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_raw_reader_methods[] = {
    ZEND_ME(hprose_raw_reader, __construct, hprose_raw_reader_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_raw_reader, readRaw, hprose_raw_reader_read_raw_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(raw_reader)

HPROSE_OBJECT_FREE_BEGIN(raw_reader)
    if (intern->_this) {
        hprose_raw_reader_free(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_SIMPLE_NEW(raw_reader)

HPROSE_CLASS_ENTRY(raw_reader)

HPROSE_STARTUP_FUNCTION(raw_reader) {
    HPROSE_REGISTER_CLASS("Hprose", "RawReader", raw_reader);
    HPROSE_REGISTER_CLASS_HANDLERS(raw_reader);
    return SUCCESS;
}
