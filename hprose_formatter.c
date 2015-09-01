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
 * LastModified: Sep 1, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_formatter.h"

static void hprose_writer_fast_serialize(hprose_writer *_this, hprose_writer_refer *refer, hprose_bytes_io *stream, zval *val TSRMLS_DC) {
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

static void hprose_serialize(hprose_bytes_io *stream, zval *val, zend_bool simple TSRMLS_DC) {
    hprose_writer writer;
    hprose_writer_init(&writer, stream, simple);
    hprose_writer_fast_serialize(&writer, writer.refer, writer.stream, val TSRMLS_CC);
    hprose_writer_destroy(&writer);
}

static zend_always_inline void hprose_fast_serialize(hprose_bytes_io *stream, zval *val, zend_bool simple TSRMLS_DC) {
    if (!val) {
        _hprose_writer_write_null(stream); return;
    }
    switch (Z_TYPE_P(val)) {
        case IS_NULL:
            _hprose_writer_write_null(stream); break;
        case IS_LONG:
            _hprose_writer_write_long(stream, Z_LVAL_P(val)); break;
        case IS_DOUBLE:
            _hprose_writer_write_double(stream, Z_DVAL_P(val)); break;
#if PHP_MAJOR_VERSION < 7
        case IS_BOOL:
            _hprose_writer_write_bool(stream, Z_BVAL_P(val)); break;
#else /* PHP_MAJOR_VERSION < 7 */
        case IS_UNDEF:
            _hprose_writer_write_null(stream); break;
        case IS_TRUE:
            _hprose_writer_write_true(stream); break;
        case IS_FALSE:
            _hprose_writer_write_false(stream); break;
#endif
        case IS_STRING: {
            char * s = Z_STRVAL_P(val);
            int32_t l = Z_STRLEN_P(val);
            if (l == 0) {
                _hprose_writer_write_empty(stream);
            }
            else if (is_utf8(s, l)) {
                if (l < 4 && ustrlen(s, l) == 1) {
                    _hprose_writer_write_utf8char(stream, val);
                }
                else {
                    _hprose_writer_write_string_with_ref(NULL, stream, val);
                }
            }
            else {
                _hprose_writer_write_bytes_with_ref(NULL, stream, val);
            }
            break;
        }
        case IS_OBJECT: {
            zend_class_entry *ce = Z_OBJCE_P(val);
            if (instanceof_function(ce, get_hprose_bytes_io_ce() TSRMLS_CC)) {
                _hprose_writer_write_bytes_io_with_ref(NULL, stream, val TSRMLS_CC);
            }
            else if (instanceof_function(ce, php_date_get_date_ce() TSRMLS_CC)) {
                _hprose_writer_write_datetime_with_ref(NULL, stream, val TSRMLS_CC);
            }
            else {
                hprose_serialize(stream, val, simple TSRMLS_CC); break;
            }
        }
        default:
            hprose_serialize(stream, val, simple TSRMLS_CC); break;
    }
}

static void hprose_reader_fast_unserialize(hprose_reader *_this, char tag, zval *return_value TSRMLS_DC) {
    switch (tag) {
        case HPROSE_TAG_LIST: {
            hprose_reader_read_list_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_MAP: {
            hprose_reader_read_map_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_CLASS: {
            hprose_reader_read_class(_this TSRMLS_CC);
            hprose_reader_unserialize(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_OBJECT: {
            hprose_reader_read_object_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_ERROR: {
            _hprose_reader_read_string(_this, return_value TSRMLS_CC);
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                                    "%s", Z_STRVAL_P(return_value));
            return;
        }
        default: unexpected_tag(tag, NULL TSRMLS_CC);
    }
}

static void hprose_unserialize(hprose_bytes_io *stream, zend_bool simple, char tag, zval *return_value TSRMLS_DC) {
    hprose_reader reader;
    hprose_reader_init(&reader, stream, simple);
    hprose_reader_fast_unserialize(&reader, tag, return_value TSRMLS_CC);
    hprose_reader_destroy(&reader);
}

static zend_always_inline void hprose_fast_unserialize(hprose_bytes_io *stream, zend_bool simple, zval *return_value TSRMLS_DC) {
    char tag = hprose_bytes_io_getc(stream);
    switch (tag) {
        case '0': RETURN_LONG(0);
        case '1': RETURN_LONG(1);
        case '2': RETURN_LONG(2);
        case '3': RETURN_LONG(3);
        case '4': RETURN_LONG(4);
        case '5': RETURN_LONG(5);
        case '6': RETURN_LONG(6);
        case '7': RETURN_LONG(7);
        case '8': RETURN_LONG(8);
        case '9': RETURN_LONG(9);
        case HPROSE_TAG_INTEGER:
            RETURN_LONG(_hprose_reader_read_integer_without_tag(stream));
        case HPROSE_TAG_LONG: {
#if PHP_MAJOR_VERSION < 7
            int32_t len = 0;
            char *num = _hprose_reader_read_long_without_tag(stream, &len);
            RETURN_STRINGL_0(num, len);
#else
            RETURN_STR(_hprose_reader_read_long_without_tag(stream));
#endif
        }
        case HPROSE_TAG_DOUBLE: {
            RETURN_DOUBLE(_hprose_reader_read_double_without_tag(stream));
        }
        case HPROSE_TAG_NAN:
            RETURN_DOUBLE(NAN);
        case HPROSE_TAG_INFINITY:
            RETURN_DOUBLE(_hprose_reader_read_infinity_without_tag(stream));
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_EMPTY: RETURN_EMPTY_STRING();
        case HPROSE_TAG_TRUE: RETURN_TRUE;
        case HPROSE_TAG_FALSE: RETURN_FALSE;
        case HPROSE_TAG_DATE: {
            _hprose_reader_read_datetime_without_tag(stream, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_TIME: {
            _hprose_reader_read_time_without_tag(stream, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_BYTES: {
            _hprose_reader_read_bytes_without_tag(stream, return_value);
            return;
        }
        case HPROSE_TAG_UTF8CHAR: {
            _hprose_reader_read_utf8char_without_tag(stream, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_STRING: {
            _hprose_reader_read_string_without_tag(stream, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_GUID: {
            _hprose_reader_read_guid_without_tag(stream, return_value);
            return;
        }
        default: {
            hprose_unserialize(stream, simple, tag, return_value TSRMLS_CC);
            return;
        }
    }
}

#if HAVE_PHP_SESSION
PS_SERIALIZER_ENCODE_FUNC(hprose) {
    hprose_bytes_io stream;
    hprose_bytes_io_init(&stream, NULL, 0);
#if PHP_MAJOR_VERSION < 7
    hprose_fast_serialize(&stream, PS(http_session_vars), 0 TSRMLS_CC);
    if (newlen) {
        *newlen = HB_LEN(stream);
    }
    *newstr = HB_BUF(stream);
    return SUCCESS;
#else
    hprose_fast_serialize(&stream, &PS(http_session_vars), 0 TSRMLS_CC);
    return stream.s;
#endif
}

PS_SERIALIZER_DECODE_FUNC(hprose) {
#if PHP_MAJOR_VERSION < 7
    int ret;
    HashPosition tmp_hash_pos;
    char *key_str;
    ulong key_long;
    uint key_len;
    zval *tmp;
    zval **value;
#else
    zend_string *str, *key_str;
    zval tmp, *value;
#endif
    HashTable *tmp_hash;
    hprose_bytes_io stream;
    if (vallen > 0) {
#if PHP_MAJOR_VERSION < 7
        hprose_bytes_io_init_readonly(&stream, val, vallen);
        MAKE_STD_ZVAL(tmp);
        hprose_fast_unserialize(&stream, 0, tmp TSRMLS_CC);
        tmp_hash = HASH_OF(tmp);
        if (tmp_hash) {
            zend_hash_internal_pointer_reset_ex(tmp_hash, &tmp_hash_pos);
            while (zend_hash_get_current_data_ex(
                       tmp_hash, (void *)&value, &tmp_hash_pos) == SUCCESS) {
                ret = zend_hash_get_current_key_ex(
                    tmp_hash, &key_str, &key_len, &key_long, 0, &tmp_hash_pos);
                switch (ret) {
                case HASH_KEY_IS_LONG:
                    /* ??? */
                    break;
                case HASH_KEY_IS_STRING:
                    php_set_session_var(
                        key_str, key_len - 1, *value, NULL TSRMLS_CC);
                    php_add_session_var(key_str, key_len - 1 TSRMLS_CC);
                    break;
                }
                zend_hash_move_forward_ex(tmp_hash, &tmp_hash_pos);
            }
        }
#else
        str = zend_string_init(val, vallen, 0);
        hprose_bytes_io_init_readonly(&stream, str);
        hprose_fast_unserialize(&stream, 0, &tmp TSRMLS_CC);
        tmp_hash = HASH_OF(&tmp);
        if (tmp_hash) {
            ZEND_HASH_FOREACH_STR_KEY_VAL(tmp_hash, key_str, value) {
                if (key_str) {
                    php_set_session_var(key_str, value, NULL);
                    php_add_session_var(key_str);
                    ZVAL_UNDEF(value);
                }
            } ZEND_HASH_FOREACH_END();
        }
        zend_string_release(str);
#endif
        zval_ptr_dtor(&tmp);
    }
    if (EG(exception)) {
#if PHP_MAJOR_VERSION < 7
        zend_clear_exception(TSRMLS_C);
#else
        zend_clear_exception();
#endif
    }
    return SUCCESS;
}
#endif

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
    hprose_fast_serialize(&stream, val, simple TSRMLS_CC);
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
        hprose_fast_unserialize(&stream, simple, return_value TSRMLS_CC);
        return;
    }
    else if (Z_TYPE_P(data) == IS_OBJECT) {
        if (instanceof_function(Z_OBJCE_P(data), get_hprose_bytes_io_ce() TSRMLS_CC)) {
            hprose_fast_unserialize(HPROSE_GET_OBJECT_P(bytes_io, data)->_this, simple, return_value TSRMLS_CC);
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
