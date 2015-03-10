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
 * php_hprose.c                                           *
 *                                                        *
 * hprose for pecl source file.                           *
 *                                                        *
 * LastModified: Mar 10, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "php_hprose.h"

/* compiled function list so Zend knows what's in this module */
const zend_function_entry hprose_functions[] = {
    ZEND_FE(hprose_serialize, NULL)
    ZEND_FE(hprose_unserialize, NULL)
    ZEND_FE(hprose_info, NULL)
    {NULL, NULL, NULL}
};

/* compiled module information */
zend_module_entry hprose_module_entry = {
    STANDARD_MODULE_HEADER,
    HPROSE_MODULE_NAME,
    hprose_functions,
    ZEND_MINIT(hprose),
    ZEND_MSHUTDOWN(hprose),
    NULL,
    NULL,
    ZEND_MINFO(hprose),
    HPROSE_VERSION,
    STANDARD_MODULE_PROPERTIES
};

/* implement standard "stub" routine to introduce ourselves to Zend */
#if defined(COMPILE_DL_HPROSE)
ZEND_GET_MODULE(hprose)
#endif

ZEND_MINIT_FUNCTION(hprose) {
    register_hprose_tags();
    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(hprose) {
    return SUCCESS;
}

ZEND_MINFO_FUNCTION(hprose) {
    php_info_print_table_start();
    php_info_print_table_row(2, "hprose support", "enabled");
    php_info_print_table_row(2, "hprose version", HPROSE_VERSION);
    php_info_print_table_row(2, "hprose author", HPROSE_AUTHOR);
    php_info_print_table_row(2, "hprose homepage", HPROSE_HOMEPAGE);
    php_info_print_table_end();
}

/* {{{ proto string hprose_serialize(mixed val, bool simple = false)
   serialize php value to hprose format data  */
ZEND_FUNCTION(hprose_serialize) {
    zval *val;
    zend_bool simple;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|b", &val, &simple) == FAILURE) {
        return;
    }
}
/* }}} */

/* {{{ proto mixed hprose_unserialize(string data, bool simple = false)
    unserialize hprose format data to php value */
ZEND_FUNCTION(hprose_unserialize) {
    char *data;
    int data_len;
    zend_bool simple;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &data, &data_len, &simple) == FAILURE) {
        return;
    }
}
/* }}} */

ZEND_FUNCTION(hprose_info) {
    array_init(return_value);
    add_assoc_string(return_value, "ext_version", HPROSE_VERSION, 1);
    add_assoc_string(return_value, "ext_build_date", HPROSE_BUILD_DATE, 1);
    add_assoc_string(return_value, "ext_author", HPROSE_AUTHOR, 1);
}
