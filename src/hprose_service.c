
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
 * hprose_service.c                                       *
 *                                                        *
 * hprose service for pecl source file.                   *
 *                                                        *
 * LastModified: Apr 1, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"
#include "hprose_result_mode.h"
#include "hprose_service.h"

static zend_always_inline void hprose_service_get_declared_only_methods(zval *class, zval *return_value TSRMLS_DC) {
    zval *parent_class, *all_methods;
    int32_t i;
    hprose_zval_new(parent_class);
    hprose_zval_new(all_methods);
    function_invoke(get_parent_class, parent_class, "z", class);
    function_invoke(get_class_methods, all_methods, "z", class);
    if (Z_TYPE_P(parent_class) == IS_STRING) {
        zval *inherit_methods;
        hprose_zval_new(inherit_methods);
        function_invoke(get_class_methods, inherit_methods, "z", parent_class);
        function_invoke_ex(array_diff, all_methods, 1, "zz", all_methods, inherit_methods);
        hprose_zval_free(inherit_methods);
    }
    hprose_zval_free(parent_class);
    array_init(return_value);
    i = Z_ARRLEN_P(all_methods);
    if (i > 0) {
        HashTable *ht = Z_ARRVAL_P(all_methods);
        zend_hash_internal_pointer_reset(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **_method, *method;
            zend_hash_get_current_data(ht, (void **)&_method);
            method = *_method;
#else
            zval *method = zend_hash_get_current_data(ht);
#endif
            if (!is_magic_method(Z_STRVAL_P(method), Z_STRLEN_P(method))) {
#if PHP_MAJOR_VERSION < 7
                Z_ADDREF_P(method);
#else
                Z_TRY_ADDREF_P(method);
#endif
                add_next_index_zval(return_value, method);
            }
            zend_hash_move_forward(ht);
        }
    }
    hprose_zval_free(all_methods);
}

static zend_always_inline void hprose_service_add_function(hprose_service *_this, zval *func, zval *alias, uint8_t mode, zval *simple TSRMLS_DC) {
    zend_fcall_info_cache fcc = _get_fcall_info_cache(func TSRMLS_CC);
    char *name;
    int32_t len, i, n;
    hprose_remote_call *call;
    zend_bool _simple = 2;
    if (EG(exception)) {
        return;
    }
    if (simple) {
        convert_to_boolean(simple);
#if PHP_MAJOR_VERSION < 7
        _simple = Z_BVAL_P(simple);
#else
        _simple = (Z_TYPE_P(simple) != IS_FALSE);
#endif
    }
    if (alias) {
        convert_to_string(alias);
    }
    if (alias == NULL || Z_STRLEN_P(alias) == 0) {
        switch (Z_TYPE_P(func)) {
            case IS_STRING: alias = func; break;
            case IS_ARRAY: alias = php_array_get(func, 1); break;
            default: zend_throw_exception(NULL, "Argument func is not callable.", 0 TSRMLS_CC); return;
        }
    }
    len = Z_STRLEN_P(alias);
    name = zend_str_tolower_dup(Z_STRVAL_P(alias), len);
    if (!zend_hash_str_exists(_this->calls, name, len)) {
#if PHP_MAJOR_VERSION < 7
        Z_ADDREF_P(alias);
#else
        Z_TRY_ADDREF_P(alias);
#endif
        add_next_index_zval(_this->names, alias);
    }
    call = emalloc(sizeof(hprose_remote_call));
    call->fcc = fcc;
    call->mode = mode;
    call->simple = _simple;
    call->byref = 0;
    n = call->fcc.function_handler->common.num_args;
    for (i = 0; i < n; ++i) {
        if (fcc.function_handler->common.arg_info[i].pass_by_reference) {
            call->byref = 1;
            break;
        }
    }
    zend_hash_str_update_ptr(_this->calls, name, len, call);
    efree(name);
}

