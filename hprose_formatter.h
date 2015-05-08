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
 * LastModified: May 8, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_FORMATTER_H
#define	HPROSE_FORMATTER_H

#include "hprose_common.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_formatter_ce();

HPROSE_STARTUP_FUNCTION(formatter);

static zend_always_inline void hprose_serialize(hprose_bytes_io *stream, zval *val, zend_bool simple TSRMLS_DC) {
    hprose_writer writer;
    hprose_writer_init(&writer, stream, simple);
    hprose_writer_serialize(&writer, val);
    hprose_writer_destroy(&writer);
}

static zend_always_inline void hprose_unserialize(hprose_bytes_io *stream, zend_bool simple, zval *return_value TSRMLS_DC) {
    hprose_reader reader;
    hprose_reader_init(&reader, stream, simple);
    hprose_reader_unserialize(&reader, return_value TSRMLS_CC);
    hprose_reader_destroy(&reader);
}

ZEND_FUNCTION(hprose_serialize);
ZEND_FUNCTION(hprose_unserialize);

END_EXTERN_C()

#endif	/* HPROSE_FORMATTER_H */
