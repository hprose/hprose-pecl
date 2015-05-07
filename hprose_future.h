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
 * hprose_future.h                                        *
 *                                                        *
 * hprose future for pecl header file.                    *
 *                                                        *
 * LastModified: May 3, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_FUTURE_H
#define	HPROSE_FUTURE_H

#include "hprose_common.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_future_ce();
zend_class_entry *get_hprose_completer_ce();

HPROSE_STARTUP_FUNCTION(future);

HPROSE_STARTUP_FUNCTION(completer);

typedef struct {
    zval *results;
    zval *callbacks;
    zval *errors;
    zval *onerror;
    int ref_count;
} hprose_future;

typedef struct {
    hprose_future *future;
} hprose_completer;

static zend_always_inline hprose_completer *hprose_completer_new() {
    hprose_completer *_this = emalloc(sizeof(hprose_completer));
    hprose_future *future = emalloc(sizeof(hprose_future));
    hprose_zval_new(future->results);
    array_init(future->results);
    hprose_zval_new(future->callbacks);
    array_init(future->callbacks);
    hprose_zval_new(future->errors);
    array_init(future->errors);
    future->onerror = NULL;
    future->ref_count = 1;
    _this->future = future;
    return _this;
}

static zend_always_inline void hprose_future_complete_error(hprose_future *_this, zval *error TSRMLS_DC) {
    if (_this->onerror != NULL) {
        callable_invoke(_this->onerror, NULL, "z", error);
    }
    else {
#if PHP_MAJOR_VERSION < 7
        Z_ADDREF_P(error);
#else
        Z_TRY_ADDREF_P(error);
#endif
        add_next_index_zval(_this->errors, error);
    }
}

static zend_always_inline void hprose_completer_complete_error(hprose_completer *_this, zval *error TSRMLS_DC) {
    hprose_future_complete_error(_this->future, error TSRMLS_CC);
}

static zend_always_inline void hprose_completer_complete(hprose_completer *_this, zval *result TSRMLS_DC) {
    int i, count;
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(result);
#else
    Z_TRY_ADDREF_P(result);
#endif
    count = Z_ARRLEN_P(_this->future->callbacks);
    if (count > 0) {
        HashTable *ht = Z_ARRVAL_P(_this->future->callbacks);
        zend_hash_internal_pointer_reset(ht);
        for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
            zval **callback;
            zval *return_value;
            hprose_zval_new(return_value);
            ZVAL_NULL(return_value);
            zend_hash_get_current_data(ht, (void **)&callback);
            if (result && Z_TYPE_P(result) == IS_OBJECT &&
                instanceof_function(Z_OBJCE_P(result),
                    get_hprose_future_ce() TSRMLS_CC)) {
                method_invoke(result, then, return_value, "z", *callback);
            }
            else {
                callable_invoke(*callback, return_value, "z", result);
            }
#else
            zval *callback = zend_hash_get_current_data(ht);
            if (result && Z_TYPE_P(result) == IS_OBJECT &&
                instanceof_function(Z_OBJCE_P(result),
                    get_hprose_future_ce() TSRMLS_CC)) {
                method_invoke(result, then, return_value, "z", callback);
            }
            else {
                callable_invoke(callback, return_value, "z", result);
            }
#endif
            if (EG(exception)) {
#if PHP_MAJOR_VERSION < 7
                zval *err = EG(exception);
                Z_ADDREF_P(err);
                SEPARATE_ZVAL(&err);
                zend_clear_exception(TSRMLS_C);
                hprose_completer_complete_error(_this, err TSRMLS_CC);
#else
                zval err;
                ZVAL_OBJ(&err, EG(exception));
                Z_ADDREF(err);
                SEPARATE_ZVAL(&err);
                zend_clear_exception();
                hprose_completer_complete_error(_this, &err TSRMLS_CC);
#endif
                zval_ptr_dtor(&err);
            }
            hprose_zval_free(result);
            result = return_value;
            zend_hash_move_forward(ht);
        }
        zend_hash_clean(ht);
    }
    add_index_zval(_this->future->results, 0, result);
}

