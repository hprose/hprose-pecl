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
 * LastModified: Mar 23, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_H
#define HPROSE_H

#include "php.h"
#include "zend_exceptions.h"

BEGIN_EXTERN_C()

/**********************************************************\
| INIT                                                     |
\**********************************************************/

#ifdef ZTS
#include "TSRM.h"
#endif

#if PHP_MAJOR_VERSION >= 7 && defined(ZTS) && defined(COMPILE_DL_HPROSE)
ZEND_TSRMLS_CACHE_EXTERN();
#endif

#ifndef ZEND_FE_END
#define ZEND_FE_END            { NULL, NULL, NULL }
#endif

#ifdef PHP_WIN32
#define PHP_HPROSE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#define PHP_HPROSE_API __attribute__ ((visibility("default")))
#else
#define PHP_HPROSE_API
#endif

ZEND_BEGIN_MODULE_GLOBALS(hprose)
    HashTable *cache1;
    HashTable *cache2;
ZEND_END_MODULE_GLOBALS(hprose)

#ifdef ZTS
#define HPROSE_G(v) TSRMG(hprose_globals_id, zend_hprose_globals *, v)
#else
#define HPROSE_G(v) (hprose_globals.v)
#endif

ZEND_EXTERN_MODULE_GLOBALS(hprose)

#if PHP_MAJOR_VERSION < 7
typedef int length_t;
#else
typedef size_t length_t;
#endif

#if PHP_API_VERSION < 20090626
#if ZEND_DEBUG
# define zend_always_inline inline
# define zend_never_inline
#else
# if defined(__GNUC__)
#  if __GNUC__ >= 3
#   define zend_always_inline inline __attribute__((always_inline))
#   define zend_never_inline __attribute__((noinline))
#  else
#   define zend_always_inline inline
#   define zend_never_inline
#  endif
# elif defined(_MSC_VER)
#  define zend_always_inline __forceinline
#  define zend_never_inline
# else
#  if __has_attribute(always_inline)
#   define zend_always_inline inline __attribute__((always_inline))
#  else
#   define zend_always_inline inline
#  endif
#  if __has_attribute(noinline)
#   define zend_never_inline __attribute__((noinline))
#  else
#   define zend_never_inline
#  endif
# endif
#endif /* ZEND_DEBUG */
#endif /* PHP_API_VERSION < 20090626 */

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
| START UP Macro definition                                |
\**********************************************************/
#define HPROSE_STARTUP_FUNCTION(module)   	ZEND_MINIT_FUNCTION(hprose_##module)
#define HPROSE_STARTUP(module)	 		ZEND_MODULE_STARTUP_N(hprose_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define HPROSE_SHUTDOWN_FUNCTION(module)   	ZEND_MSHUTDOWN_FUNCTION(hprose_##module)
#define HPROSE_SHUTDOWN(module)	 		ZEND_MODULE_SHUTDOWN_N(hprose_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)
#define HPROSE_ACTIVATE_FUNCTION(module)   	ZEND_MODULE_ACTIVATE_D(hprose_##module)
#define HPROSE_ACTIVATE(module)	 		ZEND_MODULE_ACTIVATE_N(hprose_##module)(INIT_FUNC_ARGS_PASSTHRU)
#define HPROSE_DEACTIVATE_FUNCTION(module)      ZEND_MODULE_DEACTIVATE_D(hprose_##module)
#define HPROSE_DEACTIVATE(module)               ZEND_MODULE_DEACTIVATE_N(hprose_##module)(SHUTDOWN_FUNC_ARGS_PASSTHRU)

/**********************************************************\
| Class & Object Macro definition                          |
\**********************************************************/
#if PHP_MAJOR_VERSION < 7

#define HPROSE_CLASS_BEGIN(type_name)   \
typedef struct {                        \
    zend_object std;                    \
    hprose_##type_name *_this;          \

#define HPROSE_CLASS_END(type_name)     \
} php_hprose_##type_name;               \

#define HPROSE_GET_OBJECT_P(type_name, zv) ((php_hprose_##type_name *)zend_object_store_get_object((zv) TSRMLS_CC))

#define HPROSE_OBJECT_FREE_BEGIN(type_name)                                     \
static void php_hprose_##type_name##_free(void *object TSRMLS_DC) {             \
    php_hprose_##type_name *intern = (php_hprose_##type_name *)object;          \

#define HPROSE_OBJECT_FREE_END                      \
    zend_object_std_dtor(&intern->std TSRMLS_CC);   \
    efree(intern);                                  \
}                                                   \

#if PHP_API_VERSION < 20100412

