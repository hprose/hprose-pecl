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
 * LastModified: Mar 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_formatter.h"

/* {{{ proto string hprose_serialize(mixed val, bool simple = false)
   serialize php value to hprose format data  */
ZEND_FUNCTION(hprose_serialize) {
    zval *val;
    zend_bool simple = 0;
    hprose_bytes_io *stream;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z!|b", &val, &simple) == FAILURE) {
        return;
    }
    stream = hprose_serialize(val, simple TSRMLS_CC);
    RETVAL_STRINGL_0(stream->buf, stream->len);
    efree(stream);
}
/* }}} */

/* {{{ proto mixed hprose_unserialize(string data, bool simple = false)
    unserialize hprose format data to php value */
ZEND_FUNCTION(hprose_unserialize) {
    char *data;
    length_t len;
    zend_bool simple = 0;
    hprose_bytes_io *stream;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &data, &len, &simple) == FAILURE) {
        return;
    }
    stream = hprose_bytes_io_create_readonly(data, len);
    hprose_unserialize(stream, simple, return_value TSRMLS_CC);
    efree(stream);
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
