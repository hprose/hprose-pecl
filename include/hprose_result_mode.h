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
 * hprose_result_mode.h                                   *
 *                                                        *
 * hprose result mode for pecl header file.               *
 *                                                        *
 * LastModified: Mar 24, 2015                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_RESULT_MODE_H
#define	HPROSE_RESULT_MODE_H

#include "hprose.h"

BEGIN_EXTERN_C()

#define HPROSE_RESULT_MODE_NORMAL           0
#define HPROSE_RESULT_MODE_SERIALIZED       1
#define HPROSE_RESULT_MODE_RAW              2
#define HPROSE_RESULT_MODE_RAW_WITH_END_TAG 3

HPROSE_STARTUP_FUNCTION(result_mode);

END_EXTERN_C()

#endif	/* HPROSE_RESULT_MODE_H */
