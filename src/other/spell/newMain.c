#include <stdlib.h>
#include <string.h>

#include "ispell.h"
#include "sp_spell.h"
#include "iconv.h"


/***************************************************************************/
/* Reduced Gobals needed by ispell code.                                   */
/***************************************************************************/
       int              numhits;
struct success          hits[MAX_HITS];

struct flagptr          pflagindex[SET_SIZE + MAXSTRINGCHARS];/*prefix index*/ 
struct flagent          *pflaglist; /* prefix flag control list */
       int              numpflags;  /* Number of prefix flags in table*/
struct flagptr          sflagindex[SET_SIZE + MAXSTRINGCHARS];/*suffix index*/
struct flagent          *sflaglist; /* suffix flag control list */
       int              numsflags;  /* Number of prefix flags in table*/

struct hashheader       hashheader; /* Header of hash table */
       int              hashsize;   /* Size of main hash table */
       char             *hashstrings = NULL; /* Strings in hash table */
struct dent             *hashtbl;    /* Main hash table, for dictionary */

struct strchartype      *chartypes;  /* String character type collection */
       int              defdupchar;   /* Default duplicate string type */
       unsigned int     laststringch; /* Number of last string character */


char     possibilities[MAXPOSSIBLE][INPUTWORDLEN + MAXAFFIXLEN];
                                /* Table of possible corrections */
int      pcount;         /* Count of possibilities generated */
int      maxposslen;     /* Length of longest possibility */
int      easypossibilities; /* Number of "easy" corrections found */
                                /* ..(defined as those using legal affixes) */

int deftflag = -1;              /* NZ for TeX mode by default */
int prefstringchar = -1;        /* Preferred string character type */
iconv_t  translate_in = (iconv_t)-1; /* Selected translation from/to Unicode */
iconv_t  translate_out = (iconv_t)-1;

/*
 * The following array contains a list of characters that should be tried
 * in "missingletter."  Note that lowercase characters are omitted.
 */
int      Trynum;         /* Size of "Try" array */
ichar_t  Try[SET_SIZE + MAXSTRINGCHARS];



/***************************************************************************/


static int g_bSuccessfulInit = 0;

int SpellCheckInit(char *hashname)
{
    if (linit(hashname) < 0)
    {
        /* TODO gripe -- could not load the dictionary */
        
        return 0;
    }

    g_bSuccessfulInit = 1;

    /* Test for utf8 first */
    prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag : (int *) NULL);
    if (prefstringchar >= 0)
    {
        translate_in = iconv_open("utf-8", "UCS-2-INTERNAL");
        translate_out = iconv_open("UCS-2-INTERNAL", "utf-8");
    }

    /* Test for "latinN" */
    if(translate_in == (iconv_t)-1)
    {
        char teststring[64];
        int n1;

        /* Look for "altstringtype" names from latin1 to latin9 */
        for(n1 = 1; n1 <= 9; n1++)
        {
            sprintf(teststring, "latin%u", n1);
            prefstringchar = findfiletype(teststring, 1, deftflag < 0 ? &deftflag : (int *) NULL);
            if (prefstringchar >= 0)
            {
                translate_in = iconv_open(teststring, "UCS-2-INTERNAL");
                translate_out = iconv_open("UCS-2-INTERNAL", teststring);
                break;
            }
        }
    }

    /* Test for known "hashname"s */
    if(translate_in == (iconv_t)-1)
    {
        if( strstr( hashname, "russian.hash" ) )
        {
            /* ISO-8859-5, CP1251 or KOI8-R */
            translate_in = iconv_open("CP1251", "UCS-2-INTERNAL");
            translate_out = iconv_open("UCS-2-INTERNAL", "CP1251");
        }
    }

    /* If nothing found, use latin1 */
    if(translate_in == (iconv_t)-1)
    {
        translate_in = iconv_open("latin1", "UCS-2-INTERNAL");
        translate_out = iconv_open("UCS-2-INTERNAL", "latin1");
    }

    if (prefstringchar < 0)
        defdupchar = 0;
    else
        defdupchar = prefstringchar;
    
    return 1;
}

