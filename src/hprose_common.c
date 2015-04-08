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
 * hprose_common.c                                        *
 *                                                        *
 * hprose common for pecl source file.                    *
 *                                                        *
 * LastModified: Apr 8, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose.h"

#if PHP_API_VERSION >= 20100412

#if PHP_MAJOR_VERSION < 7
HashTable *php_hprose_get_gc(zval *object, zval ***table, int *n TSRMLS_DC) {
    *table = NULL;
    *n = 0;
    return zend_std_get_properties(object TSRMLS_CC);
}
#else
HashTable *php_hprose_get_gc(zval *object, zval **table, int *n) {
    *table = NULL;
    *n = 0;
    return zend_std_get_properties(object);
}
#endif

#endif

void __function_invoke(zend_fcall_info_cache fcc, zval *obj, zval *return_value, zend_bool dtor TSRMLS_DC, const char *params_format, ...) {
#if PHP_MAJOR_VERSION < 7
    zval *retval_ptr = NULL;
    zval ***params = NULL;
#else
    zval retval;
    zval *params = NULL;
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
                    if (v) {
                        Z_ADDREF_P(v);
                        *params[i] = v;
                    }
                    else {
                        MAKE_STD_ZVAL(*params[i]);
                        ZVAL_NULL(*params[i]);
                    }
                    break;
                }
                default:
                    zend_throw_exception_ex(
                            NULL, 0 TSRMLS_CC,
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
                    if (v) {
                        ZVAL_COPY(&params[i], v);
                    }
                    else {
                        ZVAL_NULL(&params[i]);
                    }
                    break;
                }
                default:
                    zend_throw_exception_ex(
                            NULL, 0,
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
        fci.object_pp = fcc.object_pp;
#elif PHP_MAJOR_VERSION < 7
        fci.object_ptr = fcc.object_ptr;
#else
        fci.object = fcc.object;
#endif
    }

#if PHP_MAJOR_VERSION < 7
    result = zend_call_function(&fci, &fcc TSRMLS_CC);

    for (i = 0; i < argc; i++) {
        if (params_format[i] == 's') {
            ZVAL_EMPTY_STRING(*params[i]);
        }
        zval_ptr_dtor(params[i]);
        efree(params[i]);
    }
    if (argc) {
        efree(params);
    }

    if (result == FAILURE) {
        zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                                "Invocation of function %s() failed",
                                fcc.function_handler->common.function_name);
        return;
    }

    if (retval_ptr) {
        if (return_value) {
            if (return_value != retval_ptr) {
                if (dtor) {
                    zval_dtor(return_value);
                }
                COPY_PZVAL_TO_ZVAL(*return_value, retval_ptr);
            }
            else {
                if (dtor) {
                    zval_ptr_dtor(&retval_ptr);
                }
            }
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
        zend_throw_exception_ex(NULL, 0,
                                "Invocation of function %s() failed",
                                fcc.function_handler->common.function_name->val);
        return;
    }

    if (Z_TYPE(retval) != IS_UNDEF) {
        if (return_value) {
            if (dtor) {
                zval_ptr_dtor(return_value);
            }
            ZVAL_COPY_VALUE(return_value, &retval);
        }
        else {
            zval_ptr_dtor(&retval);
        }
    }
#endif
}

zend_class_entry *__create_php_object(char *class_name, int32_t len, zval *return_value TSRMLS_DC, const char *params_format, ...) {
    zend_function *constructor;
    zend_class_entry *entry;
#if PHP_MAJOR_VERSION < 7
    zval *retval_ptr = NULL;
    zval ***params = NULL;
#else
    zval retval;
    zval *params = NULL;
    zend_string *classname;
#endif
    int i;
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
                    if (v) {
                        Z_ADDREF_P(v);
                        *params[i] = v;
                    }
                    else {
                        MAKE_STD_ZVAL(*params[i]);
                        ZVAL_NULL(*params[i]);
                    }
                    break;
                }
                default:
                    zend_throw_exception_ex(
                            NULL, 0 TSRMLS_CC,
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
                    if (v) {
                        ZVAL_COPY(&params[i], v);
                    }
                    else {
                        ZVAL_NULL(&params[i]);
                    }
                    break;
                }
                default:
                    zend_throw_exception_ex(
                            NULL, 0,
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
            if (params_format[i] == 's') {
                ZVAL_EMPTY_STRING(*params[i]);
            }
            zval_ptr_dtor(params[i]);
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
