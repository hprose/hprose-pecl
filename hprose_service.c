
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
 * LastModified: May 13, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"
#include "hprose_result_mode.h"
#include "hprose_future.h"
#include "hprose_service.h"

#if PHP_MAJOR_VERSION < 7
#define zend_exception_ce zend_exception_get_default(TSRMLS_C)
#else
#define zend_exception_ce zend_exception_get_base()
#endif

static zend_bool is_magic_method(char *name, int32_t len) {
    static const char *magic_methods[14] = {
        "__construct",
        "__destruct",
        "__call",
        "__callStatic",
        "__get",
        "__set",
        "__isset",
        "__unset",
        "__sleep",
        "__wakeup",
        "__toString",
        "__invoke",
        "__set_state",
        "__clone"
    };
    static const int magic_methods_length[14] = {
        sizeof("__construct") - 1,
        sizeof("__destruct") - 1,
        sizeof("__call") - 1,
        sizeof("__callStatic") - 1,
        sizeof("__get") - 1,
        sizeof("__set") - 1,
        sizeof("__isset") - 1,
        sizeof("__unset") - 1,
        sizeof("__sleep") - 1,
        sizeof("__wakeup") - 1,
        sizeof("__toString") - 1,
        sizeof("__invoke") - 1,
        sizeof("__set_state") - 1,
        sizeof("__clone") - 1
    };
    int i, j;
    if (len < 5 || len > 12) return 0;
    if (name[0] != '_' || name[1] != '_') return 0;
    for (i = 0; i < 14; ++i) {
        zend_bool find = 1;
        if (len != magic_methods_length[i]) continue;
        for (j = 2; j < len; ++j) {
            if (magic_methods[i][j] != name[j]) {
                find = 0;
                break;
            }
        }
        if (find) return 1;
    }
    return 0;
}

static char * get_error_type_string(long e) {
    switch (e) {
        case E_ERROR: return "Error";
        case E_WARNING: return "Warning";
        case E_PARSE: return "Parse Error";
        case E_NOTICE: return "Notice";
        case E_CORE_ERROR: return "Core Error";
        case E_CORE_WARNING: return "Core Warning";
        case E_COMPILE_ERROR: return "Compile Error";
        case E_COMPILE_WARNING: return "Compile Warning";
        case E_USER_ERROR: return "User Error";
        case E_USER_WARNING: return "User Warning";
        case E_USER_NOTICE: return "User Notice";
        case E_STRICT: return "Runtime Notice";
        case E_RECOVERABLE_ERROR: return "Catchable Fatal Error";
#if PHP_API_VERSION >= 20090626
        case E_DEPRECATED: return "Deprecated";
        case E_USER_DEPRECATED: return "User Deprecated";
#endif
        default: return "Unknown Error";
    }
}

static zend_always_inline void hprose_service_input_filter(hprose_service *_this, zval *data, zval *context TSRMLS_DC) {
    HashTable *ht = Z_ARRVAL_P(_this->filters);
    int32_t i = zend_hash_num_elements(ht);
    if (i) {
        zend_hash_internal_pointer_end(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **filter;
            zend_hash_get_current_data(ht, (void **)&filter);
            method_invoke_ex(*filter, inputFilter, data, 1, "zz", data, context);
#else
            zval *filter = zend_hash_get_current_data(ht);
            method_invoke_ex(filter, inputFilter, data, 1, "zz", data, context);
#endif
            if (EG(exception)) return;
            zend_hash_move_backwards(ht);
        }
    }
}

static zend_always_inline void hprose_service_output_filter(hprose_service *_this, zval *data, zval *context TSRMLS_DC) {
    HashTable *ht = Z_ARRVAL_P(_this->filters);
    int32_t i = zend_hash_num_elements(ht);
    if (i) {
        zend_hash_internal_pointer_reset(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **filter;
            zend_hash_get_current_data(ht, (void **)&filter);
            method_invoke_ex(*filter, outputFilter, data, 1, "zz", data, context);
#else
            zval *filter = zend_hash_get_current_data(ht);
            method_invoke_ex(filter, outputFilter, data, 1, "zz", data, context);
#endif
            if (EG(exception)) return;
            zend_hash_move_forward(ht);
        }
    }
}

static zend_always_inline void hprose_service_on_send_error(zval *service, zval *err, zval *context TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    zval *on_send_error = zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onSendError"), 1 TSRMLS_CC);
    if (is_callable_p(on_send_error)) {
        callable_invoke(on_send_error, NULL, "zz", err, context);
    }
#else
    zval on_send_error;
    ZVAL_NULL(&on_send_error);
    zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onSendError"), 1, &on_send_error);
    if (is_callable(on_send_error)) {
        callable_invoke(&on_send_error, NULL, "zz", err, context);
    }
#endif
}

static zend_always_inline void hprose_service_on_before_invoke(zval *service, zval *name, zval *args, zend_bool byref, zval *context TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    zval *on_before_invoke = zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onBeforeInvoke"), 1 TSRMLS_CC);
    if (is_callable_p(on_before_invoke)) {
        callable_invoke(on_before_invoke, NULL, "zzbz", name, args, byref, context);
    }
#else
    zval on_before_invoke;
    ZVAL_NULL(&on_before_invoke);
    zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onBeforeInvoke"), 1, &on_before_invoke);
    if (is_callable(on_before_invoke)) {
        callable_invoke(&on_before_invoke, NULL, "zzbz", name, args, byref, context);
    }
#endif
}

static zend_always_inline void hprose_service_on_after_invoke(zval *service, zval *name, zval *args, zend_bool byref, zval *result, zval *context TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    zval *on_after_invoke = zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onAfterInvoke"), 1 TSRMLS_CC);
    if (is_callable_p(on_after_invoke)) {
        callable_invoke(on_after_invoke, NULL, "zzbzz", name, args, byref, result, context);
    }
#else
    zval on_after_invoke;
    ZVAL_NULL(&on_after_invoke);
    zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onAfterInvoke"), 1, &on_after_invoke);
    if (is_callable(on_after_invoke)) {
        callable_invoke(&on_after_invoke, NULL, "zzbzz", name, args, byref, result, context);
    }
#endif
}