static zend_always_inline hprose_future *hprose_completer_future(hprose_completer *_this) {
    _this->future->ref_count++;
    return _this->future;
}

static zend_always_inline hprose_future *hprose_future_then(hprose_future *_this, zval *callback TSRMLS_DC) {
    int i, count;
    count = Z_ARRLEN_P(_this->results);
    if (count > 0) {
        zval *result = php_array_get(_this->results, 0);
#if PHP_MAJOR_VERSION < 7
        Z_ADDREF_P(result);
#else
        Z_TRY_ADDREF_P(result);
#endif
        zval *return_value;
        hprose_zval_new(return_value);
        ZVAL_NULL(return_value);
        if (result && Z_TYPE_P(result) == IS_OBJECT &&
            instanceof_function(Z_OBJCE_P(result),
                get_hprose_future_ce() TSRMLS_CC)) {
            method_invoke(result, then, return_value, "z", callback);
        }
        else {
            callable_invoke(callback, return_value, "z", result);
        }
#if PHP_MAJOR_VERSION < 7
        Z_ADDREF_P(return_value);
#else
        Z_TRY_ADDREF_P(return_value);
#endif
        add_index_zval(_this->results, 0, return_value);
        hprose_zval_free(return_value);
        if (EG(exception)) {
#if PHP_MAJOR_VERSION < 7
            zval *err = EG(exception);
            Z_ADDREF_P(err);
            SEPARATE_ZVAL(&err);
            zend_clear_exception(TSRMLS_C);
            hprose_future_complete_error(_this, err TSRMLS_CC);
#else
            zval err;
            ZVAL_OBJ(&err, EG(exception));
            Z_ADDREF(err);
            SEPARATE_ZVAL(&err);
            zend_clear_exception();
            hprose_future_complete_error(_this, &err TSRMLS_CC);
#endif
            zval_ptr_dtor(&err);
        }
    }
    else {
        Z_ADDREF_P(callback);
        add_next_index_zval(_this->callbacks, callback);
    }
    return _this;
}

static zend_always_inline hprose_future *hprose_future_catch_error(hprose_future *_this, zval *onerror TSRMLS_DC) {
    int i, count;
    HashTable *ht = Z_ARRVAL_P(_this->errors);
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(onerror);
    _this->onerror = onerror;
#else
    hprose_zval_new(_this->onerror);
    ZVAL_COPY(_this->onerror, onerror);
#endif
    zend_hash_internal_pointer_reset(ht);
    count = Z_ARRLEN_P(_this->errors);
    for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
        zval **error;
        zend_hash_get_current_data(ht, (void **)&error);
        callable_invoke(onerror, NULL, "z", *error);
#else
        zval *error = zend_hash_get_current_data(ht);
        callable_invoke(onerror, NULL, "z", error);
#endif
        if (EG(exception)) break;
        zend_hash_move_forward(ht);
    }
    zend_hash_clean(ht);
    return _this;
}

static zend_always_inline void hprose_future_free(hprose_future *_this) {
    _this->ref_count--;
    if (_this->ref_count <= 0) {
        hprose_zval_free(_this->results);
        _this->results = NULL;
        hprose_zval_free(_this->callbacks);
        _this->callbacks = NULL;
        hprose_zval_free(_this->errors);
        _this->errors = NULL;
        if (_this->onerror != NULL) {
            hprose_zval_free(_this->onerror);
            _this->onerror = NULL;
        }
        efree(_this);
    }
}

static zend_always_inline void hprose_completer_free(hprose_completer *_this) {
    hprose_future_free(_this->future);
    _this->future = NULL;
    efree(_this);
}

HPROSE_CLASS_BEGIN(completer)
HPROSE_CLASS_END(completer)

HPROSE_CLASS_BEGIN(future)
HPROSE_CLASS_END(future)

END_EXTERN_C()

#endif	/* HPROSE_FUTURE_H */
