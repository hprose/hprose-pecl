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
 * LastModified: May 13, 2015                             *
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
HPROSE_STARTUP_FUNCTION(async_callback);
HPROSE_STARTUP_FUNCTION(after_invoke_callback);

typedef struct {
    zend_fcall_info_cache fcc;
    uint8_t mode;
    uint8_t simple;
    zend_bool byref;
    zend_bool async;
} hprose_remote_call;

typedef struct {
    HashTable *calls;
    zend_llist *objects;
    zval *names;
    zval *filters;
    zend_bool simple;
} hprose_service;

typedef struct {
    zval *completer;
} hprose_async_callback;

typedef struct {
    zval *service;
    zval *completer;
    zval *name;
    zval *args;
    zend_bool byref;
    uint8_t mode;
    zend_bool simple;
    zval *context;
} hprose_after_invoke_callback;

HPROSE_CLASS_BEGIN(async_callback)
HPROSE_CLASS_END(async_callback)

HPROSE_CLASS_BEGIN(after_invoke_callback)
HPROSE_CLASS_END(after_invoke_callback)

HPROSE_CLASS_BEGIN(service)
HPROSE_CLASS_END(service)

END_EXTERN_C()

#endif	/* HPROSE_SERVICE_H */

