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
 * hprose_bytes_io.h                                      *
 *                                                        *
 * hprose bytes io for pecl header file.                  *
 *                                                        *
 * LastModified: Apr 9, 2015                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_BYTES_IO_H
#define	HPROSE_BYTES_IO_H

#include "hprose_common.h"

BEGIN_EXTERN_C()

zend_class_entry *get_hprose_bytes_io_ce();

HPROSE_STARTUP_FUNCTION(bytes_io);

#if PHP_MAJOR_VERSION < 7

typedef struct {
    char * buf;
    int32_t len;
    int32_t cap;
    int32_t pos;
    zend_bool persistent;
} hprose_bytes_io;

#define HB_STR(b) ((b).buf)
#define HB_BUF(b) ((b).buf)
#define HB_LEN(b) ((b).len)

#define HB_STR_P(b) ((b)->buf)
#define HB_BUF_P(b) ((b)->buf)
#define HB_LEN_P(b) ((b)->len)

#else

typedef struct {
    zend_string *s;
    int32_t cap;
    int32_t pos;
    zend_bool persistent;
} hprose_bytes_io;

#define HB_STR(b) ((b).s)
#define HB_BUF(b) ((b).s->val)
#define HB_LEN(b) ((b).s->len)

#define HB_STR_P(b) ((b)->s)
#define HB_BUF_P(b) ((b)->s->val)
#define HB_LEN_P(b) ((b)->s->len)

#endif

#define HB_INITED(b) (HB_STR(b) != NULL)
#define HB_CAP(b) ((b).cap)
#define HB_POS(b) ((b).pos)
#define HB_PERSISTENT(b) ((b).persistent)

#define HB_INITED_P(b) (HB_STR_P(b) != NULL)
#define HB_CAP_P(b) ((b)->cap)
#define HB_POS_P(b) ((b)->pos)
#define HB_PERSISTENT_P(b) ((b)->persistent)

#ifndef HPROSE_BYTES_IO_PREALLOC
#define HPROSE_BYTES_IO_PREALLOC 64
#endif

static const char HPROSE_DIGIT3_TABLE[] =
    "000001002003004005006007008009010011012013014015016017018019020021022023024"
    "025026027028029030031032033034035036037038039040041042043044045046047048049"
    "050051052053054055056057058059060061062063064065066067068069070071072073074"
    "075076077078079080081082083084085086087088089090091092093094095096097098099"
    "100101102103104105106107108109110111112113114115116117118119120121122123124"
    "125126127128129130131132133134135136137138139140141142143144145146147148149"
    "150151152153154155156157158159160161162163164165166167168169170171172173174"
    "175176177178179180181182183184185186187188189190191192193194195196197198199"
    "200201202203204205206207208209210211212213214215216217218219220221222223224"
    "225226227228229230231232233234235236237238239240241242243244245246247248249"
    "250251252253254255256257258259260261262263264265266267268269270271272273274"
    "275276277278279280281282283284285286287288289290291292293294295296297298299"
    "300301302303304305306307308309310311312313314315316317318319320321322323324"
    "325326327328329330331332333334335336337338339340341342343344345346347348349"
    "350351352353354355356357358359360361362363364365366367368369370371372373374"
    "375376377378379380381382383384385386387388389390391392393394395396397398399"
    "400401402403404405406407408409410411412413414415416417418419420421422423424"
    "425426427428429430431432433434435436437438439440441442443444445446447448449"
    "450451452453454455456457458459460461462463464465466467468469470471472473474"
    "475476477478479480481482483484485486487488489490491492493494495496497498499"
    "500501502503504505506507508509510511512513514515516517518519520521522523524"
    "525526527528529530531532533534535536537538539540541542543544545546547548549"
    "550551552553554555556557558559560561562563564565566567568569570571572573574"
    "575576577578579580581582583584585586587588589590591592593594595596597598599"
    "600601602603604605606607608609610611612613614615616617618619620621622623624"
    "625626627628629630631632633634635636637638639640641642643644645646647648649"
    "650651652653654655656657658659660661662663664665666667668669670671672673674"
    "675676677678679680681682683684685686687688689690691692693694695696697698699"
    "700701702703704705706707708709710711712713714715716717718719720721722723724"
    "725726727728729730731732733734735736737738739740741742743744745746747748749"
    "750751752753754755756757758759760761762763764765766767768769770771772773774"
    "775776777778779780781782783784785786787788789790791792793794795796797798799"
    "800801802803804805806807808809810811812813814815816817818819820821822823824"
    "825826827828829830831832833834835836837838839840841842843844845846847848849"
    "850851852853854855856857858859860861862863864865866867868869870871872873874"
    "875876877878879880881882883884885886887888889890891892893894895896897898899"
    "900901902903904905906907908909910911912913914915916917918919920921922923924"
    "925926927928929930931932933934935936937938939940941942943944945946947948949"
    "950951952953954955956957958959960961962963964965966967968969970971972973974"
    "975976977978979980981982983984985986987988989990991992993994995996997998999";