static void hprose_service_send_error(zval *service, zval *err, zval *context, zval *return_value TSRMLS_DC) {
    hprose_bytes_io output;
    hprose_writer writer;
    hprose_bytes_io_init(&output, NULL, 0);
    hprose_writer_init(&writer, &output, 1);
    hprose_service_on_send_error(service, err, context TSRMLS_CC);
    hprose_bytes_io_putc(&output, HPROSE_TAG_ERROR);
    hprose_writer_write_string(&writer, err);
    hprose_bytes_io_putc(&output, HPROSE_TAG_END);
    hprose_writer_destroy(&writer);
#if PHP_MAJOR_VERSION < 7
    RETVAL_STRINGL_0(HB_BUF(output), HB_LEN(output));
#else
    RETVAL_STR(HB_STR(output));
#endif
    hprose_service_output_filter(HPROSE_GET_OBJECT_P(service, service)->_this, return_value, context TSRMLS_CC);
}

static void hprose_service_after_invoke(zval *service, zval *name, zval *args, zend_bool byref, uint8_t mode, zend_bool simple, zval *context, zval *result, hprose_bytes_io *output, zend_bool async, zval *return_value, zend_bool *bailout TSRMLS_DC) {
    hprose_service *_this = HPROSE_GET_OBJECT_P(service, service)->_this;
    zend_try {
        hprose_service_on_after_invoke(service, name, args, byref, result, context TSRMLS_CC);
    }
    zend_catch {
        *bailout = 1;
    } zend_end_try();
    if (*bailout || EG(exception)) {
        RETURN_NULL();
    }
    if (mode == HPROSE_RESULT_MODE_RAW_WITH_END_TAG) {
        convert_to_string(result);
        RETVAL_ZVAL(result, 0, 1);
        hprose_service_output_filter(_this, return_value, context TSRMLS_CC);
        return;
    }
    else if (mode == HPROSE_RESULT_MODE_RAW) {
        convert_to_string(result);
        hprose_bytes_io_write(output, Z_STRVAL_P(result), Z_STRLEN_P(result));
    }
    else {
        hprose_writer writer;
        hprose_writer_init(&writer, output, simple);
        hprose_bytes_io_putc(output, HPROSE_TAG_RESULT);
        if (mode == HPROSE_RESULT_MODE_SERIALIZED) {
            convert_to_string(result);
            hprose_bytes_io_write(output, Z_STRVAL_P(result), Z_STRLEN_P(result));
        }
        else {
            hprose_writer_reset(&writer);
            hprose_writer_serialize(&writer, result);
            hprose_zval_free(result);
        }
        if (byref) {
            hprose_bytes_io_putc(output, HPROSE_TAG_ARGUMENT);
            hprose_writer_reset(&writer);
            hprose_writer_write_array(&writer, args);
        }
        hprose_writer_destroy(&writer);
    }
    if (async) {
        hprose_bytes_io_putc(output, HPROSE_TAG_END);
#if PHP_MAJOR_VERSION < 7
        RETVAL_STRINGL_0(HB_BUF_P(output), HB_LEN_P(output));
#else
        RETVAL_STR(HB_STR_P(output));
#endif
        hprose_service_output_filter(_this, return_value, context TSRMLS_CC);
    }
    else {
        RETURN_NULL();
    }
}