#define HPROSE_OBJECT_NEW_BEGIN(type_name)                              \
static zend_object_value php_hprose_##type_name##_new(                  \
    zend_class_entry *ce TSRMLS_DC) {                                   \
    zend_object_value retval;                                           \
    php_hprose_##type_name *intern;                                     \
    zval *tmp;                                                          \
    intern = emalloc(sizeof(php_hprose_##type_name));                   \
    memset(intern, 0, sizeof(php_hprose_##type_name));                  \
    zend_object_std_init(&intern->std, ce TSRMLS_CC);                   \
    zend_hash_copy(                                                     \
        intern->std.properties, &ce->default_properties,                \
        (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));  \

#else  /* PHP_API_VERSION < 20100412 */

#define HPROSE_OBJECT_NEW_BEGIN(type_name)                  \
static zend_object_value php_hprose_##type_name##_new(      \
    zend_class_entry *ce TSRMLS_DC) {                       \
    zend_object_value retval;                               \
    php_hprose_##type_name *intern;                         \
    intern = emalloc(sizeof(php_hprose_##type_name));       \
    memset(intern, 0, sizeof(php_hprose_##type_name));      \
    zend_object_std_init(&intern->std, ce TSRMLS_CC);       \
    object_properties_init(&intern->std, ce);               \

#endif /* PHP_API_VERSION < 20100412 */

#define HPROSE_OBJECT_NEW_END(type_name)                                    \
    retval.handle = zend_objects_store_put(                                 \
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,     \
        (zend_objects_free_object_storage_t)php_hprose_##type_name##_free,  \
        NULL TSRMLS_CC);                                                    \
    retval.handlers = &hprose_##type_name##_handlers;                       \
    return retval;                                                          \
}                                                                           \

#if PHP_API_VERSION < 20090626

#define HPROSE_REGISTER_INTERFACE(ns, name, type_name)                          \
    zend_class_entry ce;                                                        \
    INIT_CLASS_ENTRY(ce, ns name, hprose_##type_name##_methods)                 \
    hprose_##type_name##_ce = zend_register_internal_interface(&ce TSRMLS_CC);  \

#define HPROSE_REGISTER_CLASS_EX(ns, name, type_name, parent_ce, parent_name)                               \
    zend_class_entry ce;                                                                                    \
    INIT_CLASS_ENTRY(ce, ns name, hprose_##type_name##_methods)                                             \
    hprose_##type_name##_ce = zend_register_internal_class_ex(&ce, parent_ce, parent_name TSRMLS_CC);       \

#else /* PHP_API_VERSION < 20090626 */

#define HPROSE_REGISTER_INTERFACE(ns, name, type_name)                          \
    zend_class_entry ce;                                                        \
    INIT_CLASS_ENTRY(ce, ns name, hprose_##type_name##_methods)                 \
    hprose_##type_name##_ce = zend_register_internal_interface(&ce TSRMLS_CC);  \
    zend_register_class_alias(ns "\\" name, hprose_##type_name##_ce);           \

#define HPROSE_REGISTER_CLASS_EX(ns, name, type_name, parent_ce, parent_name)                               \
    zend_class_entry ce;                                                                                    \
    INIT_CLASS_ENTRY(ce, ns name, hprose_##type_name##_methods)                                             \
    hprose_##type_name##_ce = zend_register_internal_class_ex(&ce, parent_ce, parent_name TSRMLS_CC);       \
    zend_register_class_alias(ns "\\" name, hprose_##type_name##_ce);                                       \

#endif /* PHP_API_VERSION < 20090626 */

#define HPROSE_REGISTER_CLASS(ns, name, type_name) HPROSE_REGISTER_CLASS_EX(ns, name, type_name, NULL, NULL)

#define HPROSE_REGISTER_CLASS_HANDLERS(type_name)                                                           \
    hprose_##type_name##_ce->create_object = php_hprose_##type_name##_new;                                  \
    memcpy(&hprose_##type_name##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));   \

#else  /* PHP_MAJOR_VERSION < 7 */

#define HPROSE_CLASS_BEGIN(type_name) \
typedef struct {                      \
    hprose_##type_name *_this;        \

#define HPROSE_CLASS_END(type_name)   \
    zend_object std;                  \
} php_hprose_##type_name;             \

#define _HPROSE_GET_OBJECT_P(type_name, obj) ((php_hprose_##type_name *)((char*)(obj) - XtOffsetOf(php_hprose_##type_name, std)))

#define HPROSE_GET_OBJECT_P(type_name, zv) _HPROSE_GET_OBJECT_P(type_name, Z_OBJ_P(zv))

#define HPROSE_OBJECT_FREE_BEGIN(type_name)                                         \
static void php_hprose_##type_name##_free(zend_object *object) {                    \
    php_hprose_##type_name *intern = _HPROSE_GET_OBJECT_P(type_name, object);       \

#define HPROSE_OBJECT_FREE_END          \
    zend_object_std_dtor(&intern->std); \
}                                       \

#define HPROSE_OBJECT_NEW_BEGIN(type_name)                                                       \
static zend_object *php_hprose_##type_name##_new(zend_class_entry *ce) {                         \
    php_hprose_##type_name *intern;                                                              \
    intern = ecalloc(1, sizeof(php_hprose_##type_name) + zend_object_properties_size(ce));       \
    memset(intern, 0, sizeof(php_hprose_##type_name) + zend_object_properties_size(ce));         \
    zend_object_std_init(&intern->std, ce);                                                      \
    object_properties_init(&intern->std, ce);                                                    \

#define HPROSE_OBJECT_NEW_END(type_name)                    \
    intern->std.handlers = &hprose_##type_name##_handlers;  \
    return &intern->std;                                    \
}                                                           \

#define HPROSE_REGISTER_INTERFACE(ns, name, type_name)                  \
    zend_class_entry ce;                                                \
    INIT_NS_CLASS_ENTRY(ce, ns, name, hprose_##type_name##_methods)     \
    hprose_##type_name##_ce = zend_register_internal_interface(&ce);    \
    zend_register_class_alias(ns name, hprose_##type_name##_ce);        \

#define HPROSE_REGISTER_CLASS_EX(ns, name, type_name, parent_ce, parent_name)   \
    zend_class_entry ce;                                                        \
    INIT_NS_CLASS_ENTRY(ce, ns, name, hprose_##type_name##_methods)             \
    hprose_##type_name##_ce = zend_register_internal_class_ex(&ce, parent_ce);  \
    zend_register_class_alias(ns name, hprose_##type_name##_ce);                \

#define HPROSE_REGISTER_CLASS(ns, name, type_name) HPROSE_REGISTER_CLASS_EX(ns, name, type_name, NULL, NULL)

#define HPROSE_REGISTER_CLASS_HANDLERS(type_name)                                                            \
    hprose_##type_name##_ce->create_object = php_hprose_##type_name##_new;                                   \
    memcpy(&hprose_##type_name##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));    \
    hprose_##type_name##_handlers.offset = XtOffsetOf(php_hprose_##type_name, std);                          \
    hprose_##type_name##_handlers.free_obj = php_hprose_##type_name##_free;                                  \


#endif /* PHP_MAJOR_VERSION < 7 */

#define HPROSE_OBJECT_INTERN(type_name) \
    php_hprose_##type_name *intern = HPROSE_GET_OBJECT_P(type_name, getThis());

#define HPROSE_OBJECT(type_name, name) \
    hprose_##type_name *name = HPROSE_GET_OBJECT_P(type_name, getThis())->_this;

#define HPROSE_THIS(type_name) HPROSE_OBJECT(type_name, _this)

#define HPROSE_OBJECT_HANDLERS(type_name)                  \
static zend_object_handlers hprose_##type_name##_handlers; \

#define HPROSE_CLASS_ENTRY(type_name)               \
static zend_class_entry *hprose_##type_name##_ce;   \
zend_class_entry *get_hprose_##type_name##_ce() {   \
    return hprose_##type_name##_ce;                 \
}                                                   \

/**********************************************************\
| String macros compatible PHP 7                           |
\**********************************************************/

#if PHP_MAJOR_VERSION < 7
#define ZVAL_STRINGL_0(v, s, l) ZVAL_STRINGL(v, s, l, 0)
#define ZVAL_STRINGL_1(v, s, l) ZVAL_STRINGL(v, s, l, 1)
#define RETVAL_STRINGL_0(s, l) RETVAL_STRINGL(s, l, 0)
#define RETVAL_STRINGL_1(s, l) RETVAL_STRINGL(s, l, 1)
#define RETURN_STRINGL_0(s, l) RETURN_STRINGL(s, l, 0)
#define RETURN_STRINGL_1(s, l) RETURN_STRINGL(s, l, 1)
#else  /* PHP_MAJOR_VERSION < 7 */
#define ZVAL_STRINGL_0(v, s, l) ZVAL_STRINGL(v, s, l); efree(s);
#define ZVAL_STRINGL_1(v, s, l) ZVAL_STRINGL(v, s, l)
#define RETVAL_STRINGL_0(s, l) RETVAL_STRINGL(s, l); efree(s);
#define RETVAL_STRINGL_1(s, l) RETVAL_STRINGL(s, l)
#define RETURN_STRINGL_0(s, l) RETVAL_STRINGL_0(s, l); return;
#define RETURN_STRINGL_1(s, l) RETURN_STRINGL(s, l)
#endif /* PHP_MAJOR_VERSION < 7 */

/**********************************************************\
| Hashtable functions compatible PHP 7                     |
\**********************************************************/

#if PHP_MAJOR_VERSION < 7
#define zend_hash_str_add_ptr(ht, key, len, pData) zend_hash_add((ht), (key), (len), &(pData), sizeof(pData), NULL)
#define zend_hash_index_add_ptr(ht, h, pData)  zend_hash_index_add((ht), (h), &(pData), sizeof(pData), NULL)
#define zend_hash_str_update_ptr(ht, key, len, pData) zend_hash_update((ht), (key), (len), &(pData), sizeof(pData), NULL)
#define zend_hash_index_update_ptr(ht, h, pData)  zend_hash_index_update((ht), (h), &(pData), sizeof(pData), NULL)
static zend_always_inline void * zend_hash_str_find_ptr(HashTable *ht, char *key, int len) {
    void **ppData;
    return (zend_hash_find(ht, key, len, (void **)&ppData) == FAILURE) ? NULL : *ppData;
}
static zend_always_inline void * zend_hash_index_find_ptr(HashTable *ht, ulong h) {
    void **ppData;
    return (zend_hash_index_find(ht, h, (void **)&ppData) == FAILURE) ? NULL : *ppData;
}
#endif /* PHP_MAJOR_VERSION < 7 */

/**********************************************************\
| array functions compatible PHP 7                         |
\**********************************************************/

#if PHP_MAJOR_VERSION < 7

static zend_always_inline zval *php_array_get(zval *val, ulong h) {
    zval **result;
    if (zend_hash_index_find(Z_ARRVAL_P(val), h, (void **)&result) == FAILURE) return NULL;
    return *result;
}

static zend_always_inline zval *php_assoc_array_get(zval *val, char *key, int len) {
    zval **result;
    if (zend_hash_find(Z_ARRVAL_P(val), key, len, (void **)&result) == FAILURE) return NULL;
    return *result;
}

#else /* PHP_MAJOR_VERSION < 7 */

static zend_always_inline zval *php_array_get(zval *val, ulong h) {
    return zend_hash_index_find(Z_ARRVAL_P(val), h);
}

static zend_always_inline zval *php_assoc_array_get(zval *val, char *key, int len) {
    return zend_hash_str_find(Z_ARRVAL_P(val), key, len);
}

#endif /* PHP_MAJOR_VERSION < 7 */

static zend_always_inline zend_bool php_array_get_long(zval *val, ulong h, long *rval) {
    zval *result = php_array_get(val, h);
    if (result == NULL || Z_TYPE_P(result) != IS_LONG) {
        return 0;
    }
    *rval = Z_LVAL_P(result);
    return 1;
}

static zend_always_inline zend_bool php_assoc_array_get_long(zval *val, char *key, int len, long *rval) {
    zval *result = php_assoc_array_get(val, key, len);
    if (result == NULL || Z_TYPE_P(result) != IS_LONG) {
        return 0;
    }
    *rval = Z_LVAL_P(result);
    return 1;
}

/**********************************************************\
| helper function definition                               |
\**********************************************************/

static inline char *object_hash(zval *val) {
    char *hash;
    spprintf(&hash, 32, "%016lx%016lx", (intptr_t)Z_OBJ_HANDLE_P(val), (intptr_t)Z_OBJ_HT_P(val));
    return hash;
}

static inline void hprose_str_replace(char from, char to, char *s, int len, int start) {
    register int i;
    for (i = start; i < len; i++) if (s[i] == from) s[i] = to;
}

#define str_replace(from, to, s, len) hprose_str_replace((from), (to), (s), (len), 0)

#ifndef ZEND_ACC_TRAIT
#define ZEND_ACC_TRAIT 0
#endif

static inline zend_bool hprose_class_exists(char *classname, size_t len, zend_bool autoload TSRMLS_DC) {
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
#else /* PHP_MAJOR_VERSION < 7 */
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
    zend_string_release(class_name);
    if (ce) {
        return ((ce->ce_flags & (ZEND_ACC_INTERFACE | ZEND_ACC_TRAIT)) == 0);
    }
    else {
        return 0;
    }
#endif /* PHP_MAJOR_VERSION < 7 */
}

#define class_exists(classname, len, autoload) hprose_class_exists((classname), (len), (autoload) TSRMLS_CC)

static zend_always_inline zend_bool is_utf8(char *str, int32_t len) {
    uint8_t * s = (uint8_t *)str;
    int32_t i;
    for (i = 0; i < len; ++i) {
        uint8_t c = s[i];
        switch (c >> 4) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                break;
            case 12:
            case 13:
                if ((s[++i] >> 6) != 0x2) return 0;
                break;
            case 14:
                if ((s[++i] >> 6) != 0x2) return 0;
                if ((s[++i] >> 6) != 0x2) return 0;
                break;
            case 15: {
                uint8_t b = s[++i];
                if ((s[++i] >> 6) != 0x2) return 0;
                if ((s[++i] >> 6) != 0x2) return 0;
                if ((((c & 0xf) << 2) | ((b >> 4) & 0x3)) > 0x10) return 0;
                break;
            }
            default:
                return 0;
        }
    }
    return 1;
}

static zend_always_inline int32_t ustrlen(char *str, int32_t len) {
    uint8_t *s = (uint8_t *)str;
    int32_t l = len, p = 0;
    while (p < len) {
        uint8_t a = s[p];
        if (a < 0x80) {
            ++p;
        }
        else if ((a & 0xE0) == 0xC0) {
            p += 2;
            --l;
        }
        else if ((a & 0xF0) == 0xE0) {
            p += 3;
            l -= 2;
        }
        else if ((a & 0xF8) == 0xF0) {
            p += 4;
            l -= 2;
        }
        else {
            return -1;
        }
    }
    return l;
}

static zend_always_inline zend_bool is_list(zval *val) {
    HashTable *ht = Z_ARRVAL_P(val);
    int32_t count = zend_hash_num_elements(ht);
    // zero length array
    if (count == 0) return 1;
    if (zend_hash_index_exists(ht, 0)) {
        // count == 1 and a[0] exists
        if (count == 1) return 1;
        // a[0] exists, a[count - 1] exists and the next index is count
        return zend_hash_index_exists(ht, count - 1) &&
               zend_hash_next_free_element(ht) == count;
    }
    return 0;
}

static zend_always_inline zend_bool __instanceof(zend_class_entry *ce, char *name, int len TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    zend_class_entry **_ce;
    if (zend_lookup_class(name, len, &_ce TSRMLS_CC) == FAILURE) {
        return 0;
    }
    return instanceof_function(ce, *_ce TSRMLS_CC);
#else /* PHP_MAJOR_VERSION < 7 */
    zend_string *_name = zend_string_init(name, len, 0);
    zend_class_entry *_ce = zend_lookup_class(_name);
    zend_string_release(_name);
    if (_ce == NULL) return 0;
    return instanceof_function(ce, _ce);
#endif /* PHP_MAJOR_VERSION < 7 */
}

// name must be a symbol
#define instanceof(ce, name) __instanceof(ce, ZEND_STRL(#name) TSRMLS_CC)

#if PHP_MAJOR_VERSION < 7
#define hprose_make_zval(val)      MAKE_STD_ZVAL(val)
#define hprose_zval_free(val)      zval_ptr_dtor(&(val))
#else
#define hprose_make_zval(val)      val = emalloc(sizeof(zval));
#define hprose_zval_free(val)      zval_ptr_dtor(val); efree(val);
#endif

static zend_always_inline zend_fcall_info_cache __get_fcall_info_cache(zval *obj, char *name, int32_t len TSRMLS_DC) {
    zend_fcall_info_cache fcc = {0};
    char *fname, *lcname;
    zend_function *fptr;

    if (obj == NULL && (fname = strstr(name, "::")) == NULL) {
        char *nsname;
        lcname = zend_str_tolower_dup(name, len);
        nsname = lcname;
        if (lcname[0] == '\\') {
            nsname = &lcname[1];
            --len;
        }
    #if PHP_MAJOR_VERSION < 7
        if (zend_hash_find(EG(function_table), nsname, len + 1, (void **)&fptr) == FAILURE) {
    #else
        if ((fptr = zend_hash_str_find_ptr(EG(function_table), nsname, len)) == NULL) {
    #endif
            efree(lcname);
            zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                                    "Function %s() does not exist", name);
            return fcc;
        }
        fcc.function_handler = fptr;
        fcc.calling_scope = EG(scope);
#if PHP_MAJOR_VERSION < 7
#if PHP_API_VERSION < 20090626
        fcc.object_pp = NULL;
#else
        fcc.called_scope = NULL;
        fcc.object_ptr = NULL;
#endif
#else
        fcc.called_scope = NULL;
        fcc.object = NULL;
#endif
    }
    else {
        int32_t flen, clen;
        zend_class_entry *ce;
#if PHP_MAJOR_VERSION < 7
        char *cname;
        zend_class_entry **pce;
#else
        zend_string *cname;
#endif
        if (obj == NULL) {
            clen = fname - name;
#if PHP_MAJOR_VERSION < 7
            cname = estrndup(name, clen);
#else
            cname = zend_string_init(name, clen, 0);
#endif
            flen = len - (clen + 2);
            fname += 2;
        }
        else if (Z_TYPE_P(obj) == IS_STRING) {
            clen = Z_STRLEN_P(obj);
#if PHP_MAJOR_VERSION < 7
            cname = estrndup(Z_STRVAL_P(obj), clen);
#else
            cname = zend_string_init(Z_STRVAL_P(obj), clen, 0);
#endif
            flen = len;
            fname = name;
            obj = NULL;
        }
        else if (Z_TYPE_P(obj) != IS_OBJECT) {
            zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                    "The parameter obj is expected to be either a string or an object");
            return fcc;
        }
        if (obj == NULL) {
#if PHP_MAJOR_VERSION < 7
            if (zend_lookup_class(cname, clen, &pce TSRMLS_CC) == FAILURE) {
                zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                                        "Class %s does not exist", cname);
                efree(cname);
                return fcc;
            }
            efree(cname);
            ce = *pce;
#else
            if ((ce = zend_lookup_class(cname)) == NULL) {
                zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                                        "Class %s does not exist", cname->val);
                zend_string_release(cname);
                return fcc;
            }
            zend_string_release(cname);
#endif
        }
        else {
            ce = Z_OBJCE_P(obj);
            fname = name;
            flen = len;
        }
        lcname = zend_str_tolower_dup(fname, flen);
#if PHP_MAJOR_VERSION < 7
        if (zend_hash_find(&ce->function_table, lcname, flen + 1, (void **) &fptr) == FAILURE) {
#else
        if ((fptr = zend_hash_str_find_ptr(&ce->function_table, lcname, flen)) == NULL) {
#endif
            efree(lcname);
            zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                                    "Method %s::%s() does not exist", ce->name, fname);
            return fcc;
        }
        fcc.function_handler = fptr;
        if (fptr->common.fn_flags & ZEND_ACC_STATIC) {
#if PHP_API_VERSION < 20090626
            fcc.calling_scope = NULL;
            fcc.object_pp = NULL;
#else
            fcc.calling_scope = fptr->common.scope;
            fcc.called_scope = ce;
#if PHP_MAJOR_VERSION < 7
            fcc.object_ptr = NULL;
#else
            fcc.object = NULL;
#endif
#endif
	}
        else {
#if PHP_API_VERSION < 20090626
            fcc.calling_scope = Z_OBJCE_P(obj);
            fcc.object_pp = &obj;
#else
            fcc.calling_scope = Z_OBJCE_P(obj);
            fcc.called_scope = ce;
#if PHP_MAJOR_VERSION < 7
            fcc.object_ptr = obj;
#else
            fcc.object = Z_OBJ_P(obj);
#endif
#endif
        }
    }
    efree(lcname);
    fcc.initialized = 1;
    return fcc;
}

// name is a symbol
#define get_fcall_info_cache(obj, name) __get_fcall_info_cache(obj, ZEND_STRL(#name) TSRMLS_CC)

static int _zval_array_to_c_array(zval **arg, zval ****params TSRMLS_DC) {
    *(*params)++ = arg;
    return ZEND_HASH_APPLY_KEEP;
}

static zend_always_inline void __function_invoke_args(zend_fcall_info_cache fcc, zval *obj, zval *return_value, zval *param_array TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    zval *retval_ptr = NULL;
    zval ***params = NULL;
#else
    zval retval;
    zval *params = NULL, *val;
    int i;
#endif
    int result;
    int argc;
    zend_fcall_info fci;

    argc = (param_array) ? zend_hash_num_elements(Z_ARRVAL_P(param_array)) : 0;

    if (argc) {
#if PHP_MAJOR_VERSION < 7
        params = safe_emalloc(sizeof(zval **), argc, 0);
        zend_hash_apply_with_argument(Z_ARRVAL_P(param_array), (apply_func_arg_t)_zval_array_to_c_array, &params TSRMLS_CC);
        params -= argc;
#else
        params = safe_emalloc(sizeof(zval), argc, 0);
        argc = 0;
        ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(param_array), val) {
            ZVAL_COPY(&params[argc], val);
            argc++;
        } ZEND_HASH_FOREACH_END();
#endif
    }

    fci.size = sizeof(fci);
    fci.function_table = NULL;
#if PHP_MAJOR_VERSION < 7
    fci.function_name = NULL;
    fci.retval_ptr_ptr = &retval_ptr;
#else
    ZVAL_UNDEF(&fci.function_name);
    fci.retval = &retval;
#endif
    fci.symbol_table = NULL;
    fci.param_count = argc;
    fci.params = params;
    fci.no_separation = 1;

    if (obj != NULL && Z_TYPE_P(obj) == IS_OBJECT) {
#if PHP_API_VERSION < 20090626
        fci.object_pp = &obj;
        fcc.object_pp = &obj;
#elif PHP_MAJOR_VERSION < 7
        fci.object_ptr = obj;
        fcc.object_ptr = obj;
#else
        fci.object = Z_OBJ_P(obj);
        fcc.object = Z_OBJ_P(obj);
#endif
        fcc.calling_scope = Z_OBJCE_P(obj);
    }
    else {
#if PHP_API_VERSION < 20090626
        fci.object_pp = NULL;
#elif PHP_MAJOR_VERSION < 7
        fci.object_ptr = NULL;
#else
        fci.object = NULL;
#endif
    }

#if PHP_MAJOR_VERSION < 7
    result = zend_call_function(&fci, &fcc TSRMLS_CC);

    if (argc) {
        efree(params);
    }

    if (result == FAILURE) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                                "Invocation of function %s() failed",
                                fcc.function_handler->common.function_name);
        return;
    }

    if (retval_ptr) {
        if (return_value) {
            COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
        }
        else {
            zval_ptr_dtor(&retval_ptr);
        }
    }
#else
    result = zend_call_function(&fci, &fcc);

    for (i = 0; i < argc; i++) {
        zval_ptr_dtor(&params[i]);
    }
    if (argc) {
        efree(params);
    }


    if (result == FAILURE) {
        zend_throw_exception_ex(zend_exception_get_default(), 0,
                                "Invocation of function %s() failed",
                                fcc.function_handler->common.function_name->val);
        return;
    }

    if (Z_TYPE(retval) != IS_UNDEF) {
        if (return_value) {
            ZVAL_COPY_VALUE(return_value, &retval);
        }
        else {
            zval_ptr_dtor(&retval);
        }
    }
#endif
}

static void __function_invoke(zend_fcall_info_cache fcc, zval *obj, zval *return_value TSRMLS_DC, const char *params_format, ...) {
#if PHP_MAJOR_VERSION < 7
    zval *retval_ptr = NULL;
    zval ***params = NULL;
#else
    zval retval;
    zval *params = NULL, *val;
#endif
    int i;
    int result;
    int argc;
    zend_fcall_info fci;

    argc = strlen(params_format);

    if (argc) {
#if PHP_MAJOR_VERSION < 7
        params = safe_emalloc(sizeof(zval **), argc, 0);
        va_list ap;
        va_start(ap, params_format);
        for (i = 0; i < argc; ++i) {
            params[i] = emalloc(sizeof(zval *));
            switch (params_format[i]) {
                case 's': {
                    char *str = va_arg(ap, char *);
                    long len = va_arg(ap, long);
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_STRINGL(*params[i], str, len, 0);
                    break;
                }
                case 'l': {
                    long l = va_arg(ap, long);
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_LONG(*params[i], l);
                    break;
                }
                case 'd': {
                    double d = va_arg(ap, double);
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_DOUBLE(*params[i], d);
                    break;
                }
                case 'n': {
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_NULL(*params[i]);
                    break;
                }
                case 'b': {
                    zend_bool b = va_arg(ap, int);
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_BOOL(*params[i], b);
                    break;
                }
                case 'z': {
                    zval *v = va_arg(ap, zval *);
                    *params[i] = v;
                    break;
                }
                default:
                    zend_throw_exception_ex(
                            zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                            "Unsupported type:%c in function_invoke",
                            params_format[i]);
                    return;
            }
        }
        va_end(ap);
#else
        params = safe_emalloc(sizeof(zval), argc, 0);
        va_list ap;
        va_start(ap, params_format);
        for (i = 0; i < argc; ++i) {
            switch (params_format[i]) {
                case 's': {
                    char *str = va_arg(ap, char *);
                    long len = va_arg(ap, long);
                    ZVAL_STRINGL(&params[i], str, len);
                    break;
                }
                case 'l': {
                    long l = va_arg(ap, long);
                    ZVAL_LONG(&params[i], l);
                    break;
                }
                case 'd': {
                    double d = va_arg(ap, double);
                    ZVAL_DOUBLE(&params[i], d);
                    break;
                }
                case 'n': {
                    ZVAL_NULL(&params[i]);
                    break;
                }
                case 'b': {
                    zend_bool b = va_arg(ap, int);
                    ZVAL_BOOL(&params[i], b);
                    break;
                }
                case 'z': {
                    zval *v = va_arg(ap, zval *);
                    ZVAL_COPY(&params[i], v);
                    break;
                }
                default:
                    zend_throw_exception_ex(
                            zend_exception_get_default(), 0,
                            "Unsupported type:%c in function_invoke",
                            params_format[i]);
                    return;
            }
        }
        va_end(ap);
#endif
    }

    fci.size = sizeof(fci);
    fci.function_table = NULL;
#if PHP_MAJOR_VERSION < 7
    fci.function_name = NULL;
    fci.retval_ptr_ptr = &retval_ptr;
#else
    ZVAL_UNDEF(&fci.function_name);
    fci.retval = &retval;
#endif
    fci.symbol_table = NULL;
    fci.param_count = argc;
    fci.params = params;
    fci.no_separation = 1;

    if (obj != NULL && Z_TYPE_P(obj) == IS_OBJECT) {
#if PHP_API_VERSION < 20090626
        fci.object_pp = &obj;
        fcc.object_pp = &obj;
#elif PHP_MAJOR_VERSION < 7
        fci.object_ptr = obj;
        fcc.object_ptr = obj;
#else
        fci.object = Z_OBJ_P(obj);
        fcc.object = Z_OBJ_P(obj);
#endif
        fcc.calling_scope = Z_OBJCE_P(obj);
    }
    else {
#if PHP_API_VERSION < 20090626
        fci.object_pp = NULL;
#elif PHP_MAJOR_VERSION < 7
        fci.object_ptr = NULL;
#else
        fci.object = NULL;
#endif
    }

#if PHP_MAJOR_VERSION < 7
    result = zend_call_function(&fci, &fcc TSRMLS_CC);

    for (i = 0; i < argc; i++) {
        if (params_format[i] != 'z') {
            if (params_format[i] == 's') {
                ZVAL_EMPTY_STRING(*params[i]);
            }
            zval_ptr_dtor(params[i]);
        }
        efree(params[i]);
    }
    if (argc) {
        efree(params);
    }

    if (result == FAILURE) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                                "Invocation of function %s() failed",
                                fcc.function_handler->common.function_name);
        return;
    }

    if (retval_ptr) {
        if (return_value) {
            COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
        }
        else {
            zval_ptr_dtor(&retval_ptr);
        }
    }
#else
    result = zend_call_function(&fci, &fcc);

    for (i = 0; i < argc; i++) {
        zval_ptr_dtor(&params[i]);
    }

    if (argc) {
        efree(params);
    }

    if (result == FAILURE) {
        zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C), 0,
                                "Invocation of function %s() failed",
                                fcc.function_handler->common.function_name->val);
        return;
    }

    if (Z_TYPE(retval) != IS_UNDEF) {
        if (return_value) {
            ZVAL_COPY_VALUE(return_value, &retval);
        }
        else {
            zval_ptr_dtor(&retval);
        }
    }
#endif
}

#define function_invoke_no_args(name, return_value) __function_invoke(get_fcall_info_cache(NULL, name), NULL, return_value TSRMLS_CC, "")
#define function_invoke(name, return_value, params_format, ...) __function_invoke(get_fcall_info_cache(NULL, name), NULL, return_value TSRMLS_CC, params_format, __VA_ARGS__)
#define function_invoke_args(name, return_value, param_array) __function_invoke_args(get_fcall_info_cache(NULL, name), NULL, return_value, param_array TSRMLS_CC)
#define method_invoke_no_args(obj, name, return_value) __function_invoke(get_fcall_info_cache(obj, name), obj, return_value TSRMLS_CC, "")
#define method_invoke(obj, name, return_value, params_format, ...) __function_invoke(get_fcall_info_cache(obj, name), obj, return_value TSRMLS_CC, params_format, __VA_ARGS__)
#define method_invoke_args(obj, name, return_value, param_array) __function_invoke_args(get_fcall_info_cache(obj, name), obj, return_value, param_array TSRMLS_CC)

static zend_class_entry *__create_php_object(char *class_name, int32_t len, zval *return_value TSRMLS_DC, const char *params_format, ...) {
    zend_function *constructor;
    zend_class_entry *entry;
#if PHP_MAJOR_VERSION < 7
    zval *retval_ptr = NULL;
    zval ***params = NULL;
#else
    zval retval;
    zval *params = NULL, *val;
    zend_string *classname;
#endif
    int i;
    int result;
    int argc;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;

    argc = strlen(params_format);

    if (argc) {
#if PHP_MAJOR_VERSION < 7
        params = safe_emalloc(sizeof(zval **), argc, 0);
        va_list ap;
        va_start(ap, params_format);
        for (i = 0; i < argc; ++i) {
            params[i] = emalloc(sizeof(zval *));
            switch (params_format[i]) {
                case 's': {
                    char *str = va_arg(ap, char *);
                    long len = va_arg(ap, long);
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_STRINGL(*params[i], str, len, 0);
                    break;
                }
                case 'l': {
                    long l = va_arg(ap, long);
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_LONG(*params[i], l);
                    break;
                }
                case 'd': {
                    double d = va_arg(ap, double);
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_DOUBLE(*params[i], d);
                    break;
                }
                case 'n': {
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_NULL(*params[i]);
                    break;
                }
                case 'b': {
                    zend_bool b = va_arg(ap, int);
                    MAKE_STD_ZVAL(*params[i]);
                    ZVAL_BOOL(*params[i], b);
                    break;
                }
                case 'z': {
                    zval *v = va_arg(ap, zval *);
                    *params[i] = v;
                    break;
                }
                default:
                    zend_throw_exception_ex(
                            zend_exception_get_default(TSRMLS_C), 0 TSRMLS_CC,
                            "Unsupported type:%c in create_php_object",
                            params_format[i]);
                    return NULL;
            }
        }
        va_end(ap);
#else
        params = safe_emalloc(sizeof(zval), argc, 0);
        va_list ap;
        va_start(ap, params_format);
        for (i = 0; i < argc; ++i) {
            switch (params_format[i]) {
                case 's': {
                    char *str = va_arg(ap, char *);
                    long len = va_arg(ap, long);
                    ZVAL_STRINGL(&params[i], str, len);
                    break;
                }
                case 'l': {
                    long l = va_arg(ap, long);
                    ZVAL_LONG(&params[i], l);
                    break;
                }
                case 'd': {
                    double d = va_arg(ap, double);
                    ZVAL_DOUBLE(&params[i], d);
                    break;
                }
                case 'n': {
                    ZVAL_NULL(&params[i]);
                    break;
                }
                case 'b': {
                    zend_bool b = va_arg(ap, int);
                    ZVAL_BOOL(&params[i], b);
                    break;
                }
                case 'z': {
                    zval *v = va_arg(ap, zval *);
                    ZVAL_COPY(&params[i], v);
                    break;
                }
                default:
                    zend_throw_exception_ex(
                            zend_exception_get_default(), 0,
                            "Unsupported type:%c in create_php_object",
                            params_format[i]);
                    return NULL;
            }
        }
        va_end(ap);
#endif
    }

#if PHP_MAJOR_VERSION < 7
    entry = zend_fetch_class(class_name, len, ZEND_FETCH_CLASS_DEFAULT TSRMLS_CC);
    object_init_ex(return_value, entry);
    constructor = Z_OBJ_HT_P(return_value)->get_constructor(return_value TSRMLS_CC);
    if (constructor && constructor->common.num_args >= argc &&
        argc >= constructor->common.required_num_args) {

        fci.size = sizeof(fci);
        fci.function_table = EG(function_table);
        fci.function_name = NULL;
        fci.symbol_table = NULL;
#if PHP_API_VERSION < 20090626
        fci.object_pp = &return_value;
#else
        fci.object_ptr = return_value;
#endif
        fci.retval_ptr_ptr = &retval_ptr;
        fci.param_count = argc;
        fci.params = params;
        fci.no_separation = 1;

        fcc.initialized = 1;
        fcc.function_handler = constructor;
        fcc.calling_scope = EG(scope);
#if PHP_API_VERSION < 20090626
        fcc.object_pp = &return_value;
#else
        fcc.called_scope = Z_OBJCE_P(return_value);
        fcc.object_ptr = return_value;
#endif
        zend_call_function(&fci, &fcc TSRMLS_CC);

        for (i = 0; i < argc; i++) {
            if (params_format[i] != 'z') {
                if (params_format[i] == 's') {
                    ZVAL_EMPTY_STRING(*params[i]);
                }
                zval_ptr_dtor(params[i]);
            }
            efree(params[i]);
        }
        if (argc) {
            efree(params);
        }

        if (retval_ptr) {
            zval_ptr_dtor(&retval_ptr);
        }
    }
#else
    classname = zend_string_init(class_name, len, 0);
    entry = zend_lookup_class(classname);
    zend_string_release(classname);
    object_init_ex(return_value, entry);
    constructor = Z_OBJ_HT_P(return_value)->get_constructor(Z_OBJ_P(return_value));
    if (constructor && constructor->common.num_args >= argc &&
        argc >= constructor->common.required_num_args) {

        fci.size = sizeof(fci);
        fci.function_table = EG(function_table);
        ZVAL_UNDEF(&fci.function_name);
        fci.symbol_table = NULL;
        fci.object = Z_OBJ_P(return_value);
        fci.retval = &retval;
        fci.param_count = argc;
        fci.params = params;
        fci.no_separation = 1;

        fcc.initialized = 1;
        fcc.function_handler = constructor;
        fcc.calling_scope = EG(scope);
        fcc.called_scope = Z_OBJCE_P(return_value);
        fcc.object = Z_OBJ_P(return_value);

        zend_call_function(&fci, &fcc);

        for (i = 0; i < argc; i++) {
            zval_ptr_dtor(&params[i]);
        }

        if (argc) {
            efree(params);
        }

        if (Z_TYPE(retval) != IS_UNDEF) {
            zval_ptr_dtor(&retval);
        }
    }
#endif
    return entry;
}

#define create_php_object_no_args(name, return_value) __create_php_object(ZEND_STRL(#name), return_value TSRMLS_CC, "")
#define create_php_object(name, return_value, params_format, ...) __create_php_object(ZEND_STRL(#name), return_value TSRMLS_CC, params_format, __VA_ARGS__)

/**********************************************************/
END_EXTERN_C()

#endif	/* HPROSE_H */
