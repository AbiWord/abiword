#define  DONT_USE_GLOBALS

#include "ispell.h"
#include "ut_iconv.h"

#include "sp_spell.h"
#include "ispell_checker.h"
#include "ut_vector.h"

#include "xap_App.h"
#include "ut_string_class.h"

#include "ut_string.h"
#include "ut_debugmsg.h"

// for a silly messagebox
#include <stdio.h>
#include "xap_Frame.h"
#include "xap_Strings.h"

#ifdef HAVE_CURL
#include "ap_HashDownloader.h"
#endif

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
UT_iconv_t  translate_in = (UT_iconv_t)-1; /* Selected translation from/to Unicode */
UT_iconv_t  translate_out = (UT_iconv_t)-1;

/*
 * The following array contains a list of characters that should be tried
 * in "missingletter."  Note that lowercase characters are omitted.
 */
int      Trynum;         /* Size of "Try" array */
ichar_t  Try[SET_SIZE + MAXSTRINGCHARS];

#endif

static void try_autodetect_charset(FIRST_ARG(istate) char* hashname)
{
  int len = 0 ;
  char buf[3000];
  FILE* f;
  if (strlen(hashname)>(3000-15))
    return;
  f = fopen(UT_String_sprintf("%s-%s",hashname,"encoding").c_str(), "r");
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
    DEREF(istate, translate_in) = UT_iconv_open(start, UCS_INTERNAL);
    DEREF(istate, translate_out) = UT_iconv_open(UCS_INTERNAL, start);
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
  if(UT_iconv_isValid (DEREF(m_pISpellState, translate_in) ))
    UT_iconv_close(DEREF(m_pISpellState, translate_in));
  DEREF(m_pISpellState, translate_in) = (UT_iconv_t)-1;
  if(UT_iconv_isValid(DEREF(m_pISpellState, translate_out)))
    UT_iconv_close(DEREF(m_pISpellState, translate_out));
  DEREF(m_pISpellState, translate_out) = (UT_iconv_t)-1;
  
#if defined(DONT_USE_GLOBALS)
  FREEP(m_pISpellState);
#endif
}