static void hprose_service_do_invoke(zval *service, hprose_bytes_io *input, zval *context, zval *return_value TSRMLS_DC) {
    hprose_service *_this = HPROSE_GET_OBJECT_P(service, service)->_this;
    hprose_bytes_io output;
    hprose_reader reader;
    char tag;
    hprose_bytes_io_init(&output, NULL, 0);
    hprose_reader_init(&reader, input, 0);
    do {
        zval *_name, *args, *result = NULL;
        char *name, *alias;
        int32_t nlen;
        hprose_remote_call *call;
        zend_bool simple = _this->simple;
        zend_bool byref = 0;
        zend_bool bailout = 0;
        hprose_zval_new(_name);
        hprose_reader_reset(&reader);
        hprose_reader_read_string(&reader, _name TSRMLS_CC);
        name = Z_STRVAL_P(_name);
        nlen = Z_STRLEN_P(_name);
        alias = zend_str_tolower_dup(name, nlen);
        call = (hprose_remote_call *)zend_hash_str_find_ptr(_this->calls, alias, nlen);
        efree(alias);
        if (call == NULL) {
            call = (hprose_remote_call *)zend_hash_str_find_ptr(_this->calls, ZEND_STRL("*"));
            if (call == NULL) {
                zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                        "Can't find this function %s().", name);
                hprose_reader_destroy(&reader);
                hprose_bytes_io_close(&output);
                hprose_zval_free(_name);
                return;
            }
        }
        if (call->simple < 2) {
            simple = (zend_bool)call->simple;
        }
        tag = hprose_bytes_io_getc(input);
        hprose_zval_new(args);
        if (tag == HPROSE_TAG_LIST) {
            hprose_reader_reset(&reader);
            hprose_reader_read_list_without_tag(&reader, args TSRMLS_CC);
            tag = hprose_bytes_io_getc(input);
            if (tag == HPROSE_TAG_TRUE) {
                byref = 1;
                tag = hprose_bytes_io_getc(input);
            }
            if (call->byref) {
                zval *_args;
                uint32_t count = Z_ARRLEN_P(args);
                uint32_t i, n = MIN(count, call->fcc.function_handler->common.num_args);
                hprose_zval_new(_args);
                array_init_size(_args, count);
                for (i = 0; i < n; ++i) {
                    zval *e = php_array_get(args, i);
#if PHP_MAJOR_VERSION < 7
                    Z_ADDREF_P(e);
                    if (call->fcc.function_handler->common.arg_info[i].pass_by_reference) {
                        Z_SET_ISREF_P(e);
                    }
                    add_next_index_zval(_args, e);
#else
                    if (call->fcc.function_handler->common.arg_info[i].pass_by_reference) {
                        zval r;
                        ZVAL_NEW_REF(&r, e);
                        add_next_index_zval(_args, &r);
                    }
                    else {
                        Z_TRY_ADDREF_P(e);
                        add_next_index_zval(_args, e);
                    }
#endif
                }
                for (i = n; i < count; ++i) {
                    zval *e = php_array_get(args, i);
#if PHP_MAJOR_VERSION < 7
                    Z_ADDREF_P(e);
#else
                    Z_TRY_ADDREF_P(e);
#endif
                    add_next_index_zval(_args, e);
                }
                hprose_zval_free(args);
                args = _args;
            }
        }
        else {
            array_init_size(args, 0);
        }
        if ((tag != HPROSE_TAG_END) && (tag != HPROSE_TAG_CALL)) {
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                    "Unknown tag: %c\r\nwith following data: %s",
                    tag, hprose_bytes_io_to_string(input));
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            return;
        }
        zend_try {
            hprose_service_on_before_invoke(service, _name, args, byref, context TSRMLS_CC);
        }
        zend_catch {
            bailout = 1;
        } zend_end_try();
        if (bailout || EG(exception)) {
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            if (bailout) {
                zend_bailout();
            }
            return;
        }
        if (zend_hash_str_find_ptr(_this->calls, ZEND_STRL("*")) == call) {
            zval *_args;
            hprose_zval_new(_args);
            array_init_size(_args, 2);
#if PHP_MAJOR_VERSION < 7
            Z_ADDREF_P(_name);
            Z_ADDREF_P(args);
#else
            Z_TRY_ADDREF_P(_name);
            Z_TRY_ADDREF_P(args);
#endif
            add_next_index_zval(_args, _name);
            add_next_index_zval(_args, args);
            hprose_zval_free(args);
            args = _args;
        }
        hprose_zval_new(result);
        ZVAL_NULL(result);
        if (call->async) {
            zval *completer, *async_callback, *callback;
            hprose_zval_new(completer);
            create_php_object_no_args(HproseCompleter, completer);
            hprose_zval_new(async_callback);
            create_php_object(HproseAsyncCallback, async_callback, "z", completer);
            hprose_zval_new(callback);
            array_init_size(callback, 2);
#if PHP_MAJOR_VERSION < 7
            Z_ADDREF_P(async_callback);
#else
            Z_TRY_ADDREF_P(async_callback);
#endif
            add_next_index_zval(callback, async_callback);
#if PHP_MAJOR_VERSION < 7
            add_next_index_string(callback, "handler", 1);
#else
            add_next_index_string(callback, "handler");
#endif
            hprose_zval_free(async_callback);
#if PHP_MAJOR_VERSION < 7
            Z_ADDREF_P(callback);
#else
            Z_TRY_ADDREF_P(callback);
#endif
            add_next_index_zval(args, callback);
            hprose_zval_free(callback);
            hprose_completer_future(HPROSE_GET_OBJECT_P(completer, completer)->_this, result TSRMLS_CC);
            hprose_zval_free(completer);
        }
        zend_try {
            if (call->async) {
                __function_invoke_args(call->fcc, NULL, NULL, args TSRMLS_CC);
            }
            else {
                __function_invoke_args(call->fcc, NULL, result, args TSRMLS_CC);
            }
        }
        zend_catch {
            bailout = 1;
        } zend_end_try();
        if (bailout || EG(exception)) {
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            hprose_zval_free(result);
            if (bailout) {
                zend_bailout();
            }
            return;
        }
        if (Z_TYPE_P(result) == IS_OBJECT &&
            instanceof_function(Z_OBJCE_P(result), get_hprose_future_ce() TSRMLS_CC)) {
            zval *completer, *after_invoke_callback, *callback;
            hprose_zval_new(completer);
            create_php_object_no_args(HproseCompleter, completer);
            hprose_zval_new(after_invoke_callback);
            create_php_object(HproseAfterInvokeCallback, after_invoke_callback, "zzzzblbz", service, completer, _name, args, byref, (long)(call->mode), simple, context);
            hprose_zval_new(callback);
            array_init_size(callback, 2);
#if PHP_MAJOR_VERSION < 7
            Z_ADDREF_P(after_invoke_callback);
#else
            Z_TRY_ADDREF_P(after_invoke_callback);
#endif
            add_index_zval(callback, 0, after_invoke_callback);
#if PHP_MAJOR_VERSION < 7
            add_index_string(callback, 1, "handler", 1);
#else
            add_index_string(callback, 1, "handler");
#endif
            hprose_future_then(HPROSE_GET_OBJECT_P(future, result)->_this, callback TSRMLS_CC);
            hprose_zval_free(callback);
            hprose_zval_new(callback);
            array_init_size(callback, 2);
#if PHP_MAJOR_VERSION < 7
            Z_ADDREF_P(after_invoke_callback);
#else
            Z_TRY_ADDREF_P(after_invoke_callback);
#endif
            add_index_zval(callback, 0, after_invoke_callback);
#if PHP_MAJOR_VERSION < 7
            add_index_string(callback, 1, "errorHandler", 1);
#else
            add_index_string(callback, 1, "errorHandler");
#endif
            hprose_future_catch_error(HPROSE_GET_OBJECT_P(future, result)->_this, callback TSRMLS_CC);
            hprose_zval_free(callback);
            hprose_zval_free(after_invoke_callback);
            hprose_completer_future(HPROSE_GET_OBJECT_P(completer, completer)->_this, return_value TSRMLS_CC);
            hprose_zval_free(completer);
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            hprose_zval_free(result);
            return;
        }
        RETVAL_NULL();
        hprose_service_after_invoke(service, _name, args, byref, call->mode, simple, context, result, &output, 0, return_value, &bailout TSRMLS_CC);
        if (bailout || EG(exception)) {
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            hprose_zval_free(result);
            if (bailout) {
                zend_bailout();
            }
            return;
        }
        if (Z_TYPE_P(return_value) != IS_NULL) {
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            return;
        }
        hprose_zval_free(_name);
        hprose_zval_free(args);
    } while (tag == HPROSE_TAG_CALL);
    hprose_reader_destroy(&reader);
    hprose_bytes_io_putc(&output, HPROSE_TAG_END);
#if PHP_MAJOR_VERSION < 7
    RETVAL_STRINGL_0(HB_BUF(output), HB_LEN(output));
#else
    RETVAL_STR(HB_STR(output));
#endif
    hprose_service_output_filter(_this, return_value, context TSRMLS_CC);
}

static void hprose_service_do_function_list(zval *service, zval *context, zval *return_value TSRMLS_DC) {
    hprose_service *_this = HPROSE_GET_OBJECT_P(service, service)->_this;
    hprose_bytes_io output;
    hprose_writer writer;
    hprose_bytes_io_init(&output, NULL, 0);
    hprose_writer_init(&writer, &output, 1);
    hprose_bytes_io_putc(&output, HPROSE_TAG_FUNCTIONS);
    hprose_writer_write_array(&writer, _this->names);
    hprose_bytes_io_putc(&output, HPROSE_TAG_END);
    hprose_writer_destroy(&writer);
#if PHP_MAJOR_VERSION < 7
    RETVAL_STRINGL_0(HB_BUF(output), HB_LEN(output));
#else
    RETVAL_STR(HB_STR(output));
#endif
    hprose_service_output_filter(_this, return_value, context TSRMLS_CC);
}

