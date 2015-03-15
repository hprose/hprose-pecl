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
 * LastModified: Mar 14, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_COMMON_H
#define HPROSE_COMMON_H

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

#define RETURN_STRINGL_0(s, l) RETURN_STRINGL(s, l, 0)
#define RETURN_STRINGL_1(s, l) RETURN_STRINGL(s, l, 1)

#define HPROSE_CLASS_BEGIN_EX(type_name, field_name) \
typedef struct {                                     \
    zend_object std;                                 \
    hprose_##type_name *field_name;                  \

#define HPROSE_CLASS_END(type_name)                  \
} php_hprose_##type_name;                            \

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

#define HPROSE_REGISTER_CLASS(ns, name, type_name)                                                          \
    zend_class_entry ce;                                                                                    \
    INIT_NS_CLASS_ENTRY(ce, ns, name, hprose_##type_name##_methods)                                         \
    hprose_##type_name##_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);                   \
    zend_register_class_alias(ns name, hprose_##type_name##_ce);                                            \

#define HPROSE_REGISTER_CLASS_HANDLERS(type_name)                                                           \
    hprose_##type_name##_ce->create_object = php_hprose_##type_name##_new;                                                        \
    memcpy(&hprose_##type_name##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));   \

#else  /* PHP_MAJOR_VERSION < 7 */

#define RETURN_STRINGL_0(s, l) RETURN_STRINGL(s, l)
#define RETURN_STRINGL_1(s, l) RETURN_PSTRINGL(s, l)

#define HPROSE_CLASS_BEGIN_EX(type_name, fieldname) \
typedef struct {                                    \
    hprose_##type_name *fieldname;                  \

#define HPROSE_CLASS_END(type_name)                 \
    zend_object std;                                \
} php_hprose_##type_name;                           \

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

#define HPROSE_REGISTER_CLASS(ns, name, type_name)                                                           \
    zend_class_entry ce;                                                                                     \
    INIT_NS_CLASS_ENTRY(ce, ns, name, hprose_##type_name##_methods)                                          \
    hprose_##type_name##_ce = zend_register_internal_class_ex(&ce, NULL);                                    \
    zend_register_class_alias(ns name, hprose_##type_name##_ce);                                             \

#define HPROSE_REGISTER_CLASS_HANDLERS(type_name)                                                            \
    hprose_##type_name##_ce->create_object = php_hprose_##type_name##_new;                                                         \
    memcpy(&hprose_##type_name##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));    \
    hprose_##type_name##_handlers.offset = XtOffsetOf(php_hprose_##type_name, std);                          \
    hprose_##type_name##_handlers.free_obj = php_hprose_##type_name##_free;                                  \


#endif /* PHP_MAJOR_VERSION < 7 */

#define HPROSE_CLASS_BEGIN(type_name) HPROSE_CLASS_BEGIN_EX(type_name, type_name)

#define HPROSE_OBJECT_INTERN(type_name) \
    php_hprose_##type_name *intern = HPROSE_GET_OBJECT_P(type_name, getThis());

#define HPROSE_OBJECT(type_name, name) \
    hprose_##type_name *name = HPROSE_GET_OBJECT_P(type_name, getThis())->name;

#define HPROSE_OBJECT_HANDLERS(type_name)                  \
static zend_object_handlers hprose_##type_name##_handlers; \

#define HPROSE_CLASS_ENTRY(type_name)               \
zend_class_entry *hprose_##type_name##_ce;          \
zend_class_entry *get_hprose_##type_name##_ce() {   \
    return hprose_##type_name##_ce;                 \
}                                                   \

/**********************************************************\
| Hashtable functions compatible PHP 7                     |
\**********************************************************/

#if PHP_MAJOR_VERSION < 7
#define zend_hash_str_add_ptr(ht, key, len, pData) zend_hash_add((ht), (key), (len), &(pData), sizeof(pData), NULL)
#define zend_hash_index_add_ptr(ht, h, pData)  zend_hash_index_add((ht), (h), &(pData), sizeof(pData), NULL)
#define zend_hash_str_update_ptr(ht, key, len, pData) zend_hash_update((ht), (key), (len), &(pData), sizeof(pData), NULL)
#define zend_hash_index_update_ptr(ht, h, pData)  zend_hash_index_update((ht), (h), &(pData), sizeof(pData), NULL)
static zend_always_inline void * zend_hash_str_find_ptr(HashTable *ht, const char *key, int len) {
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

static zend_always_inline zval *php_assoc_array_get(zval *val, const char *key, int len) {
    zval **result;
    if (zend_hash_find(Z_ARRVAL_P(val), key, len, (void **)&result) == FAILURE) return NULL;
    return *result;
}

#else /* PHP_MAJOR_VERSION < 7 */

static zend_always_inline zval *php_array_get(zval *val, ulong h) {
    return zend_hash_index_find(Z_ARRVAL_P(val), h);
}

static zend_always_inline zval *php_assoc_array_get(zval *val, const char *key, int len) {
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

static zend_always_inline zend_bool php_assoc_array_get_long(zval *val, const char *key, int len, long *rval) {
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

#define class_exists(classname, len, autoload) hprose_class_exists((classname), (len), (autoload) TSRMLS_CC)

static zend_always_inline zend_bool is_utf8(const char *str, size_t len) {
    const uint8_t * s = (const uint8_t *)str;
    size_t i;
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

static zend_always_inline size_t ustrlen(const char *str, size_t len) {
    const uint8_t *s = (const uint8_t *)str;
    size_t l = len, p = 0;
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
        else if ((a * 0xF8) == 0xF0) {
            p += 4;
            l -= 2;
        }
    }
    return l;
}

static zend_always_inline zend_bool is_list(zval *val) {
    HashTable *ht = Z_ARRVAL_P(val);
    ulong count = zend_hash_num_elements(ht);
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

/**********************************************************/
END_EXTERN_C()

#endif	/* HPROSE_COMMON_H */