static const char HPROSE_DIGIT2_TABLE[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";


static zend_always_inline int32_t _hprose_pow2roundup(int32_t x) {
#if defined(__GNUC__)
    return 0x2 << (__builtin_clz(x - 1) ^ 0x1f);
#else
    x -= 1;
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >> 16);
    return x + 1;
#endif
}

static zend_always_inline void _hprose_bytes_io_grow(hprose_bytes_io *_this, int32_t n) {
    register int32_t size;
    if (HB_INITED_P(_this)) {
        register int32_t len = HB_LEN_P(_this);
        size = len + n + 1;
        if (size > HB_CAP_P(_this)) {
            size = _hprose_pow2roundup(size);
#if PHP_MAJOR_VERSION < 7
            HB_STR_P(_this) = perealloc(HB_STR_P(_this), size, HB_PERSISTENT_P(_this));
#else
            HB_STR_P(_this) = zend_string_realloc(HB_STR_P(_this), size, HB_PERSISTENT_P(_this));
            HB_LEN_P(_this) = len;
#endif
            HB_BUF_P(_this)[len] = '\0';
            HB_CAP_P(_this) = size;
        }
    }
    else {
        size = _hprose_pow2roundup(n + 1);
        HB_CAP_P(_this) = (size > HPROSE_BYTES_IO_PREALLOC) ? size : HPROSE_BYTES_IO_PREALLOC;
#if PHP_MAJOR_VERSION < 7
        HB_STR_P(_this) = pemalloc(HB_CAP_P(_this), HB_PERSISTENT_P(_this));
#else
        HB_STR_P(_this) = zend_string_alloc(HB_CAP_P(_this), HB_PERSISTENT_P(_this));
#endif
        HB_LEN_P(_this) = 0;
        HB_POS_P(_this) = 0;
        HB_BUF_P(_this)[0] = '\0';
    }
}

static zend_always_inline void hprose_bytes_io_pinit(hprose_bytes_io *_this, const char *buf, int32_t len, zend_bool persistent) {
    if (buf) {
#if PHP_MAJOR_VERSION < 7
        HB_STR_P(_this) = pestrndup(buf, len, persistent);
        HB_LEN_P(_this) = len;
#else
        HB_STR_P(_this) = zend_string_init(buf, len, persistent);
#endif
        HB_CAP_P(_this) = len;
    }
    else {
#if PHP_MAJOR_VERSION < 7
        HB_STR_P(_this) = NULL;
        HB_LEN_P(_this) = 0;
#else
        HB_STR_P(_this) = NULL;
#endif
        HB_CAP_P(_this) = 0;
    }
    HB_POS_P(_this) = 0;
    HB_PERSISTENT_P(_this) = persistent;
}

#define hprose_bytes_io_init(_this, buf, len) hprose_bytes_io_pinit(_this, buf, len, 0)

static zend_always_inline hprose_bytes_io *hprose_bytes_io_pnew(zend_bool persistent) {
    hprose_bytes_io *_this = pecalloc(1, sizeof(hprose_bytes_io), persistent);
    hprose_bytes_io_pinit(_this, NULL, 0, persistent);
    return _this;
}

#define hprose_bytes_io_new() hprose_bytes_io_pnew(0)