static zend_always_inline void hprose_service_add_functions(hprose_service *_this, zval *funcs, zval *aliases, uint8_t mode, zval *simple TSRMLS_DC) {
    int32_t i, count;
    HashTable *ht = Z_ARRVAL_P(funcs);
    zend_hash_internal_pointer_reset(ht);
    if (aliases) {
        count = Z_ARRLEN_P(aliases);
        if (count > 0) {
            if (count == Z_ARRLEN_P(funcs)) {
                for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
                    zval **func, *alias;
                    zend_hash_get_current_data(ht, (void **)&func);
                    alias = php_array_get(aliases, i);
                    hprose_service_add_function(_this, *func, alias, mode, simple TSRMLS_CC);
#else
                    zval *func = zend_hash_get_current_data(ht);
                    zval *alias = php_array_get(aliases, i);
                    hprose_service_add_function(_this, func, alias, mode, simple TSRMLS_CC);
#endif
                    if (EG(exception)) return;
                    zend_hash_move_forward(ht);
                }
            }
            else {
                zend_throw_exception(NULL, "The count of functions is not matched with aliases", 0 TSRMLS_CC);
            }
            return;
        }
    }
    count = Z_ARRLEN_P(funcs);
    for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
        zval **func;
        zend_hash_get_current_data(ht, (void **)&func);
        hprose_service_add_function(_this, *func, NULL, mode, simple TSRMLS_CC);
#else
        zval *func = zend_hash_get_current_data(ht);
        hprose_service_add_function(_this, func, NULL, mode, simple TSRMLS_CC);
#endif
        if (EG(exception)) return;
        zend_hash_move_forward(ht);
    }
}

static zend_always_inline void hprose_service_add_method(hprose_service *_this, zval *methodname, zval *belongto, zval *alias, uint8_t mode, zval *simple TSRMLS_DC) {
    if (Z_TYPE_P(methodname) != IS_STRING) {
        zend_throw_exception(NULL, "method name must be a string", 0 TSRMLS_CC);
        return;
    }
    zval *func;
    hprose_zval_new(func);
    array_init_size(func, 2);
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(belongto);
    Z_ADDREF_P(methodname);
#else
    Z_TRY_ADDREF_P(belongto);
    Z_TRY_ADDREF_P(methodname);
#endif
    add_next_index_zval(func, belongto);
    add_next_index_zval(func, methodname);
    hprose_service_add_function(_this, func, alias, mode, simple TSRMLS_CC);
    hprose_zval_free(func);    
}

