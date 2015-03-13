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
 * hprose_class_manager.h                                 *
 *                                                        *
 * hprose class manager for pecl header file.             *
 *                                                        *
 * LastModified: Mar 11, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_CLASS_MANAGER_H
#define	HPROSE_CLASS_MANAGER_H

#include "hprose_common.h"
#if PHP_MAJOR_VERSION < 7
#include "ext/standard/php_smart_str.h"
#else
#include "ext/standard/php_smart_string.h"
#endif

BEGIN_EXTERN_C()

extern zend_class_entry *hprose_class_manager_ce;

HPROSE_STARTUP_FUNCTION(class_manager);

extern void hprose_class_manager_register(const char *classname, int nameLen, const char *alias, int aliasLen TSRMLS_DC);
extern smart_str hprose_class_manager_get_alias(const char *classname, int len TSRMLS_DC);
extern smart_str hprose_class_manager_get_class(const char *alias, int len TSRMLS_DC);

END_EXTERN_C()

#endif	/* HPROSE_CLASS_MANAGER_H */
