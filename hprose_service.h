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
 * hprose_service.h                                       *
 *                                                        *
 * hprose service for pecl header file.                   *
 *                                                        *
 * LastModified: Apr 13, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_SERVICE_H
#define	HPROSE_SERVICE_H

#include "hprose_common.h"
#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"
#include "hprose_result_mode.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_service_ce();

HPROSE_STARTUP_FUNCTION(service);

typedef struct {
    zend_fcall_info_cache fcc;
    uint8_t mode;
    uint8_t simple;
    zend_bool byref;
} hprose_remote_call;

typedef struct {
    HashTable *calls;
    zval *names;
    zval *filters;
    zend_bool simple;
} hprose_service;

HPROSE_CLASS_BEGIN(service)
HPROSE_CLASS_END(service)

static zend_always_inline zend_bool is_magic_method(char *name, int32_t len) {
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

static zend_always_inline char * get_error_type_string(long e) {
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

static zend_always_inline void hprose_service_send_error(zval *service, zval *err, zval *context, zval *return_value TSRMLS_DC) {
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

static zend_always_inline void hprose_service_do_invoke(zval *service, hprose_bytes_io *input, zval *context, zval *return_value TSRMLS_DC) {
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
        hprose_service_on_before_invoke(service, _name, args, byref, context TSRMLS_CC);
        if (EG(exception)) {
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
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
        __function_invoke_args(call->fcc, NULL, result, args TSRMLS_CC);
        if (EG(exception)) {
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            hprose_zval_free(result);
            return;
        }
        hprose_service_on_after_invoke(service, _name, args, byref, result, context TSRMLS_CC);
        if (EG(exception)) {
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            hprose_zval_free(result);
            return;
        }
        if (call->mode == HPROSE_RESULT_MODE_RAW_WITH_END_TAG) {
            convert_to_string(result);
            RETVAL_ZVAL(result, 0, 1);
            hprose_service_output_filter(_this, return_value, context TSRMLS_CC);
            hprose_reader_destroy(&reader);
            hprose_bytes_io_close(&output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            return;
        }
        else if (call->mode == HPROSE_RESULT_MODE_RAW) {
            convert_to_string(result);
            hprose_bytes_io_write(&output, Z_STRVAL_P(result), Z_STRLEN_P(result));
        }
        else {
            hprose_writer writer;
            hprose_writer_init(&writer, &output, simple);
            hprose_bytes_io_putc(&output, HPROSE_TAG_RESULT);
            if (call->mode == HPROSE_RESULT_MODE_SERIALIZED) {
                convert_to_string(result);
                hprose_bytes_io_write(&output, Z_STRVAL_P(result), Z_STRLEN_P(result));
            }
            else {
                hprose_writer_reset(&writer);
                hprose_writer_serialize(&writer, result TSRMLS_CC);
                hprose_zval_free(result);
            }
            if (byref) {
                hprose_bytes_io_putc(&output, HPROSE_TAG_ARGUMENT);
                hprose_writer_reset(&writer);
                hprose_writer_write_array(&writer, args TSRMLS_CC);
            }
            hprose_writer_destroy(&writer);
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

static zend_always_inline void hprose_service_do_function_list(zval *service, zval *context, zval *return_value TSRMLS_DC) {
    hprose_service *_this = HPROSE_GET_OBJECT_P(service, service)->_this;
    hprose_bytes_io output;
    hprose_writer writer;
    hprose_bytes_io_init(&output, NULL, 0);
    hprose_writer_init(&writer, &output, 1);
    hprose_bytes_io_putc(&output, HPROSE_TAG_FUNCTIONS);
    hprose_writer_write_array(&writer, _this->names TSRMLS_CC);
    hprose_bytes_io_putc(&output, HPROSE_TAG_END);
    hprose_writer_destroy(&writer);
#if PHP_MAJOR_VERSION < 7
    RETVAL_STRINGL_0(HB_BUF(output), HB_LEN(output));
#else
    RETVAL_STR(HB_STR(output));
#endif
    hprose_service_output_filter(_this, return_value, context TSRMLS_CC);
}

static zend_always_inline void hprose_service_catch_error(zval *service, zval *err, zval *context, zval *return_value TSRMLS_DC) {
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
END_EXTERN_C()

#endif	/* HPROSE_SERVICE_H */

