
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
 * LastModified: Mar 28, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"
#include "hprose_result_mode.h"
#include "hprose_service.h"


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

#if PHP_MAJOR_VERSION < 7
static void hprose_service_remote_call_dtor(void *pDest) {
    hprose_remote_call **rc = (hprose_remote_call **)pDest;
    efree(*rc);
}
#else
static void hprose_service_remote_call_dtor(zval *pDest) {
    hprose_remote_call *rc = (hprose_remote_call *)Z_PTR_P(pDest);
    efree(rc);
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
    hprose_make_zval(intern->_this->names);
    array_init(intern->_this->names);
    hprose_make_zval(intern->_this->filters);
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

ZEND_METHOD(hprose_service, getFilter) {
    HPROSE_THIS(service);
    if (zend_hash_num_elements(Z_ARRVAL_P(_this->filters))) {
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
    add_index_zval(_this->filters, 0, filter);
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

ZEND_BEGIN_ARG_INFO_EX(hprose_service_construct_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, url)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_service_void_arginfo, 0, 0, 0)
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

static zend_function_entry hprose_service_methods[] = {
    ZEND_ME(hprose_service, __construct, hprose_service_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_service, __destruct, hprose_service_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_ME(hprose_service, getFilter, hprose_service_get_filter_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, setFilter, hprose_service_set_filter_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, addFilter, hprose_service_add_filter_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, removeFilter, hprose_service_remove_filter_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, getSimple, hprose_service_get_simple_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_service, setSimple, hprose_service_set_simple_arginfo, ZEND_ACC_PUBLIC)
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
