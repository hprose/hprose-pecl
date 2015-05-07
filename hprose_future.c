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
 * hprose_future.c                                        *
 *                                                        *
 * hprose future for pecl source file.                    *
 *                                                        *
 * LastModified: May 7, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_future.h"

ZEND_METHOD(hprose_completer, __construct) {
    HPROSE_OBJECT_INTERN(completer);
    intern->_this = hprose_completer_new();
}

ZEND_METHOD(hprose_completer, complete) {
    zval *result;
    HPROSE_THIS(completer);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &result) == FAILURE) {
        RETURN_NULL();
    }
    hprose_completer_complete(_this, result TSRMLS_CC);
}

ZEND_METHOD(hprose_completer, completeError) {
    zval *error;
    HPROSE_THIS(completer);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &error) == FAILURE) {
        RETURN_NULL();
    }
    hprose_completer_complete_error(_this, error TSRMLS_CC);
}

ZEND_METHOD(hprose_completer, future) {
    HPROSE_THIS(completer);
    hprose_completer_future(_this, return_value TSRMLS_CC);
}

ZEND_BEGIN_ARG_INFO_EX(hprose_completer_construct_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_completer_complete_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, result)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_completer_complete_error_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, error, Exception, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_completer_future_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_completer_methods[] = {
    ZEND_ME(hprose_completer, __construct, hprose_completer_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_completer, complete, hprose_completer_complete_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_completer, completeError, hprose_completer_complete_error_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_completer, future, hprose_completer_future_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(completer)

HPROSE_OBJECT_FREE_BEGIN(completer)
    if (intern->_this) {
        hprose_completer_free(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_SIMPLE_NEW(completer)

HPROSE_CLASS_ENTRY(completer)

HPROSE_STARTUP_FUNCTION(completer) {
    HPROSE_REGISTER_CLASS("Hprose", "Completer", completer);
    HPROSE_REGISTER_CLASS_HANDLERS(completer);
    return SUCCESS;
}

ZEND_BEGIN_ARG_INFO_EX(hprose_future_then_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_future_catch_error_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, errorHandler)
ZEND_END_ARG_INFO()

ZEND_METHOD(hprose_future, then) {
    zval *handler;
    HPROSE_THIS(future);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &handler) == FAILURE) {
        RETURN_NULL();
    }
    if (is_callable_p(handler)) {
        hprose_future_then(_this, handler TSRMLS_CC);
        RETURN_ZVAL(getThis(), 1, 0);
    }
}

ZEND_METHOD(hprose_future, catchError) {
    zval *errorHandler;
    HPROSE_THIS(future);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &errorHandler) == FAILURE) {
        RETURN_NULL();
    }
    if (is_callable_p(errorHandler)) {
        hprose_future_catch_error(_this, errorHandler TSRMLS_CC);
        RETURN_ZVAL(getThis(), 1, 0);
    }
}

static zend_function_entry hprose_future_methods[] = {
    ZEND_ME(hprose_future, then, hprose_future_then_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_future, catchError, hprose_future_catch_error_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(future)

HPROSE_OBJECT_FREE_BEGIN(future)
    if (intern->_this) {
        hprose_future_free(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_SIMPLE_NEW(future)

HPROSE_CLASS_ENTRY(future)

HPROSE_STARTUP_FUNCTION(future) {
    HPROSE_REGISTER_CLASS("Hprose", "Future", future);
    HPROSE_REGISTER_CLASS_HANDLERS(future);
    return SUCCESS;
}
