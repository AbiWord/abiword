/* Copyright (c) 1997-1998 by Juliusz Chroboczek */
#ifndef TTF_TYPES_H
#define TTF_TYPES_H

#include <sys/types.h>
typedef unsigned char	TTF_BYTE;
typedef signed char		TTF_CHAR;
typedef unsigned short	TTF_USHORT;
typedef signed short    TTF_SHORT;
typedef unsigned long   TTF_ULONG;
typedef signed long     TTF_LONG;
typedef unsigned long   TTF_FIXED_L;
typedef unsigned short  TTF_FUNIT;
typedef signed short    TTF_FWORD;
typedef unsigned short  TTF_UFWORD;
typedef unsigned short  TTF_F2DOT14;

#include "ut_endian.h"

/* Endianness conversions */
#ifdef UT_LITTLE_ENDIAN
#define H(x) ((TTF_SHORT)((((TTF_SHORT)(x))&0xFF)<<8)+(((TTF_USHORT)(x))>>8))
#define UH(x) ((TTF_USHORT)((((TTF_USHORT)(x))&0xFF)<<8)+(((TTF_USHORT)(x))>>8))
#define L(x) ((TTF_LONG)(((TTF_LONG)UH((x)&0xFFFF))<<16)+UH(((TTF_ULONG)(x))>>16))
#define UL(x) ((TTF_ULONG)(((TTF_ULONG)UH((x)&0xFFFF))<<16)+UH(((TTF_ULONG)(x))>>16))
#else
#define H(x) ((TTF_SHORT)x)
#define UH(x) ((TTF_USHORT)x)
#define L(x) ((TTF_LONG)x)
#define UL(x) ((TTF_ULONG)x)
#endif

#define FIX_H(x) x=H(x)
#define FIX_UH(x) x=UH(x)
#define FIX_L(x) x=L(x)
#define FIX_UL(x) x=UL(x)

/* We are assuming that the compiler will not pad the following
 * structures; note that their members are intrinsically properly
 * aligned.  This will probably break on some machines. */

typedef struct {
  TTF_SHORT mantissa;
  TTF_USHORT fraction;
} TTF_FIXED;

#define FIX_Fixed(x) {(x).mantissa=H((x).mantissa); (x).fraction=UH((x).fraction);}


typedef struct { TTF_BYTE data[8];} longDateTime;

#define MAKE_ULONG(a,b,c,d) ((TTF_ULONG)(((TTF_ULONG)(a)<<24)|((b)<<16)|((c)<<8)|(d)))

/*------------------------------------------------------------*/

struct OffsetTable {
  TTF_FIXED version;
  TTF_USHORT numTables;
  TTF_USHORT searchRange;
  TTF_USHORT entrySelector;
  TTF_USHORT rangeShift;
};
#define FIX_OffsetTable(x) \
  {FIX_Fixed((x).version);\
   FIX_UH((x).numTables);\
   FIX_UH((x).searchRange);\
   FIX_UH((x).entrySelector);}                            


struct TableDirectoryEntry {
  TTF_ULONG tag;
  TTF_ULONG checkSum;
  TTF_ULONG offset;
  TTF_ULONG length;
};


typedef struct {
    char tag[4];
    TTF_ULONG checksum;
    TTF_ULONG offset;
    TTF_ULONG length;
} dirtab_entry;

#define FIX_TableDirectoryEntry(x) \
  {FIX_UL((x).tag); FIX_UL((x).checkSum);\
   FIX_UL((x).offset); FIX_UL((x).length);}


/*------------------------------------------------------------*/
typedef struct {
    TTF_ULONG wx;
    char *name;
    TTF_USHORT index;
    TTF_LONG bbox[4];
    TTF_LONG offset;
    char found;
    TTF_USHORT uni;
} mtx_entry;

typedef struct _kern_entry {
    TTF_FWORD value;
    TTF_USHORT adjacent;
    struct _kern_entry *next;
} kern_entry;

typedef struct {
    TTF_USHORT platform_id;
    TTF_USHORT encoding_id;
    TTF_ULONG  offset;
} cmap_entry;

typedef struct {
    TTF_USHORT endCode;
    TTF_USHORT startCode;
    TTF_SHORT idDelta;
    TTF_USHORT idRangeOffset;
} seg_entry;

#define TTF_BYTE_SIZE       1
#define TTF_CHAR_SIZE       1
#define TTF_USHORT_SIZE     2
#define TTF_SHORT_SIZE      2
#define TTF_ULONG_SIZE      4
#define TTF_LONG_SIZE       4
#define TTF_FIXED_L_SIZE      4
#define TTF_FWORD_SIZE      2
#define TTF_UFWORD_SIZE     2
#define TTF_F2DOT14_SIZE    2