static zend_always_inline hprose_bytes_io *hprose_bytes_io_pcreate(const char *buf, int32_t len, zend_bool persistent) {
    hprose_bytes_io *_this = pecalloc(1, sizeof(hprose_bytes_io), persistent);
    hprose_bytes_io_pinit(_this, buf, len, persistent);
    return _this;
}

#define hprose_bytes_io_create(buf, len) hprose_bytes_io_pcreate((buf), (len), 0)

#if PHP_MAJOR_VERSION < 7

static zend_always_inline void hprose_bytes_io_init_readonly(hprose_bytes_io *_this, const char *buf, int32_t len) {
    HB_STR_P(_this) = (char *)buf;
    HB_LEN_P(_this) = len;
    HB_CAP_P(_this) = len;
    HB_POS_P(_this) = 0;
    HB_PERSISTENT_P(_this) = 0;
}

/* only for read, don't call hprose_bytes_io_close or hprose_bytes_io_free on it, using efree to free it. */
static zend_always_inline hprose_bytes_io *hprose_bytes_io_create_readonly(const char *buf, int32_t len) {
    hprose_bytes_io *_this = emalloc(sizeof(hprose_bytes_io));
    hprose_bytes_io_init_readonly(_this, buf, len);
    return _this;
}

static zend_always_inline void hprose_bytes_io_close(hprose_bytes_io *_this) {
    if (HB_INITED_P(_this)) {
        pefree(HB_STR_P(_this), HB_PERSISTENT_P(_this));
        HB_STR_P(_this) = NULL;
    }
    HB_LEN_P(_this) = 0;
    HB_CAP_P(_this) = 0;
    HB_POS_P(_this) = 0;
}

#else

static zend_always_inline void hprose_bytes_io_init_readonly(hprose_bytes_io *_this, zend_string *s) {
    HB_STR_P(_this) = s;
    HB_CAP_P(_this) = HB_LEN_P(_this);
    HB_POS_P(_this) = 0;
    HB_PERSISTENT_P(_this) = 0;
}

/* only for read, don't call hprose_bytes_io_close or hprose_bytes_io_free on it, using efree to free it. */
static zend_always_inline hprose_bytes_io *hprose_bytes_io_create_readonly(zend_string *s) {
    hprose_bytes_io *_this = emalloc(sizeof(hprose_bytes_io));
    hprose_bytes_io_init_readonly(_this, s);
    return _this;
}

static zend_always_inline void hprose_bytes_io_close(hprose_bytes_io *_this) {
    if (HB_INITED_P(_this)) {
        zend_string_release(HB_STR_P(_this));
        HB_STR_P(_this) = NULL;
    }
    HB_CAP_P(_this) = 0;
    HB_POS_P(_this) = 0;
}

#endif

static zend_always_inline void hprose_bytes_io_free(hprose_bytes_io *_this) {
    hprose_bytes_io_close(_this);
    pefree(_this, HB_PERSISTENT_P(_this));
}

static zend_always_inline char hprose_bytes_io_getc(hprose_bytes_io *_this) {
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) < HB_LEN_P(_this));
    return HB_BUF_P(_this)[HB_POS_P(_this)++];
}

#if PHP_MAJOR_VERSION < 7

static zend_always_inline char *hprose_bytes_io_pread(hprose_bytes_io *_this, int32_t n, zend_bool persistent) {
    char *s;
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) + n <= HB_LEN_P(_this));
    s = pestrndup(HB_BUF_P(_this) + HB_POS_P(_this), n, persistent);
    HB_POS_P(_this) += n;
    return s;
}

static zend_always_inline char * hprose_bytes_io_preadfull(hprose_bytes_io *_this, int32_t *len_ptr, zend_bool persistent) {
    char *s;
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) <= HB_LEN_P(_this));
    assert(len_ptr);
    *len_ptr = HB_LEN_P(_this) - HB_POS_P(_this);
    s = pestrndup(HB_BUF_P(_this) + HB_POS_P(_this), *len_ptr, persistent);
    HB_POS_P(_this) = HB_LEN_P(_this);
    return s;
}

