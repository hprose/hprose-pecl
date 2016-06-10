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
 * LastModified: Jun 10, 2016                             *
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
}

ZEND_METHOD(hprose_bytes_io, close) {
    HPROSE_OBJECT_INTERN(bytes_io);
    hprose_bytes_io_close(intern->_this);
    intern->mark = -1;
}

ZEND_METHOD(hprose_bytes_io, length) {
    HPROSE_THIS(bytes_io);
    if (HB_INITED_P(_this)) {
        RETURN_LONG(HB_LEN_P(_this));
    }
    else {
        RETURN_LONG(0);
    }
}

ZEND_METHOD(hprose_bytes_io, getc) {
    HPROSE_THIS(bytes_io);
    if (HB_INITED_P(_this) && (HB_POS_P(_this) < HB_LEN_P(_this))) {
#if PHP_MAJOR_VERSION < 7
        RETURN_STRINGL_0(hprose_bytes_io_read(_this, 1), 1);
#else
        RETURN_STR(hprose_bytes_io_read(_this, 1));
#endif
    }
    RETURN_EMPTY_STRING();
}

ZEND_METHOD(hprose_bytes_io, read) {
    long n;
    HPROSE_THIS(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        RETURN_NULL();
    }
    if (HB_INITED_P(_this) && (HB_POS_P(_this) + n > HB_LEN_P(_this))) {
        n = HB_LEN_P(_this) - HB_POS_P(_this);
    }
#if PHP_MAJOR_VERSION < 7
    RETURN_STRINGL_0(hprose_bytes_io_read(_this, n), n);
#else
    RETURN_STR(hprose_bytes_io_read(_this, n));
#endif
}

ZEND_METHOD(hprose_bytes_io, readfull) {
    HPROSE_THIS(bytes_io);
#if PHP_MAJOR_VERSION < 7
    int32_t l;
    char *s = hprose_bytes_io_readfull(_this, &l);
    RETURN_STRINGL_0(s, l);
#else
    RETURN_STR(hprose_bytes_io_readfull(_this));
#endif
}

ZEND_METHOD(hprose_bytes_io, readuntil) {
#if PHP_MAJOR_VERSION < 7
    char *s;
    int32_t l;
#else
    zend_string *s;
#endif
    char *tag;
    length_t len;
    HPROSE_THIS(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &tag, &len) == FAILURE) {
        RETURN_NULL();
    }
#if PHP_MAJOR_VERSION < 7
    if (len > 0) {
        s = hprose_bytes_io_readuntil(_this, tag[0], &l);
    }
    else {
        s = hprose_bytes_io_readfull(_this, &l);
    }
    RETURN_STRINGL_0(s, l);
#else
    if (len > 0) {
        s = hprose_bytes_io_readuntil(_this, tag[0]);
    }
    else {
        s = hprose_bytes_io_readfull(_this);
    }
    RETURN_STR(s);
#endif
}

ZEND_METHOD(hprose_bytes_io, readString) {
    long n;
    HPROSE_THIS(bytes_io);
#if PHP_MAJOR_VERSION < 7
    char *s;
    int32_t l;
#endif
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &n) == FAILURE) {
        RETURN_NULL();
    }
#if PHP_MAJOR_VERSION < 7
    s = hprose_bytes_io_read_string(_this, n, &l);
    RETURN_STRINGL_0(s, l);
#else
    RETURN_STR(hprose_bytes_io_read_string(_this, n));
