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
 * LastModified: Mar 28, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_SERVICE_H
#define	HPROSE_SERVICE_H

#include "hprose.h"

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

static zend_always_inline zend_bool is_magic_method(char *name, int32_t len) {
    static const char *magic_methods[14] = {
        "__construct",
        "__destruct",
        "__call",
        "__callStatic",
        "__get",
        "__set",
        "__isset",
        "__unset",
        "__sleep",
        "__wakeup",
        "__toString",
        "__invoke",
        "__set_state",
        "__clone"
    };
    static const int magic_methods_length[14] = {
        sizeof("__construct") - 1,
        sizeof("__destruct") - 1,
        sizeof("__call") - 1,
        sizeof("__callStatic") - 1,
        sizeof("__get") - 1,
        sizeof("__set") - 1,
        sizeof("__isset") - 1,
        sizeof("__unset") - 1,
        sizeof("__sleep") - 1,
        sizeof("__wakeup") - 1,
        sizeof("__toString") - 1,
        sizeof("__invoke") - 1,
        sizeof("__set_state") - 1,
        sizeof("__clone") - 1
    };
    int i, j;
    if (len < 5 || len > 12) return 0;
    if (name[0] != '_' || name[1] != '_') return 0;
    for (i = 0; i < 14; ++i) {
        zend_bool find = 1;
        if (len != magic_methods_length[i]) continue;
        for (j = 2; j < len; ++j) {
            if (magic_methods[i][j] != name[j]) {
                find = 0;
                break;
            }
        }
        if (find) return 1;
    }
    return 0;
}

HPROSE_CLASS_BEGIN(service)
HPROSE_CLASS_END(service)

END_EXTERN_C()

#endif	/* HPROSE_SERVICE_H */