static zend_always_inline char * hprose_bytes_io_preaduntil_ex(hprose_bytes_io *_this, char tag, int32_t *len_ptr, zend_bool persistent, zend_bool skiptag) {
    char *s;
    int32_t i = HB_POS_P(_this), n = HB_LEN_P(_this), p = HB_LEN_P(_this);
    assert(HB_INITED_P(_this));
    for (; i < n; ++i) {
        if (HB_BUF_P(_this)[i] == tag) {
            p = i;
            break;
        }
    }
    *len_ptr = p - HB_POS_P(_this);
    s = pestrndup(HB_BUF_P(_this) + HB_POS_P(_this), *len_ptr, persistent);
    HB_POS_P(_this) = p;
    if (HB_POS_P(_this) < HB_LEN_P(_this) && skiptag) {
        HB_POS_P(_this)++;
    }
    return s;
}

#define hprose_bytes_io_readfull(_this, len_ptr) hprose_bytes_io_preadfull((_this), (len_ptr), 0)

#define hprose_bytes_io_readuntil_ex(_this, tag, len_ptr, skiptag) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), 0, (skiptag))
#define hprose_bytes_io_preaduntil(_this, tag, len_ptr, persistent) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), (persistent), 1)
#define hprose_bytes_io_readuntil(_this, tag, len_ptr) hprose_bytes_io_preaduntil_ex((_this), (tag), (len_ptr), 0, 1)

#else

static zend_always_inline zend_string *hprose_bytes_io_pread(hprose_bytes_io *_this, int32_t n, zend_bool persistent) {
    zend_string *s;
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) + n <= HB_LEN_P(_this));
    s = zend_string_init(HB_BUF_P(_this) + HB_POS_P(_this), n, persistent);
    HB_POS_P(_this) += n;
    return s;
}

static zend_always_inline zend_string *hprose_bytes_io_preadfull(hprose_bytes_io *_this, zend_bool persistent) {
    zend_string *s;
    assert(HB_INITED_P(_this));
    assert(HB_POS_P(_this) <= HB_LEN_P(_this));
    s = zend_string_init(HB_BUF_P(_this) + HB_POS_P(_this), HB_LEN_P(_this) - HB_POS_P(_this), persistent);
    HB_POS_P(_this) = HB_LEN_P(_this);
    return s;
}

static zend_always_inline zend_string *hprose_bytes_io_preaduntil_ex(hprose_bytes_io *_this, char tag, zend_bool persistent, zend_bool skiptag) {
    zend_string *s;
    int32_t i = HB_POS_P(_this), n = HB_LEN_P(_this), p = HB_LEN_P(_this);
    assert(HB_INITED_P(_this));
    for (; i < n; ++i) {
        if (HB_BUF_P(_this)[i] == tag) {
            p = i;
            break;
        }
    }
    s = zend_string_init(HB_BUF_P(_this) + HB_POS_P(_this), p - HB_POS_P(_this), persistent);
    HB_POS_P(_this) = p;
    if (HB_POS_P(_this) < HB_LEN_P(_this) && skiptag) {
        HB_POS_P(_this)++;
    }
    return s;
}

#define hprose_bytes_io_readfull(_this) hprose_bytes_io_preadfull((_this), 0)

#define hprose_bytes_io_readuntil_ex(_this, tag, skiptag) hprose_bytes_io_preaduntil_ex((_this), (tag), 0, (skiptag))
#define hprose_bytes_io_preaduntil(_this, tag, persistent) hprose_bytes_io_preaduntil_ex((_this), (tag), (persistent), 1)
#define hprose_bytes_io_readuntil(_this, tag) hprose_bytes_io_preaduntil_ex((_this), (tag), 0, 1)

#endif

#define hprose_bytes_io_read(_this, n) hprose_bytes_io_pread((_this), (n), 0)

