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

#include "hprose_client.h"

ZEND_METHOD(hprose_proxy, __construct) {
    zval *client = NULL;
    zend_class_entry *ce = NULL;
    char *nstr = "";
    length_t nlen = 0;
    HPROSE_OBJECT_INTERN(proxy);
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O|s", &client, &ce, &nstr, &nlen) == FAILURE) {
        return;
    }
    if (!client || !instanceof(ce, HproseClient)) {
        zend_throw_exception(zend_exception_get_default(TSRMLS_C),
                             "client must be an instance of HproseClient.", 0 TSRMLS_CC);
        return;
    }
    intern->_this = emalloc(sizeof(hprose_proxy));
    intern->_this->client = HPROSE_GET_OBJECT_P(client, client)->_this;
    intern->_this->ns = nstr;
}

ZEND_METHOD(hprose_proxy, __destruct) {
    HPROSE_OBJECT_INTERN(proxy);
    if (intern->_this) {
        efree(intern->_this);
        intern->_this = NULL;
    }
}

ZEND_BEGIN_ARG_INFO_EX(hprose_proxy_construct_arginfo, 0, 0, 1)
    ZEND_ARG_INFO(0, client)
    ZEND_ARG_INFO(0, ns)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(hprose_proxy_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

static zend_function_entry hprose_proxy_methods[] = {
    ZEND_ME(hprose_proxy, __construct, hprose_proxy_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
    ZEND_ME(hprose_proxy, __destruct, hprose_proxy_void_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
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
    intern->_this->client = intern->_this;
    intern->_this->ns = "";
    intern->_this->url = url;
    intern->_this->simple = 0;
    hprose_make_zval(intern->_this->filters);
    array_init(intern->_this->filters);
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
    return SUCCESS;
}
