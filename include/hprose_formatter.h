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
 * hprose_formatter.h                                     *
 *                                                        *
 * hprose formatter for pecl header file.                 *
 *                                                        *
 * LastModified: Mar 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_FORMATTER_H
#define	HPROSE_FORMATTER_H

#include "hprose.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_formatter_ce();

HPROSE_STARTUP_FUNCTION(formatter);

static zend_always_inline hprose_bytes_io *hprose_serialize(zval *val, zend_bool simple TSRMLS_DC) {
    hprose_bytes_io *stream = hprose_bytes_io_new();
    hprose_writer *writer = hprose_writer_create(stream, simple);
    hprose_writer_serialize(writer, val TSRMLS_CC);
    hprose_writer_free(writer);
    return stream;
}

static zend_always_inline void hprose_unserialize(hprose_bytes_io *stream, zend_bool simple, zval *return_value TSRMLS_DC) {
    hprose_reader *reader = hprose_reader_create(stream, simple);
    hprose_reader_unserialize(reader, return_value TSRMLS_CC);
    hprose_reader_free(reader);
}

ZEND_FUNCTION(hprose_serialize);
ZEND_FUNCTION(hprose_unserialize);

END_EXTERN_C()

#endif	/* HPROSE_FORMATTER_H */