#endif
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
    if (HB_INITED_P(_this) && (n > 0)) {
        if (n > HB_LEN_P(_this) - HB_POS_P(_this)) {
            HB_POS_P(_this) = HB_LEN_P(_this);
        }
        else {
            HB_POS_P(_this) += n;
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
    if (n > len || n < 0) n = len;
    hprose_bytes_io_write(_this, str, n);
}

ZEND_METHOD(hprose_bytes_io, toString) {
    HPROSE_THIS(bytes_io);
#if PHP_MAJOR_VERSION < 7
    RETURN_STRINGL_0(hprose_bytes_io_to_string(_this), _this->len);
#else
    RETURN_STR(hprose_bytes_io_to_string(_this));
#endif
}

ZEND_METHOD(hprose_bytes_io, load) {
    php_stream *stream;
    char *filename;
#if PHP_MAJOR_VERSION < 7
    char *buf;
#else
    zend_string *s;
#endif
    length_t len;
    HPROSE_OBJECT_INTERN(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &len) == FAILURE) {
        return;
    }
    stream = php_stream_open_wrapper(filename, "rb", REPORT_ERRORS, NULL);
    if (stream == NULL) {
        RETURN_FALSE;
    }
#if PHP_MAJOR_VERSION < 7
    if ((len = php_stream_copy_to_mem(stream, &buf, PHP_STREAM_COPY_ALL, HB_PERSISTENT_P(intern->_this))) > 0) {
        hprose_bytes_io_close(intern->_this);
        intern->_this->buf = buf;
        HB_CAP_P(intern->_this) = len;
        HB_LEN_P(intern->_this) = len;
        HB_POS_P(intern->_this) = 0;
        intern->mark = -1;
    }
#else
    if ((s = php_stream_copy_to_mem(stream, PHP_STREAM_COPY_ALL, HB_PERSISTENT_P(intern->_this))) != NULL) {
        hprose_bytes_io_close(intern->_this);
        intern->_this->s = s;
        HB_CAP_P(intern->_this) = HB_LEN_P(intern->_this);
        HB_POS_P(intern->_this) = 0;
        intern->mark = -1;
    }
#endif
    else if (len == 0) {
        hprose_bytes_io_close(intern->_this);
        intern->mark = -1;
    }
    else {
        php_stream_close(stream);
        RETURN_FALSE;
    }
    php_stream_close(stream);
    RETURN_TRUE;
}

ZEND_METHOD(hprose_bytes_io, save) {
    php_stream *stream;
    char *filename;
    length_t len;
    int32_t numbytes = 0;
    HPROSE_THIS(bytes_io);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &filename, &len) == FAILURE) {
        return;
    }
    stream = php_stream_open_wrapper(filename, "wb", REPORT_ERRORS, NULL);
    if (stream == NULL) {
        RETURN_FALSE;
    }
    if (HB_INITED_P(_this) && HB_LEN_P(_this)) {
        numbytes = php_stream_write(stream, HB_BUF_P(_this), HB_LEN_P(_this));
        if (numbytes != HB_LEN_P(_this)) {
            php_error_docref(NULL TSRMLS_CC, E_WARNING, "Only %d of %d bytes written, possibly out of free disk space", numbytes, HB_LEN_P(_this));
            numbytes = -1;
        }
    }
    php_stream_close(stream);
    RETURN_LONG(numbytes);    
}

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

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

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_load_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_bytes_io_save_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, filename)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_bytes_io_methods[] = {
    ZEND_ME(hprose_bytes_io, __construct, hprose_bytes_io_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_bytes_io, close, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, length, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, getc, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, read, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readfull, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readuntil, hprose_bytes_io_readuntil_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, readString, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, mark, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, unmark, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, reset, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, skip, hprose_bytes_io_long_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, eof, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, write, hprose_bytes_io_write_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, load, hprose_bytes_io_load_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, save, hprose_bytes_io_save_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_bytes_io, toString, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_MALIAS(hprose_bytes_io, __toString, toString, hprose_bytes_io_void_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(bytes_io)

HPROSE_OBJECT_FREE_BEGIN(bytes_io)
    if (intern->_this) {
        hprose_bytes_io_free(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_NEW_EX_BEGIN(bytes_io)
    intern->mark = -1;
HPROSE_OBJECT_NEW_EX_END(bytes_io)

HPROSE_OBJECT_NEW(bytes_io)

HPROSE_OBJECT_CLONE_BEGIN(bytes_io)
    if (old_obj->_this) {
        if (HB_INITED_P(old_obj->_this)) {
            new_obj->_this = hprose_bytes_io_pcreate(HB_BUF_P(old_obj->_this), HB_LEN_P(old_obj->_this), HB_PERSISTENT_P(old_obj->_this));
            HB_POS_P(new_obj->_this) = HB_POS_P(old_obj->_this);
        }
        else {
            new_obj->_this = hprose_bytes_io_pnew(HB_PERSISTENT_P(old_obj->_this));
        }
    }
    new_obj->mark = old_obj->mark;
HPROSE_OBJECT_CLONE_END

HPROSE_CLASS_ENTRY(bytes_io)

HPROSE_STARTUP_FUNCTION(bytes_io) {
    HPROSE_REGISTER_CLASS("Hprose", "BytesIO", bytes_io);
    HPROSE_REGISTER_CLASS_HANDLERS(bytes_io);
    HPROSE_REGISTER_CLASS_CLONE_HANDLER(bytes_io);
    return SUCCESS;
}