static zend_always_inline void hprose_service_add_methods(hprose_service *_this, zval *methods, zval *belongto, zval *aliases, uint8_t mode, zval *simple TSRMLS_DC) {
    int32_t i, count;
    zval *_aliases;
    HashTable *ht = Z_ARRVAL_P(methods);
    if (aliases) {
        if ((Z_TYPE_P(aliases) == IS_STRING && Z_STRLEN_P(aliases) == 0) ||
            (Z_TYPE_P(aliases) == IS_ARRAY && Z_ARRLEN_P(aliases) == 0)) {
            aliases = NULL;
        }
    }
    count = Z_ARRLEN_P(methods);
    if (aliases) {
        if (Z_TYPE_P(aliases) == IS_STRING) {
            hprose_zval_new(_aliases);
            array_init_size(_aliases, count);
            hprose_bytes_io alias_prefix;
            hprose_bytes_io_init(&alias_prefix, NULL, 0);
            hprose_bytes_io_write(&alias_prefix, Z_STRVAL_P(aliases), Z_STRLEN_P(aliases));
            hprose_bytes_io_putc(&alias_prefix, '_');
            zend_hash_internal_pointer_reset(ht);
            for (i = 0; i < count; ++i) {
                hprose_bytes_io alias;
#if PHP_MAJOR_VERSION < 7
                zval **method;
                zend_hash_get_current_data(ht, (void **)&method);
#else
                zval *method = zend_hash_get_current_data(ht);
#endif
                hprose_bytes_io_init(&alias, NULL, 0);
                alias_prefix.pos = 0;
                hprose_bytes_io_read_to(&alias_prefix, &alias, alias_prefix.len);
#if PHP_MAJOR_VERSION < 7
                hprose_bytes_io_write(&alias, Z_STRVAL_PP(method), Z_STRLEN_PP(method));
                add_next_index_stringl(_aliases, alias.buf, alias.len, 0);
#else
                hprose_bytes_io_write(&alias, Z_STRVAL_P(method), Z_STRLEN_P(method));
                add_next_index_stringl(_aliases, alias.buf, alias.len);
                hprose_bytes_io_close(&alias);
#endif
                zend_hash_move_forward(ht);
            }
            hprose_bytes_io_close(&alias_prefix);
        }
        else if (Z_TYPE_P(aliases) == IS_ARRAY) {
#if PHP_MAJOR_VERSION < 7
            _aliases = aliases;
            Z_ADDREF_P(_aliases);
            SEPARATE_ZVAL(&_aliases);
#else
            hprose_zval_new(_aliases);
            ZVAL_COPY(_aliases, aliases);
#endif
        }
        if (Z_ARRLEN_P(_aliases) != count) {
            zend_throw_exception(NULL, "The count of methods is not matched with aliases", 0 TSRMLS_CC);
            return;
        }
    }
    else {
        hprose_zval_new(_aliases);
        array_init_size(_aliases, count);
        zend_hash_internal_pointer_reset(ht);
        for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
            zval **method;
            zend_hash_get_current_data(ht, (void **)&method);
            add_next_index_stringl(_aliases, Z_STRVAL_PP(method), Z_STRLEN_PP(method), 1);
#else
            zval *method = zend_hash_get_current_data(ht);
            add_next_index_stringl(_aliases, Z_STRVAL_P(method), Z_STRLEN_P(method));
#endif
            zend_hash_move_forward(ht);
        }
    }
    zend_hash_internal_pointer_reset(ht);
    for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
        zval **method;
        zend_hash_get_current_data(ht, (void **)&method);
        hprose_service_add_method(_this, *method, belongto, php_array_get(_aliases, i), mode, simple TSRMLS_CC);
#else
        zval *method = zend_hash_get_current_data(ht);
        hprose_service_add_method(_this, method, belongto, php_array_get(_aliases, i), mode, simple TSRMLS_CC);
#endif
        if (EG(exception)) return;
        zend_hash_move_forward(ht);
    }
    hprose_zval_free(_aliases);
}

static zend_always_inline void hprose_service_add_instance_methods(hprose_service *_this, zval *obj, zval *class_name, zval *alias_prefix, uint8_t mode, zval *simple TSRMLS_DC) {
    zval *methods = NULL;
    if (class_name) {
        convert_to_string(class_name);
        if (Z_STRLEN_P(class_name)) {
            hprose_zval_new(methods);
            hprose_service_get_declared_only_methods(class_name, methods TSRMLS_CC);
        }
    }
    if (methods == NULL) {
        zval *cname;
        hprose_zval_new(cname);
        function_invoke(get_class, cname, "z", obj);
        hprose_zval_new(methods);
        hprose_service_get_declared_only_methods(cname, methods TSRMLS_CC);
        hprose_zval_free(cname);
    }
    hprose_service_add_methods(_this, methods, obj, alias_prefix, mode, simple TSRMLS_CC);
    hprose_zval_free(methods);
}

static zend_always_inline void hprose_service_add_class_methods(hprose_service *_this, zval *class_name, zval *exec_class, zval *alias_prefix, uint8_t mode, zval *simple TSRMLS_DC) {
    zval *methods = NULL;
    hprose_zval_new(methods);
    convert_to_string(class_name);
    hprose_service_get_declared_only_methods(class_name, methods TSRMLS_CC);
    if (exec_class) {
        hprose_service_add_methods(_this, methods, exec_class, alias_prefix, mode, simple TSRMLS_CC);
    }
    else {
        hprose_service_add_methods(_this, methods, class_name, alias_prefix, mode, simple TSRMLS_CC);
    }
    hprose_zval_free(methods);
}

