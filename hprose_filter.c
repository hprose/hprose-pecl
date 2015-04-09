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
 * hprose_filter.c                                        *
 *                                                        *
 * hprose filter for pecl source file.                    *
 *                                                        *
 * LastModified: Mar 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_filter.h"

ZEND_BEGIN_ARG_INFO_EX(hprose_filter_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_filter_methods[] = {
    ZEND_ABSTRACT_ME(hprose_filter, inputFilter, hprose_filter_arginfo)
    ZEND_ABSTRACT_ME(hprose_filter, outputFilter, hprose_filter_arginfo)
    ZEND_FE_END
};

HPROSE_CLASS_ENTRY(filter)

HPROSE_STARTUP_FUNCTION(filter) {
    HPROSE_REGISTER_INTERFACE("Hprose", "Filter", filter);
    return SUCCESS;
}
