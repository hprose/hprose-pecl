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
 * php_hprose.h                                           *
 *                                                        *
 * hprose for pecl header file.                           *
 *                                                        *
 * LastModified: Mar 12, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef PHP_HPROSE_H
#define PHP_HPROSE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hprose_tags.h"
/*
#include "hprose_bytes_io.h"
#include "hprose_class_manager.h"
 */

#include "ext/standard/info.h" /* for phpinfo() functions */

BEGIN_EXTERN_C()

zend_module_entry hprose_module_entry;
#define phpext_hprose_ptr &hprose_module_entry

#define HPROSE_MODULE_NAME        "hprose"
#define HPROSE_BUILD_DATE         __DATE__ " " __TIME__
#define HPROSE_VERSION            "1.0.0"
#define HPROSE_AUTHOR             "Ma Bingyao"
#define HPROSE_HOMEPAGE           "https://github.com/hprose/hprose-pecl"

ZEND_MINIT_FUNCTION(hprose);
ZEND_MSHUTDOWN_FUNCTION(hprose);
ZEND_MINFO_FUNCTION(hprose);

/* declaration of functions to be exported */
ZEND_FUNCTION(hprose_serialize);
ZEND_FUNCTION(hprose_unserialize);
ZEND_FUNCTION(hprose_info);

END_EXTERN_C()

#endif /* ifndef PHP_HPROSE_H */