#if PHP_MAJOR_VERSION < 7
static void hprose_service_remote_call_dtor(void *pDest) {
    efree(*(hprose_remote_call **)pDest);
}
#else
static void hprose_service_remote_call_dtor(zval *pDest) {
    efree((hprose_remote_call *)Z_PTR_P(pDest));
}
#endif

ZEND_METHOD(hprose_service, __construct) {
    char *url = "";
    length_t len = 0;
    HPROSE_OBJECT_INTERN(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &url, &len) == FAILURE) {
        return;
    }
    intern->_this = emalloc(sizeof(hprose_service));
    intern->_this->simple = 0;
    ALLOC_HASHTABLE(intern->_this->calls);
    zend_hash_init(intern->_this->calls, 0, NULL, hprose_service_remote_call_dtor, 0);
    hprose_zval_new(intern->_this->names);
    array_init(intern->_this->names);
    hprose_zval_new(intern->_this->filters);
    array_init(intern->_this->filters);
}

ZEND_METHOD(hprose_service, __destruct) {
    HPROSE_OBJECT_INTERN(service);
    if (intern->_this) {
        zend_hash_destroy(intern->_this->calls);
        FREE_HASHTABLE(intern->_this->calls);
        hprose_zval_free(intern->_this->names);
        hprose_zval_free(intern->_this->filters);
        efree(intern->_this);
        intern->_this = NULL;
    }
}

ZEND_METHOD(hprose_service, getErrorTypeString) {
    long e;
    char *err;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &e) == FAILURE) {
        return;
    }
    err = get_error_type_string(e);
#if PHP_MAJOR_VERSION < 7
    RETURN_STRING(err, 1);
#else
    RETURN_STRING(err);
#endif
}

ZEND_METHOD(hprose_service, sendError) {
    zval *err, *context;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &err, &context) == FAILURE) {
        return;
    }    
    hprose_service_send_error(getThis(), err, context, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_service, doInvoke) {
    zval *input, *context;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "oz", &input, &context) == FAILURE) {
        return;
    }    
    hprose_service_do_invoke(getThis(), HPROSE_GET_OBJECT_P(bytes_io, input)->_this, context, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_service, doFunctionList) {
    zval *context;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &context) == FAILURE) {
        return;
    }    
    hprose_service_do_function_list(getThis(), context, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_service, isDebugEnabled) {
#if PHP_MAJOR_VERSION < 7
    zval *_debug = zend_read_property(get_hprose_service_ce(), getThis(), ZEND_STRL("debug"), 1 TSRMLS_CC);
    zend_bool debug = Z_BVAL_P(_debug);
#else
    zval _debug;
    zend_bool debug;
    zend_read_property(get_hprose_service_ce(), getThis(), ZEND_STRL("debug"), 1, &_debug);
    convert_to_boolean(&_debug);
    debug = (Z_TYPE(_debug) != IS_FALSE);
#endif
    RETURN_BOOL(debug);
}

ZEND_METHOD(hprose_service, setDebugEnabled) {
    zend_bool debug = 1;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &debug) == FAILURE) {
        return;
    }
    zend_update_property_bool(get_hprose_service_ce(), getThis(), ZEND_STRL("debug"), debug TSRMLS_CC);
}

ZEND_METHOD(hprose_service, getErrorTypes) {
#if PHP_MAJOR_VERSION < 7
    zval *_error_types = zend_read_property(get_hprose_service_ce(), getThis(), ZEND_STRL("error_types"), 1 TSRMLS_CC);
    long error_types = Z_LVAL_P(_error_types);
#else
    zval _error_types;
    long error_types;
    zend_read_property(get_hprose_service_ce(), getThis(), ZEND_STRL("error_types"), 1, &_error_types);
    error_types = Z_LVAL(_error_types);
#endif
    RETURN_LONG(error_types);
}

ZEND_METHOD(hprose_service, setErrorTypes) {
    long error_types;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &error_types) == FAILURE) {
        return;
    }
    zend_update_property_long(get_hprose_service_ce(), getThis(), ZEND_STRL("error_types"), error_types TSRMLS_CC);
}

