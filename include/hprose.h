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
 * hprose.h                                               *
 *                                                        *
 * hprose for pecl header file.                           *
 *                                                        *
 * LastModified: Mar 10, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_H
#define HPROSE_H

#include "php.h"
#include "hprose_config.h"

BEGIN_EXTERN_C()

#define HPROSE_STARTUP_FUNCTION(module)   	ZEND_MINIT_FUNCTION(hprose_##module)
#define HPROSE_STARTUP(module)	 		  	ZEND_MODULE_STARTUP_N(hprose_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define HPROSE_SHUTDOWN_FUNCTION(module)   	ZEND_MSHUTDOWN_FUNCTION(hprose_##module)
#define HPROSE_SHUTDOWN(module)	 		  	ZEND_MODULE_SHUTDOWN_N(hprose_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)
#define HPROSE_ACTIVATE_FUNCTION(module)   	ZEND_MODULE_ACTIVATE_D(hprose_##module)
#define HPROSE_ACTIVATE(module)	 		  	ZEND_MODULE_ACTIVATE_N(hprose_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define HPROSE_DEACTIVATE_FUNCTION(module)  ZEND_MODULE_DEACTIVATE_D(hprose_##module)
#define HPROSE_DEACTIVATE(module)           ZEND_MODULE_DEACTIVATE_N(hprose_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)

#define HPROSE_CLASS_EX(name, fieldname)    \
typedef struct {                            \
    zend_object object;                     \
    hprose_##name##_t *fieldname;           \
} php_hprose_##name##_t;

#define HPROSE_CLASS(name) HPROSE_CLASS_EX(name, name)

#define HPROSE_OBJECT(name)       \
    php_hprose_##name##_t *name;  \
    name = (php_hprose_##name##_t *)zend_object_store_get_object(getThis() TSRMLS_CC);

#define HPROSE_OBJECT_EX(name, fieldname) \
    hprose_##name##_t *fieldname;  \
    fieldname = ((php_hprose_##name##_t *)zend_object_store_get_object(getThis() TSRMLS_CC))->fieldname;

#define HPROSE_OBJECT_FREE(name)  \
static void php_hprose_##name##_free(php_hprose_##name##_t *name TSRMLS_DC) \
{                                                                           \
    zend_object_std_dtor(&name->object TSRMLS_CC);                          \
    efree(name);                                                            \
}

#define HPROSE_OBJECT_NEW(name)                                                     \
static zend_object_value php_hprose_##name##_new(zend_class_entry *ce TSRMLS_DC) {  \
    zend_object_value retval;                                                       \
    php_hprose_##name##_t *name;                                                    \
    name = emalloc(sizeof(php_hprose_##name##_t));                                  \
    zend_object_std_init(&name->object, ce TSRMLS_CC);                              \
    object_properties_init(&name->object, ce);                                      \
    retval.handle = zend_objects_store_put(                                         \
        name, (zend_objects_store_dtor_t)zend_objects_destroy_object,               \
        (zend_objects_free_object_storage_t)php_hprose_##name##_free,               \
        NULL TSRMLS_CC);                                                            \
    retval.handlers = zend_get_std_object_handlers();                               \
    return retval;                                                                  \
}                                                                                   \

static inline void php_str_replace(char from, char to, char *s, int len) {
    register int i;
    for (i = 0; i < len; i++) if (s[i] == from) s[i] = to;
}


#ifndef ZEND_ACC_TRAIT
#define ZEND_ACC_TRAIT 0
#endif

static inline zend_bool php_class_exists(const char *class_name, size_t len, zend_bool autoload TSRMLS_DC) {
    char *lc_name;
    zend_class_entry **ce = NULL;
    if (!autoload) {
        if (class_name[0] == '\\') {
            /* Ignore leading "\" */
            lc_name = zend_str_tolower_dup(class_name + 1, len - 1);
        }
        else {
            lc_name = zend_str_tolower_dup(class_name, len);
        }
        zend_hash_find(EG(class_table), lc_name, len + 1, (void **)&ce);
        efree(lc_name);
    }
    else {
        zend_lookup_class(class_name, len, &ce TSRMLS_CC);
    }
    if (ce) {
        return (((*ce)->ce_flags & (ZEND_ACC_INTERFACE | ZEND_ACC_TRAIT)) == 0);
    }
    else {
        return 0;
    }
}

ZEND_BEGIN_ARG_INFO_EX(hprose_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

END_EXTERN_C()

#endif	/* HPROSE_H */
