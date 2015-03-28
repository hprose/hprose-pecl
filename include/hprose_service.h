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
 * LastModified: Mar 28, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_SERVICE_H
#define	HPROSE_SERVICE_H

#include "hprose.h"

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

static zend_always_inline void hprose_service_input_filter(hprose_service *_this, zval *data, zval *context TSRMLS_DC) {
    HashTable *ht = Z_ARRVAL_P(_this->filters);
    int32_t i = zend_hash_num_elements(ht);
    if (i) {
        zend_hash_internal_pointer_end(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **filter;
            zend_hash_get_current_data(ht, (void **)&filter);
            method_invoke(*filter, inputFilter, data, "zz", data, context);
#else
            zval *filter = zend_hash_get_current_data(ht);
            method_invoke(filter, inputFilter, data, "zz", data, context);
#endif
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
            method_invoke(*filter, outputFilter, data, "zz", data, context);
#else
            zval *filter = zend_hash_get_current_data(ht);
            method_invoke(filter, outputFilter, data, "zz", data, context);
#endif
            zend_hash_move_forward(ht);
        }
    }
}

static zend_always_inline void hprose_service_on_send_error(zval *service, zval *err, zval *context TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    zval *on_send_error = zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onSendError"), 1 TSRMLS_CC);
#if PHP_API_VERSION < 20090626
    if (on_send_error && zend_is_callable(on_send_error, 0, NULL)) {
#else
    if (on_send_error && zend_is_callable(on_send_error, 0, NULL TSRMLS_CC)) {
#endif
        callable_invoke(on_send_error, NULL, "zz", err, context);
    }
#else
    zval on_send_error;
    zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onSendError"), 1, &on_send_error);
    if (zend_is_callable(&on_send_error, 0, NULL)) {
        callable_invoke(&on_send_error, NULL, "zz", err, context);
    }
#endif
}

static zend_always_inline void hprose_service_on_before_invoke(zval *service, zval *name, zval *args, zend_bool byref, zval *context TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    zval *on_before_invoke = zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onBeforeInvoke"), 1 TSRMLS_CC);
#if PHP_API_VERSION < 20090626
    if (on_before_invoke && zend_is_callable(on_before_invoke, 0, NULL)) {
#else
    if (on_before_invoke && zend_is_callable(on_before_invoke, 0, NULL TSRMLS_CC)) {
#endif
        callable_invoke(on_before_invoke, NULL, "zzbz", name, args, byref, context);
    }
#else
    zval on_before_invoke;
    zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onBeforeInvoke"), 1, &on_before_invoke);
    if (zend_is_callable(&on_before_invoke, 0, NULL)) {
        callable_invoke(&on_before_invoke, NULL, "zzbz", name, args, byref, context);
    }
#endif
}

static zend_always_inline void hprose_service_on_after_invoke(zval *service, zval *name, zval *args, zend_bool byref, zval *result, zval *context TSRMLS_DC) {
#if PHP_MAJOR_VERSION < 7
    zval *on_after_invoke = zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onAfterInvoke"), 1 TSRMLS_CC);
#if PHP_API_VERSION < 20090626
    if (on_after_invoke && zend_is_callable(on_after_invoke, 0, NULL)) {
#else
    if (on_after_invoke && zend_is_callable(on_after_invoke, 0, NULL TSRMLS_CC)) {
#endif
        callable_invoke(on_after_invoke, NULL, "zzbzz", name, args, byref, result, context);
    }
#else
    zval on_after_invoke;
    zend_read_property(get_hprose_service_ce(), service, ZEND_STRL("onAfterInvoke"), 1, &on_after_invoke);
    if (zend_is_callable(&on_after_invoke, 0, NULL)) {
        callable_invoke(&on_after_invoke, NULL, "zzbzz", name, args, byref, result, context);
    }
#endif
}

static zend_always_inline void hprose_service_send_error(zval *service, zval *err, zval *context, zval *return_value TSRMLS_DC) {
    hprose_bytes_io *stream = hprose_bytes_io_new();
    hprose_writer *writer = hprose_writer_create(stream, 1);
    hprose_service_on_send_error(service, err, context TSRMLS_CC);
    hprose_bytes_io_putc(stream, HPROSE_TAG_ERROR);
    hprose_writer_write_string(writer, err);
    hprose_bytes_io_putc(stream, HPROSE_TAG_END);
    hprose_writer_free(writer);
    ZVAL_STRINGL_0(return_value, stream->buf, stream->len);
    efree(stream);
    hprose_service_output_filter(HPROSE_GET_OBJECT_P(service, service)->_this, return_value, context TSRMLS_CC);
}

static zend_always_inline void hprose_service_do_invoke(zval *service, hprose_bytes_io *input, zval *context, zval *return_value TSRMLS_DC) {
    hprose_service *_this = HPROSE_GET_OBJECT_P(service, service)->_this;
    hprose_bytes_io *output = hprose_bytes_io_new();
    hprose_reader *reader = hprose_reader_create(input, 0);
    char tag;
    do {
        zval *_name, *args = NULL, *result = NULL;
        char *name, *alias;
        int32_t nlen;
        hprose_remote_call *call;
        zend_bool simple = _this->simple;
        zend_bool byref = 0;
        hprose_make_zval(_name);
        hprose_reader_reset(reader);
        hprose_reader_read_string(reader, _name TSRMLS_CC);
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
                hprose_reader_free(reader);
                hprose_bytes_io_free(output);
                hprose_zval_free(_name);
                return;
            }
        }
        if (call->simple < 2) {
            simple = (zend_bool)call->simple;
        }
        tag = hprose_bytes_io_getc(input);
        if (tag == HPROSE_TAG_LIST) {
            hprose_make_zval(args);
            hprose_reader_reset(reader);
            hprose_reader_read_list_without_tag(reader, args TSRMLS_CC);
            tag = hprose_bytes_io_getc(input);
            if (tag == HPROSE_TAG_TRUE) {
                byref = 1;
                tag = hprose_bytes_io_getc(input);
            }
            if (call->byref) {
                int32_t i, n = MIN(Z_ARRLEN_P(args), call->fcc.function_handler->common.num_args);
                zval *_args;
                hprose_make_zval(_args);
                array_init_size(_args, n);
                for (i = 0; i < n; ++i) {
                    zval *e = php_array_get(args, i);
#if PHP_MAJOR_VERSION < 7
                    Z_ADDREF_P(e);
#else
                    Z_TRY_ADDREF_P(e);
#endif
                    if (call->fcc.function_handler->common.arg_info[i].pass_by_reference) {
                        Z_SET_ISREF_P(e);
                    }
                    add_next_index_zval(_args, e);
                }
                hprose_zval_free(args);
                args = _args;
            }
        }
        if ((tag != HPROSE_TAG_END) && (tag != HPROSE_TAG_CALL)) {
            zend_throw_exception_ex(NULL, 0 TSRMLS_CC,
                    "Unknown tag: %c\r\nwith following data: %s",
                    tag, hprose_bytes_io_to_string(input));
            hprose_reader_free(reader);
            hprose_bytes_io_free(output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            return;
        }
        hprose_service_on_before_invoke(service, _name, args, byref, context TSRMLS_CC);
        if (EG(exception)) {
            hprose_reader_free(reader);
            hprose_bytes_io_free(output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            return;
        }
        if (zend_hash_str_find_ptr(_this->calls, ZEND_STRL("*")) == call) {
            zval *_args;
            hprose_make_zval(_args);
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
        hprose_make_zval(result);
        __function_invoke_args(call->fcc, NULL, result, args TSRMLS_CC);
        if (EG(exception)) {
            hprose_reader_free(reader);
            hprose_bytes_io_free(output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            hprose_zval_free(result);
            return;
        }
        hprose_service_on_after_invoke(service, _name, args, byref, result, context TSRMLS_CC);
        if (EG(exception)) {
            hprose_reader_free(reader);
            hprose_bytes_io_free(output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            hprose_zval_free(result);
            return;
        }
        if (call->mode == HPROSE_RESULT_MODE_RAW_WITH_END_TAG) {
            convert_to_string(result);
            RETVAL_ZVAL(result, 1, 1);
            hprose_service_output_filter(_this, return_value, context TSRMLS_CC);
            hprose_reader_free(reader);
            hprose_bytes_io_free(output);
            hprose_zval_free(_name);
            hprose_zval_free(args);
            return;
        }
        else if (call->mode == HPROSE_RESULT_MODE_RAW) {
            convert_to_string(result);
            hprose_bytes_io_write(output, Z_STRVAL_P(result), Z_STRLEN_P(result));
        }
        else {
            hprose_writer *writer = hprose_writer_create(output, simple);
            hprose_bytes_io_putc(output, HPROSE_TAG_RESULT);
            if (call->mode == HPROSE_RESULT_MODE_SERIALIZED) {
                convert_to_string(result);
                hprose_bytes_io_write(output, Z_STRVAL_P(result), Z_STRLEN_P(result));
            }
            else {
                hprose_writer_reset(writer);
                hprose_writer_serialize(writer, result TSRMLS_CC);
                hprose_zval_free(result);
            }
            if (byref) {
                hprose_bytes_io_putc(output, HPROSE_TAG_ARGUMENT);
                hprose_writer_reset(writer);
                hprose_writer_write_array(writer, args TSRMLS_CC);
            }
            hprose_writer_free(writer);
        }
        hprose_zval_free(_name);
        hprose_zval_free(args);
    } while (tag == HPROSE_TAG_CALL);
    hprose_reader_free(reader);
    hprose_bytes_io_putc(output, HPROSE_TAG_END);
    ZVAL_STRINGL_0(return_value, output->buf, output->len);
    efree(output);
    hprose_service_output_filter(HPROSE_GET_OBJECT_P(service, service)->_this, return_value, context TSRMLS_CC);
}

END_EXTERN_C()

#endif	/* HPROSE_SERVICE_H */