#define get_type(t)         ((t)getnum(t##_SIZE))

#define get_byte()      get_type(TTF_BYTE)
#define get_char()      get_type(TTF_CHAR)
#define get_ushort()    get_type(TTF_USHORT)
#define get_short()     get_type(TTF_SHORT)
#define get_ulong()     get_type(TTF_ULONG)
#define get_long()      get_type(TTF_LONG)
#define get_fixed()     get_type(TTF_FIXED_L)
#define get_funit()     get_type(TTF_FUNIT)
#define get_fword()     get_type(TTF_FWORD)
#define get_ufword()    get_type(TTF_UFWORD)
#define get_f2dot14()   get_type(TTF_F2DOT14)

#define NTABS           24

#define strend(s)       strchr(s, 0)

#define append_char_to_buf(c, p, buf, buf_size) do {       \
    if (c == 9)                                            \
        c = 32;                                            \
    if (c == 13 || c == EOF)                               \
        c = 10;                                            \
    if (c != ' ' || (p > buf && p[-1] != 32)) {            \
        check_buf(p - buf, buf_size);                      \
        *p++ = c;                                          \
    }                                                      \
} while (0)

#define append_eol(p, buf, buf_size) do {                  \
    if (p - buf > 1 && p[-1] != 10) {                      \
        check_buf(p - buf, buf_size);                      \
        *p++ = 10;                                         \
    }                                                      \
    if (p - buf > 2 && p[-2] == 32) {                      \
        p[-2] = 10;                                        \
        p--;                                               \
    }                                                      \
    *p = 0;                                                \
} while (0)

#define read_field(r, q, buf) do {                         \
    for (q = buf; *r != 32 && *r != 10; *q++ = *r++);      \
    *q = 0;                                                \
    if (*r == 32)                                          \
        r++;                                               \
} while (0)

#define ttf_alloc(n, t) ((t *) mymalloc ((n) * sizeof (t)))

#define NMACGLYPHS      258
#define INDEX_OFFSET    256
#define MAX_CHAR_CODE   0xFFFF /* to support UCS-2 */
#define ENC_BUF_SIZE    1024
#define INDEXED_GLYPH_PREFIX    "index"

/*
#define enc_getchar()   xgetc(encfile)
#define enc_eof()       feof(encfile)
*/

#define ttf_skip(n)         getnum(n)
#define print_str(S)    if (S != 0) fprintf(outfile, #S " %s\n", S)
#define print_dimen(N)  if (N != 0) fprintf(outfile, #N " %i\n", (int)get_ttf_funit(N))

#define get_ttf_funit(n) \
    (n < 0 ? -((-n/upem)*1000 + ((-n%upem)*1000)/upem) :\
    ((n/upem)*1000 + ((n%upem)*1000)/upem))


/*------------------------------------------------------------*/

struct HeadTable {
  TTF_FIXED version;
  TTF_FIXED fontRevision;
  TTF_ULONG checkSumAdjustment;
  TTF_ULONG magicNumber;
  TTF_USHORT flags;
  TTF_USHORT unitsPerEm;
  longDateTime created;
  longDateTime modified;
  TTF_FWORD xMin;
  TTF_FWORD yMin;
  TTF_FWORD xMax;
  TTF_FWORD yMax;
  TTF_USHORT macStyle;
  TTF_USHORT lowestRecPPEM;
  TTF_SHORT fontDirectionHint;
  TTF_SHORT indexToLocFormat;
  TTF_SHORT glyphDataFormat;
};

#define FIX_HeadTable(x) \
  {FIX_Fixed((x).version); FIX_Fixed((x).fontRevision);\
   FIX_UL((x).checkSumAdjustment); FIX_UL((x).magicNumber);\
   FIX_UH((x).flags); FIX_UH((x).unitsPerEm);\
   FIX_UH((x).xMin); FIX_UH((x).yMin); FIX_UH((x).xMax); FIX_UH((x).yMax);\
   FIX_UH((x).macStyle); FIX_UH((x).lowestRecPPEM);\
   FIX_H((x).fontDirectionHint); FIX_H((x).indexToLocFormat);\
   FIX_H((x).glyphDataFormat);}


/*------------------------------------------------------------*/

struct NameRecord {
  TTF_USHORT platformID;
  TTF_USHORT encodingID;
  TTF_USHORT languageID;
  TTF_USHORT nameID;
  TTF_USHORT length;
  TTF_USHORT offset;
};
#define FIX_NameRecord(x) \
  {FIX_UH((x).platformID); FIX_UH((x).encodingID); FIX_UH((x).languageID);\
   FIX_UH((x).nameID); FIX_UH((x).length); FIX_UH((x).offset);}