ZEND_METHOD(hprose_service, getFilter) {
    HPROSE_THIS(service);
    if (Z_ARRLEN_P(_this->filters)) {
        zval *filter = php_array_get(_this->filters, 0);
        RETURN_ZVAL(filter, 1, 0);
    }
    RETURN_NULL();
}    

ZEND_METHOD(hprose_service, setFilter) {
    zval *filter = NULL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o!", &filter) == FAILURE) {
        return;
    }
    zend_hash_clean(Z_ARRVAL_P(_this->filters));
    if (filter) {
        Z_ADDREF_P(filter);        
        add_index_zval(_this->filters, 0, filter);
    }
}

ZEND_METHOD(hprose_service, addFilter) {
    zval *filter;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &filter) == FAILURE) {
        return;
    }
    Z_ADDREF_P(filter);        
    add_next_index_zval(_this->filters, filter);
}

ZEND_METHOD(hprose_service, removeFilter) {
    zval *filter;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &filter) == FAILURE) {
        return;
    }
    zval i;
    function_invoke(array_search, &i, "zz", filter, _this->filters);
#if PHP_MAJOR_VERSION < 7
    if ((Z_TYPE(i) == IS_BOOL && Z_BVAL(i) == 0) || Z_TYPE(i) == IS_NULL) {
#else
    if (Z_TYPE(i) == IS_FALSE || Z_TYPE(i) == IS_NULL) {
#endif
        RETURN_FALSE;
    }
    function_invoke(array_splice, _this->filters, "zzl", _this->filters, &i, 1);
    RETURN_TRUE;
}

ZEND_METHOD(hprose_service, getSimple) {
    HPROSE_THIS(service);
    RETURN_BOOL(_this->simple);
}    

ZEND_METHOD(hprose_service, setSimple) {
    zend_bool simple = 1;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|b", &simple) == FAILURE) {
        return;
    }
    _this->simple = simple;
}

ZEND_METHOD(hprose_service, defaultHandle) {
    zval *request, *context;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &request, &context) == FAILURE) {
        return;
    }    
    hprose_service_default_handle(getThis(), request, context, return_value TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addFunction) {
    zval *func, *alias = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!lz!", &func, &alias, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_function(_this, func, alias, mode, simple TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addMissingFunction) {
    zval *func, *alias = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lz!", &func, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_zval_new(alias);
    ZVAL_STRINGL_1(alias, "*", 1);
    hprose_service_add_function(_this, func, alias, mode, simple TSRMLS_CC);
    hprose_zval_free(alias);
}

ZEND_METHOD(hprose_service, addFunctions) {
    zval *funcs, *aliases = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!lz!", &funcs, &aliases, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_functions(_this, funcs, aliases, mode, simple TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addMethod) {
    zval *methodname, *belongto, *alias = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z!lz!", &methodname, &belongto, &alias, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_method(_this, methodname, belongto, alias, mode, simple TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addMethods) {
    zval *methods, *belongto, *aliases = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z!lz!", &methods, &belongto, &aliases, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_methods(_this, methods, belongto, aliases, mode, simple TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addInstanceMethods) {
    zval *obj, *class_name = NULL, *alias_prefix = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!lz!", &obj, &class_name, &alias_prefix, &mode, &simple) == FAILURE) {
        return;
    }
    if (Z_TYPE_P(obj) != IS_OBJECT) {
        hprose_service_add_class_methods(_this, obj, class_name, alias_prefix, mode, simple TSRMLS_CC);        
    }
    else {
        hprose_service_add_instance_methods(_this, obj, class_name, alias_prefix, mode, simple TSRMLS_CC);
    }
}