static void hprose_service_catch_error(zval *service, zval *err, zval *context, zval *return_value TSRMLS_DC) {
    hprose_bytes_io output;
    zval result;
#if PHP_MAJOR_VERSION < 7
    zval *_debug = zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("debug"), 1 TSRMLS_CC);
    zend_bool debug = Z_BVAL_P(_debug);
#else
    zval _debug;
    zend_bool debug;
    zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("debug"), 1, &_debug);
    debug = (Z_TYPE(_debug) == IS_TRUE);
#endif
    hprose_bytes_io_init(&output, NULL, 0);

    method_invoke_no_args(err, getMessage, &result);
    hprose_bytes_io_write(&output, Z_STRVAL(result), Z_STRLEN(result));
    zval_dtor(&result);

    if (debug) {
        method_invoke_no_args(err, getFile, &result);
        hprose_bytes_io_write(&output, ZEND_STRL("\nfile: "));
        hprose_bytes_io_write(&output, Z_STRVAL(result), Z_STRLEN(result));
        zval_dtor(&result);

        method_invoke_no_args(err, getLine, &result);
        hprose_bytes_io_write(&output, ZEND_STRL("\nline: "));
        hprose_bytes_io_write_int(&output, Z_LVAL(result));

        method_invoke_no_args(err, getTraceAsString, &result);
        hprose_bytes_io_write(&output, ZEND_STRL("\ntrace: "));
        hprose_bytes_io_write(&output, Z_STRVAL(result), Z_STRLEN(result));
        zval_dtor(&result);
    }
#if PHP_MAJOR_VERSION < 7
    ZVAL_STRINGL_0(&result, HB_BUF(output), HB_LEN(output));
#else
    ZVAL_STR(&result, HB_STR(output));
#endif
    hprose_service_send_error(service, &result, context, return_value TSRMLS_CC);
    zval_dtor(&result);
}

static zend_always_inline zend_bool hprose_service_try_catch_error(zval *service, zval *context, zval *return_value TSRMLS_DC) {
    if (EG(exception)) {
#if PHP_MAJOR_VERSION < 7
        zval *err = EG(exception);
        Z_ADDREF_P(err);
        SEPARATE_ZVAL(&err);
        zend_clear_exception(TSRMLS_C);
        hprose_service_catch_error(service, err, context, return_value TSRMLS_CC);
#else
        zval err;
        ZVAL_OBJ(&err, EG(exception));
        Z_ADDREF(err);
        SEPARATE_ZVAL(&err);
        zend_clear_exception();
        hprose_service_catch_error(service, &err, context, return_value TSRMLS_CC);
#endif
        zval_ptr_dtor(&err);
        return 1;
    }
    return 0;
}

static zend_always_inline void hprose_service_default_handle(zval *service, zval *request, zval *context, zval *return_value TSRMLS_DC) {
    hprose_service *_this = HPROSE_GET_OBJECT_P(service, service)->_this;
    hprose_bytes_io input;
    hprose_service_input_filter(_this, request, context TSRMLS_CC);
    if (hprose_service_try_catch_error(service, context, return_value TSRMLS_CC)) {
        return;
    }
#if PHP_MAJOR_VERSION < 7
    hprose_bytes_io_init_readonly(&input, Z_STRVAL_P(request), Z_STRLEN_P(request));
#else
    hprose_bytes_io_init_readonly(&input, Z_STR_P(request));
#endif
    switch (hprose_bytes_io_getc(&input)) {
        case HPROSE_TAG_CALL:
            hprose_service_do_invoke(service, &input, context, return_value TSRMLS_CC); break;
        case HPROSE_TAG_END:
            hprose_service_do_function_list(service, context, return_value TSRMLS_CC); break;
        default:
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                    "Wrong Request: \r\n%s", Z_STRVAL_P(request));
            break;
    }
    hprose_service_try_catch_error(service, context, return_value TSRMLS_CC);
}

static void hprose_service_get_declared_only_methods(zval *class, zval *return_value TSRMLS_DC) {
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

static void hprose_service_add_function(hprose_service *_this, zval *func, zval *alias, uint8_t mode, zval *simple, zend_bool async TSRMLS_DC) {
    zend_fcall_info_cache fcc = _get_fcall_info_cache(func TSRMLS_CC);
    char *name;
    int32_t len, i, n;
    hprose_remote_call *call;
    zend_bool _simple = 2;
    if (EG(exception)) {
        return;
    }
    if (Z_TYPE_P(func) == IS_ARRAY) {
        zval *val = php_array_get(func, 0);
        if (Z_TYPE_P(val) == IS_OBJECT) {
#if PHP_MAJOR_VERSION < 7
            Z_ADDREF_P(val);
            zend_llist_add_element(_this->objects, &val);
#else
            Z_ADDREF_P(val);
            zend_llist_add_element(_this->objects, &(Z_OBJ_P(val)));
#endif
        }
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
    call->async = async;
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

static void hprose_service_add_functions(hprose_service *_this, zval *funcs, zval *aliases, uint8_t mode, zval *simple, zend_bool async TSRMLS_DC) {
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
                    hprose_service_add_function(_this, *func, alias, mode, simple, async TSRMLS_CC);
#else
                    zval *func = zend_hash_get_current_data(ht);
                    zval *alias = php_array_get(aliases, i);
                    hprose_service_add_function(_this, func, alias, mode, simple, async TSRMLS_CC);
#endif
                    if (EG(exception)) return;
                    zend_hash_move_forward(ht);
                }
            }
            else {
                zend_throw_exception(NULL, "The count of functions is not matched with aliases", 0 TSRMLS_CC);
            }
        }
        return;
    }
    count = Z_ARRLEN_P(funcs);
    for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
        zval **func;
        zend_hash_get_current_data(ht, (void **)&func);
        hprose_service_add_function(_this, *func, NULL, mode, simple, async TSRMLS_CC);
#else
        zval *func = zend_hash_get_current_data(ht);
        hprose_service_add_function(_this, func, NULL, mode, simple, async TSRMLS_CC);
#endif
        if (EG(exception)) return;
        zend_hash_move_forward(ht);
    }
}

static void hprose_service_add_method(hprose_service *_this, zval *methodname, zval *belongto, zval *alias, uint8_t mode, zval *simple, zend_bool async TSRMLS_DC) {
    zval *func;
    if (Z_TYPE_P(methodname) != IS_STRING) {
        zend_throw_exception(NULL, "method name must be a string", 0 TSRMLS_CC);
        return;
    }
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
    hprose_service_add_function(_this, func, alias, mode, simple, async TSRMLS_CC);
    hprose_zval_free(func);
}

