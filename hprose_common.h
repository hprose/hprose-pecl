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
 * LastModified: Jan 5, 2016                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_H
#define HPROSE_H

#include "php.h"
#include "zend_exceptions.h"
#if PHP_API_VERSION >= 20090626
#include "zend_closures.h"
#endif

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
    zend_bool active;
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

/* {{{ refcount macros */
#ifndef Z_ADDREF_P
#define Z_ADDREF_P(pz)                (pz)->refcount++
#define Z_ADDREF_PP(ppz)              Z_ADDREF_P(*(ppz))
#endif

#ifndef Z_DELREF_P
#define Z_DELREF_P(pz)                (pz)->refcount--
#define Z_DELREF_PP(ppz)              Z_DELREF_P(*(ppz))
#endif
/* }}} */

#ifndef array_init_size
#define array_init_size(arg, size) array_init(arg)
#endif

#endif /* PHP_API_VERSION < 20090626 */

/**********************************************************\
| int type definition                                      |
\**********************************************************/
#if defined(_MSC_VER)
#include <float.h>
#include "win32/php_stdint.h"
#ifndef isnan
#define isnan(x) _isnan(x)
#endif
#ifndef isinf
#define isinf(x) !_finite(x)
#endif
#elif defined(__FreeBSD__) && __FreeBSD__ < 5
/* FreeBSD 4 doesn't have stdint.h file */
#include <inttypes.h>
#else
#include <stdint.h>
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

#if PHP_API_VERSION >= 20100412

#if PHP_MAJOR_VERSION < 7
PHP_HPROSE_API HashTable *php_hprose_get_gc(zval *object, zval ***table, int *n TSRMLS_DC);
#else
PHP_HPROSE_API HashTable *php_hprose_get_gc(zval *object, zval **table, int *n);
#endif

#endif

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

