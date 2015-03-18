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
 * LastModified: Mar 16, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "php_hprose.h"

ZEND_MINIT_FUNCTION(hprose) {
    HPROSE_STARTUP(tags);
    HPROSE_STARTUP(bytes_io);
    HPROSE_STARTUP(class_manager);
    HPROSE_STARTUP(writer);
    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(hprose) {
    return SUCCESS;
}

ZEND_RINIT_FUNCTION(hprose) {
#if PHP_MAJOR_VERSION >= 7 && defined(COMPILE_DL_HPROSE) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    HPROSE_ACTIVATE(class_manager);
    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(hprose) {
    HPROSE_DEACTIVATE(class_manager);
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
#if PHP_MAJOR_VERSION < 7
    add_assoc_string(return_value, "ext_version", HPROSE_VERSION, 1);
    add_assoc_string(return_value, "ext_build_date", HPROSE_BUILD_DATE, 1);
    add_assoc_string(return_value, "ext_author", HPROSE_AUTHOR, 1);
#else
    add_assoc_string(return_value, "ext_version", HPROSE_VERSION);
    add_assoc_string(return_value, "ext_build_date", HPROSE_BUILD_DATE);
    add_assoc_string(return_value, "ext_author", HPROSE_AUTHOR);
#endif
}

ZEND_BEGIN_ARG_INFO_EX(hprose_serialize_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, val)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_unserialize_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, data)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_info_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

/* compiled function list so Zend knows what's in this module */
const zend_function_entry hprose_functions[] = {
    ZEND_FE(hprose_serialize, hprose_serialize_arginfo)
    ZEND_FE(hprose_unserialize, hprose_unserialize_arginfo)
    ZEND_FE(hprose_info, hprose_info_arginfo)
    ZEND_FE_END
};

ZEND_DECLARE_MODULE_GLOBALS(hprose)

static ZEND_GINIT_FUNCTION(hprose) {
    hprose_globals->cache1 = NULL;
    hprose_globals->cache2 = NULL;
}

static ZEND_GSHUTDOWN_FUNCTION(hprose) {
    if (hprose_globals->cache1) {
        zend_hash_destroy(hprose_globals->cache1);
        FREE_HASHTABLE(hprose_globals->cache1);
        hprose_globals->cache1 = NULL;
    }
    if (hprose_globals->cache2) {
        zend_hash_destroy(hprose_globals->cache2);
        FREE_HASHTABLE(hprose_globals->cache2);
        hprose_globals->cache2 = NULL;
    }
}

/* compiled module information */
zend_module_entry hprose_module_entry = {
    STANDARD_MODULE_HEADER,
    HPROSE_MODULE_NAME,            /* extension name */
    hprose_functions,              /* function list */
    ZEND_MINIT(hprose),            /* process startup */
    ZEND_MSHUTDOWN(hprose),        /* process shutdown */
    ZEND_RINIT(hprose),            /* request startup */
    ZEND_RSHUTDOWN(hprose),        /* request shutdown */
    ZEND_MINFO(hprose),            /* extension info */
    HPROSE_VERSION,                /* extension version */
    ZEND_MODULE_GLOBALS(hprose),   /* globals descriptor */
    ZEND_GINIT(hprose),            /* globals ctor */
    ZEND_GSHUTDOWN(hprose),        /* globals dtor */
    NULL,                          /* post deactivate */
    STANDARD_MODULE_PROPERTIES_EX
};

#ifdef COMPILE_DL_HPROSE
#if PHP_MAJOR_VERSION >= 7 && defined(ZTS)
ZEND_TSRMLS_CACHE_DEFINE();
#endif
ZEND_GET_MODULE(hprose)
#endif