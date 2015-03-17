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
 * hprose_bytes_io.c                                      *
 *                                                        *
 * hprose bytes io for pecl source file.                  *
 *                                                        *
 * LastModified: Mar 17, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_bytes_io.h"

ZEND_METHOD(hprose_bytes_io, __construct) {
    char *buf = NULL;
    length_t len = 0;
    HPROSE_OBJECT_INTERN(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &buf, &len) == FAILURE) {
        return;
    }
    if (buf) {
        intern->_this = hprose_bytes_io_create(buf, len);
    }
    else {
        intern->_this = hprose_bytes_io_new();
    }
    intern->mark = -1;
}

ZEND_METHOD(hprose_bytes_io, __destruct) {
    HPROSE_OBJECT_INTERN(bytes_io);
    if (intern->_this) {
        hprose_bytes_io_free(intern->_this);
        intern->_this = NULL;
    }
}

ZEND_METHOD(hprose_bytes_io, close) {
    HPROSE_OBJECT_INTERN(bytes_io);
    hprose_bytes_io_close(intern->_this);
    intern->mark = -1;
}

ZEND_METHOD(hprose_bytes_io, length) {
    HPROSE_THIS(bytes_io);
    RETURN_LONG(_this->len);
}

ZEND_METHOD(hprose_bytes_io, getc) {
    HPROSE_THIS(bytes_io);
    if (_this->pos < _this->len) {
        char *c = hprose_bytes_io_read(_this, 1);
        RETURN_STRINGL_0(c, 1);
    }
    RETURN_EMPTY_STRING();
}

ZEND_METHOD(hprose_bytes_io, read) {
    char *s;
    long n;
    HPROSE_THIS(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        RETURN_NULL();
    }
    if (_this->pos + n > _this->len) {
        n = _this->len - _this->pos;
    }
    s = hprose_bytes_io_read(_this, n);
    RETURN_STRINGL_0(s, n);
}

ZEND_METHOD(hprose_bytes_io, readfull) {
    char *s;
    int32_t l;
    HPROSE_THIS(bytes_io);
    s = hprose_bytes_io_readfull(_this, &l);
    RETURN_STRINGL_0(s, l);
}

ZEND_METHOD(hprose_bytes_io, readuntil) {
    char *s, *tag;
    length_t len;
    int32_t l;
    HPROSE_THIS(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &tag, &len) == FAILURE) {
        RETURN_NULL();
    }
    if (len > 0) {
        s = hprose_bytes_io_readuntil(_this, tag[0], &l);
    }
    else {
        s = hprose_bytes_io_readfull(_this, &l);
    }
    RETURN_STRINGL_0(s, l);
}

ZEND_METHOD(hprose_bytes_io, readString) {
    char *s;
    long n;
    int32_t l;
    HPROSE_THIS(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        RETURN_NULL();
    }
    s = hprose_bytes_io_read_string(_this, n, &l);
    RETURN_STRINGL_0(s, l);
}

ZEND_METHOD(hprose_bytes_io, mark) {
    HPROSE_OBJECT_INTERN(bytes_io);
    intern->mark = intern->_this->pos;
}

ZEND_METHOD(hprose_bytes_io, unmark) {
    HPROSE_OBJECT_INTERN(bytes_io);
    intern->mark = -1;
}

ZEND_METHOD(hprose_bytes_io, reset) {
    HPROSE_OBJECT_INTERN(bytes_io);
    if (intern->mark != -1) {
        intern->_this->pos = intern->mark;
    }
}

ZEND_METHOD(hprose_bytes_io, skip) {
    long n;
    HPROSE_THIS(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        return;
    }
    if (n > 0) {
        if (n > _this->len - _this->pos) {
            _this->pos = _this->len;
        }
        else {
            _this->pos += n;
        }
    }
}

ZEND_METHOD(hprose_bytes_io, eof) {
    HPROSE_THIS(bytes_io);
    RETURN_BOOL(hprose_bytes_io_eof(_this));
}

ZEND_METHOD(hprose_bytes_io, write) {
    char *str;
    length_t len;
    long n = -1;
    HPROSE_THIS(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &str, &len, &n) == FAILURE) {
        return;
    }
    if (n > len) n = len;
    hprose_bytes_io_write(_this, str, n);
}

ZEND_METHOD(hprose_bytes_io, toString) {
    char *str;
    HPROSE_THIS(bytes_io);
    str = hprose_bytes_io_to_string(_this);
    RETURN_STRINGL_0(str, _this->len);
}

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_construct_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_long_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_readuntil_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, tag)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_write_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, str)
    ZEND_ARG_INFO(0, n)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_bytes_io_methods[] = {
    ZEND_ME(hprose_bytes_io, __construct, hprose_bytes_io_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_bytes_io, __destruct, hprose_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_ME(hprose_bytes_io, close, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, length, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, getc, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, read, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readfull, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readuntil, hprose_bytes_io_readuntil_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readString, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, mark, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, unmark, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, reset, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, skip, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, eof, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, write, hprose_bytes_io_write_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, toString, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(hprose_bytes_io, __toString, toString, hprose_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(bytes_io)

HPROSE_OBJECT_FREE_BEGIN(bytes_io)
    if (intern->_this) {
        hprose_bytes_io_free(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_NEW_BEGIN(bytes_io)
HPROSE_OBJECT_NEW_END(bytes_io)

HPROSE_CLASS_ENTRY(bytes_io)

HPROSE_STARTUP_FUNCTION(bytes_io) {
    HPROSE_REGISTER_CLASS("Hprose", "BytesIO", bytes_io);
    HPROSE_REGISTER_CLASS_HANDLERS(bytes_io);
    return SUCCESS;
}