static void hprose_service_add_methods(hprose_service *_this, zval *methods, zval *belongto, zval *aliases, uint8_t mode, zval *simple, zend_bool async TSRMLS_DC) {
    int32_t i, count;
    zval *_aliases = NULL;
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
            hprose_bytes_io alias_prefix;
            hprose_zval_new(_aliases);
            array_init_size(_aliases, count);
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
                hprose_bytes_io_read_to(&alias_prefix, &alias, HB_LEN(alias_prefix));
#if PHP_MAJOR_VERSION < 7
                hprose_bytes_io_write(&alias, Z_STRVAL_PP(method), Z_STRLEN_PP(method));
                add_next_index_stringl(_aliases, HB_BUF(alias), HB_LEN(alias), 0);
#else
                hprose_bytes_io_write(&alias, Z_STRVAL_P(method), Z_STRLEN_P(method));
                add_next_index_str(_aliases, HB_STR(alias));
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
        else {
            zend_throw_exception(NULL, "The aliases must be string, array or null", 0 TSRMLS_CC);
            return;
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
        hprose_service_add_method(_this, *method, belongto, php_array_get(_aliases, i), mode, simple, async TSRMLS_CC);
#else
        zval *method = zend_hash_get_current_data(ht);
        hprose_service_add_method(_this, method, belongto, php_array_get(_aliases, i), mode, simple, async TSRMLS_CC);
#endif
        if (EG(exception)) return;
        zend_hash_move_forward(ht);
    }
    assert(_aliases != NULL);
    hprose_zval_free(_aliases);
}

static void hprose_service_add_instance_methods(hprose_service *_this, zval *obj, zval *class_name, zval *alias_prefix, uint8_t mode, zval *simple, zend_bool async TSRMLS_DC) {
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
    hprose_service_add_methods(_this, methods, obj, alias_prefix, mode, simple, async TSRMLS_CC);
    hprose_zval_free(methods);
}

static void hprose_service_add_class_methods(hprose_service *_this, zval *class_name, zval *exec_class, zval *alias_prefix, uint8_t mode, zval *simple, zend_bool async TSRMLS_DC) {
    zval *methods = NULL;
    hprose_zval_new(methods);
    convert_to_string(class_name);
    hprose_service_get_declared_only_methods(class_name, methods TSRMLS_CC);
    if (exec_class) {
        hprose_service_add_methods(_this, methods, exec_class, alias_prefix, mode, simple, async TSRMLS_CC);
    }
    else {
        hprose_service_add_methods(_this, methods, class_name, alias_prefix, mode, simple, async TSRMLS_CC);
    }
    hprose_zval_free(methods);
}

static void hprose_service_add(hprose_service *_this, int num, zval *arg1, zval *arg2, zval *arg3, zend_bool async TSRMLS_DC) {
    zval *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    switch (num) {
        case 1:
            if (arg1) {
                if (is_callable_p(arg1)) {
                    hprose_service_add_function(_this, arg1, NULL, (uint8_t)mode, simple, async TSRMLS_CC);
                    return;
                }                
                switch (Z_TYPE_P(arg1)) {
                    case IS_ARRAY:
                        hprose_service_add_functions(_this, arg1, NULL, (uint8_t)mode, simple, async TSRMLS_CC);
                        return;
                    case IS_OBJECT:
                        hprose_service_add_instance_methods(_this, arg1, NULL, NULL, (uint8_t)mode, simple, async TSRMLS_CC);
                        return;
                    case IS_STRING:
                        hprose_service_add_class_methods(_this, arg1, NULL, NULL, (uint8_t)mode, simple, async TSRMLS_CC);
                        return;
                }
            }
            break;
        case 2:
            if (is_callable_p(arg1) && is_string_p(arg2)) {
                hprose_service_add_function(_this, arg1, arg2, (uint8_t)mode, simple, async TSRMLS_CC);
                return;
            }
            else if (is_string_p(arg1)) {
                if (is_string_p(arg2)) {
                    zval *a;
                    zend_bool is_not_callable;
                    hprose_zval_new(a);
                    array_init_size(a, 2);
                    Z_ADDREF_P(arg1);
                    Z_ADDREF_P(arg2);
                    add_next_index_zval(a, arg2);
                    add_next_index_zval(a, arg1);
                    is_not_callable = !is_callable_p(a);
                    hprose_zval_free(a);
                    if (is_not_callable) {
                        if (class_exists(Z_STRVAL_P(arg2), Z_STRLEN_P(arg2), 1)) {
                            hprose_service_add_class_methods(_this, arg1, arg2, NULL, (uint8_t)mode, simple, async TSRMLS_CC);
                        }
                        else {
                            hprose_service_add_class_methods(_this, arg1, NULL, arg2, (uint8_t)mode, simple, async TSRMLS_CC);
                        }
                        return;
                    }
                }
                hprose_service_add_method(_this, arg1, arg2, NULL, (uint8_t)mode, simple, async TSRMLS_CC);
                return;
            }
            else if (is_array_p(arg1)) {
                if (is_array_p(arg2)) {
                    hprose_service_add_functions(_this, arg1, arg2, (uint8_t)mode, simple, async TSRMLS_CC);
                }
                else {
                    hprose_service_add_methods(_this, arg1, arg2, NULL, (uint8_t)mode, simple, async TSRMLS_CC);
                }
                return;
            }
            else if (is_object_p(arg1)) {
                hprose_service_add_instance_methods(_this, arg1, arg2, NULL, (uint8_t)mode, simple, async TSRMLS_CC);
                return;
            }
            break;
        case 3:
            if (is_callable_p(arg1) &&
                    ((arg2 == NULL) || (is_string_p(arg2) && Z_STRLEN_P(arg2) == 0)) &&
                    is_string_p(arg3)) {
                hprose_service_add_function(_this, arg1, arg3, (uint8_t)mode, simple, async TSRMLS_CC);
                return;
            }
            else if (is_string_p(arg1) && is_string_p(arg3)) {
                if (is_string_p(arg2)) {
                    zval *a;
                    zend_bool is_not_callable;
                    hprose_zval_new(a);
                    array_init_size(a, 2);
                    Z_ADDREF_P(arg1);
                    Z_ADDREF_P(arg2);
                    add_next_index_zval(a, arg2);
                    add_next_index_zval(a, arg1);
                    is_not_callable = !is_callable_p(a);
                    hprose_zval_free(a);
                    if (is_not_callable) {
                        hprose_service_add_class_methods(_this, arg1, arg2, arg3, (uint8_t)mode, simple, async TSRMLS_CC);
                        return;
                    }
                }
                if ((arg2 == NULL) || (is_string_p(arg2) && Z_STRLEN_P(arg2) == 0)) {
                    hprose_service_add_class_methods(_this, arg1, NULL, arg3, (uint8_t)mode, simple, async TSRMLS_CC);
                    return;
                }
                if (is_string_p(arg2) || is_object_p(arg2)) {
                    hprose_service_add_method(_this, arg1, arg2, arg3, (uint8_t)mode, simple, async TSRMLS_CC);
                    return;
                }
            }
            else if (is_array_p(arg1)) {
                if (((arg2 == NULL) || (is_string_p(arg2) && Z_STRLEN_P(arg2) == 0)) && is_array_p(arg3)) {
                    hprose_service_add_functions(_this, arg1, arg3, (uint8_t)mode, simple, async TSRMLS_CC);
                }
                else {
                    hprose_service_add_methods(_this, arg1, arg2, arg3, (uint8_t)mode, simple, async TSRMLS_CC);
                }
                return;
            }
            else if (is_object_p(arg1)) {
                hprose_service_add_instance_methods(_this, arg1, arg2, arg3, (uint8_t)mode, simple, async TSRMLS_CC);
                return;
            }
            break;
    }
    WRONG_PARAM_COUNT;    
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

static void hprose_service_object_dtor(void *data) {
#if PHP_MAJOR_VERSION < 7
        zval **val = (zval **)data;
        if (data) {
            zval_ptr_dtor(val);
        }
#else
        zend_object **obj = (zend_object **)data;
        if (data) {
            OBJ_RELEASE(*obj);
        }
#endif
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
    zval i;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "o", &filter) == FAILURE) {
        return;
    }
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
    zend_bool async = 0;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!lz!b", &func, &alias, &mode, &simple, &async) == FAILURE) {
        return;
    }
    hprose_service_add_function(_this, func, alias, (uint8_t)mode, simple, async TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addMissingFunction) {
    zval *func, *alias = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    zend_bool async = 0;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lz!b", &func, &mode, &simple, &async) == FAILURE) {
        return;
    }
    hprose_zval_new(alias);
    ZVAL_STRINGL_1(alias, "*", 1);
    hprose_service_add_function(_this, func, alias, (uint8_t)mode, simple, async TSRMLS_CC);
    hprose_zval_free(alias);
}

