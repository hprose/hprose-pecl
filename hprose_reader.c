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
 * LastModified: May 9, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_reader.h"

void hprose_reader_unserialize(hprose_reader *_this, zval *return_value TSRMLS_DC) {
    char tag = hprose_bytes_io_getc(_this->stream);
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
            RETURN_LONG(hprose_reader_read_integer_without_tag(_this));
        case HPROSE_TAG_LONG: {
#if PHP_MAJOR_VERSION < 7
            int32_t len = 0;
            char *num = hprose_reader_read_long_without_tag(_this, &len);
            RETURN_STRINGL_0(num, len);
#else
            RETURN_STR(hprose_reader_read_long_without_tag(_this));
#endif
        }
        case HPROSE_TAG_DOUBLE: {
            RETURN_DOUBLE(hprose_reader_read_double_without_tag(_this));
        }
        case HPROSE_TAG_NAN:
            RETURN_DOUBLE(NAN);
        case HPROSE_TAG_INFINITY:
            RETURN_DOUBLE(hprose_reader_read_infinity_without_tag(_this));
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_EMPTY: RETURN_EMPTY_STRING();
        case HPROSE_TAG_TRUE: RETURN_TRUE;
        case HPROSE_TAG_FALSE: RETURN_FALSE;
        case HPROSE_TAG_DATE: {
            hprose_reader_read_datetime_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_TIME: {
            hprose_reader_read_time_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_BYTES: {
            hprose_reader_read_bytes_without_tag(_this, return_value);
            return;
        }
        case HPROSE_TAG_UTF8CHAR: {
            hprose_reader_read_utf8char_without_tag(_this, return_value);
            return;
        }
        case HPROSE_TAG_STRING: {
            hprose_reader_read_string_without_tag(_this, return_value TSRMLS_CC);
            return;
        }
        case HPROSE_TAG_GUID: {
            hprose_reader_read_guid_without_tag(_this, return_value);
            return;
        }
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

ZEND_METHOD(hprose_reader, __construct) {
    zval *obj = NULL;
    zend_bool simple = 0;
    HPROSE_OBJECT_INTERN(reader);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &obj, &simple) == SUCCESS) {
        intern->_this = hprose_reader_create(HPROSE_GET_OBJECT_P(bytes_io, obj)->_this, simple);
    }
}

ZEND_METHOD(hprose_reader, unserialize) {
    HPROSE_THIS(reader);
    hprose_reader_unserialize(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, checkTag) {
    char ch;
    char *expectTag = NULL, *tag = NULL;
    length_t len, tlen;
    HPROSE_THIS(reader);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &expectTag, &len, &tag, &tlen) == SUCCESS) {
        if (tag == NULL || tlen < 1) {
            ch = hprose_bytes_io_getc(_this->stream);
            tag = &ch;
        }
        if (len < 1 || expectTag[0] != tag[0]) {
            unexpected_tag(tag[0], expectTag TSRMLS_CC);
        }
    }
}

ZEND_METHOD(hprose_reader, checkTags) {
    char ch;
    char *expectTags = NULL, *tag = NULL;
    length_t len, tlen;
    HPROSE_THIS(reader);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &expectTags, &len, &tag, &tlen) == SUCCESS) {
        if (tag == NULL || tlen < 1) {
            ch = hprose_bytes_io_getc(_this->stream);
            tag = &ch;
        }
        if (len < 1 || strchr(expectTags, tag[0]) == NULL) {
            unexpected_tag(tag[0], expectTags TSRMLS_CC);
        }
    }
}

ZEND_METHOD(hprose_reader, readIntegerWithoutTag) {
    HPROSE_THIS(reader);
    RETURN_LONG(hprose_reader_read_integer_without_tag(_this));
}

ZEND_METHOD(hprose_reader, readInteger) {
    HPROSE_THIS(reader);
    RETURN_LONG(hprose_reader_read_integer(_this TSRMLS_CC));
}

ZEND_METHOD(hprose_reader, readLongWithoutTag) {
    HPROSE_THIS(reader);
#if PHP_MAJOR_VERSION < 7
    int32_t len = 0;
    char *num = hprose_reader_read_long_without_tag(_this, &len);
    RETURN_STRINGL_0(num, len);
#else
    RETURN_STR(hprose_reader_read_long_without_tag(_this));
#endif
}

ZEND_METHOD(hprose_reader, readLong) {
    HPROSE_THIS(reader);
#if PHP_MAJOR_VERSION < 7
    int32_t len = 0;
    char *num = hprose_reader_read_long(_this, &len TSRMLS_CC);
    RETURN_STRINGL_0(num, len);
#else
    RETURN_STR(hprose_reader_read_long(_this TSRMLS_CC));
#endif
}

ZEND_METHOD(hprose_reader, readDoubleWithoutTag) {
    HPROSE_THIS(reader);
    RETURN_DOUBLE(hprose_reader_read_double_without_tag(_this));
}

ZEND_METHOD(hprose_reader, readDouble) {
    HPROSE_THIS(reader);
    RETURN_DOUBLE(hprose_reader_read_double(_this TSRMLS_CC));
}

