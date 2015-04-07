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
 * hprose_formatter.c                                     *
 *                                                        *
 * hprose formatter for pecl source file.                 *
 *                                                        *
 * LastModified: Apr 7, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_formatter.h"

/* {{{ proto string hprose_serialize(mixed $val, bool $simple = false)
   serialize php value to hprose format data  */
ZEND_FUNCTION(hprose_serialize) {
    zval *val;
    zend_bool simple = 0;
    hprose_bytes_io stream;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z!|b", &val, &simple) == FAILURE) {
        return;
    }
    hprose_bytes_io_init(&stream, NULL, 0);
    hprose_serialize(&stream, val, simple TSRMLS_CC);
#if PHP_MAJOR_VERSION < 7
    RETVAL_STRINGL_0(HB_BUF(stream), HB_LEN(stream));
#else
    RETVAL_STR(HB_STR(stream));
#endif
}
/* }}} */

/* {{{ proto mixed hprose_unserialize(mixed $data, bool $simple = false)
 *     unserialize hprose format data to php value
 *     $data can be a string or an object of HproseBytesIO */
ZEND_FUNCTION(hprose_unserialize) {
    zval *data;
    zend_bool simple = 0;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &data, &simple) == FAILURE) {
        return;
    }
    if (Z_TYPE_P(data) == IS_STRING) {
        hprose_bytes_io stream;
#if PHP_MAJOR_VERSION < 7
        hprose_bytes_io_init_readonly(&stream, Z_STRVAL_P(data), Z_STRLEN_P(data));
#else
        hprose_bytes_io_init_readonly(&stream, Z_STR_P(data));
#endif
        hprose_unserialize(&stream, simple, return_value TSRMLS_CC);
        return;
    }
    else if (Z_TYPE_P(data) == IS_OBJECT) {
        if (instanceof_function(Z_OBJCE_P(data), get_hprose_bytes_io_ce() TSRMLS_CC)) {
            hprose_unserialize(HPROSE_GET_OBJECT_P(bytes_io, data)->_this, simple, return_value TSRMLS_CC);
            return;
        }
    }
    zend_throw_exception(NULL, "data must be a string or an object of HproseBytesIO", 0 TSRMLS_CC);
}
/* }}} */

ZEND_BEGIN_ARG_INFO_EX(hprose_formatter_serialize_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, val)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_formatter_unserialize_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_formatter_methods[] = {
    ZEND_ME_MAPPING(serialize, hprose_serialize, hprose_formatter_serialize_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_ME_MAPPING(unserialize, hprose_unserialize, hprose_formatter_unserialize_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_CLASS_ENTRY(formatter)

HPROSE_STARTUP_FUNCTION(formatter) {
    HPROSE_REGISTER_CLASS("Hprose", "Formatter", formatter);
    return SUCCESS;
}
