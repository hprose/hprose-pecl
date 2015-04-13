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
 * hprose_raw_reader.h                                    *
 *                                                        *
 * hprose raw reader for pecl header file.                *
 *                                                        *
 * LastModified: Apr 13, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_RAW_READER_H
#define	HPROSE_RAW_READER_H

#include "hprose_common.h"
#include "hprose_tags.h"
#include "hprose_bytes_io.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_raw_reader_ce();

HPROSE_STARTUP_FUNCTION(raw_reader);

typedef struct {
    hprose_bytes_io *stream;
} hprose_raw_reader;

static zend_always_inline void unexpected_tag(char tag, char *expected_tags TSRMLS_DC) {
    if (tag && expected_tags) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Tag '%s' expected, but '%c' found in stream", expected_tags, tag);
    }
    else if (tag) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC, "Unexpected serialize tag '%c' in stream", tag);
    }
    zend_throw_exception(NULL, "No byte found in stream", 0 TSRMLS_CC);
}

static zend_always_inline hprose_raw_reader * hprose_raw_reader_create(hprose_bytes_io *stream) {
    hprose_raw_reader *_this = emalloc(sizeof(hprose_raw_reader));
    _this->stream = stream;
    return _this;
}

static zend_always_inline void hprose_raw_reader_free(hprose_raw_reader *_this) {
    _this->stream = NULL;
    efree(_this);
}

static zend_always_inline void hprose_raw_reader_read_num_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream);
static zend_always_inline void hprose_raw_reader_read_datetime_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream);
static zend_always_inline void hprose_raw_reader_read_utf8char_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream TSRMLS_DC);
static zend_always_inline void hprose_raw_reader_read_bytes_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream);
static zend_always_inline void hprose_raw_reader_read_string_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream TSRMLS_DC);
static zend_always_inline void hprose_raw_reader_read_guid_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream);
static void hprose_raw_reader_read_complex_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream TSRMLS_DC);

#define _hprose_raw_reader_read_raw(_this, ostream) __hprose_raw_reader_read_raw((_this), (ostream), hprose_bytes_io_getc((_this)->stream) TSRMLS_CC)

static void __hprose_raw_reader_read_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream, char tag TSRMLS_DC) {
    hprose_bytes_io_putc(ostream, tag);
    switch (tag) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
        case HPROSE_TAG_NULL:
        case HPROSE_TAG_EMPTY:
        case HPROSE_TAG_TRUE:
        case HPROSE_TAG_FALSE:
        case HPROSE_TAG_NAN:
            break;
        case HPROSE_TAG_INFINITY:
            hprose_bytes_io_getc_to(_this->stream, ostream);
            break;
        case HPROSE_TAG_INTEGER:
        case HPROSE_TAG_LONG:
        case HPROSE_TAG_DOUBLE:
        case HPROSE_TAG_REF:
            hprose_raw_reader_read_num_raw(_this, ostream);
            break;
        case HPROSE_TAG_DATE:
        case HPROSE_TAG_TIME:
            hprose_raw_reader_read_datetime_raw(_this, ostream);
            break;
        case HPROSE_TAG_UTF8CHAR:
            hprose_raw_reader_read_utf8char_raw(_this, ostream TSRMLS_CC);
            break;
        case HPROSE_TAG_BYTES:
            hprose_raw_reader_read_bytes_raw(_this, ostream);
            break;
        case HPROSE_TAG_STRING:
            hprose_raw_reader_read_string_raw(_this, ostream TSRMLS_CC);
            break;
        case HPROSE_TAG_GUID:
            hprose_raw_reader_read_guid_raw(_this, ostream);
            break;
        case HPROSE_TAG_LIST:
        case HPROSE_TAG_MAP:
        case HPROSE_TAG_OBJECT:
            hprose_raw_reader_read_complex_raw(_this, ostream TSRMLS_CC);
            break;
        case HPROSE_TAG_CLASS:
            hprose_raw_reader_read_complex_raw(_this, ostream TSRMLS_CC);
            _hprose_raw_reader_read_raw(_this, ostream);
            break;
        case HPROSE_TAG_ERROR:
            _hprose_raw_reader_read_raw(_this, ostream);
            break;
        default:
            unexpected_tag(tag, NULL TSRMLS_CC);
    }
}

static zend_always_inline hprose_bytes_io *hprose_raw_reader_read_raw(hprose_raw_reader *_this TSRMLS_DC) {
    hprose_bytes_io *ostream = hprose_bytes_io_new();
    _hprose_raw_reader_read_raw(_this, ostream);
    return ostream;
}

static zend_always_inline void hprose_raw_reader_read_num_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream) {
    hprose_bytes_io_readuntil_to(_this->stream, ostream, HPROSE_TAG_SEMICOLON, 1);
}

static zend_always_inline void hprose_raw_reader_read_datetime_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream) {
    char tag;
    do {
        tag = hprose_bytes_io_getc(_this->stream);
        hprose_bytes_io_putc(ostream, tag);
    } while (tag != HPROSE_TAG_SEMICOLON && tag != HPROSE_TAG_UTC);
}

static zend_always_inline void hprose_raw_reader_read_utf8char_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream TSRMLS_DC) {
    hprose_bytes_io_read_string_to(_this->stream, ostream, 1);
}

static zend_always_inline void hprose_raw_reader_read_bytes_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream) {
    int32_t len = hprose_bytes_io_read_int_to(_this->stream, ostream, HPROSE_TAG_QUOTE, 1);
    hprose_bytes_io_read_to(_this->stream, ostream, len + 1);
}

static zend_always_inline void hprose_raw_reader_read_string_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream TSRMLS_DC) {
    int32_t len = hprose_bytes_io_read_int_to(_this->stream, ostream, HPROSE_TAG_QUOTE, 1);
    hprose_bytes_io_read_string_to(_this->stream, ostream, len + 1);
}

static zend_always_inline void hprose_raw_reader_read_guid_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream) {
    hprose_bytes_io_read_to(_this->stream, ostream, 38);
}

static zend_always_inline void hprose_raw_reader_read_complex_raw(hprose_raw_reader *_this, hprose_bytes_io *ostream TSRMLS_DC) {
    char tag;
    hprose_bytes_io_readuntil_to(_this->stream, ostream, HPROSE_TAG_OPENBRACE, 1);
    while ((tag = hprose_bytes_io_getc(_this->stream)) != HPROSE_TAG_CLOSEBRACE) {
        __hprose_raw_reader_read_raw(_this, ostream, tag TSRMLS_CC);
    }
    hprose_bytes_io_putc(ostream, tag);
}

HPROSE_CLASS_BEGIN(raw_reader)
HPROSE_CLASS_END(raw_reader)

END_EXTERN_C()

#endif	/* HPROSE_RAW_READER_H */