static zend_always_inline int32_t hprose_bytes_io_read_int(hprose_bytes_io *_this, char tag) {
    int32_t result = 0, sign = 1;
    char c = hprose_bytes_io_getc(_this);
    if (c == tag) {
        return 0;
    }
    switch (c) {
        case '-': sign = -1; /* fallthrough */
        case '+': c = hprose_bytes_io_getc(_this); break;
    }
    while ((HB_POS_P(_this) < HB_LEN_P(_this)) && (c != tag)) {
        result *= 10;
        result += (c - '0') * sign;
        c = hprose_bytes_io_getc(_this);
    }
    return result;
}

static zend_always_inline int32_t _hprose_bytes_io_get_string_len(hprose_bytes_io *_this, int32_t n TSRMLS_DC) {
    int32_t i, p = HB_POS_P(_this), l = HB_LEN_P(_this);
    uint8_t *buf = (uint8_t *)HB_BUF_P(_this);
    assert(HB_INITED_P(_this));
    for (i = 0; i < n && p < l; ++i) {
        switch (buf[p] >> 4) {
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
                /* 0xxx xxxx */
                ++p;
                break;
            case 12:
            case 13:
                /* 110x xxxx   10xx xxxx */
                p += 2;
                break;
            case 14:
                /* 1110 xxxx  10xx xxxx  10xx xxxx */
                p += 3;
                break;
            case 15:
                /* 1111 0xxx  10xx xxxx  10xx xxxx  10xx xxxx */
                p += 4;
                ++i;
                if (i < n) break;
                /* fall through */
            default:
                zend_throw_exception(NULL, "bad utf-8 encoding", 0 TSRMLS_CC);
                break;
        }
    }
    return p - HB_POS_P(_this);
}

#if PHP_MAJOR_VERSION < 7

static zend_always_inline char * _hprose_bytes_io_read_pstring(hprose_bytes_io *_this, int32_t n, int32_t *len_ptr, zend_bool persistent TSRMLS_DC) {
    *len_ptr = _hprose_bytes_io_get_string_len(_this, n TSRMLS_CC);
    return hprose_bytes_io_pread(_this, *len_ptr, persistent);
}

#define hprose_bytes_io_read_pstring(_this, n, len_ptr, persistent) _hprose_bytes_io_read_pstring((_this), (n), (len_ptr), (persistent) TSRMLS_CC)
#define hprose_bytes_io_read_string(_this, n, len_ptr) hprose_bytes_io_read_pstring((_this), (n), (len_ptr), 0)

static zend_always_inline char * hprose_bytes_io_to_string(hprose_bytes_io *_this) {
    return estrndup(HB_BUF_P(_this), HB_LEN_P(_this));
}

#else

static zend_always_inline zend_string *_hprose_bytes_io_read_pstring(hprose_bytes_io *_this, int32_t n, zend_bool persistent TSRMLS_DC) {
    return hprose_bytes_io_pread(_this, _hprose_bytes_io_get_string_len(_this, n TSRMLS_CC), persistent);
}

#define hprose_bytes_io_read_pstring(_this, n, persistent) _hprose_bytes_io_read_pstring((_this), (n), (persistent) TSRMLS_CC)
#define hprose_bytes_io_read_string(_this, n) hprose_bytes_io_read_pstring((_this), (n), 0)

static zend_always_inline zend_string *hprose_bytes_io_to_string(hprose_bytes_io *_this) {
    return zend_string_init(HB_BUF_P(_this), HB_LEN_P(_this), 0);
}

#endif

static zend_always_inline void hprose_bytes_io_skip(hprose_bytes_io *_this, int32_t n) {
    assert(HB_INITED_P(_this));
    assert(n >= 0);
    HB_POS_P(_this) += n;
    assert(HB_POS_P(_this) <= HB_LEN_P(_this));
}

static zend_always_inline zend_bool hprose_bytes_io_eof(hprose_bytes_io *_this) {
    return (HB_POS_P(_this) >= HB_LEN_P(_this));
}

static zend_always_inline void hprose_bytes_io_write(hprose_bytes_io *_this, const char *str, int32_t n) {
    if (n < 0) n = strlen(str);
    if (n == 0) return;
    _hprose_bytes_io_grow(_this, n);
    memcpy(HB_BUF_P(_this) + HB_LEN_P(_this), str, n);
    HB_LEN_P(_this) += n;
    HB_BUF_P(_this)[HB_LEN_P(_this)] = '\0';
}

