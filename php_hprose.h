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
 * LastModified: Apr 2, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef PHP_HPROSE_H
#define PHP_HPROSE_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_class_manager.h"
#include "hprose_writer.h"
#include "hprose_raw_reader.h"
#include "hprose_reader.h"
#include "hprose_formatter.h"
#include "hprose_result_mode.h"
#include "hprose_filter.h"
#include "hprose_client.h"
#include "hprose_service.h"

#include "ext/standard/info.h" /* for phpinfo() functions */

BEGIN_EXTERN_C()

zend_module_entry hprose_module_entry;
#define phpext_hprose_ptr &hprose_module_entry

#define PHP_HPROSE_MODULE_NAME        "hprose"
#define PHP_HPROSE_BUILD_DATE         __DATE__ " " __TIME__
#define PHP_HPROSE_VERSION            "1.1.0"
#define PHP_HPROSE_AUTHOR             "Ma Bingyao"
#define PHP_HPROSE_HOMEPAGE           "https://github.com/hprose/hprose-pecl"

END_EXTERN_C()

#endif /* ifndef PHP_HPROSE_H */