ZEND_METHOD(hprose_service, addFunctions) {
    zval *funcs, *aliases = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    zend_bool async = 0;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!lz!b", &funcs, &aliases, &mode, &simple, &async) == FAILURE) {
        return;
    }
    hprose_service_add_functions(_this, funcs, aliases, (uint8_t)mode, simple, async TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addMethod) {
    zval *methodname, *belongto, *alias = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    zend_bool async = 0;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z!lz!b", &methodname, &belongto, &alias, &mode, &simple, &async) == FAILURE) {
        return;
    }
    hprose_service_add_method(_this, methodname, belongto, alias, (uint8_t)mode, simple, async TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addMethods) {
    zval *methods, *belongto, *aliases = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    zend_bool async = 0;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z!lz!b", &methods, &belongto, &aliases, &mode, &simple, &async) == FAILURE) {
        return;
    }
    hprose_service_add_methods(_this, methods, belongto, aliases, (uint8_t)mode, simple, async TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addInstanceMethods) {
    zval *obj, *class_name = NULL, *alias_prefix = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    zend_bool async = 0;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!lz!b", &obj, &class_name, &alias_prefix, &mode, &simple, &async) == FAILURE) {
        return;
    }
    if (Z_TYPE_P(obj) != IS_OBJECT) {
        hprose_service_add_class_methods(_this, obj, class_name, alias_prefix, (uint8_t)mode, simple, async TSRMLS_CC);        
    }
    else {
        hprose_service_add_instance_methods(_this, obj, class_name, alias_prefix, (uint8_t)mode, simple, async TSRMLS_CC);
    }
}

ZEND_METHOD(hprose_service, addClassMethods) {
    zval *class_name, *exec_class = NULL, *alias_prefix = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    zend_bool async = 0;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!lz!b", &class_name, &exec_class, &alias_prefix, &mode, &simple, &async) == FAILURE) {
        return;
    }
    hprose_service_add_class_methods(_this, class_name, exec_class, alias_prefix, (uint8_t)mode, simple, async TSRMLS_CC);
}

ZEND_METHOD(hprose_service, add) {
    zval *arg1 = NULL, *arg2 = NULL, *arg3 = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!", &arg1, &arg2, &arg3) == FAILURE) {
        return;
    }
    hprose_service_add(_this, ZEND_NUM_ARGS(), arg1, arg2, arg3, 0 TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addAsyncFunction) {
    zval *func, *alias = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!lz!", &func, &alias, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_function(_this, func, alias, (uint8_t)mode, simple, 1 TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addAsyncMissingFunction) {
    zval *func, *alias = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|lz!", &func, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_zval_new(alias);
    ZVAL_STRINGL_1(alias, "*", 1);
    hprose_service_add_function(_this, func, alias, (uint8_t)mode, simple, 1 TSRMLS_CC);
    hprose_zval_free(alias);
}

ZEND_METHOD(hprose_service, addAsyncFunctions) {
    zval *funcs, *aliases = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!lz!", &funcs, &aliases, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_functions(_this, funcs, aliases, (uint8_t)mode, simple, 1 TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addAsyncMethod) {
    zval *methodname, *belongto, *alias = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z!lz!", &methodname, &belongto, &alias, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_method(_this, methodname, belongto, alias, (uint8_t)mode, simple, 1 TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addAsyncMethods) {
    zval *methods, *belongto, *aliases = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz|z!lz!", &methods, &belongto, &aliases, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_methods(_this, methods, belongto, aliases, (uint8_t)mode, simple, 1 TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addAsyncInstanceMethods) {
    zval *obj, *class_name = NULL, *alias_prefix = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!lz!", &obj, &class_name, &alias_prefix, &mode, &simple) == FAILURE) {
        return;
    }
    if (Z_TYPE_P(obj) != IS_OBJECT) {
        hprose_service_add_class_methods(_this, obj, class_name, alias_prefix, (uint8_t)mode, simple, 1 TSRMLS_CC);        
    }
    else {
        hprose_service_add_instance_methods(_this, obj, class_name, alias_prefix, (uint8_t)mode, simple, 1 TSRMLS_CC);
    }
}