SpellChecker::SpellCheckResult
ISpellChecker::checkWord(const UT_UCSChar *word32, size_t length)
{
  SpellChecker::SpellCheckResult retVal;
  ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
  char  word8[INPUTWORDLEN + MAXAFFIXLEN];
  
  if (!g_bSuccessfulInit)
    {
      return SpellChecker::LOOKUP_FAILED;
    }
  
  if (!word32 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
    return SpellChecker::LOOKUP_FAILED;
  
  if(!UT_iconv_isValid(DEREF(m_pISpellState, translate_in)))
    {
      /* copy to 8bit string and null terminate */
      register char *p;
      register size_t x;
      
      for (x = 0, p = word8; x < length; x++)
	*p++ = (unsigned char)*word32++;
      *p = '\0';
    }
  else
    {
      /* convert to 8bit string and null terminate */
      /* TF CHANGE: Use the right types 
	 unsigned int len_in, len_out; 
      */
      size_t len_in, len_out;
      const char *In = (const char *)word32;
      char *Out = word8;
      
      len_in = length * sizeof(UT_UCSChar);
      len_out = sizeof( word8 ) - 1;
      UT_iconv(DEREF(m_pISpellState, translate_in), &In, &len_in, &Out, &len_out);
      *Out = '\0';
    }
  
  if( !strtoichar(DEREF_FIRST_ARG(m_pISpellState) iWord, word8, sizeof(iWord), 0) )
    if ( good(DEREF_FIRST_ARG(m_pISpellState) iWord, 0, 0, 1, 0) == 1 ||
	 compoundgood(DEREF_FIRST_ARG(m_pISpellState) iWord, 1 ) == 1 )
      retVal = SpellChecker::LOOKUP_SUCCEEDED;
    else 
      retVal = SpellChecker::LOOKUP_FAILED;
  else
    retVal = SpellChecker::LOOKUP_ERROR;
  
  return retVal; /* 0 - not found, 1 on found, -1 on error */
}

UT_Vector *
ISpellChecker::suggestWord(const UT_UCSChar *word32, size_t length)
{
  UT_Vector *sgvec = new UT_Vector();
  ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
  char  word8[INPUTWORDLEN + MAXAFFIXLEN];
  int  c;
  
  if (!g_bSuccessfulInit) 
    return 0;
  if (!word32 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
    return 0;
  if (!sgvec)
    return 0;
  
  if(!UT_iconv_isValid(DEREF(m_pISpellState, translate_in)))
    {
      /* copy to 8bit string and null terminate */
      register char *p;
      register size_t x;
      
      for (x = 0, p = word8; x < length; ++x)
	{
	  *p++ = (unsigned char)*word32++;
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
      const char *In = (const char *)word32;
      char *Out = word8;
      len_in = length * sizeof(UT_UCSChar);
      len_out = sizeof( word8 ) - 1;
      UT_iconv(DEREF(m_pISpellState, translate_in), &In, &len_in, &Out, &len_out);
      *Out = '\0';
    }
  
  if( !strtoichar(DEREF_FIRST_ARG(m_pISpellState) iWord, word8, sizeof(iWord), 0) )
    makepossibilities(DEREF_FIRST_ARG(m_pISpellState) iWord);
  
  for (c = 0; c < DEREF(m_pISpellState, pcount); c++) 
    {
      int l;
      
      l = strlen(DEREF(m_pISpellState, possibilities[c]));
      
      UT_UCS4Char *theWord = (UT_UCS4Char*)malloc(sizeof(UT_UCS4Char) * (l + 1));
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
	  char *Out = (char *)theWord;
	  
	  len_in = l;
	  len_out = sizeof(UT_UCS4Char) * (l+1);
	  UT_iconv(DEREF(m_pISpellState, translate_out), &In, &len_in, &Out, &len_out);	    
	  *((UT_UCS4Char *)Out) = 0;
        }
      
      sgvec->addItem((void *)theWord);
    }
  return sgvec;
}

static void couldNotLoadDictionary ( const char * szLang )
{
  XAP_Frame           * pFrame = XAP_App::getApp()->getLastFocussedFrame ();
  
  UT_return_if_fail(pFrame && szLang);
  
  const XAP_StringSet * pSS    = XAP_App::getApp()->getStringSet ();
  
  const char * text = pSS->getValue (XAP_STRING_ID_DICTIONARY_CANTLOAD);
  pFrame->showMessageBox (UT_String_sprintf(text, szLang).c_str(),
			  XAP_Dialog_MessageBox::b_O,
			  XAP_Dialog_MessageBox::a_OK);
}

static char *
s_buildHashName ( const char * base, const char * dict )
{
  UT_String hName ( base ) ;
  hName += "/dictionary/";
  hName += dict ;
  return UT_strdup (hName.c_str());  
}


char *
ISpellChecker::loadGlobalDictionary ( const char *szHash )
{
  char *hashname = NULL;
  hashname = s_buildHashName ( XAP_App::getApp()->getAbiSuiteLibDir(), szHash ) ;
  if (linit(DEREF_FIRST_ARG(m_pISpellState) const_cast<char*>(hashname)) < 0)
  {
    FREEP( hashname );
    return(NULL);
  }
  return(hashname);
}


char *
ISpellChecker::loadLocalDictionary ( const char *szHash )
{
  char *hashname = NULL;
  hashname = s_buildHashName ( XAP_App::getApp()->getUserPrivateDirectory(), szHash ) ;
  if (linit(DEREF_FIRST_ARG(m_pISpellState) const_cast<char*>(hashname)) < 0)
  {
    FREEP( hashname );
    return(NULL);
  }
  return(hashname);
}


char *
ISpellChecker::loadDictionaryForLanguage ( const char * szLang )
{
  char *hashname = NULL;
  char * hFile = NULL ;

  for (UT_uint32 i = 0; i < (sizeof (m_mapping) / sizeof (m_mapping[0])); i++)
    {
      if (!strcmp (szLang, m_mapping[i].lang)) 
	{
	  hFile = m_mapping[i].dict;
	  break;
	}
    }
  
  if ( hFile == NULL )
    return NULL ;

#if defined(DONT_USE_GLOBALS)
  m_pISpellState = alloc_ispell_struct();
#endif
  
  if (!(hashname = loadGlobalDictionary(hFile))) {
    if (!(hashname = loadLocalDictionary(hFile))) {
#ifdef HAVE_CURL
      AP_HashDownloader *hd = (AP_HashDownloader *)XAP_App::getApp()->getHashDownloader();
      XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();

      if (!hd || (hd->suggestDownload(pFrame, szLang) != 1)
      	      || (!(hashname = loadGlobalDictionary(hFile))
              && !(hashname = loadLocalDictionary(hFile))) )
#endif
        return NULL;
    }
  }

  // one of the two above calls succeeded
  return hashname ;
}

bool
ISpellChecker::requestDictionary(const char *szLang)
{
  char * hashname = NULL ;

  hashname = loadDictionaryForLanguage ( szLang ) ;
  if ( !hashname )
    {
      couldNotLoadDictionary ( szLang );
      return false ;
    }

  g_bSuccessfulInit = true;
  
  /* Test for utf8 first */
  prefstringchar = findfiletype(DEREF_FIRST_ARG(m_pISpellState) "utf8", 1, deftflag < 0 ? &deftflag : (int *) NULL);
  if (prefstringchar >= 0)
    {
		DEREF(m_pISpellState, translate_in) = UT_iconv_open("utf-8", UCS_INTERNAL);
		DEREF(m_pISpellState, translate_out) = UT_iconv_open(UCS_INTERNAL, "utf-8");
		
    }
  
    /* Test for "latinN" */
    if(!UT_iconv_isValid(DEREF(m_pISpellState, translate_in)))
    {
      UT_String teststring;
        int n1;

        /* Look for "altstringtype" names from latin1 to latin15 */
        for(n1 = 1; n1 <= 15; n1++)
        {
            UT_String_sprintf(teststring, "latin%u", n1);
            prefstringchar = findfiletype(DEREF_FIRST_ARG(m_pISpellState) teststring.c_str(), 1, deftflag < 0 ? &deftflag : (int *) NULL);
            if (prefstringchar >= 0)
            {
                DEREF(m_pISpellState, translate_in) = UT_iconv_open(teststring.c_str(), UCS_INTERNAL);
																						  DEREF(m_pISpellState, translate_out) = UT_iconv_open(UCS_INTERNAL, teststring.c_str());
                break;
            }
        }
    }
    try_autodetect_charset(DEREF_FIRST_ARG(m_pISpellState) const_cast<char*>(hashname));

    /* Test for known "hashname"s */
    if(!UT_iconv_isValid(DEREF(m_pISpellState, translate_in)))
    {
        if( strstr( hashname, "russian.hash" ))
        {
            /* ISO-8859-5, CP1251 or KOI8-R */
            DEREF(m_pISpellState, translate_in) = UT_iconv_open("KOI8-R", UCS_INTERNAL);
            DEREF(m_pISpellState, translate_out) = UT_iconv_open(UCS_INTERNAL, "KOI8-R");
        }
    }

    /* If nothing found, use latin1 */
    if(!UT_iconv_isValid(DEREF(m_pISpellState, translate_in)))
    {
        DEREF(m_pISpellState, translate_in) = UT_iconv_open("latin1", UCS_INTERNAL);
        DEREF(m_pISpellState, translate_out) = UT_iconv_open(UCS_INTERNAL, "latin1");
    }

    if (prefstringchar < 0)
        DEREF(m_pISpellState, defdupchar) = 0;
    else
        DEREF(m_pISpellState, defdupchar) = prefstringchar;

    FREEP(hashname);
	return true;
}
