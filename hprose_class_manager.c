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
 * LastModified: Apr 20, 2015                             *
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

void _hprose_class_manager_register(char *name, int32_t nlen, char *alias, int32_t alen TSRMLS_DC) {
    hprose_bytes_io *_name = hprose_bytes_io_create(name, nlen);
    hprose_bytes_io *_alias = hprose_bytes_io_create(alias, alen);
    if (!HPROSE_G(cache1)) {
        ALLOC_HASHTABLE(HPROSE_G(cache1));
        zend_hash_init(HPROSE_G(cache1), 64, NULL, hprose_bytes_io_dtor, 0);
    }
    if (!HPROSE_G(cache2)) {
        ALLOC_HASHTABLE(HPROSE_G(cache2));
        zend_hash_init(HPROSE_G(cache2), 64, NULL, hprose_bytes_io_dtor, 0);
    }
    zend_hash_str_update_ptr(HPROSE_G(cache1), name, nlen, _alias);
    zend_hash_str_update_ptr(HPROSE_G(cache2), alias, alen, _name);
}

#if PHP_MAJOR_VERSION < 7
char * _hprose_class_manager_get_alias(char *name, int32_t len, int32_t* len_ptr TSRMLS_DC) {
    char *alias;
    hprose_bytes_io *_alias;
    if (!HPROSE_G(cache1) || (_alias = zend_hash_str_find_ptr(HPROSE_G(cache1), name, len)) == NULL) {
        alias = estrndup(name, len);
        *len_ptr = len;
        str_replace('\\', '_', alias, len);
        if (HPROSE_G(active)) {
            hprose_class_manager_register(name, len, alias, len);
        }
    }
    else {
        alias = estrndup(HB_BUF_P(_alias), HB_LEN_P(_alias));
        *len_ptr = HB_LEN_P(_alias);
    }
    return alias;
}
#else
zend_string *_hprose_class_manager_get_alias(char *name, int32_t len TSRMLS_DC) {
    zend_string *alias;
    hprose_bytes_io *_alias;
    if (!HPROSE_G(cache1) || (_alias = zend_hash_str_find_ptr(HPROSE_G(cache1), name, len)) == NULL) {
        alias = zend_string_init(name, len, 0);
        str_replace('\\', '_', alias->val, len);
        if (HPROSE_G(active)) {
            hprose_class_manager_register(name, len, alias->val, len);
        }
    }
    else {
        alias = zend_string_copy(HB_STR_P(_alias));
    }
    return alias;
}
#endif

#if PHP_MAJOR_VERSION < 7
char * _hprose_class_manager_get_class(char *alias, int32_t len, int32_t* len_ptr TSRMLS_DC) {
    char * name;
    hprose_bytes_io *_name;
    if (!HPROSE_G(cache2) || (_name = zend_hash_str_find_ptr(HPROSE_G(cache2), alias, len)) == NULL) {
        name = estrndup(alias, len);
        *len_ptr = len;
        if (!class_exists(alias, len, 0) && !class_exists(alias, len, 1)) {
            str_replace('_', '\\', name, len);
            if (class_exists(name, len, 0) || class_exists(name, len, 1)) {
                if (HPROSE_G(active)) {
                    hprose_class_manager_register(name, len, alias, len);
                }
            }
            else {
                efree(name);
                name = estrndup("stdClass", sizeof("stdClass") - 1);
                *len_ptr = sizeof("stdClass") - 1;
            }
        }
    }
    else {
        name = estrndup(HB_BUF_P(_name), HB_LEN_P(_name));
        *len_ptr = HB_LEN_P(_name);
    }
    return name;
}
#else
zend_string *_hprose_class_manager_get_class(char *alias, int32_t len TSRMLS_DC) {
    zend_string *name;
    hprose_bytes_io *_name;
    if (!HPROSE_G(cache2) || (_name = zend_hash_str_find_ptr(HPROSE_G(cache2), alias, len)) == NULL) {
        name = zend_string_init(alias, len, 0);
        if (!class_exists(alias, len, 0) && !class_exists(alias, len, 1)) {
            str_replace('_', '\\', name->val, len);
            if (_class_exists(name, 0) || _class_exists(name, 1)) {
                if (HPROSE_G(active)) {
                    hprose_class_manager_register(name->val, len, alias, len);
                }
            }
            else {
                zend_string_release(name);
                name = zend_string_init("stdClass", sizeof("stdClass") - 1, 0);
            }
        }
    }
    else {
        name = zend_string_copy(HB_STR_P(_name));
    }
    return name;
}
#endif

ZEND_METHOD(hprose_class_manager, register) {
    char *name, *alias;
    length_t nlen, alen;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", &name, &nlen, &alias, &alen) == FAILURE) {
        return;
    }
    hprose_class_manager_register(name, nlen, alias, alen);
}

ZEND_METHOD(hprose_class_manager, getAlias) {
    char *name;
    length_t nlen;
#if PHP_MAJOR_VERSION < 7
    char *alias;
    int32_t alen;
#else
    zend_string *alias;
#endif
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &nlen) == FAILURE) {
        RETURN_NULL();
    }
#if PHP_MAJOR_VERSION < 7
    alias = hprose_class_manager_get_alias(name, nlen, &alen);
    RETURN_STRINGL_0(alias, alen);
#else
    alias = hprose_class_manager_get_alias(name, nlen);
    RETURN_STR(alias);
#endif
}

ZEND_METHOD(hprose_class_manager, getClass) {
    char *alias;
    length_t alen;
#if PHP_MAJOR_VERSION < 7
    char *name;
    int32_t nlen;
#else
    zend_string *name;
#endif
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &alias, &alen) == FAILURE) {
        RETURN_NULL();
    }
#if PHP_MAJOR_VERSION < 7
    name = hprose_class_manager_get_class(alias, alen, &nlen);
    RETURN_STRINGL_0(name, nlen);
#else
    name = hprose_class_manager_get_class(alias, alen);
    RETURN_STR(name);
#endif
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
    ZEND_ME(hprose_class_manager, getAlias, hprose_class_manager_get_alias_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_class_manager, getClass, hprose_class_manager_get_class_arginfo, ZEND_ACC_STATIC | ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_CLASS_ENTRY(class_manager)

HPROSE_STARTUP_FUNCTION(class_manager) {
    HPROSE_REGISTER_CLASS("Hprose", "ClassManager", class_manager);
    return SUCCESS;
}

HPROSE_ACTIVATE_FUNCTION(class_manager) {
    HPROSE_G(cache1) = NULL;
    HPROSE_G(cache2) = NULL;
    HPROSE_G(active) = 1;
    return SUCCESS;
}

HPROSE_DEACTIVATE_FUNCTION(class_manager) {
    HPROSE_G(active) = 0;
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
    return SUCCESS;
}