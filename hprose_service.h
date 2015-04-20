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
 * hprose_service.h                                       *
 *                                                        *
 * hprose service for pecl header file.                   *
 *                                                        *
 * LastModified: Apr 20, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_SERVICE_H
#define	HPROSE_SERVICE_H

#include "hprose_common.h"
#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"
#include "hprose_result_mode.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_service_ce();

HPROSE_STARTUP_FUNCTION(service);

typedef struct {
    zend_fcall_info_cache fcc;
    uint8_t mode;
    uint8_t simple;
    zend_bool byref;
} hprose_remote_call;

typedef struct {
    HashTable *calls;
    zval *names;
    zval *filters;
    zend_bool simple;
} hprose_service;

HPROSE_CLASS_BEGIN(service)
HPROSE_CLASS_END(service)

END_EXTERN_C()

#endif	/* HPROSE_SERVICE_H */