ZEND_METHOD(hprose_service, addAsyncClassMethods) {
    zval *class_name, *exec_class = NULL, *alias_prefix = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!lz!", &class_name, &exec_class, &alias_prefix, &mode, &simple) == FAILURE) {
        return;
    }
    hprose_service_add_class_methods(_this, class_name, exec_class, alias_prefix, (uint8_t)mode, simple, 1 TSRMLS_CC);
}

ZEND_METHOD(hprose_service, addAsync) {
    zval *arg1 = NULL, *arg2 = NULL, *arg3 = NULL, *simple = NULL;
    long mode = HPROSE_RESULT_MODE_NORMAL;
    HPROSE_THIS(service);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z!z!", &arg1, &arg2, &arg3) == FAILURE) {
        return;
    }
    hprose_service_add(_this, ZEND_NUM_ARGS(), arg1, arg2, arg3, 1 TSRMLS_CC);
}

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
    ZEND_ARG_INFO(0, async)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_missing_function_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, func)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
    ZEND_ARG_INFO(0, async)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_functions_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, funcs, 0)
    ZEND_ARG_ARRAY_INFO(0, aliases, 1)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
    ZEND_ARG_INFO(0, async)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_method_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, methodname)
    ZEND_ARG_INFO(0, belongto)
    ZEND_ARG_INFO(0, alias)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
    ZEND_ARG_INFO(0, async)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_methods_arginfo, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, methods, 0)
    ZEND_ARG_INFO(0, belongto)
    ZEND_ARG_INFO(0, aliases)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
    ZEND_ARG_INFO(0, async)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_instance_methods_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, obj)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_INFO(0, alias_prefix)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
    ZEND_ARG_INFO(0, async)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_class_methods_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_INFO(0, exec_class)
    ZEND_ARG_INFO(0, alias_prefix)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
    ZEND_ARG_INFO(0, async)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
    ZEND_ARG_INFO(0, arg3)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_async_function_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, func)
    ZEND_ARG_INFO(0, alias)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_async_missing_function_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, func)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_async_functions_arginfo, 0, 0, 1)
    ZEND_ARG_ARRAY_INFO(0, funcs, 0)
    ZEND_ARG_ARRAY_INFO(0, aliases, 1)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_async_method_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, methodname)
    ZEND_ARG_INFO(0, belongto)
    ZEND_ARG_INFO(0, alias)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_async_methods_arginfo, 0, 0, 2)
    ZEND_ARG_ARRAY_INFO(0, methods, 0)
    ZEND_ARG_INFO(0, belongto)
    ZEND_ARG_INFO(0, aliases)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_async_instance_methods_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, obj)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_INFO(0, alias_prefix)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_async_class_methods_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, class_name)
    ZEND_ARG_INFO(0, exec_class)
    ZEND_ARG_INFO(0, alias_prefix)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_add_async_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, arg1)
    ZEND_ARG_INFO(0, arg2)
    ZEND_ARG_INFO(0, arg3)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_service_methods[] = {
    ZEND_ME(hprose_service, getErrorTypeString, hprose_service_get_error_type_string_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
    ZEND_ME(hprose_service, sendError, hprose_service_send_error_arginfo, ZEND_ACC_PUBLIC)
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
    ZEND_ME(hprose_service, add, hprose_service_add_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addAsyncMissingFunction, hprose_service_add_async_missing_function_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addAsyncFunction, hprose_service_add_async_function_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addAsyncFunctions, hprose_service_add_async_functions_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addAsyncMethod, hprose_service_add_async_method_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addAsyncMethods, hprose_service_add_async_methods_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addAsyncInstanceMethods, hprose_service_add_async_instance_methods_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addAsyncClassMethods, hprose_service_add_async_class_methods_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addAsync, hprose_service_add_async_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(service)

HPROSE_OBJECT_FREE_BEGIN(service)
    if (intern->_this) {
        zend_hash_destroy(intern->_this->calls);
        FREE_HASHTABLE(intern->_this->calls);
        zend_llist_destroy(intern->_this->objects);
        efree(intern->_this->objects);
        intern->_this->objects = NULL;
        hprose_zval_free(intern->_this->names);
        hprose_zval_free(intern->_this->filters);
        efree(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_NEW_EX_BEGIN(service)
    intern->_this = emalloc(sizeof(hprose_service));
    intern->_this->simple = 0;
    ALLOC_HASHTABLE(intern->_this->calls);
    zend_hash_init(intern->_this->calls, 0, NULL, hprose_service_remote_call_dtor, 0);
    intern->_this->objects = emalloc(sizeof(zend_llist));
#if PHP_MAJOR_VERSION < 7
    zend_llist_init(intern->_this->objects, sizeof(zval *), (llist_dtor_func_t)hprose_service_object_dtor, 0);
#else
    zend_llist_init(intern->_this->objects, sizeof(zend_object *), (llist_dtor_func_t)hprose_service_object_dtor, 0);
#endif
    hprose_zval_new(intern->_this->names);
    array_init(intern->_this->names);
    hprose_zval_new(intern->_this->filters);
    array_init(intern->_this->filters);
HPROSE_OBJECT_NEW_EX_END(service)

HPROSE_OBJECT_NEW(service)

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

ZEND_METHOD(hprose_async_callback, __construct) {
    zval *completer;
    HPROSE_OBJECT_INTERN(async_callback);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &completer) == FAILURE) {
        RETURN_NULL();
    }
    intern->_this = emalloc(sizeof(hprose_async_callback));
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(completer);
    intern->_this->completer = completer;
#else
    hprose_zval_new(intern->_this->completer);
    ZVAL_COPY(intern->_this->completer, completer);
#endif
}

ZEND_METHOD(hprose_async_callback, handler) {
    zval *result;
    HPROSE_THIS(async_callback);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &result) == FAILURE) {
        RETURN_NULL();
    }
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(result);
#else
    Z_TRY_ADDREF_P(result);
#endif
    if (Z_TYPE_P(result) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(result), zend_exception_ce TSRMLS_CC)) {
        hprose_completer_complete_error(HPROSE_GET_OBJECT_P(completer, _this->completer)->_this, result TSRMLS_CC);
    }
    else {
        hprose_completer_complete(HPROSE_GET_OBJECT_P(completer, _this->completer)->_this, result TSRMLS_CC);
    }
}

