#define  DONT_USE_GLOBALS

#include "ispell.h"
#include "iconv.h"

#include "sp_spell.h"
#include "ispell_checker.h"
#include "ut_vector.h"

#include "xap_App.h"
#include "ut_string_class.h"

#include "ut_debugmsg.h"

#	define UCS_2_INTERNAL "UCS-2"

/***************************************************************************/
/* Reduced Gobals needed by ispell code.                                   */
/***************************************************************************/
#ifndef DONT_USE_GLOBALS
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

#endif

extern "C" {
extern int XAP_EncodingManager__swap_utos, XAP_EncodingManager__swap_stou;
}

/*this one fills ucs2 with values that iconv will treat as UCS-2. */
static void toucs2(const unsigned short *word16, int length, unsigned short *out)
{
	int i = 0;
	const unsigned short* in = word16;
/*	unsigned short* out = ucs2; */
	for(;i<length;++i)
	{
		if (XAP_EncodingManager__swap_utos)
		    out[i] = ((in[i]>>8) & 0xff) | ((in[i]&0xff)<<8);
		else
		    out[i] = in[i];
	}
	out[i]= 0;
}

/*this one copies from 'ucs2' to word16 swapping bytes if necessary */
static void fromucs2(unsigned short *word16, int length, unsigned short *ucs2)
{
	int i = 0;
	unsigned short *in = ucs2;
	unsigned short *out = word16; 
	for(;i<length;++i)
	{
		if (XAP_EncodingManager__swap_stou)
			out[i] = ((in[i]>>8) & 0xff) | ((in[i]&0xff)<<8);
		else
			out[i] = in[i];
	}
	out[i]= 0;
}

static void try_autodetect_charset(FIRST_ARG(istate) char* hashname)
{
	int len;
	char buf[3000];
	FILE* f;
	if (strlen(hashname)>(3000-15))
		return;
	sprintf(buf,"%s-%s",hashname,"encoding");
	f = fopen(buf,"r");
	if (!f)
		return;
	len = fread(buf,1,sizeof(buf),f);
	if (len<=0)
		return;
	buf[len]=0;
	fclose(f);
	{
		char* start, *p = buf;
		while (*p==' ' || *p=='\t' || *p=='\n')
			++p;
		start = p;
		while (!(*p==' ' || *p=='\t' || *p=='\n' || *p=='\0'))
			++p;
		*p = '\0';
		if (!*start) /* empty enc */
			return;
        	DEREF(istate, translate_in) = iconv_open(start, UCS_2_INTERNAL);
        	DEREF(istate, translate_out) = iconv_open(UCS_2_INTERNAL, start);
	}
	
}

/***************************************************************************/

ISpellChecker::ISpellChecker()
  : deftflag(-1), prefstringchar(-1), g_bSuccessfulInit(false)
{
#if defined(DONT_USE_GLOBALS)
	m_pISpellState = NULL;
#endif
}

ISpellChecker::~ISpellChecker()
{
#if defined(DONT_USE_GLOBALS)
  if (!m_pISpellState)
    return;

    lcleanup(m_pISpellState);
#else
    lcleanup();
#endif
    if(DEREF(m_pISpellState, translate_in) != (iconv_t)-1)
        iconv_close(DEREF(m_pISpellState, translate_in));
    DEREF(m_pISpellState, translate_in) = (iconv_t)-1;
    if(DEREF(m_pISpellState, translate_out) != (iconv_t)-1)
        iconv_close(DEREF(m_pISpellState, translate_out));
    DEREF(m_pISpellState, translate_out) = (iconv_t)-1;

#if defined(DONT_USE_GLOBALS)
	//TODO: Free the structure?
#endif
}

