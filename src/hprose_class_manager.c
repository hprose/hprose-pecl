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
 * hprose_class_manager.c                                 *
 *                                                        *
 * hprose class manager for pecl source file.             *
 *                                                        *
 * LastModified: Mar 12, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_class_manager.h"

ZEND_BEGIN_MODULE_GLOBALS(hprose_class_manager)
HashTable *classCache1;
HashTable *classCache2;
ZEND_END_MODULE_GLOBALS(hprose_class_manager)

#ifdef ZTS
#define HPROSE_CLASS_MANAGER_G(v) TSRMG(hprose_class_manager_globals_id, zend_hprose_class_manager_globals *, v)
#else
#define HPROSE_CLASS_MANAGER_G(v) (hprose_class_manager_globals.v)
#endif

ZEND_DECLARE_MODULE_GLOBALS(hprose_class_manager)

void smart_str_dtor(void *s) {
    if (s) {
        smart_str_free_ex(s, 1);
        pefree(s, 1);
    }
}

static void hprose_class_manager_ctor(zend_hprose_class_manager_globals * _globals) {
    ALLOC_HASHTABLE(_globals->classCache1);
    ALLOC_HASHTABLE(_globals->classCache2);
    zend_hash_init(_globals->classCache1, 64, NULL, &smart_str_dtor, 1);
    zend_hash_init(_globals->classCache2, 64, NULL, &smart_str_dtor, 1);
}

static void hprose_class_manager_dtor(zend_hprose_class_manager_globals * _globals) {
    if (_globals->classCache1) {
        zend_hash_destroy(_globals->classCache1);
        FREE_HASHTABLE(_globals->classCache1);
        _globals->classCache1 = NULL;
    }
    if (_globals->classCache2) {
        zend_hash_destroy(_globals->classCache2);
        FREE_HASHTABLE(_globals->classCache2);
        _globals->classCache2 = NULL;
    }
}

void hprose_class_manager_register(const char *classname, int namelen, const char *alias, int aliaslen TSRMLS_DC) {
    smart_str *_classname = pemalloc(sizeof(smart_str), 1);
    smart_str *_alias = pemalloc(sizeof(smart_str), 1);
    memset(_classname, 0, sizeof(smart_str));
    memset(_alias, 0, sizeof(smart_str));
    smart_str_appendl_ex(_classname, classname, namelen, 1);
    smart_str_appendl_ex(_alias, alias, aliaslen, 1);
    smart_str_0(_classname);
    smart_str_0(_alias);
    zend_hash_update(HPROSE_CLASS_MANAGER_G(classCache1), classname, namelen + 1, &_alias, sizeof(_alias), NULL);
    zend_hash_update(HPROSE_CLASS_MANAGER_G(classCache2), alias, aliaslen + 1, &_classname, sizeof(_classname), NULL);
}

smart_str hprose_class_manager_get_alias(const char *classname, int len TSRMLS_DC) {
    smart_str alias = {0}, **_alias;
    if (zend_hash_find(HPROSE_CLASS_MANAGER_G(classCache1), classname, len + 1, (void **)&_alias) == FAILURE) {
        smart_str_appendl(&alias, classname, len);
        smart_str_0(&alias);
        php_str_replace('\\', '_', alias.c, alias.len);
        hprose_class_manager_register(classname, len, alias.c, alias.len TSRMLS_CC);
        return alias;
    }
    else {
        smart_str_append(&alias, *_alias);
        smart_str_0(&alias);
    }
    return alias;
}

smart_str hprose_class_manager_get_class(const char *alias, int len TSRMLS_DC) {
    smart_str classname = {0}, **_classname;
    if (zend_hash_find(HPROSE_CLASS_MANAGER_G(classCache2), alias, len + 1, (void **)&_classname) == FAILURE) {
        smart_str_appendl(&classname, alias, len);
        smart_str_0(&classname);
        if (!php_class_exists(alias, len, 0 TSRMLS_CC) &&
            !php_class_exists(alias, len, 1 TSRMLS_CC)) {
            php_str_replace('_', '\\', classname.c, classname.len);
            if (php_class_exists(classname.c, classname.len, 0 TSRMLS_CC) ||
                php_class_exists(classname.c, classname.len, 1 TSRMLS_CC)) {
                hprose_class_manager_register(classname.c, classname.len, alias, len TSRMLS_CC);
            }
            else {
                smart_str_free(&classname);
                smart_str_appendl(&classname, "stdClass", sizeof("stdClass") - 1);
                smart_str_0(&classname);
            }
        }
    }
    else {
        smart_str_append(&classname, *_classname);
        smart_str_0(&classname);
    }
    return classname;
}

ZEND_FUNCTION(register) {
    char *classname, *alias;
    int namelen, aliaslen;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &classname, &namelen, &alias, &aliaslen) == FAILURE) {
        return;
    }
    hprose_class_manager_register(classname, namelen, alias, aliaslen TSRMLS_CC);
}

ZEND_FUNCTION(get_alias) {
    smart_str alias;
    char *classname;
    int len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname, &len) == FAILURE) {
        RETURN_NULL();
    }
    alias = hprose_class_manager_get_alias(classname, len TSRMLS_CC);
    RETVAL_STRINGL(alias.c, alias.len, 0);
}

ZEND_FUNCTION(get_class) {
    smart_str classname;
    char *alias;
    int len;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &alias, &len) == FAILURE) {
        RETURN_NULL();
    }
    classname = hprose_class_manager_get_class(alias, len TSRMLS_CC);
    RETVAL_STRINGL(classname.c, classname.len, 0);
}

ZEND_BEGIN_ARG_INFO_EX(hprose_class_manager_register_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, classname)
    ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_class_manager_get_alias_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_class_manager_get_class_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_class_manager_methods[] = {
    ZEND_ME_MAPPING(register, register, hprose_class_manager_register_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_ME_MAPPING(getAlias, get_alias, hprose_class_manager_get_alias_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_ME_MAPPING(getClass, get_class, hprose_class_manager_get_class_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    {NULL, NULL, NULL}
};

zend_class_entry *hprose_class_manager_ce = NULL;

HPROSE_STARTUP_FUNCTION(class_manager) {
    ZEND_INIT_MODULE_GLOBALS(hprose_class_manager, hprose_class_manager_ctor, hprose_class_manager_dtor);
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Hprose", "ClassManager", hprose_class_manager_methods);
    hprose_class_manager_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_register_class_alias("HproseClassManager", hprose_class_manager_ce);
    return SUCCESS;
}