void SpellCheckCleanup(void)
{
    lcleanup();
    if(translate_in != (iconv_t)-1)
        iconv_close(translate_in);
    translate_in = (iconv_t)-1;
    if(translate_out != (iconv_t)-1)
        iconv_close(translate_out);
    translate_out = (iconv_t)-1;
}

int SpellCheckNWord16(const unsigned short *word16, int length)
{
    int retVal;
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char  word8[INPUTWORDLEN + MAXAFFIXLEN];

    if (!g_bSuccessfulInit)
        return 1;

    if (!word16 || length >= (INPUTWORDLEN + MAXAFFIXLEN))
        return 0;

    if(translate_in == (iconv_t)-1)
    {
        /* copy to 8bit string and null terminate */
        register char *p;
        register int x;

        for (x = 0, p = word8; x < length; x++)
            *p++ = (unsigned char)*word16++;
        *p = '\0';
    }
    else
    {
        /* convert to 8bit string and null terminate */
        unsigned int len_in, len_out;
        const char *In = (const char *)word16;
        char *Out = word8;

        len_in = length * 2;
        len_out = sizeof( word8 ) - 1;
        iconv(translate_in, &In, &len_in, &Out, &len_out);
        *Out = '\0';
    }
    
/*UT_ASSERT(0);*/
    if( !strtoichar(iWord, word8, sizeof(iWord), 0) )
        retVal = good(iWord, 0, 0, 1, 0);
    else
        retVal = -1;

    return retVal; /* 0 - not found, 1 on found, -1 on error */
}

int SpellCheckSuggestNWord16(const unsigned short *word16, int length, sp_suggestions *sg)
{
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char  word8[INPUTWORDLEN + MAXAFFIXLEN];
    int  c;

    if (!g_bSuccessfulInit) 
        return 0;
    if (!word16 || length >= (INPUTWORDLEN + MAXAFFIXLEN))
        return 0;
    if (!sg) 
        return 0;

    if(translate_in == (iconv_t)-1)
    {
        /* copy to 8bit string and null terminate */
        register char *p;
        register int x;

        for (x = 0, p = word8; x < length; x++)
            *p++ = (unsigned char)*word16++;
        *p = '\0';
    }
    else
    {
        /* convert to 8bit string and null terminate */
        unsigned int len_in, len_out;
        const char *In = (const char *)word16;
        char *Out = word8;

        len_in = length * 2;
        len_out = sizeof( word8 ) - 1;
        iconv(translate_in, &In, &len_in, &Out, &len_out);
        *Out = '\0';
    }
   
    if( !strtoichar(iWord, word8, sizeof(iWord), 0) )
        makepossibilities(iWord);

    sg->count = pcount;
    sg->score = (unsigned short*)malloc(sizeof(unsigned short) * pcount);
    sg->word = (unsigned short**)malloc(sizeof(unsigned short**) * pcount);
    if (sg->score == NULL || sg->word == NULL) 
    {
        sg->count = 0;
        return 0;
    }

    for (c = 0; c < pcount; c++) 
    {
        int l;

        sg->score[c] = 1000;
        l = strlen(possibilities[c]);

        sg->word[c] = (unsigned short*)malloc(sizeof(unsigned short) * l + 2);
        if (sg->word[c] == NULL) 
        {
            /* out of memory, but return what was copied so far */
            sg->count = c;
            return c;
        }

        if(translate_out == (iconv_t)-1)
        {
            /* copy to 16bit string and null terminate */
            register int x;

            for (x = 0; x < l; x++)
                sg->word[c][x] = (unsigned char)possibilities[c][x];
            sg->word[c][l] = 0;
        }
        else
        {
            /* convert to 16bit string and null terminate */
            unsigned int len_in, len_out;
            const char *In = possibilities[c];
            char *Out = (char *)sg->word[c];

            len_in = l;
            len_out = sizeof(unsigned short) * l;
            iconv(translate_out, &In, &len_in, &Out, &len_out);
            *((unsigned short *)Out) = 0;
        }
    }

    return sg->count;
}