SpellChecker::SpellCheckResult
ISpellChecker::checkWord(const UT_UCSChar *word16, size_t length)
{
    SpellChecker::SpellCheckResult retVal;
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char  word8[INPUTWORDLEN + MAXAFFIXLEN];

    if (!g_bSuccessfulInit)
        return SpellChecker::LOOKUP_SUCCEEDED;

    if (!word16 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
        return SpellChecker::LOOKUP_FAILED;

    if(DEREF(m_pISpellState, translate_in) == (iconv_t)-1)
    {
        /* copy to 8bit string and null terminate */
        register char *p;
        register size_t x;

        for (x = 0, p = word8; x < length; x++)
            *p++ = (unsigned char)*word16++;
        *p = '\0';
    }
    else
    {
        /* convert to 8bit string and null terminate */
		/* TF CHANGE: Use the right types 
        unsigned int len_in, len_out; 
		*/
	size_t len_in, len_out;
        const char *In = (const char *)ucs2;
        char *Out = word8;

	toucs2(word16,length, ucs2);
        len_in = length * 2;
        len_out = sizeof( word8 ) - 1;
        iconv(DEREF(m_pISpellState, translate_in), const_cast<ICONV_CONST char **>(&In), &len_in, &Out, &len_out);
        *Out = '\0';
    }
    
/*UT_ASSERT(0);*/
    if( !strtoichar(DEREF_FIRST_ARG(m_pISpellState) iWord, word8, sizeof(iWord), 0) )
        retVal = (good(DEREF_FIRST_ARG(m_pISpellState) iWord, 0, 0, 1, 0) == 1 ? SpellChecker::LOOKUP_SUCCEEDED : SpellChecker::LOOKUP_FAILED);
    else
        retVal = SpellChecker::LOOKUP_ERROR;

    return retVal; /* 0 - not found, 1 on found, -1 on error */
}

UT_Vector *
ISpellChecker::suggestWord(const UT_UCSChar *word16, size_t length)
{
  UT_Vector *sgvec = new UT_Vector();
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char  word8[INPUTWORDLEN + MAXAFFIXLEN];
    int  c;

    if (!g_bSuccessfulInit) 
        return 0;
    if (!word16 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
        return 0;
    if (!sgvec)
      return 0;

    if(DEREF(m_pISpellState, translate_in) == (iconv_t)-1)
    {
        /* copy to 8bit string and null terminate */
        register char *p;
        register size_t x;

        for (x = 0, p = word8; x < length; ++x)
		{
            *p++ = (unsigned char)*word16++;
		}
        *p = '\0';
    }
    else
    {
        /* convert to 8bit string and null terminate */
		/* TF CHANGE: Use the right types
        unsigned int len_in, len_out; 
		*/
        size_t len_in, len_out; 
        const char *In = (const char *)ucs2;
        char *Out = word8;
		toucs2(word16,length, ucs2);	
        len_in = length * 2;
        len_out = sizeof( word8 ) - 1;
        iconv(DEREF(m_pISpellState, translate_in), const_cast<ICONV_CONST char **>(&In), &len_in, &Out, &len_out);
        *Out = '\0';
    }
   
    if( !strtoichar(DEREF_FIRST_ARG(m_pISpellState) iWord, word8, sizeof(iWord), 0) )
        makepossibilities(DEREF_FIRST_ARG(m_pISpellState) iWord);

    for (c = 0; c < DEREF(m_pISpellState, pcount); c++) 
    {
        int l;

        l = strlen(DEREF(m_pISpellState, possibilities[c]));

        UT_UCSChar *theWord = (unsigned short*)malloc(sizeof(unsigned short) * l + 2);
        if (theWord == NULL) 
        {
	  // OOM, but return what we have so far
	  return sgvec;
        }

        if (DEREF(m_pISpellState, translate_out) == (iconv_t)-1)
        {
            /* copy to 16bit string and null terminate */
            register int x;

            for (x = 0; x < l; x++)
                theWord[x] = (unsigned char)DEREF(m_pISpellState, possibilities[c][x]);
            theWord[l] = 0;
        }
        else
        {
            /* convert to 16bit string and null terminate */
			/* TF CHANGE: Use the right types
			unsigned int len_in, len_out; 
			*/
			size_t len_in, len_out; 
            const char *In = DEREF(m_pISpellState, possibilities[c]);
            char *Out = (char *)ucs2;

            len_in = l;
            len_out = sizeof(unsigned short) * l;
            iconv(DEREF(m_pISpellState, translate_out), const_cast<ICONV_CONST char **>(&In), &len_in, &Out, &len_out);	    
            *((unsigned short *)Out) = 0;
			fromucs2(theWord, (unsigned short*)Out-ucs2, ucs2);
        }

		sgvec->addItem((void *)theWord);
    }
	return sgvec;
}

typedef struct {
  char * dict;
  char * lang;
} Ispell2Lang_t;

// please try to keep this ordered alphabetically by country-code
static const Ispell2Lang_t m_mapping[] = {
  { "danish.hash",     "da-DK" },
  { "german.hash",     "de-DE" },
  { "australian.hash", "en-AU" },
  { "canadian.hash",   "en-CA" },
  { "british.hash",    "en-GB" },
  { "american.hash",   "en-US" },
  { "spanish.hash",    "es-ES" },
  { "nynorsk.hash",    "nn-NO" },
  { "french.hash",     "fr-FR" },
  { "lietuviu.hash",   "lt-LT" },
  { "portugal.hash",   "pt-PT" },
  { "russian.hash",    "ru-RU" },
  { "swedish.hash",    "sv-SE"}
};

bool
ISpellChecker::requestDictionary(const char *szLang)
{
        char *hashname = NULL;

	for (int i = 0; i < (sizeof (m_mapping) / sizeof (m_mapping[0])); i++)
	  {
	    if (!strcmp (szLang, m_mapping[i].lang)) {
	      UT_String hName = XAP_App::getApp()->getAbiSuiteLibDir();
	      hName += "/dictionary/";
	      hName += m_mapping[i].dict;
	      hashname = UT_strdup (hName.c_str());
	      break;
	    }
	  }

	if (hashname == NULL) {
	  UT_DEBUGMSG(("DOM: dictionary for lang:%s not found\n", szLang));
	  return false;
	}

#if defined(DONT_USE_GLOBALS)
	m_pISpellState = alloc_ispell_struct();
#endif
    if (linit(DEREF_FIRST_ARG(m_pISpellState) const_cast<char*>(hashname)) < 0)
    {
        /* TODO gripe -- could not load the dictionary */
      UT_DEBUGMSG(("DOM: could not load dictionary (%s, %s)\n", hashname, szLang));
        FREEP(hashname);
        return false;
    }

    UT_DEBUGMSG(("DOM: loaded dictionary (%s %s)\n", hashname, szLang));

    g_bSuccessfulInit = true;

    /* Test for utf8 first */
    prefstringchar = findfiletype(DEREF_FIRST_ARG(m_pISpellState) "utf8", 1, deftflag < 0 ? &deftflag : (int *) NULL);
    if (prefstringchar >= 0)
    {
        DEREF(m_pISpellState, translate_in) = iconv_open("utf-8", UCS_2_INTERNAL);
        DEREF(m_pISpellState, translate_out) = iconv_open(UCS_2_INTERNAL, "utf-8");
    }

    /* Test for "latinN" */
    if(DEREF(m_pISpellState, translate_in) == (iconv_t)-1)
    {
        char teststring[64];
        int n1;

        /* Look for "altstringtype" names from latin1 to latin9 */
        for(n1 = 1; n1 <= 9; n1++)
        {
            sprintf(teststring, "latin%u", n1);
            prefstringchar = findfiletype(DEREF_FIRST_ARG(m_pISpellState) teststring, 1, deftflag < 0 ? &deftflag : (int *) NULL);
            if (prefstringchar >= 0)
            {
                DEREF(m_pISpellState, translate_in) = iconv_open(teststring, UCS_2_INTERNAL);
                DEREF(m_pISpellState, translate_out) = iconv_open(UCS_2_INTERNAL, teststring);
                break;
            }
        }
    }
    try_autodetect_charset(DEREF_FIRST_ARG(m_pISpellState) const_cast<char*>(hashname));

    /* Test for known "hashname"s */
    if(DEREF(m_pISpellState, translate_in) == (iconv_t)-1)
    {
        if( strstr( hashname, "russian.hash" ))
        {
            /* ISO-8859-5, CP1251 or KOI8-R */
            DEREF(m_pISpellState, translate_in) = iconv_open("KOI8-R", UCS_2_INTERNAL);
            DEREF(m_pISpellState, translate_out) = iconv_open(UCS_2_INTERNAL, "KOI8-R");
        }
    }

    /* If nothing found, use latin1 */
    if(DEREF(m_pISpellState, translate_in) == (iconv_t)-1)
    {
        DEREF(m_pISpellState, translate_in) = iconv_open("latin1", UCS_2_INTERNAL);
        DEREF(m_pISpellState, translate_out) = iconv_open(UCS_2_INTERNAL, "latin1");
    }

    if (prefstringchar < 0)
        DEREF(m_pISpellState, defdupchar) = 0;
    else
        DEREF(m_pISpellState, defdupchar) = prefstringchar;

    FREEP(hashname);
	return true;
}