#define HPROSE_OBJECT_NEW_EX_BEGIN(type_name)                           \
static zend_object_value php_hprose_##type_name##_new_ex(               \
    zend_class_entry *ce,                                               \
    php_hprose_##type_name **ptr TSRMLS_DC) {                           \
    zend_object_value retval;                                           \
    php_hprose_##type_name *intern;                                     \
    zval *tmp;                                                          \
    intern = emalloc(sizeof(php_hprose_##type_name));                   \
    memset(intern, 0, sizeof(php_hprose_##type_name));                  \
    if (ptr) {                                                          \
        *ptr = intern;                                                  \
    }                                                                   \
    zend_object_std_init(&intern->std, ce TSRMLS_CC);                   \
    zend_hash_copy(                                                     \
        intern->std.properties, &ce->default_properties,                \
        (copy_ctor_func_t)zval_add_ref, (void *)&tmp, sizeof(zval *));  \

#else  /* PHP_API_VERSION < 20100412 */

#define HPROSE_OBJECT_NEW_EX_BEGIN(type_name)               \
static zend_object_value php_hprose_##type_name##_new_ex(   \
    zend_class_entry *ce,                                   \
    php_hprose_##type_name **ptr TSRMLS_DC) {               \
    zend_object_value retval;                               \
    php_hprose_##type_name *intern;                         \
    intern = emalloc(sizeof(php_hprose_##type_name));       \
    memset(intern, 0, sizeof(php_hprose_##type_name));      \
    if (ptr) {                                              \
        *ptr = intern;                                      \
    }                                                       \
    zend_object_std_init(&intern->std, ce TSRMLS_CC);       \
    object_properties_init(&intern->std, ce);               \

#endif /* PHP_API_VERSION < 20100412 */

#define HPROSE_OBJECT_NEW_EX_END(type_name)                                 \
    retval.handle = zend_objects_store_put(                                 \
        intern, (zend_objects_store_dtor_t)zend_objects_destroy_object,     \
        (zend_objects_free_object_storage_t)php_hprose_##type_name##_free,  \
        NULL TSRMLS_CC);                                                    \
    retval.handlers = &hprose_##type_name##_handlers;                       \
    return retval;                                                          \
}                                                                           \

#define HPROSE_OBJECT_NEW(type_name)                            \
static zend_object_value php_hprose_##type_name##_new(          \
    zend_class_entry *ce TSRMLS_DC) {                           \
    return php_hprose_##type_name##_new_ex(ce, NULL TSRMLS_CC); \
}                                                               \

#define HPROSE_OBJECT_CLONE_BEGIN(type_name)                                                                \
static zend_object_value php_hprose_##type_name##_clone(zval *this_ptr TSRMLS_DC) {                         \
    php_hprose_##type_name *new_obj = NULL;                                                                 \
    php_hprose_##type_name *old_obj = HPROSE_GET_OBJECT_P(type_name, this_ptr);                             \
    zend_object_value new_ov = php_hprose_##type_name##_new_ex(old_obj->std.ce, &new_obj TSRMLS_CC);        \
    zend_objects_clone_members(&new_obj->std, new_ov, &old_obj->std, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);   \

#define HPROSE_OBJECT_CLONE_END \
	return new_ov;          \
}                               \

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

#if PHP_API_VERSION < 20100412

#define HPROSE_REGISTER_CLASS_HANDLERS(type_name)                                                           \
    hprose_##type_name##_ce->create_object = php_hprose_##type_name##_new;                                  \
    memcpy(&hprose_##type_name##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));   \

#else /* PHP_API_VERSION < 20100412 */

#define HPROSE_REGISTER_CLASS_HANDLERS(type_name)                                                           \
    hprose_##type_name##_ce->create_object = php_hprose_##type_name##_new;                                  \
    memcpy(&hprose_##type_name##_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));   \
    hprose_##type_name##_handlers.get_gc = php_hprose_get_gc;                                              \

#endif /* PHP_API_VERSION < 20100412 */

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

#define HPROSE_OBJECT_NEW_EX_BEGIN(type_name)                                                    \
static zend_object *php_hprose_##type_name##_new_ex(zend_class_entry *ce, int init_props) {      \
    php_hprose_##type_name *intern;                                                              \
    intern = ecalloc(1, sizeof(php_hprose_##type_name) + zend_object_properties_size(ce));       \
    memset(intern, 0, sizeof(php_hprose_##type_name) + zend_object_properties_size(ce));         \
    zend_object_std_init(&intern->std, ce);                                                      \
    if (init_props) {                                                                            \
        object_properties_init(&intern->std, ce);                                                \
    }                                                                                            \

#define HPROSE_OBJECT_NEW_EX_END(type_name)                 \
    intern->std.handlers = &hprose_##type_name##_handlers;  \
    return &intern->std;                                    \
}                                                           \

#define HPROSE_OBJECT_NEW(type_name)                                        \
static zend_object *php_hprose_##type_name##_new(zend_class_entry *ce) {    \
    return php_hprose_##type_name##_new_ex(ce, 1);                          \
}                                                                           \

#define HPROSE_OBJECT_CLONE_BEGIN(type_name)                                                                                \
static zend_object *php_hprose_##type_name##_clone(zval *this_ptr) {                                                        \
    php_hprose_##type_name *old_obj = HPROSE_GET_OBJECT_P(type_name, this_ptr);                                             \
    php_hprose_##type_name *new_obj = _HPROSE_GET_OBJECT_P(type_name, php_hprose_##type_name##_new_ex(old_obj->std.ce, 0)); \
    zend_objects_clone_members(&new_obj->std, &old_obj->std);                                                               \

#define HPROSE_OBJECT_CLONE_END \
	return &new_obj->std;   \
}                               \

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
    hprose_##type_name##_handlers.get_gc = php_hprose_get_gc;                                               \

#endif /* PHP_MAJOR_VERSION < 7 */

#define HPROSE_OBJECT_SIMPLE_NEW(type_name) \
HPROSE_OBJECT_NEW_EX_BEGIN(type_name)       \
HPROSE_OBJECT_NEW_EX_END(type_name)         \
                                            \
HPROSE_OBJECT_NEW(type_name)                \

#define HPROSE_REGISTER_CLASS_CLONE_HANDLER(type_name)                          \
    hprose_##type_name##_handlers.clone_obj = php_hprose_##type_name##_clone;   \

#define HPROSE_OBJECT_INTERN(type_name) \
    php_hprose_##type_name *intern = HPROSE_GET_OBJECT_P(type_name, getThis())

#define HPROSE_OBJECT(type_name, name) \
    hprose_##type_name *name = HPROSE_GET_OBJECT_P(type_name, getThis())->_this

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
#define zend_hash_str_exists(ht, key, len) zend_hash_exists((ht), (key), (len))
#endif /* PHP_MAJOR_VERSION < 7 */

/**********************************************************\
| array functions compatible PHP 7                         |
\**********************************************************/

#define Z_ARRLEN(v) zend_hash_num_elements(Z_ARRVAL(v))
#define Z_ARRLEN_P(pv) zend_hash_num_elements(Z_ARRVAL_P(pv))
#define Z_ARRLEN_PP(ppv) zend_hash_num_elements(Z_ARRVAL_PP(ppv))

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

static zend_always_inline void hprose_str_replace(char from, char to, char *s, int len, int start) {
    register int i;
    for (i = start; i < len; i++) if (s[i] == from) s[i] = to;
}

#define str_replace(from, to, s, len) hprose_str_replace((from), (to), (s), (len), 0)

#ifndef ZEND_ACC_TRAIT
#define ZEND_ACC_TRAIT 0
#endif

#if PHP_MAJOR_VERSION < 7
static zend_always_inline zend_bool hprose_class_exists(char *classname, size_t len, zend_bool autoload TSRMLS_DC) {
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
}
#else /* PHP_MAJOR_VERSION < 7 */
static zend_always_inline zend_bool _hprose_class_exists(zend_string *class_name, zend_bool autoload TSRMLS_DC) {
    zend_string *lc_name;
    zend_class_entry *ce;

    if (!autoload) {
        if (class_name->val[0] == '\\') {
            /* Ignore leading "\" */
            lc_name = zend_string_alloc(class_name->len - 1, 0);
            zend_str_tolower_copy(lc_name->val, class_name->val + 1, class_name->len - 1);
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
}

#define _class_exists(class_name, autoload) _hprose_class_exists((class_name), (autoload) TSRMLS_CC)

static zend_always_inline zend_bool hprose_class_exists(char *classname, size_t len, zend_bool autoload TSRMLS_DC) {
    zend_string *class_name = zend_string_init(classname, len, 0);
    zend_bool result = _hprose_class_exists(class_name, autoload TSRMLS_CC);
    zend_string_release(class_name);
    return result;
}
#endif /* PHP_MAJOR_VERSION < 7 */

#define class_exists(classname, len, autoload) hprose_class_exists((classname), (len), (autoload) TSRMLS_CC)

static zend_always_inline zend_bool hprose_has_property(zend_class_entry *ce, zval *obj, zval *prop TSRMLS_DC) {
    zend_property_info *property_info;
#if PHP_API_VERSION < 20100412
    char *name = Z_STRVAL_P(prop);
    int name_len = Z_STRLEN_P(prop);
    if (zend_hash_find(&ce->properties_info, name, name_len+1, (void **) &property_info) == SUCCESS) {
        if (property_info->flags & ZEND_ACC_SHADOW) {
            return 0;
        }
        return 1;
    }
    else {
        if (obj && Z_OBJ_HANDLER_P(obj, has_property)) {
            if (Z_OBJ_HANDLER_P(obj, has_property)(obj, prop, 2 TSRMLS_CC)) {
                return 1;
            }
        }
        return 0;
    }
#elif PHP_MAJOR_VERSION < 7
    char *name = Z_STRVAL_P(prop);
    int name_len = Z_STRLEN_P(prop);
    if (zend_hash_find(&ce->properties_info, name, name_len+1, (void **) &property_info) == SUCCESS) {
        if (property_info->flags & ZEND_ACC_SHADOW) {
            return 0;
        }
        return 1;
    }
    else {
        if (obj && Z_OBJ_HANDLER_P(obj, has_property)) {
            if (Z_OBJ_HANDLER_P(obj, has_property)(obj, prop, 2, NULL TSRMLS_CC)) {
                return 1;
            }
        }
        return 0;
    }
#else
    zend_string *name = Z_STR_P(prop);
    if ((property_info = zend_hash_find_ptr(&ce->properties_info, name)) != NULL) {
        if (property_info->flags & ZEND_ACC_SHADOW) {
            return 0;
        }
        return 1;
    }
    else {
        if (Z_TYPE_P(obj) != IS_UNDEF && Z_OBJ_HANDLER_P(obj, has_property)) {
            if (Z_OBJ_HANDLER_P(obj, has_property)(obj, prop, 2, NULL)) {
                return 1;
            }
        }
        return 0;
    }
#endif
}

#define has_property(ce, obj, prop) hprose_has_property((ce), (obj), (prop) TSRMLS_CC)

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
    /* zero length array */
    if (count == 0) return 1;
    if (zend_hash_index_exists(ht, 0)) {
        /* count == 1 and a[0] exists */
        if (count == 1) return 1;
        /* a[0] exists, a[count - 1] exists and the next index is count */
        return zend_hash_index_exists(ht, count - 1) &&
               zend_hash_next_free_element(ht) == count;
    }
    return 0;
}

#define is_string_p(v) ((v) && Z_TYPE_P(v) == IS_STRING)
#define is_array_p(v) ((v) && Z_TYPE_P(v) == IS_ARRAY)
#define is_object_p(v) ((v) && Z_TYPE_P(v) == IS_OBJECT)

#if PHP_API_VERSION < 20090626
#define is_callable_p(v) ((v) && zend_is_callable((v), 0, NULL))
#define is_callable(v) (zend_is_callable(&(v), 0, NULL))
#else
#define is_callable_p(v) ((v) && zend_is_callable((v), IS_CALLABLE_CHECK_SILENT, NULL TSRMLS_CC))
#define is_callable(v) (zend_is_callable(&(v), IS_CALLABLE_CHECK_SILENT, NULL TSRMLS_CC))
#endif

#if PHP_MAJOR_VERSION < 7
#define hprose_zval_new(val)      MAKE_STD_ZVAL(val)
#define hprose_zval_free(val)      zval_ptr_dtor(&(val))
#else
#define hprose_zval_new(val)      val = emalloc(sizeof(zval));
#define hprose_zval_free(val)      zval_ptr_dtor(val); efree(val);
#endif

PHP_HPROSE_API zend_fcall_info_cache __get_fcall_info_cache(zval *obj, char *name, int32_t len TSRMLS_DC);

static zend_always_inline zend_fcall_info_cache _get_fcall_info_cache(zval *callable TSRMLS_DC) {
    switch (Z_TYPE_P(callable)) {
        case IS_ARRAY: {
            zval *obj = php_array_get(callable, 0);
            zval *name = php_array_get(callable, 1);
            convert_to_string(name);
            return __get_fcall_info_cache(obj, Z_STRVAL_P(name), Z_STRLEN_P(name) TSRMLS_CC);
        }
        case IS_STRING: return __get_fcall_info_cache(NULL, Z_STRVAL_P(callable), Z_STRLEN_P(callable) TSRMLS_CC);
        default: return __get_fcall_info_cache(callable, "", 0 TSRMLS_CC);
    }
}

/* name is a symbol */
#define get_fcall_info_cache(obj, name) __get_fcall_info_cache(obj, ZEND_STRL(#name) TSRMLS_CC)

#if PHP_MAJOR_VERSION < 7
static int _zval_array_to_c_array(zval **arg, zval ****params TSRMLS_DC) {
    *(*params)++ = arg;
    return ZEND_HASH_APPLY_KEEP;
}
#endif

PHP_HPROSE_API void __function_invoke_args(zend_fcall_info_cache fcc, zval *obj, zval *return_value, zval *param_array TSRMLS_DC);
PHP_HPROSE_API void __function_invoke(zend_fcall_info_cache fcc, zval *obj, zval *return_value, zend_bool dtor TSRMLS_DC, const char *params_format, ...);
#define function_invoke_no_args(name, return_value) __function_invoke(get_fcall_info_cache(NULL, name), NULL, return_value, 0 TSRMLS_CC, "")
#define function_invoke_no_args_ex(name, return_value, dtor) __function_invoke(get_fcall_info_cache(NULL, name), NULL, return_value, dtor TSRMLS_CC, "")
#define function_invoke(name, return_value, params_format, ...) __function_invoke(get_fcall_info_cache(NULL, name), NULL, return_value, 0 TSRMLS_CC, params_format, __VA_ARGS__)
#define function_invoke_ex(name, return_value, dtor, params_format, ...) __function_invoke(get_fcall_info_cache(NULL, name), NULL, return_value, dtor TSRMLS_CC, params_format, __VA_ARGS__)
#define function_invoke_args(name, return_value, param_array) __function_invoke_args(get_fcall_info_cache(NULL, name), NULL, return_value, param_array TSRMLS_CC)
#define method_invoke_no_args(obj, name, return_value) __function_invoke(get_fcall_info_cache(obj, name), obj, return_value, 0 TSRMLS_CC, "")
#define method_invoke_no_args_ex(obj, name, return_value, dtor) __function_invoke(get_fcall_info_cache(obj, name), obj, return_value, dtor TSRMLS_CC, "")
#define method_invoke(obj, name, return_value, params_format, ...) __function_invoke(get_fcall_info_cache(obj, name), obj, return_value, 0 TSRMLS_CC, params_format, __VA_ARGS__)
#define method_invoke_ex(obj, name, return_value, dtor, params_format, ...) __function_invoke(get_fcall_info_cache(obj, name), obj, return_value, dtor TSRMLS_CC, params_format, __VA_ARGS__)
#define method_invoke_args(obj, name, return_value, param_array) __function_invoke_args(get_fcall_info_cache(obj, name), obj, return_value, param_array TSRMLS_CC)
#define callable_invoke_no_args(callable, return_value) __function_invoke(_get_fcall_info_cache(callable TSRMLS_CC), NULL, return_value, 0 TSRMLS_CC, "")
#define callable_invoke_no_args_ex(callable, return_value, dtor) __function_invoke(_get_fcall_info_cache(callable TSRMLS_CC), NULL, return_value, dtor TSRMLS_CC, "")
#define callable_invoke(callable, return_value, params_format, ...) __function_invoke(_get_fcall_info_cache(callable TSRMLS_CC), NULL, return_value, 0 TSRMLS_CC, params_format, __VA_ARGS__)
#define callable_invoke_ex(callable, return_value, dtor, params_format, ...) __function_invoke(_get_fcall_info_cache(callable TSRMLS_CC), NULL, return_value, dtor TSRMLS_CC, params_format, __VA_ARGS__)
#define callable_invoke_args(callable, return_value, param_array) __function_invoke_args(_get_fcall_info_cache(callable TSRMLS_CC), NULL, return_value, param_array TSRMLS_CC)

PHP_HPROSE_API zend_class_entry *__create_php_object(char *class_name, int32_t len, zval *return_value TSRMLS_DC, const char *params_format, ...);
#define create_php_object_no_args(name, return_value) __create_php_object(ZEND_STRL(#name), return_value TSRMLS_CC, "")
#define create_php_object(name, return_value, params_format, ...) __create_php_object(ZEND_STRL(#name), return_value TSRMLS_CC, params_format, __VA_ARGS__)

/**********************************************************/
END_EXTERN_C()

#endif	/* HPROSE_H */