ZEND_METHOD(hprose_service, addClassMethods) {
    zval *class_name, *exec_class = NULL, *alias_prefix = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!lz!", &class_name, &exec_class, &alias_prefix, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_class_methods(_this, class_name, exec_class, alias_prefix, mode, simple TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(hprose_service_construct_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, url)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_get_error_type_string_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, errno)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_send_error_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, error)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_do_invoke_arginfo, 0, 0, 2)
    ZEND_ARG_OBJ_INFO(0, input, HproseBytesIO, 0)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_do_function_list_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_is_debug_enabled_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_set_debug_enabled_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, enable)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_get_error_types_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_set_error_types_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, error_types)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_get_filter_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_set_filter_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, filter, HproseFilter, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_filter_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, filter, HproseFilter, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_remove_filter_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, filter, HproseFilter, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_get_simple_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_set_simple_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_default_handle_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, request)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_function_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, func)
    ZEND_ARG_INFO(0, alias)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_missing_function_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, func)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_functions_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, funcs, 0)
    ZEND_ARG_ARRAY_INFO(0, aliases, 1)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_method_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, methodname)
    ZEND_ARG_INFO(0, belongto)
    ZEND_ARG_INFO(0, alias)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_methods_arginfo, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, methods, 0)
    ZEND_ARG_INFO(0, belongto)
    ZEND_ARG_INFO(0, aliases)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_instance_methods_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, obj)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_INFO(0, alias_prefix)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_class_methods_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_INFO(0, exec_class)
    ZEND_ARG_INFO(0, alias_prefix)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_service_methods[] = {
    ZEND_ME(hprose_service, __construct, hprose_service_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_service, __destruct, hprose_service_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_ME(hprose_service, getErrorTypeString, hprose_service_get_error_type_string_arginfo, ZEND_ACC_PROTECTED)
    ZEND_ME(hprose_service, sendError, hprose_service_send_error_arginfo, ZEND_ACC_PROTECTED)
    ZEND_ME(hprose_service, doInvoke, hprose_service_do_invoke_arginfo, ZEND_ACC_PROTECTED)
    ZEND_ME(hprose_service, doFunctionList, hprose_service_do_function_list_arginfo, ZEND_ACC_PROTECTED)
    ZEND_ME(hprose_service, isDebugEnabled, hprose_service_is_debug_enabled_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, setDebugEnabled, hprose_service_set_debug_enabled_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, getErrorTypes, hprose_service_get_error_types_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, setErrorTypes, hprose_service_set_error_types_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, getFilter, hprose_service_get_filter_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, setFilter, hprose_service_set_filter_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addFilter, hprose_service_add_filter_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, removeFilter, hprose_service_remove_filter_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, getSimple, hprose_service_get_simple_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, setSimple, hprose_service_set_simple_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, defaultHandle, hprose_service_default_handle_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addMissingFunction, hprose_service_add_missing_function_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addFunction, hprose_service_add_function_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addFunctions, hprose_service_add_functions_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addMethod, hprose_service_add_method_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addMethods, hprose_service_add_methods_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addInstanceMethods, hprose_service_add_instance_methods_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addClassMethods, hprose_service_add_class_methods_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(service)

HPROSE_OBJECT_FREE_BEGIN(service)
    if (intern->_this) {
        zend_hash_destroy(intern->_this->calls);
        FREE_HASHTABLE(intern->_this->calls);
        hprose_zval_free(intern->_this->names);
        hprose_zval_free(intern->_this->filters);
        efree(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_NEW_BEGIN(service)
HPROSE_OBJECT_NEW_END(service)

HPROSE_CLASS_ENTRY(service)

HPROSE_STARTUP_FUNCTION(service) {
    HPROSE_REGISTER_CLASS("Hprose", "Service", service);
    HPROSE_REGISTER_CLASS_HANDLERS(service);
    zend_declare_property_bool(hprose_service_ce, ZEND_STRL("debug"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_long(hprose_service_ce, ZEND_STRL("error_types"), E_ALL & ~E_NOTICE, ZEND_ACC_PROTECTED TSRMLS_CC);
    zend_declare_property_null(hprose_service_ce, ZEND_STRL("onBeforeInvoke"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(hprose_service_ce, ZEND_STRL("onAfterInvoke"), ZEND_ACC_PUBLIC TSRMLS_CC);
    zend_declare_property_null(hprose_service_ce, ZEND_STRL("onSendError"), ZEND_ACC_PUBLIC TSRMLS_CC);
    hprose_service_ce->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
    return SUCCESS;
}