ZEND_METHOD(hprose_reader, readNaN) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_NAN, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_NAN) {
        RETURN_DOUBLE(NAN);
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readInfinityWithoutTag) {
    HPROSE_THIS(reader);
    RETURN_DOUBLE(hprose_reader_read_infinity_without_tag(_this));
}

ZEND_METHOD(hprose_reader, readInfinity) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_INFINITY, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_INFINITY) {
        RETURN_DOUBLE(hprose_reader_read_infinity_without_tag(_this));
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readNull) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_NULL, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_NULL) {
        RETURN_NULL();
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readEmpty) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_EMPTY, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_EMPTY) {
        RETURN_EMPTY_STRING();
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readBoolean) {
    HPROSE_THIS(reader);
    RETURN_BOOL(hprose_reader_read_boolean(_this TSRMLS_CC));
}

ZEND_METHOD(hprose_reader, readDateWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_datetime_without_tag(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readDate) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_NULL,
                            HPROSE_TAG_DATE,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_DATE: {
            hprose_reader_read_datetime_without_tag(_this, return_value TSRMLS_CC);
            return;            
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            return;
        }
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readTimeWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_time_without_tag(_this, return_value TSRMLS_CC);    
}

ZEND_METHOD(hprose_reader, readTime) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_NULL,
                            HPROSE_TAG_TIME,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_TIME: {
            hprose_reader_read_time_without_tag(_this, return_value TSRMLS_CC);
            return;            
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            return;
        }
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readBytesWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_bytes_without_tag(_this, return_value);
}

ZEND_METHOD(hprose_reader, readBytes) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_NULL,
                            HPROSE_TAG_EMPTY,
                            HPROSE_TAG_BYTES,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_EMPTY: RETURN_EMPTY_STRING();
        case HPROSE_TAG_BYTES: {
            hprose_reader_read_bytes_without_tag(_this, return_value);
            return;            
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            return;
        }
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readUTF8CharWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_utf8char_without_tag(_this, return_value);
}

ZEND_METHOD(hprose_reader, readUTF8Char) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_UTF8CHAR, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    if (tag == HPROSE_TAG_UTF8CHAR) {
        hprose_reader_read_utf8char_without_tag(_this, return_value);
        return;            
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readStringWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_string_without_tag(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readString) {
    HPROSE_THIS(reader);
    hprose_reader_read_string(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readGuidWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_guid_without_tag(_this, return_value);
}

ZEND_METHOD(hprose_reader, readGuid) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_NULL,
                            HPROSE_TAG_GUID,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_NULL: RETURN_NULL();
        case HPROSE_TAG_GUID: {
            hprose_reader_read_guid_without_tag(_this, return_value);
            return;            
        }
        case HPROSE_TAG_REF: {
            hprose_reader_read_ref(_this, return_value TSRMLS_CC);
            return;
        }
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readListWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_list_without_tag(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readList) {
    HPROSE_THIS(reader);
    hprose_reader_read_list(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readMapWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_map_without_tag(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readMap) {
    HPROSE_THIS(reader);
    hprose_reader_read_map(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readObjectWithoutTag) {
    HPROSE_THIS(reader);
    hprose_reader_read_object_without_tag(_this, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_reader, readObject) {
    HPROSE_THIS(reader);
    char expected_tags[] = {HPROSE_TAG_NULL,
                            HPROSE_TAG_CLASS,
                            HPROSE_TAG_OBJECT,
                            HPROSE_TAG_REF, '\0'};
    char tag = hprose_bytes_io_getc(_this->stream);
    switch (tag) {
        case HPROSE_TAG_NULL: RETURN_NULL();
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
    }
    unexpected_tag(tag, expected_tags TSRMLS_CC);
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

ZEND_BEGIN_ARG_INFO_EX(hprose_reader_check_tag_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, expectTag)
    ZEND_ARG_INFO(0, tag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_reader_check_tags_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, expectTags)
    ZEND_ARG_INFO(0, tag)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_reader_methods[] = {
    ZEND_ME(hprose_reader, __construct, hprose_reader_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_reader, unserialize, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, checkTag, hprose_reader_check_tag_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, checkTags, hprose_reader_check_tags_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readIntegerWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readInteger, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readLongWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readLong, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readDoubleWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readDouble, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readNaN, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readInfinityWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readInfinity, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readNull, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readEmpty, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readBoolean, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readDateWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readDate, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readTimeWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readTime, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readBytesWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readBytes, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readUTF8CharWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readUTF8Char, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readStringWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readString, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readGuidWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readGuid, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readListWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readList, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readMapWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readMap, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readObjectWithoutTag, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_reader, readObject, hprose_reader_void_arginfo, ZEND_ACC_PUBLIC)
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

HPROSE_OBJECT_SIMPLE_NEW(reader)

HPROSE_CLASS_ENTRY(reader)

HPROSE_STARTUP_FUNCTION(reader) {
    HPROSE_REGISTER_CLASS_EX("Hprose", "Reader", reader, get_hprose_raw_reader_ce(), "HproseRawReader");
    HPROSE_REGISTER_CLASS_HANDLERS(reader);
    return SUCCESS;
}
