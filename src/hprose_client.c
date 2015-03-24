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
 * hprose_client.c                                        *
 *                                                        *
 * hprose client for pecl source file.                    *
 *                                                        *
 * LastModified: Mar 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_tags.h"
#include "hprose_bytes_io.h"
#include "hprose_writer.h"
#include "hprose_reader.h"
#include "hprose_result_mode.h"
#include "hprose_client.h"

static zend_always_inline void hprose_client_sync_invoke(hprose_client *_this, char *name, int32_t len, zval *args, zend_bool byref, int mode, zend_bool simple, zval *return_value) {

}

static zend_always_inline void hprose_client_async_invoke(hprose_client *_this, char *name, int32_t len, zval *args, zend_bool byref, int mode, zend_bool simple, zval *callback) {
    
}

ZEND_METHOD(hprose_proxy, __construct) {
    zval *client = NULL;
    zend_class_entry *ce = NULL;
    char *ns = "";
    length_t len = 0;
    HPROSE_OBJECT_INTERN(proxy);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|s", &client, &ce, &ns, &len) == FAILURE) {
        return;
    }
    if (!client || !instanceof(ce, HproseClient)) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C),
                             "client must be an instance of HproseClient.", 0 TSRMLS_CC);
        return;
    }
    intern->_this = emalloc(sizeof(hprose_proxy));
#if PHP_MAJOR_VERSION < 7
    Z_ADDREF_P(client);
#else
    Z_TRY_ADDREF_P(client);
#endif
    intern->_this->client = client;
    intern->_this->ns = ns;
}

ZEND_METHOD(hprose_proxy, __destruct) {
    HPROSE_OBJECT_INTERN(proxy);
    if (intern->_this) {
#if PHP_MAJOR_VERSION < 7
    zval_ptr_dtor(&(intern->_this->client));
#else
    zval_ptr_dtor(intern->_this->client);
#endif
        efree(intern->_this);
        intern->_this = NULL;
    }
}

ZEND_METHOD(hprose_proxy, __call) {
    char *name;
    length_t len;
    zval *args;
    int32_t n;
    hprose_bytes_io *_name = hprose_bytes_io_new();
    HPROSE_THIS(proxy);
    hprose_client *client = HPROSE_GET_OBJECT_P(client, _this->client)->_this;
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sa", &name, &len, &args) == FAILURE) {
        return;
    }
    hprose_bytes_io_write(_name, _this->ns, strlen(_this->ns));
    hprose_bytes_io_write(_name, name, len);
    n = zend_hash_num_elements(Z_ARRVAL_P(args));
    if (n > 0) {
        zval *callback = php_array_get(args, n - 1);
#if PHP_API_VERSION < 20090626
        if (zend_is_callable(callback, 0, NULL)) {
#else
        if (zend_is_callable(callback, 0, NULL TSRMLS_CC)) {
#endif
            zend_hash_index_del(Z_ARRVAL_P(args), n - 1);
            hprose_client_async_invoke(client, _name->buf, _name->len, args, 0, HPROSE_RESULT_MODE_NORMAL, 0, callback);
            hprose_bytes_io_free(_name);
            return;
        }
    }
    hprose_client_sync_invoke(client, _name->buf, _name->len, args, 0, HPROSE_RESULT_MODE_NORMAL, 0, return_value);
    hprose_bytes_io_free(_name);
}

ZEND_METHOD(hprose_proxy, __get) {
    hprose_bytes_io *_name = hprose_bytes_io_new();
    char *name;
    length_t len;
    HPROSE_THIS(proxy);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &name, &len) == FAILURE) {
        return;
    }
    hprose_bytes_io_write(_name, _this->ns, strlen(_this->ns));
    hprose_bytes_io_write(_name, name, len);
    hprose_bytes_io_write_char(_name, '-');
    create_php_object(HproseProxy, return_value, "zs", _this->client, _name->buf, (long)_name->len);
    hprose_bytes_io_free(_name);    
}

ZEND_BEGIN_ARG_INFO_EX(hprose_proxy_construct_arginfo, 0, 0, 1)
    ZEND_ARG_OBJ_INFO(0, client, HproseClient, 0)
    ZEND_ARG_INFO(0, ns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_proxy_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_proxy_call_arginfo, 0, 0, 2)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_ARRAY_INFO(0, args, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_proxy_get_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()


static zend_function_entry hprose_proxy_methods[] = {
    ZEND_ME(hprose_proxy, __construct, hprose_proxy_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_proxy, __destruct, hprose_proxy_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_ME(hprose_proxy, __call, hprose_proxy_call_arginfo, ZEND_ACC_PUBLIC)
    ZEND_ME(hprose_proxy, __get, hprose_proxy_get_arginfo, ZEND_ACC_PUBLIC)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(proxy)

HPROSE_OBJECT_FREE_BEGIN(proxy)
    if (intern->_this) {
        efree(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_NEW_BEGIN(proxy)
HPROSE_OBJECT_NEW_END(proxy)

HPROSE_CLASS_ENTRY(proxy)

HPROSE_STARTUP_FUNCTION(proxy) {
    HPROSE_REGISTER_CLASS("Hprose", "Proxy", proxy);
    HPROSE_REGISTER_CLASS_HANDLERS(proxy);
    return SUCCESS;
}

ZEND_METHOD(hprose_client, __construct) {
    char *url = "";
    length_t len = 0;
    HPROSE_OBJECT_INTERN(client);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &url, &len) == FAILURE) {
        return;
    }
    intern->_this = emalloc(sizeof(hprose_client));
    intern->_this->client = getThis();
    intern->_this->ns = "";
    intern->_this->simple = 0;
    hprose_make_zval(intern->_this->filters);
    array_init(intern->_this->filters);
    
    zend_update_property_stringl(get_hprose_client_ce(), getThis(), STR_ARG("url"), url, len TSRMLS_CC);    
}

ZEND_METHOD(hprose_client, __destruct) {
    HPROSE_OBJECT_INTERN(client);
    if (intern->_this) {
        hprose_zval_free(intern->_this->filters);
        efree(intern->_this);
        intern->_this = NULL;
    }
}

ZEND_BEGIN_ARG_INFO_EX(hprose_client_construct_arginfo, 0, 0, 0)
    ZEND_ARG_INFO(0, url)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_client_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_client_methods[] = {
    ZEND_ME(hprose_client, __construct, hprose_client_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_client, __destruct, hprose_client_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
    ZEND_FE_END
};

HPROSE_OBJECT_HANDLERS(client)

HPROSE_OBJECT_FREE_BEGIN(client)
    if (intern->_this) {
        efree(intern->_this);
        intern->_this = NULL;
    }
HPROSE_OBJECT_FREE_END

HPROSE_OBJECT_NEW_BEGIN(client)
HPROSE_OBJECT_NEW_END(client)

HPROSE_CLASS_ENTRY(client)

HPROSE_STARTUP_FUNCTION(client) {
    HPROSE_REGISTER_CLASS_EX("Hprose", "Client", client, get_hprose_proxy_ce(), "HproseProxy");
    HPROSE_REGISTER_CLASS_HANDLERS(client);
    zend_declare_property_stringl(hprose_client_ce, STR_ARG("url"), STR_ARG(""), ZEND_ACC_PROTECTED TSRMLS_CC);
    return SUCCESS;
}
