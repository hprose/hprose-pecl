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
    zval *callback;
    zval *errorCallback;
    zval *results;
    zval *errors;
    int ref_count;
} hprose_future;

typedef struct {
    hprose_future *future;
} hprose_completer;

static zend_always_inline hprose_completer *hprose_completer_new() {
    hprose_completer *_this = emalloc(sizeof(hprose_completer));
    hprose_future *future = emalloc(sizeof(hprose_future));
    future->callback = NULL;
    future->errorCallback = NULL;
    hprose_zval_new(future->results);
    array_init(future->results);
    hprose_zval_new(future->errors);
    array_init(future->errors);
    future->ref_count = 1;
    _this->future = future;
    return _this;
}

static zend_always_inline void hprose_completer_complete(hprose_completer *_this, zval *result) {
    if (_this->future->callback != NULL) {
        callable_invoke(_this->future->callback, NULL, "z", result);
    }
    else {
        Z_ADDREF_P(result);
        add_next_index_zval(_this->future->results, result);
    }
}

static zend_always_inline void hprose_completer_complete_error(hprose_completer *_this, zval *error) {
    if (_this->future->errorCallback != NULL) {
        callable_invoke(_this->future->errorCallback, NULL, "z", error);
    }
    else {
        Z_ADDREF_P(error);
        add_next_index_zval(_this->future->errors, error);
    }
}

static zend_always_inline hprose_future *hprose_completer_future(hprose_completer *_this) {
    _this->future->ref_count++;
    return _this->future;
}

static zend_always_inline hprose_future *hprose_future_then(hprose_future *_this, zval *handler) {
    int i, count;
    HashTable *ht = Z_ARRVAL_P(_this->results);
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(handler);
    _this->callback = handler;
#else
    hprose_zval_new(_this->callback);
    ZVAL_COPY(_this->callback, handler);
#endif
    zend_hash_internal_pointer_reset(ht);
    count = Z_ARRLEN_P(_this->results);
    for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
        zval **result;
        zend_hash_get_current_data(ht, (void **)&result);
        callable_invoke(handler, NULL, "z", *result);
#else
        zval *result = zend_hash_get_current_data(ht);
        callable_invoke(handler, NULL, "z", result);
#endif
        if (EG(exception)) break;
        zend_hash_move_forward(ht);
    }
    zend_hash_clean(ht);
    return _this;
}

static zend_always_inline hprose_future *hprose_future_catch_error(hprose_future *_this, zval *error_handler) {
    int i, count;
    HashTable *ht = Z_ARRVAL_P(_this->errors);
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(error_handler);
    _this->errorCallback = error_handler;
#else
    hprose_zval_new(_this->errorCallback);
    ZVAL_COPY(_this->errorCallback, error_handler);
#endif
    zend_hash_internal_pointer_reset(ht);
    count = Z_ARRLEN_P(_this->errors);
    for (i = 0; i < count; ++i) {
#if PHP_MAJOR_VERSION < 7
        zval **error;
        zend_hash_get_current_data(ht, (void **)&error);
        callable_invoke(error_handler, NULL, "z", *error);
#else
        zval *error = zend_hash_get_current_data(ht);
        callable_invoke(error_handler, NULL, "z", error);
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
        if (_this->callback != NULL) {
            hprose_zval_free(_this->callback);
            _this->callback = NULL;
        }
        if (_this->errorCallback != NULL) {
            hprose_zval_free(_this->errorCallback);
            _this->errorCallback = NULL;
        }
        hprose_zval_free(_this->results);
        hprose_zval_free(_this->errors);
        efree(_this);
    }
}

static zend_always_inline void hprose_completer_free(hprose_completer *_this) {
    hprose_future_free(_this->future);
    efree(_this);
}

HPROSE_CLASS_BEGIN(completer)
HPROSE_CLASS_END(completer)

HPROSE_CLASS_BEGIN(future)
HPROSE_CLASS_END(future)

END_EXTERN_C()

#endif	/* HPROSE_FUTURE_H */
