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
 * hprose_result_mode.c                                   *
 *                                                        *
 * hprose result mode for pecl source file.               *
 *                                                        *
 * LastModified: Mar 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_result_mode.h"

static zend_function_entry hprose_result_mode_methods[] = {
    ZEND_FE_END
};

#define DECLARE_RESULT_MODE(name, value) \
    zend_declare_class_constant_long(hprose_result_mode_ce, ZEND_STRL(name), value TSRMLS_CC);

HPROSE_CLASS_ENTRY(result_mode)

HPROSE_STARTUP_FUNCTION(result_mode) {
    HPROSE_REGISTER_CLASS("Hprose", "ResultMode", result_mode);
    DECLARE_RESULT_MODE("Normal",           HPROSE_RESULT_MODE_NORMAL)
    DECLARE_RESULT_MODE("Serialized",       HPROSE_RESULT_MODE_SERIALIZED)
    DECLARE_RESULT_MODE("Raw",              HPROSE_RESULT_MODE_RAW)
    DECLARE_RESULT_MODE("RawWithEndTag",    HPROSE_RESULT_MODE_RAW_WITH_END_TAG)
    return SUCCESS;
}
