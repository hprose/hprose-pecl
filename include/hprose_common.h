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
 * hprose_common.h                                        *
 *                                                        *
 * hprose common for pecl header file.                    *
 *                                                        *
 * LastModified: Mar 12, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_COMMON_H
#define HPROSE_COMMON_H

#include "php.h"
#include "zend_exceptions.h"
#if PHP_MAJOR_VERSION < 7
#include "ext/standard/php_smart_str.h"
#else
#include "ext/standard/php_smart_string.h"
#endif

BEGIN_EXTERN_C()

/**********************************************************\
| int type definition                                      |
\**********************************************************/
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef __int8 int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#if defined(__FreeBSD__) && __FreeBSD__ < 5
/* FreeBSD 4 doesn't have stdint.h file */
#include <inttypes.h>
#else
#include <stdint.h>
#endif
#endif

/**********************************************************\
| BYTE_ORDER definition                                    |
\**********************************************************/
#include <sys/types.h> /* This will likely define BYTE_ORDER */

#ifndef BYTE_ORDER
#if (BSD >= 199103)
# include <machine/endian.h>
#else
#if defined(linux) || defined(__linux__)
# include <endian.h>
#else
#define LITTLE_ENDIAN   1234    /* least-significant byte first (vax, pc) */
#define BIG_ENDIAN  4321    /* most-significant byte first (IBM, net) */
#define PDP_ENDIAN  3412    /* LSB first in word, MSW first in long (pdp)*/

#if defined(__i386__) || defined(__x86_64__) || defined(__amd64__) || \
   defined(vax) || defined(ns32000) || defined(sun386) || \
   defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
   defined(__alpha__) || defined(__alpha)
#define BYTE_ORDER    LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined (BIT_ZERO_ON_LEFT) || defined(m68k) || defined(__sparc)
#define BYTE_ORDER  BIG_ENDIAN
#endif
#endif /* linux */
#endif /* BSD */
#endif /* BYTE_ORDER */

#ifndef BYTE_ORDER
#ifdef __BYTE_ORDER
#if defined(__LITTLE_ENDIAN) && defined(__BIG_ENDIAN)
#ifndef LITTLE_ENDIAN
#define LITTLE_ENDIAN __LITTLE_ENDIAN
#endif
#ifndef BIG_ENDIAN
#define BIG_ENDIAN __BIG_ENDIAN
#endif
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#define BYTE_ORDER BIG_ENDIAN
#endif
#endif
#endif
#endif

/**********************************************************\
| void arginfo definition                                  |
\**********************************************************/
ZEND_BEGIN_ARG_INFO_EX(hprose_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

/**********************************************************\
| START UP Macro definition                                |
\**********************************************************/
#define HPROSE_STARTUP_FUNCTION(module)   	ZEND_MINIT_FUNCTION(hprose_##module)
#define HPROSE_STARTUP(module)	 		  	ZEND_MODULE_STARTUP_N(hprose_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define HPROSE_SHUTDOWN_FUNCTION(module)   	ZEND_MSHUTDOWN_FUNCTION(hprose_##module)
#define HPROSE_SHUTDOWN(module)	 		  	ZEND_MODULE_SHUTDOWN_N(hprose_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)
#define HPROSE_ACTIVATE_FUNCTION(module)   	ZEND_MODULE_ACTIVATE_D(hprose_##module)
#define HPROSE_ACTIVATE(module)	 		  	ZEND_MODULE_ACTIVATE_N(hprose_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define HPROSE_DEACTIVATE_FUNCTION(module)  ZEND_MODULE_DEACTIVATE_D(hprose_##module)
#define HPROSE_DEACTIVATE(module)           ZEND_MODULE_DEACTIVATE_N(hprose_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)

/**********************************************************\
| Class & Object Macro definition                          |
\**********************************************************/
#define HPROSE_CLASS_BEGIN_EX(name, fieldname) \
typedef struct {                               \
    zend_object object;                        \
    hprose_##name##_t *fieldname;              \

#define HPROSE_CLASS_BEGIN(name) HPROSE_CLASS_BEGIN_EX(name, name)

#define HPROSE_CLASS_END(name) \
} php_hprose_##name##_t;

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
}

/**********************************************************\
| helper function definition                               |
\**********************************************************/
static inline void hprose_str_replace(char from, char to, char *s, int len) {
    register int i;
    for (i = 0; i < len; i++) if (s[i] == from) s[i] = to;
}

#ifndef ZEND_ACC_TRAIT
#define ZEND_ACC_TRAIT 0
#endif

static inline zend_bool hprose_class_exists(const char *classname, size_t len, zend_bool autoload TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    char *lc_name;
    zend_class_entry **ce = NULL;
    if (!autoload) {
        if (classname[0] == '\\') {
            /* Ignore leading "\" */
            lc_name = zend_str_tolower_dup(classname + 1, len - 1);
        }
        else {
            lc_name = zend_str_tolower_dup(classname, len);
        }
        zend_hash_find(EG(class_table), lc_name, len + 1, (void **)&ce);
        efree(lc_name);
    }
    else {
        zend_lookup_class(classname, len, &ce TSRMLS_CC);
    }
    if (ce) {
        return (((*ce)->ce_flags & (ZEND_ACC_INTERFACE | ZEND_ACC_TRAIT)) == 0);
    }
    else {
        return 0;
    }
#else
    zend_string *class_name = zend_string_init(classname, len, 0);
    zend_string *lc_name;
    zend_class_entry *ce;

    if (!autoload) {
        if (class_name->val[0] == '\\') {
            /* Ignore leading "\" */
            lc_name = zend_string_alloc(len - 1, 0);
            zend_str_tolower_copy(lc_name->val, classname + 1, len - 1);
        }
        else {
            lc_name = zend_string_tolower(class_name);
        }
        ce = zend_hash_find_ptr(EG(class_table), lc_name);
        zend_string_release(lc_name);
    }
    else {
        ce = zend_lookup_class(class_name);
    }

    if (ce) {
        return ((ce->ce_flags & (ZEND_ACC_INTERFACE | ZEND_ACC_TRAIT)) == 0);
    }
    else {
        return 0;
    }
#endif
}

/**********************************************************/
END_EXTERN_C()

#endif	/* HPROSE_COMMON_H */
