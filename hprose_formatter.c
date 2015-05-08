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
 * LastModified: Apr 8, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_formatter.h"

static zend_always_inline void hprose_writer_fast_serialize(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
    switch (Z_TYPE_P(val)) {
#if PHP_MAJOR_VERSION >= 7
        case IS_REFERENCE:
            _hprose_writer_serialize(_this, refer, stream, &(Z_REF_P(val)->val) TSRMLS_CC); break;
#endif
        case IS_ARRAY:
            if (is_list(val)) {
                _hprose_writer_write_array(_this, refer, stream, val TSRMLS_CC);
            }
            else {
                _hprose_writer_write_assoc_array(_this, refer, stream, val TSRMLS_CC);
            }
            break;
        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(val);
            if (instanceof_function(ce, spl_ce_SplObjectStorage TSRMLS_CC)) {
                _hprose_writer_write_map_with_ref(_this, refer, stream, val TSRMLS_CC);
            }
            else if (instanceof_function(ce, zend_ce_traversable TSRMLS_CC)) {
                _hprose_writer_write_list_with_ref(_this, refer, stream, val TSRMLS_CC);
            }
            else if (instanceof_function(ce, zend_standard_class_def TSRMLS_CC)) {
                _hprose_writer_write_stdclass_with_ref(_this, refer, stream, val TSRMLS_CC);
            }
            else {
                _hprose_writer_write_object_with_ref(_this, refer, stream, val TSRMLS_CC);
            }
            break;
        }
        default:
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                    "Not support to serialize this data: %d", Z_TYPE_P(val));
            break;
    }
}

static zend_always_inline void hprose_serialize(hprose_bytes_io *stream, zval *val, zend_bool simple TSRMLS_DC) {
    hprose_writer writer;
    hprose_writer_init(&writer, stream, simple);
    hprose_writer_fast_serialize(&writer, writer.refer, writer.stream, val TSRMLS_CC);
    hprose_writer_destroy(&writer);
}

static zend_always_inline void hprose_unserialize(hprose_bytes_io *stream, zend_bool simple, zval *return_value TSRMLS_DC) {
    hprose_reader reader;
    hprose_reader_init(&reader, stream, simple);
    hprose_reader_unserialize(&reader, return_value TSRMLS_CC);
    hprose_reader_destroy(&reader);
}

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
    if (!val) {
        _hprose_writer_write_null(&stream); return;
    }
    switch (Z_TYPE_P(val)) {
        case IS_NULL:
            _hprose_writer_write_null(&stream); break;
        case IS_LONG:
            _hprose_writer_write_long(&stream, Z_LVAL_P(val)); break;
        case IS_DOUBLE:
            _hprose_writer_write_double(&stream, Z_DVAL_P(val)); break;
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
            _hprose_writer_write_bool(&stream, Z_BVAL_P(val)); break;
#else /* PHP_MAJOR_VERSION < 7 */
        case IS_UNDEF:
            _hprose_writer_write_null(&stream); break;
        case IS_TRUE:
            _hprose_writer_write_true(&stream); break;
        case IS_FALSE:
            _hprose_writer_write_false(&stream); break;
#endif
        case IS_STRING: {
            char * s = Z_STRVAL_P(val);
            int32_t l = Z_STRLEN_P(val);
            if (l == 0) {
                _hprose_writer_write_empty(&stream);
            }
            else if (is_utf8(s, l)) {
                if (l < 4 && ustrlen(s, l) == 1) {
                    _hprose_writer_write_utf8char(&stream, val);
                }
                else {
                    _hprose_writer_write_string_with_ref(NULL, &stream, val);
                }
            }
            else {
                _hprose_writer_write_bytes_with_ref(NULL, &stream, val);
            }
            break;
        }
        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(val);
            if (instanceof_function(ce, get_hprose_bytes_io_ce() TSRMLS_CC)) {
                _hprose_writer_write_bytes_io_with_ref(NULL, &stream, val TSRMLS_CC);
            }
            else if (instanceof_function(ce, php_date_get_date_ce() TSRMLS_CC)) {
                _hprose_writer_write_datetime_with_ref(NULL, &stream, val TSRMLS_CC);
            }
            else {
                hprose_serialize(&stream, val, simple TSRMLS_CC); break;
            }
        }
        default:
            hprose_serialize(&stream, val, simple TSRMLS_CC); break;
    }
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
