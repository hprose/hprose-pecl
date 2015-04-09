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
 * hprose_client.h                                        *
 *                                                        *
 * hprose client for pecl header file.                    *
 *                                                        *
 * LastModified: Mar 27, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_CLIENT_H
#define	HPROSE_CLIENT_H

#include "hprose_common.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_proxy_ce();
zend_class_entry *get_hprose_client_ce();

HPROSE_STARTUP_FUNCTION(proxy);
HPROSE_STARTUP_FUNCTION(client);

typedef struct {
#if PHP_MAJOR_VERSION < 7
    zval *client;
#else
    zend_object *client;
#endif
    char *ns;
    zend_bool simple;
    zval *filters;
} hprose_client;

typedef struct {
#if PHP_MAJOR_VERSION < 7
    zval *client;
#else
    zend_object *client;
#endif
    char *ns;
} hprose_proxy;

HPROSE_CLASS_BEGIN(proxy)
HPROSE_CLASS_END(proxy)

HPROSE_CLASS_BEGIN(client)
HPROSE_CLASS_END(client)

END_EXTERN_C()

#endif	/* HPROSE_CLIENT_H */

