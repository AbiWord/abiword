#ifndef _CHARSET_H
#define _CHARSET_H
/* ASCII is ISO 646, except the ISO version admits national alternatives */
enum charset { e_usascii, e_iso646_no, e_iso646_se, e_iso8859_1,
    e_iso8859_2, e_iso8859_3, e_iso8859_4, e_iso8859_5, e_iso8859_6,
    e_iso8859_7, e_iso8859_8, e_iso8859_9, e_iso8859_10,
    e_iso8859_11/* same as TIS */, e_iso8859_13, e_iso8859_14, e_iso8859_15,
    e_koi8_r,	/* RFC 1489 */
    e_jis201,	/* 8 bit, ascii & katakana */
    e_win, e_mac,
    e_user,
/* korean appears to fit into the jis/euc encoding schemes */
/* the difference between jis & jis2 is what the output encoding should be (presence of '(') */
    e_jis, e_jis2, e_jiskorean, e_jisgb, e_sjis,	/* multi-byte */
    e_euc, e_euckorean, e_eucgb,
    e_big5,
    e_unicode, e_unicode_backwards,			/* wide chars */
    e_utf7, e_utf8,					/* unicode encodings */
    e_ucs4,						/* 4 byte chars */
    e_notrans,					/* _inch returns 16bits */
    e_charsetmax, e_unknown=-1, e_first2byte=e_jis };

enum charmaps { em_none = -1,
    em_iso8859_1, em_iso8859_2, em_iso8859_3, em_iso8859_4, em_iso8859_5,
    em_iso8859_6, em_iso8859_7, em_iso8859_8, em_iso8859_9, em_iso8859_10,
    em_iso8859_11/* same as TIS */, em_iso8859_13, em_iso8859_14, em_iso8859_15,
    em_koi8_r,
    em_jis201,
    em_win, em_mac, em_symbol, em_zapfding, em_user,
    em_sjis, em_euc,			/* used by postscript */
    em_jis208, em_jis212, em_ksc5601, em_gb2312, em_big5,
    em_unicode, em_max, em_first2byte=em_sjis, em_last96x96=em_gb2312 };

extern enum charset local_charset;
extern struct namemap { char *name; int map; } maps[];
#endif
