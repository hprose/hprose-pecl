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

ZEND_FUNCTION(hprose_serialize);
ZEND_FUNCTION(hprose_unserialize);

END_EXTERN_C()

#endif	/* HPROSE_FORMATTER_H */
