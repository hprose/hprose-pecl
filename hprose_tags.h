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
 * hprose_tags.h                                          *
 *                                                        *
 * hprose tags for pecl header file.                      *
 *                                                        *
 * LastModified: Mar 10, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_TAGS_H
#define	HPROSE_TAGS_H

#include "hprose_common.h"

BEGIN_EXTERN_C()

/* Serialize Tags */
#define HPROSE_TAG_INTEGER      'i'
#define HPROSE_TAG_LONG         'l'
#define HPROSE_TAG_DOUBLE       'd'
#define HPROSE_TAG_NULL         'n'
#define HPROSE_TAG_EMPTY        'e'
#define HPROSE_TAG_TRUE         't'
#define HPROSE_TAG_FALSE        'f'
#define HPROSE_TAG_NAN          'N'
#define HPROSE_TAG_INFINITY     'I'
#define HPROSE_TAG_DATE         'D'
#define HPROSE_TAG_TIME         'T'
#define HPROSE_TAG_UTC          'Z'
#define HPROSE_TAG_BYTES        'b'
#define HPROSE_TAG_UTF8CHAR     'u'
#define HPROSE_TAG_STRING       's'
#define HPROSE_TAG_GUID         'g'
#define HPROSE_TAG_LIST         'a'
#define HPROSE_TAG_MAP          'm'
#define HPROSE_TAG_CLASS        'c'
#define HPROSE_TAG_OBJECT       'o'
#define HPROSE_TAG_REF          'r'

/* Serialize Marks */
#define HPROSE_TAG_POS          '+'
#define HPROSE_TAG_NEG          '-'
#define HPROSE_TAG_SEMICOLON    ';'
#define HPROSE_TAG_OPENBRACE    '{'
#define HPROSE_TAG_CLOSEBRACE   '}'
#define HPROSE_TAG_QUOTE        '"'
#define HPROSE_TAG_POINT        '.'

/* Protocol Tags */
#define HPROSE_TAG_FUNCTIONS    'F'
#define HPROSE_TAG_CALL         'C'
#define HPROSE_TAG_RESULT       'R'
#define HPROSE_TAG_ARGUMENT     'A'
#define HPROSE_TAG_ERROR        'E'
#define HPROSE_TAG_END          'z'

HPROSE_STARTUP_FUNCTION(tags);

END_EXTERN_C()

#endif	/* HPROSE_TAGS_H */
