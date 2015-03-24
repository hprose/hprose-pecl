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

static zend_always_inline void hprose_client_do_output(zval *client, zval *name, zval *args, zend_bool byref, zend_bool *simple, zval *context, zval *return_value TSRMLS_DC) {
    hprose_client *_this = HPROSE_GET_OBJECT_P(client, client)->_this;
    hprose_bytes_io *stream = hprose_bytes_io_new();
    hprose_writer *writer;
    HashTable *ht;
    int32_t i;
    if (simple) {
        simple = &(_this->simple);
    }
    hprose_bytes_io_write_char(stream, HPROSE_TAG_CALL);
    writer = hprose_writer_create(stream, *simple);
    hprose_writer_write_string(writer, name);
    if (zend_hash_num_elements(Z_ARRVAL_P(args)) > 0 || byref) {
        hprose_writer_reset(writer);
        hprose_writer_write_array(writer, args TSRMLS_CC);
        if (byref) {
            hprose_writer_write_true(writer);
        }
    }
    hprose_writer_free(writer);
    hprose_bytes_io_write_char(stream, HPROSE_TAG_END);
    RETVAL_STRINGL_0(stream->buf, stream->len);
    efree(stream);
    ht = Z_ARRVAL_P(_this->filters);
    i = zend_hash_num_elements(ht);
    if (i) {
        zend_hash_internal_pointer_reset(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **filter;
            zend_hash_get_current_data(ht, (void **)&filter);
            method_invoke(*filter, outputFilter, return_value, "zz", return_value, context);
#else
            zval *filter = zend_hash_get_current_data(ht);
            method_invoke(filter, outputFilter, return_value, "zz", return_value, context);
#endif
            zend_hash_move_forward(ht);
        }
    }
}

static zend_always_inline void hprose_client_do_input(zval *client, zval *response, zval *args, int mode, zval *context, zval *return_value TSRMLS_DC) {
    hprose_client *_this = HPROSE_GET_OBJECT_P(client, client)->_this;
    HashTable *ht = Z_ARRVAL_P(_this->filters);
    int32_t i = zend_hash_num_elements(ht);
    if (i) {
        zend_hash_internal_pointer_end(ht);
        for (; i > 0; --i) {
#if PHP_MAJOR_VERSION < 7
            zval **filter;
            zend_hash_get_current_data(ht, (void **)&filter);
            method_invoke(*filter, inputFilter, response, "zz", response, context);
#else
            zval *filter = zend_hash_get_current_data(ht);
            method_invoke(filter, inputFilter, response, "zz", response, context);
#endif
            zend_hash_move_backwards(ht);
        }
    }
    if (mode == HPROSE_RESULT_MODE_RAW_WITH_END_TAG) {
        // need to check
        RETURN_ZVAL(response, 0, 0);
    }
    else if (mode == HPROSE_RESULT_MODE_RAW) {
        RETURN_STRINGL_1(Z_STRVAL_P(response), Z_STRLEN_P(response) - 1);
    }
    else {
        hprose_bytes_io *stream = hprose_bytes_io_create_readonly(Z_STRVAL_P(response), Z_STRLEN_P(response));
        hprose_reader *reader = hprose_reader_create(stream, 0);
        char tag;
        RETVAL_NULL();
        while ((tag = hprose_bytes_io_getc(stream)) != HPROSE_TAG_END) {
            switch (tag) {
                case HPROSE_TAG_RESULT:
                    if (mode == HPROSE_RESULT_MODE_SERIALIZED) {
                        hprose_bytes_io *result = hprose_raw_reader_read_raw((hprose_raw_reader *)reader TSRMLS_CC);
                        RETVAL_STRINGL_0(result->buf, result->len);
                        efree(result);
                    }
                    else {
                        hprose_reader_reset(reader);
                        hprose_reader_unserialize(reader, return_value TSRMLS_CC);
                    }
                    break;
                case HPROSE_TAG_ARGUMENT: {
                    zval _args;
                    int32_t n, n1, n2, i;
                    hprose_reader_reset(reader);
                    hprose_reader_read_list(reader, &_args TSRMLS_CC);
                    n1 = zend_hash_num_elements(Z_ARRVAL_P(args));
                    n2 = zend_hash_num_elements(Z_ARRVAL(_args));
                    n = (n1 < n2) ? n1 : n2;
                    for (i = 0; i < n; ++i) {
                        zval *val = php_array_get(&_args, i);
                        add_index_zval(args, i, val);
#if PHP_MAJOR_VERSION < 7
                        Z_ADDREF_P(val);
#else
                        Z_TRY_ADDREF_P(val);
#endif
                    }
                    zval_dtor(&_args);
                    break;
                }
                case HPROSE_TAG_ERROR: {
                    hprose_reader_reset(reader);
                    zval errstr;
                    hprose_reader_read_string(reader, &errstr TSRMLS_CC);
                    zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
                                            0 TSRMLS_CC,
                        "%s", Z_STRVAL(errstr));
                    zval_dtor(&errstr);
                    break;
                }
                default:
                    zend_throw_exception_ex(zend_exception_get_default(TSRMLS_C),
                                            0 TSRMLS_CC,
                        "Wrong Response:\r\n%s", Z_STRVAL_P(response));
                    break;
            }
        }
    }
}

static zend_always_inline void hprose_client_sync_invoke(zval *client, char *name, int32_t len, zval *args, zend_bool byref, int mode, zend_bool simple, zval *return_value TSRMLS_DC) {

}

static zend_always_inline void hprose_client_async_invoke(zval *client, char *name, int32_t len, zval *args, zend_bool byref, int mode, zend_bool simple, zval *callback TSRMLS_DC) {
    
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
            hprose_client_async_invoke(_this->client, _name->buf, _name->len, args, 0, HPROSE_RESULT_MODE_NORMAL, 0, callback TSRMLS_CC);
            hprose_bytes_io_free(_name);
            return;
        }
    }
    hprose_client_sync_invoke(_this->client, _name->buf, _name->len, args, 0, HPROSE_RESULT_MODE_NORMAL, 0, return_value TSRMLS_CC);
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
    hprose_client_ce->ce_flags |= ZEND_ACC_EXPLICIT_ABSTRACT_CLASS;
    return SUCCESS;
}
