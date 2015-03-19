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
 * LastModified: Mar 19, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_class_manager.h"
#include "hprose_bytes_io.h"

#if PHP_MAJOR_VERSION < 7
static void hprose_bytes_io_dtor(void *s) {
    hprose_bytes_io_free(*(hprose_bytes_io **)s);
}
#else
static void hprose_bytes_io_dtor(zval *zv) {
    hprose_bytes_io_free((hprose_bytes_io *)Z_PTR_P(zv));
}
#endif

#ifdef ZTS
#define PERSISTENT_CACHE 0
#else
#define PERSISTENT_CACHE 1
#endif

void _hprose_class_manager_register(char *name, int32_t nlen, char *alias, int32_t alen TSRMLS_DC) {
    hprose_bytes_io *_name = hprose_bytes_io_pcreate(name, nlen, PERSISTENT_CACHE);
    hprose_bytes_io *_alias = hprose_bytes_io_pcreate(alias, alen, PERSISTENT_CACHE);
    if (!HPROSE_G(cache1)) {
        ALLOC_HASHTABLE(HPROSE_G(cache1));
        zend_hash_init(HPROSE_G(cache1), 64, NULL, hprose_bytes_io_dtor, PERSISTENT_CACHE);
    }
    if (!HPROSE_G(cache2)) {
        ALLOC_HASHTABLE(HPROSE_G(cache2));
        zend_hash_init(HPROSE_G(cache2), 64, NULL, hprose_bytes_io_dtor, PERSISTENT_CACHE);
    }
    zend_hash_str_update_ptr(HPROSE_G(cache1), name, nlen, _alias);
    zend_hash_str_update_ptr(HPROSE_G(cache2), alias, alen, _name);
}

char * _hprose_class_manager_get_alias(char *name, int32_t len, int32_t* len_ptr TSRMLS_DC) {
    char *alias;
    hprose_bytes_io *_alias;
    if (!HPROSE_G(cache1) || (_alias = zend_hash_str_find_ptr(HPROSE_G(cache1), name, len)) == NULL) {
        alias = estrndup(name, len);
        *len_ptr = len;
        str_replace('\\', '_', alias, len);
        hprose_class_manager_register(name, len, alias, len);
    }
    else {
        alias = hprose_bytes_io_to_string(_alias);
        *len_ptr = _alias->len;
    }
    return alias;
}

char * _hprose_class_manager_get_class(char *alias, int32_t len, int32_t* len_ptr TSRMLS_DC) {
    char * name;
    hprose_bytes_io *_name;
    if (!HPROSE_G(cache2) || (_name = zend_hash_str_find_ptr(HPROSE_G(cache2), alias, len)) == NULL) {
        name = estrndup(alias, len);
        *len_ptr = len;
        if (!class_exists(alias, len, 0) && !class_exists(alias, len, 1)) {
            str_replace('_', '\\', name, len);
            if (class_exists(name, len, 0) || class_exists(name, len, 1)) {
                hprose_class_manager_register(name, len, alias, len);
            }
            else {
                efree(name);
                name = estrndup("stdClass", sizeof("stdClass") - 1);
                *len_ptr = sizeof("stdClass") - 1;
            }
        }
    }
    else {
        name = hprose_bytes_io_to_string(_name);
        *len_ptr = _name->len;
    }
    return name;
}

ZEND_METHOD(hprose_class_manager, register) {
    char *name, *alias;
    length_t nlen, alen;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &nlen, &alias, &alen) == FAILURE) {
        return;
    }
    hprose_class_manager_register(name, nlen, alias, alen);
}

ZEND_METHOD(hprose_class_manager, get_alias) {
    char *name, *alias;
    length_t nlen;
    int32_t alen;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &nlen) == FAILURE) {
        RETURN_NULL();
    }
    alias = hprose_class_manager_get_alias(name, nlen, &alen);
    RETURN_STRINGL_0(alias, alen);
}

ZEND_METHOD(hprose_class_manager, get_class) {
    char *name, *alias;
    int32_t nlen;
    length_t alen;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &alias, &alen) == FAILURE) {
        RETURN_NULL();
    }
    name = hprose_class_manager_get_class(alias, alen, &nlen);
    RETURN_STRINGL_0(name, nlen);
}

ZEND_BEGIN_ARG_INFO_EX(hprose_class_manager_register_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_class_manager_get_alias_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_class_manager_get_class_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, alias)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_class_manager_methods[] = {
    ZEND_ME(hprose_class_manager, register, hprose_class_manager_register_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_MALIAS(hprose_class_manager, getAlias, get_alias, hprose_class_manager_get_alias_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_MALIAS(hprose_class_manager, getClass, get_class, hprose_class_manager_get_class_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_CLASS_ENTRY(class_manager)

HPROSE_STARTUP_FUNCTION(class_manager) {
    HPROSE_REGISTER_CLASS("Hprose", "ClassManager", class_manager);
    return SUCCESS;
}

HPROSE_ACTIVATE_FUNCTION(class_manager) {
#ifdef ZTS
    HPROSE_G(cache1) = NULL;
    HPROSE_G(cache2) = NULL;
#endif
    return SUCCESS;
}

HPROSE_DEACTIVATE_FUNCTION(class_manager) {
#ifdef ZTS
    if (HPROSE_G(cache1)) {
        zend_hash_destroy(HPROSE_G(cache1));
        FREE_HASHTABLE(HPROSE_G(cache1));
        HPROSE_G(cache1) = NULL;
    }
    if (HPROSE_G(cache2)) {
        zend_hash_destroy(HPROSE_G(cache2));
        FREE_HASHTABLE(HPROSE_G(cache2));
        HPROSE_G(cache2) = NULL;
    }
#endif
    return SUCCESS;
}