static zend_always_inline void hprose_bytes_io_putc(hprose_bytes_io *_this, char c) {
    if (!HB_INITED_P(_this) || HB_LEN_P(_this) + 1 >= HB_CAP_P(_this)) {
        _hprose_bytes_io_grow(_this, HPROSE_BYTES_IO_PREALLOC);
    }
    HB_BUF_P(_this)[HB_LEN_P(_this)] = c;
    HB_BUF_P(_this)[++HB_LEN_P(_this)] = '\0';
}

#define hprose_bytes_io_write_integer(_this, i, uint_type)                      \
    if (i >= 0 && i < 10) {                                                     \
        hprose_bytes_io_putc(_this, (char)('0' + i));                           \
    }                                                                           \
    else if (i >= 10 && i < 100) {                                              \
        hprose_bytes_io_write(_this, &HPROSE_DIGIT2_TABLE[2 * i], 2);           \
    }                                                                           \
    else if (i >= 100 && i < 1000) {                                            \
        hprose_bytes_io_write(_this, &HPROSE_DIGIT3_TABLE[3 * i], 3);           \
    }                                                                           \
    else {                                                                      \
        char buf[32];                                                           \
        char *p = &buf[32];                                                     \
        zend_bool neg = (i < 0);                                                \
        uint_type n = (neg) ? -i : i;                                           \
        if (n) {                                                                \
            while (n >= 100) {                                                  \
                uint_type t = n / 1000;                                         \
                p -= 3;                                                         \
                memcpy(p, &HPROSE_DIGIT3_TABLE[3 * (n - (t * 1000))], 3);       \
                n = t;                                                          \
            }                                                                   \
            while (n >= 10) {                                                   \
                uint_type t = n / 100;                                          \
                p -= 2;                                                         \
                memcpy(p, &HPROSE_DIGIT2_TABLE[2 * (n - (t * 100))], 2);        \
                n = t;                                                          \
            }                                                                   \
            if (n) {                                                            \
                *(--p) = (char)('0' + n);                                       \
            }                                                                   \
            if (neg) {                                                          \
                *(--p) = '-';                                                   \
            }                                                                   \
        } else {                                                                \
            *(--p) = '0';                                                       \
        }                                                                       \
        hprose_bytes_io_write(_this, p, &buf[32] - p);                          \
    }

static zend_always_inline void hprose_bytes_io_write_int(hprose_bytes_io *_this, int32_t i) {
    hprose_bytes_io_write_integer(_this, i, uint32_t);
}

static zend_always_inline void hprose_bytes_io_write_long(hprose_bytes_io *_this, int64_t i) {
    hprose_bytes_io_write_integer(_this, i, uint64_t);
}

#define hprose_bytes_io_write_uinteger(_this, i, uint_type)                     \
    if (i >= 0 && i < 10) {                                                     \
        hprose_bytes_io_putc(_this, (char)('0' + i));                           \
    }                                                                           \
    else if (i >= 10 && i < 100) {                                              \
        hprose_bytes_io_write(_this, &HPROSE_DIGIT2_TABLE[2 * i], 2);           \
    }                                                                           \
    else if (i >= 100 && i < 1000) {                                            \
        hprose_bytes_io_write(_this, &HPROSE_DIGIT3_TABLE[3 * i], 3);           \
    }                                                                           \
    else {                                                                      \
        char buf[32];                                                           \
        char *p = &buf[32];                                                     \
        if (i) {                                                                \
            while (i >= 100) {                                                  \
                uint_type t = i / 1000;                                         \
                p -= 3;                                                         \
                memcpy(p, &HPROSE_DIGIT3_TABLE[3 * (i - (t * 1000))], 3);       \
                i = t;                                                          \
            }                                                                   \
            while (i >= 10) {                                                   \
                uint_type t = i / 100;                                          \
                p -= 2;                                                         \
                memcpy(p, &HPROSE_DIGIT2_TABLE[2 * (i - (t * 100))], 2);        \
                i = t;                                                          \
            }                                                                   \
            if (i) {                                                            \
                *(--p) = (char)('0' + i);                                       \
            }                                                                   \
        } else {                                                                \
            *(--p) = '0';                                                       \
        }                                                                       \
        hprose_bytes_io_write(_this, p, &buf[32] - p);                          \
    }

