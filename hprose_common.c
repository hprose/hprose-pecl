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
 * LastModified: Jun 10, 2016                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_common.h"

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

zend_fcall_info_cache __get_fcall_info_cache(zval *obj, char *name, int32_t len TSRMLS_DC) {
    zend_fcall_info_cache fcc = {0};
    char *fname, *lcname = NULL;
    zend_function *fptr;
#if PHP_VERSION_ID < 70100
    zend_class_entry *scope = EG(scope);
#else
    zend_class_entry *scope = EG(fake_scope) ? EG(fake_scope) : zend_get_executed_scope();
#endif

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
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                                    "Function %s() does not exist", name);
            return fcc;
        }
        fcc.function_handler = fptr;
        fcc.calling_scope = scope;
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
#if PHP_API_VERSION >= 20090626
    else if (obj && Z_TYPE_P(obj) == IS_OBJECT &&
             instanceof_function(Z_OBJCE_P(obj), zend_ce_closure TSRMLS_CC) &&
             (fptr = (zend_function*)zend_get_closure_method_def(obj TSRMLS_CC)) != NULL) {
        fcc.function_handler = fptr;
        fcc.calling_scope = scope;
#if PHP_MAJOR_VERSION < 7
        fcc.called_scope = NULL;
        fcc.object_ptr = NULL;
#else
        fcc.called_scope = NULL;
        fcc.object = NULL;
#endif
    }
#endif
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
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                    "The parameter obj is expected to be either a string or an object");
            return fcc;
        }
        if (obj == NULL) {
#if PHP_MAJOR_VERSION < 7
            if (zend_lookup_class(cname, clen, &pce TSRMLS_CC) == FAILURE) {
                zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                                        "Class %s does not exist", cname);
                efree(cname);
                return fcc;
            }
            efree(cname);
            ce = *pce;
#else
            if ((ce = zend_lookup_class(cname)) == NULL) {
                zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
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
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                                    "Method %s::%s() does not exist", ce->name, fname);
            return fcc;
        }
        fcc.function_handler = fptr;
        if ((fptr->common.fn_flags & ZEND_ACC_STATIC) || obj == NULL) {
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
    if (lcname) {
        efree(lcname);
    }
    fcc.initialized = 1;
    return fcc;
}

void __function_invoke_args(zend_fcall_info_cache fcc, zval *obj, zval *return_value, zval *param_array TSRMLS_DC) {
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

    argc = (param_array) ? Z_ARRLEN_P(param_array) : 0;

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
#if PHP_VERSION_ID < 70100
    fci.function_table = NULL;
    fci.symbol_table = NULL;
#endif
#if PHP_MAJOR_VERSION < 7
    fci.function_name = NULL;
    fci.retval_ptr_ptr = &retval_ptr;
#else
    ZVAL_UNDEF(&fci.function_name);
    fci.retval = &retval;
#endif
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
        zend_throw_exception_ex(NULL, 0,
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

void __function_invoke(zend_fcall_info_cache fcc, zval *obj, zval *return_value, zend_bool dtor TSRMLS_DC, const char *params_format, ...) {
#if PHP_MAJOR_VERSION < 7
    zval *retval_ptr = NULL;
    zval ***params = NULL;
#else
    zval retval;
    zval *params = NULL;
#endif
    uint32_t i, argc;
    int result;
    zend_fcall_info fci;

    argc = strlen(params_format);

    if (argc) {
#if PHP_MAJOR_VERSION < 7
        va_list ap;
        va_start(ap, params_format);
        params = safe_emalloc(sizeof(zval **), argc, 0);
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
        va_list ap;
        va_start(ap, params_format);
        params = safe_emalloc(sizeof(zval), argc, 0);
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
#if PHP_VERSION_ID < 70100
    fci.function_table = NULL;
    fci.symbol_table = NULL;
#endif
#if PHP_MAJOR_VERSION < 7
    fci.function_name = NULL;
    fci.retval_ptr_ptr = &retval_ptr;
#else
    ZVAL_UNDEF(&fci.function_name);
    fci.retval = &retval;
#endif
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
    uint32_t i, argc;
    zend_fcall_info fci;
    zend_fcall_info_cache fcc;
#if PHP_VERSION_ID < 70100
    zend_class_entry *scope = EG(scope);
#else
    zend_class_entry *scope = EG(fake_scope) ? EG(fake_scope) : zend_get_executed_scope();
#endif

    argc = strlen(params_format);

    if (argc) {
#if PHP_MAJOR_VERSION < 7
        va_list ap;
        va_start(ap, params_format);
        params = safe_emalloc(sizeof(zval **), argc, 0);
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
        va_list ap;
        va_start(ap, params_format);
        params = safe_emalloc(sizeof(zval), argc, 0);
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
        ZVAL_UNDEF(&fci.function_name);
#if PHP_VERSION_ID < 70100
        fci.function_table = EG(function_table);
        fci.symbol_table = NULL;
#endif
        fci.object = Z_OBJ_P(return_value);
        fci.retval = &retval;
        fci.param_count = argc;
        fci.params = params;
        fci.no_separation = 1;

        fcc.initialized = 1;
        fcc.function_handler = constructor;
        fcc.calling_scope = scope;
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
