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
#if PHP_MAJOR_VERSION < 7

#define RETURN_STRINGL_0(s, l) RETURN_STRINGL(s, l, 0)
#define RETURN_STRINGL_1(s, l) RETURN_STRINGL(s, l, 1)

#define HPROSE_CLASS_BEGIN_EX(type_name, field_name) \
typedef struct {                                     \
    zend_object std;                                 \
    hprose_##type_name##_t *field_name;              \

#define HPROSE_CLASS_END(type_name) \
} php_hprose_##type_name##_t;

#define HPROSE_GET_OBJECT_P(type_name, zv) ((php_hprose_##type_name##_t *)zend_object_store_get_object((zv) TSRMLS_CC))

#else
#define RETURN_STRINGL_0(s, l) RETURN_STRINGL(s, l)
#define RETURN_STRINGL_1(s, l) RETURN_STRINGL(s, l)

#define HPROSE_CLASS_BEGIN_EX(type_name, fieldname) \
typedef struct {                                    \
    hprose_##type_name##_t *fieldname;              \

#define HPROSE_CLASS_END(type_name)                 \
    zend_object std;                                \
} php_hprose_##type_name##_t;

#define HPROSE_GET_OBJECT_P(type_name, zv) ((php_hprose_##type_name##_t *)((char*)(Z_OBJ_P(zv)) - XtOffsetOf(php_hprose_##type_name##_t, std)))
#endif

#define HPROSE_CLASS_BEGIN(type_name) HPROSE_CLASS_BEGIN_EX(type_name, type_name)

#define HPROSE_OBJECT_INTERN(type_name) \
    php_hprose_##type_name##_t *intern = HPROSE_GET_OBJECT_P(type_name, getThis());

#define HPROSE_OBJECT(type_name, name) \
    hprose_##type_name##_t *name = HPROSE_GET_OBJECT_P(type_name, getThis())->name;


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
