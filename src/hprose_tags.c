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
 * hprose_tags.c                                          *
 *                                                        *
 * hprose tags for pecl source file.                      *
 *                                                        *
 * LastModified: Mar 10, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#include "hprose_tags.h"

static zend_function_entry hprose_tags_methods[] = {
    {NULL, NULL, NULL}
};

#define DECLARE_TAG(name, value) \
    zend_declare_class_constant_stringl(hprose_tags_ce, ZEND_STRL(name), ZEND_STRL(value) TSRMLS_CC);

zend_class_entry *hprose_tags_ce;

HPROSE_STARTUP_FUNCTION(tags) {
    zend_class_entry ce;
    INIT_NS_CLASS_ENTRY(ce, "Hprose", "Tags", hprose_tags_methods)
    hprose_tags_ce = zend_register_internal_class(&ce TSRMLS_CC);
    zend_register_class_alias("HproseTags", hprose_tags_ce);
    /* Serialize Tags */
    DECLARE_TAG("TagInteger",       "i")
    DECLARE_TAG("TagLong",          "l")
    DECLARE_TAG("TagDouble",        "d")
    DECLARE_TAG("TagNull",          "n")
    DECLARE_TAG("TagEmpty",         "e")
    DECLARE_TAG("TagTrue",          "t")
    DECLARE_TAG("TagFalse",         "f")
    DECLARE_TAG("TagNaN",           "N")
    DECLARE_TAG("TagInfinity",      "I")
    DECLARE_TAG("TagDate",          "D")
    DECLARE_TAG("TagTime",          "T")
    DECLARE_TAG("TagUTC",           "Z")
    DECLARE_TAG("TagBytes",         "b")
    DECLARE_TAG("TagUTF8Char",      "u")
    DECLARE_TAG("TagString",        "s")
    DECLARE_TAG("TagGuid",          "g")
    DECLARE_TAG("TagList",          "a")
    DECLARE_TAG("TagMap",           "m")
    DECLARE_TAG("TagClass",         "c")
    DECLARE_TAG("TagObject",        "o")
    DECLARE_TAG("TagRef",           "r")
    /* Serialize Marks */
    DECLARE_TAG("TagPos",           "+")
    DECLARE_TAG("TagNeg",           "-")
    DECLARE_TAG("TagSemicolon",     ";")
    DECLARE_TAG("TagOpenbrace",     "{")
    DECLARE_TAG("TagClosebrace",    "}")
    DECLARE_TAG("TagQuote",         "\"")
    DECLARE_TAG("TagPoint",         ".")
    /* Protocol Tags */
    DECLARE_TAG("TagFunctions",     "F")
    DECLARE_TAG("TagCall",          "C")
    DECLARE_TAG("TagResult",        "R")
    DECLARE_TAG("TagArgument",      "A")
    DECLARE_TAG("TagError",         "E")
    DECLARE_TAG("TagEnd",           "z")
    return SUCCESS;
}
