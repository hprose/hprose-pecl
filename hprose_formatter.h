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
 * LastModified: Sep 1, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_FORMATTER_H
#define	HPROSE_FORMATTER_H

#include "hprose_common.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"
#if HAVE_PHP_SESSION
#include "ext/session/php_session.h" /* for php_session_register_serializer */
#endif

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_formatter_ce();

HPROSE_STARTUP_FUNCTION(formatter);

#if HAVE_PHP_SESSION
PS_SERIALIZER_FUNCS(hprose);
#endif

ZEND_FUNCTION(hprose_serialize);
ZEND_FUNCTION(hprose_unserialize);

END_EXTERN_C()

#endif	/* HPROSE_FORMATTER_H */