ZEND_BEGIN_ARG_INFO_EX(hprose_async_callback_construct_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, completer, HproseCompleter, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_async_callback_handler_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_async_callback_methods[] = {
    ZEND_ME(hprose_async_callback, __construct, hprose_async_callback_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_async_callback, handler, hprose_async_callback_handler_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(async_callback)

HPROSE_OBJECT_FREE_BEGIN(async_callback)
    if (intern->_this) {
        hprose_zval_free(intern->_this->completer);
        efree(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_SIMPLE_NEW(async_callback)

HPROSE_CLASS_ENTRY(async_callback)

HPROSE_STARTUP_FUNCTION(async_callback) {
    HPROSE_REGISTER_CLASS("Hprose", "AsyncCallback", async_callback);
    HPROSE_REGISTER_CLASS_HANDLERS(async_callback);
    return SUCCESS;
}

ZEND_METHOD(hprose_after_invoke_callback, __construct) {
    zval *service;
    zval *completer;
    zval *name;
    zval *args;
    zend_bool byref;
    long mode;
    zend_bool simple;
    zval *context;
    HPROSE_OBJECT_INTERN(after_invoke_callback);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zzzzblbz", &service, &completer, &name, &args, &byref, &mode, &simple, &context) == FAILURE) {
        RETURN_NULL();
    }
    intern->_this = emalloc(sizeof(hprose_after_invoke_callback));
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(service);
    intern->_this->service = service;
    Z_ADDREF_P(completer);
    intern->_this->completer = completer;
    Z_ADDREF_P(name);
    intern->_this->name = name;
    Z_ADDREF_P(args);
    intern->_this->args = args;
    Z_ADDREF_P(context);
    intern->_this->context = context;
#else
    hprose_zval_new(intern->_this->service);
    ZVAL_COPY(intern->_this->service, service);
    hprose_zval_new(intern->_this->completer);
    ZVAL_COPY(intern->_this->completer, completer);
    hprose_zval_new(intern->_this->name);
    ZVAL_COPY(intern->_this->name, name);
    hprose_zval_new(intern->_this->args);
    ZVAL_COPY(intern->_this->args, args);
    hprose_zval_new(intern->_this->context);
    ZVAL_COPY(intern->_this->context, context);
#endif
    intern->_this->byref = byref;
    intern->_this->mode = (uint8_t)mode;
    intern->_this->simple = simple;
}

static zend_always_inline void hprose_after_invoke_callback_handler(hprose_after_invoke_callback *_this, zval *result TSRMLS_DC) {
    hprose_completer *completer = HPROSE_GET_OBJECT_P(completer, _this->completer)->_this;
    zval *return_value;
    zend_bool bailout = 0;
    hprose_bytes_io output;
    hprose_bytes_io_init(&output, NULL, 0);
    hprose_zval_new(return_value);
    hprose_service_after_invoke(_this->service,
                                _this->name,
                                _this->args,
                                _this->byref,
                                _this->mode,
                                _this->simple,
                                _this->context,
                                result,
                                &output,
                                1,
                                return_value,
                                &bailout TSRMLS_CC);
    if (bailout || EG(exception)) {
        hprose_bytes_io_close(&output);
        if (bailout) {
            hprose_zval_free(return_value);
            zend_bailout();
            return;
        }
        hprose_service_try_catch_error(_this->service, _this->context, return_value TSRMLS_CC);
    }
    hprose_completer_complete(completer, return_value TSRMLS_CC);
    hprose_zval_free(return_value);
}

static zend_always_inline void hprose_after_invoke_callback_error_handler(hprose_after_invoke_callback *_this, zval *err TSRMLS_DC) {
    hprose_completer *completer = HPROSE_GET_OBJECT_P(completer, _this->completer)->_this;
    zval *return_value;
    hprose_zval_new(return_value);
    hprose_service_catch_error(_this->service, err, _this->context, return_value TSRMLS_CC);
    hprose_completer_complete(completer, return_value TSRMLS_CC);
    hprose_zval_free(return_value);
}

ZEND_METHOD(hprose_after_invoke_callback, handler) {
    zval *result;
    HPROSE_THIS(after_invoke_callback);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &result) == FAILURE) {
        RETURN_NULL();
    }
    if (Z_TYPE_P(result) == IS_OBJECT &&
        instanceof_function(Z_OBJCE_P(result), zend_exception_ce TSRMLS_CC)) {
        hprose_after_invoke_callback_error_handler(_this, result TSRMLS_CC);
    }
    else {
        hprose_after_invoke_callback_handler(_this, result TSRMLS_CC);
    }
}

ZEND_METHOD(hprose_after_invoke_callback, errorHandler) {
    zval *error;
    HPROSE_THIS(after_invoke_callback);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &error) == FAILURE) {
        RETURN_NULL();
    }
    hprose_after_invoke_callback_error_handler(_this, error TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(hprose_after_invoke_callback_construct_arginfo, 0, 0, 8)
    ZEND_ARG_OBJ_INFO(0, service, HproseService, 0)
    ZEND_ARG_OBJ_INFO(0, completer, HproseCompleter, 0)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_ARRAY_INFO(0, args, 0)
    ZEND_ARG_INFO(0, byref)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, simple)
    ZEND_ARG_INFO(0, context)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_after_invoke_callback_handler_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_after_invoke_callback_error_handler_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, error, Exception, 0)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_after_invoke_callback_methods[] = {
    ZEND_ME(hprose_after_invoke_callback, __construct, hprose_after_invoke_callback_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_after_invoke_callback, handler, hprose_after_invoke_callback_handler_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_after_invoke_callback, errorHandler, hprose_after_invoke_callback_error_handler_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(after_invoke_callback)

HPROSE_OBJECT_FREE_BEGIN(after_invoke_callback)
    if (intern->_this) {
        hprose_zval_free(intern->_this->service);
        hprose_zval_free(intern->_this->completer);
        hprose_zval_free(intern->_this->name);
        hprose_zval_free(intern->_this->args);
        hprose_zval_free(intern->_this->context);
        efree(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_SIMPLE_NEW(after_invoke_callback)

HPROSE_CLASS_ENTRY(after_invoke_callback)

HPROSE_STARTUP_FUNCTION(after_invoke_callback) {
    HPROSE_REGISTER_CLASS("Hprose", "AfterInvokeCallback", after_invoke_callback);
    HPROSE_REGISTER_CLASS_HANDLERS(after_invoke_callback);
    return SUCCESS;
}