/*------------------------------------------------------------*/

struct PostTable {
TTF_FIXED formatType;
TTF_FIXED italicAngle;
TTF_FWORD underlinePosition;
TTF_FWORD underlineThickness;
TTF_ULONG isFixedPitch;
TTF_ULONG minMemType42;
TTF_ULONG maxMemType42;
TTF_ULONG minMemType1;
TTF_ULONG maxMemType1;
};
#define FIX_PostTable(x) \
  {FIX_Fixed((x).formatType); FIX_Fixed((x).italicAngle);\
   FIX_H((x).underlinePosition); FIX_H((x).underlineThickness);\
   FIX_UL((x).isFixedPitch);\
   FIX_UL((x).minMemType42); FIX_UL((x).maxMemType42);\
   FIX_UL((x).minMemType1); FIX_UL((x).maxMemType1); }

struct GlyphName {
  int type;
  union {
    int index;
    char *name;
  } name;
};

/*-----------------------------------------------------------------*/
struct HheaTable {
  TTF_FIXED version;
  TTF_FWORD Ascender;
  TTF_FWORD Descender;
  TTF_FWORD LineGap;
  TTF_UFWORD advanceWidthMax;
  TTF_FWORD minLeftSideBearing;
  TTF_FWORD minRightSideBearing;
  TTF_FWORD xMaxExtent;
  TTF_SHORT caretSlopeRise;
  TTF_SHORT caretSlopeRun;
  TTF_SHORT reserved[5];
  TTF_SHORT metricDataFormat;
  TTF_USHORT numberOfHMetrics;
};
#define FIX_HheaTable(x) \
  {FIX_Fixed((x).version); FIX_H((x).Ascender); FIX_H((x).Descender); FIX_H((x).LineGap);\
   FIX_UH((x).advanceWidthMax);\
   FIX_H((x).minLeftSideBearing); FIX_H((x).minRightSideBearing);\
   FIX_H((x).xMaxExtent); FIX_H((x).caretSlopeRise); FIX_H((x).caretSlopeRun);\
   FIX_H((x).metricDataFormat); FIX_UH((x).numberOfHMetrics);}

struct Box {
  TTF_FWORD xMin;
  TTF_FWORD yMin;
  TTF_FWORD xMax;
  TTF_FWORD yMax;
};
#define FIX_Box(x) {FIX_H((x).xMin); FIX_H((x).yMin); FIX_H((x).xMax); FIX_H((x).yMax);}

typedef struct {
  TTF_UFWORD advanceWidth;
  TTF_FWORD lsb;
} longHorMetric;
#define FIX_longHorMetric(x) {FIX_UH((x).advanceWidth); FIX_H((x).lsb);}

/*------------------------------------------------------------*/
struct KernTable {
  TTF_USHORT version;
  TTF_USHORT nTables;
};
#define FIX_KernTable(x) {FIX_UH((x).version); FIX_UH((x).nTables);}

struct KernSubTableHeader {
  TTF_USHORT version;
  TTF_USHORT length;
  TTF_USHORT coverage;
};
#define FIX_KernSubTableHeader(x) \
  {FIX_UH((x).version); FIX_UH((x).length); FIX_UH((x).coverage);}


#define kernHorizontal 0x0001
#define kernMinimum 0x0002
#define kernCrossStream 0x0004
#define kernOverride 0x0008
#define kernFormat(coverage) ((coverage)>>8)

struct KernSubTable0 {
  TTF_USHORT nPairs;
  TTF_USHORT searchRange;
  TTF_USHORT entrySelector;
  TTF_USHORT rangeShift;
};

#define FIX_KernSubTable0(x) \
  {FIX_UH((x).nPairs); FIX_UH((x).searchRange);\
   FIX_UH((x).entrySelector); FIX_UH((x).rangeShift);}

struct KernEntry0 {
  TTF_USHORT left;
  TTF_USHORT right;
  TTF_FWORD value;
};
#define FIX_KernEntry0(x) \
  {FIX_UH((x).left); FIX_UH((x).right); FIX_H((x).value);}


/*------------------------------------------------------------*/
/* Hashtables */

struct hashtable_entry {
  char *key;
  int value;
};

struct hashtable_bucket {
  int size;
  int nentries;
  struct hashtable_entry *entries;
};

struct hashtable {
  int size;
  struct hashtable_bucket **buckets;
};


struct enc_vector {
   char name[20];
   TTF_USHORT vec[256];
};

#endif
