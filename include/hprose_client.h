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
 * LastModified: Mar 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_CLIENT_H
#define	HPROSE_CLIENT_H

#include "hprose.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_proxy_ce();
zend_class_entry *get_hprose_client_ce();

HPROSE_STARTUP_FUNCTION(proxy);
HPROSE_STARTUP_FUNCTION(client);

typedef struct _hprose_client {
    zval *client;
    char *ns;
    zend_bool simple;
    zval *filters;
} hprose_client;

typedef struct {
    zval *client;
    char *ns;
} hprose_proxy;

HPROSE_CLASS_BEGIN(proxy)
HPROSE_CLASS_END(proxy)

HPROSE_CLASS_BEGIN(client)
HPROSE_CLASS_END(client)

END_EXTERN_C()

#endif	/* HPROSE_CLIENT_H */