static zend_always_inline void hprose_bytes_io_write_uint(hprose_bytes_io *_this, uint32_t i) {
    hprose_bytes_io_write_uinteger(_this, i, uint32_t);
}

static zend_always_inline void hprose_bytes_io_write_ulong(hprose_bytes_io *_this, uint64_t i) {
    hprose_bytes_io_write_uinteger(_this, i, uint64_t);
}

static zend_always_inline void hprose_bytes_io_write_double(hprose_bytes_io *_this, double d) {
    char buf[1024];
    int32_t n;
    php_gcvt(d, 14, '.', 'e', &buf[0]);
    n = strlen(buf);
    hprose_bytes_io_write(_this, buf, n);
}

static zend_always_inline void hprose_bytes_io_getc_to(hprose_bytes_io *from, hprose_bytes_io *to) {
    hprose_bytes_io_putc(to, HB_BUF_P(from)[HB_POS_P(from)++]);
}

static zend_always_inline void hprose_bytes_io_read_to(hprose_bytes_io *from, hprose_bytes_io *to, int32_t n) {
    assert(HB_INITED_P(from));
    assert(HB_POS_P(from) + n <= HB_LEN_P(from));
    hprose_bytes_io_write(to, HB_BUF_P(from) + HB_POS_P(from), n);
    HB_POS_P(from) += n;
}

static zend_always_inline void hprose_bytes_io_readuntil_to(hprose_bytes_io *from, hprose_bytes_io *to, char tag, zend_bool include_tag) {
    int32_t i = HB_POS_P(from), n = HB_LEN_P(from), p = HB_LEN_P(from);
    assert(HB_INITED_P(from));
    for (; i < n; ++i) {
        if (HB_BUF_P(from)[i] == tag) {
            p = include_tag ? i + 1 : i;
            break;
        }
    }
    hprose_bytes_io_write(to, HB_BUF_P(from) + HB_POS_P(from), p - HB_POS_P(from));
    HB_POS_P(from) = p;
    if (HB_POS_P(from) < HB_LEN_P(from) && !include_tag) {
        HB_POS_P(from)++;
    }
}

static zend_always_inline int32_t hprose_bytes_io_read_int_to(hprose_bytes_io *from, hprose_bytes_io *to, char tag, zend_bool include_tag) {
    int32_t result = 0, sign = 1, p = HB_POS_P(from);
    char c = hprose_bytes_io_getc(from);
    if (c == tag) {
        if (include_tag) {
            hprose_bytes_io_putc(to, c);
        }
        return 0;
    }
    switch (c) {
        case '-': sign = -1; /* fallthrough */
        case '+': c = hprose_bytes_io_getc(from); break;
    }
    while ((HB_POS_P(from) < HB_LEN_P(from)) && (c != tag)) {
        result *= 10;
        result += (c - '0') * sign;
        c = hprose_bytes_io_getc(from);
    }
    hprose_bytes_io_write(to, HB_BUF_P(from) + p, HB_POS_P(from) - p - 1 + ((int32_t)include_tag & 1));
    return result;
}

static zend_always_inline void _hprose_bytes_io_read_string_to(hprose_bytes_io *from, hprose_bytes_io *to, int32_t n TSRMLS_DC) {
    int32_t len = _hprose_bytes_io_get_string_len(from, n TSRMLS_CC);
    hprose_bytes_io_write(to, HB_BUF_P(from) + HB_POS_P(from), len);
    HB_POS_P(from) += len;
}

#define hprose_bytes_io_read_string_to(from, to, n) _hprose_bytes_io_read_string_to((from), (to), (n) TSRMLS_CC)

HPROSE_CLASS_BEGIN(bytes_io)
    int32_t mark;
HPROSE_CLASS_END(bytes_io)

END_EXTERN_C()

#endif	/* HPROSE_BYTES_IO_H */
