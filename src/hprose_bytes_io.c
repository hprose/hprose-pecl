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
 * LastModified: Mar 12, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_bytes_io.h"

HPROSE_CLASS_EX(bytes_io, bytes);

HPROSE_OBJECT_FREE(bytes_io);

HPROSE_OBJECT_NEW(bytes_io);

ZEND_METHOD(hprose_bytes_io, __construct) {
    char *buf = NULL;
    int len = 0;
    HPROSE_OBJECT(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &buf, &len) == FAILURE) {
        return;
    }
    bytes_io->bytes = hprose_bytes_io_create(buf, len);
}

ZEND_METHOD(hprose_bytes_io, __destruct) {
    HPROSE_OBJECT(bytes_io);
    if (bytes_io->bytes) {
        hprose_bytes_io_free(bytes_io->bytes);
        bytes_io->bytes = NULL;
    }
}

ZEND_METHOD(hprose_bytes_io, close) {
    HPROSE_OBJECT(bytes_io);
    hprose_bytes_io_close(bytes_io->bytes);
}

ZEND_METHOD(hprose_bytes_io, length) {
    HPROSE_OBJECT(bytes_io);
    RETURN_LONG(hprose_bytes_io_length(bytes_io->bytes));
}

ZEND_METHOD(hprose_bytes_io, getc) {
    char c;
    HPROSE_OBJECT_EX(bytes_io, bytes);
    if (bytes->pos < hprose_bytes_io_length(bytes)) {
        c = hprose_bytes_io_getc(bytes);
        RETURN_STRINGL(&c, 1, 1);
    }
    RETURN_EMPTY_STRING();
}

ZEND_METHOD(hprose_bytes_io, read) {
    smart_str s;
    long n, l;
    HPROSE_OBJECT_EX(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        RETURN_NULL();
    }
    l = hprose_bytes_io_length(bytes);
    if (bytes->pos + n > l) {
        n = l - bytes->pos;
    }
    s = hprose_bytes_io_read(bytes, n);
    RETURN_STRINGL(s.c, s.len, 0);
}

ZEND_METHOD(hprose_bytes_io, readfull) {
    smart_str s;
    HPROSE_OBJECT(bytes_io);
    s = hprose_bytes_io_readfull(bytes_io->bytes);
    RETURN_STRINGL(s.c, s.len, 0);
}

ZEND_METHOD(hprose_bytes_io, readuntil) {
    smart_str s;
    char *tag;
    int len;
    HPROSE_OBJECT_EX(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &tag, &len) == FAILURE) {
        RETURN_NULL();
    }
    if (len > 0) {
        s = hprose_bytes_io_readuntil(bytes, tag[0], 1);
    }
    else {
        s = hprose_bytes_io_readfull(bytes);
    }
    RETURN_STRINGL(s.c, s.len, 0);
}

ZEND_METHOD(hprose_bytes_io, readString) {
    smart_str s;
    long n;
    HPROSE_OBJECT_EX(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        RETURN_NULL();
    }
    s = hprose_bytes_io_read_string(bytes, n TSRMLS_CC);
    RETURN_STRINGL(s.c, s.len, 0);
}

ZEND_METHOD(hprose_bytes_io, mark) {
    HPROSE_OBJECT(bytes_io);
    hprose_bytes_io_mark(bytes_io->bytes);
}

ZEND_METHOD(hprose_bytes_io, unmark) {
    HPROSE_OBJECT(bytes_io);
    hprose_bytes_io_unmark(bytes_io->bytes);
}

ZEND_METHOD(hprose_bytes_io, reset) {
    HPROSE_OBJECT(bytes_io);
    hprose_bytes_io_reset(bytes_io->bytes);
}

ZEND_METHOD(hprose_bytes_io, skip) {
    long n;
    HPROSE_OBJECT_EX(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        return;
    }
    if (n > 0) {
        if (n > bytes->buffer.len - bytes->pos) {
            n = bytes->buffer.len - bytes->pos;
        }
        hprose_bytes_io_skip(bytes, n);
    }
}

ZEND_METHOD(hprose_bytes_io, eof) {
    HPROSE_OBJECT(bytes_io);
    RETURN_BOOL(hprose_bytes_io_eof(bytes_io->bytes));
}

ZEND_METHOD(hprose_bytes_io, write) {
    char *str;
    int len;
    long n = -1;
    HPROSE_OBJECT_EX(bytes_io, bytes);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &str, &len, &n) == FAILURE) {
        return;
    }
    if (n > len) n = len;
    hprose_bytes_io_write(bytes, str, n);
}

ZEND_METHOD(hprose_bytes_io, toString) {
    HPROSE_OBJECT_EX(bytes_io, bytes);
    RETURN_STRINGL(bytes->buffer.c, bytes->buffer.len, 1);
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
    {NULL, NULL, NULL}
};

zend_class_entry *hprose_bytes_io_ce;

HPROSE_STARTUP_FUNCTION(bytes_io) {
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Hprose", "BytesIO", hprose_bytes_io_methods)
    hprose_bytes_io_ce = zend_register_internal_class(&ce TSRMLS_CC);
    hprose_bytes_io_ce->create_object = php_hprose_bytes_io_new;
    zend_register_class_alias("HproseBytesIO", hprose_bytes_io_ce);
    return SUCCESS;
}
