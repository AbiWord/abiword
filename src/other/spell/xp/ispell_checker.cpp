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

/*!
 * Read encoding file which must accompany dictionary hash file
 * The filename must be identical to the hash name with "-encoding"
 * appended.  The file must contain only an encoding name in ASCII
 * suitable for use with iconv.
 * 
 * \param hashname Name of spelling hash file
 */
static void
s_try_autodetect_charset(ispell_state_t *istate, char* hashname)
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
		istate->translate_in = UT_iconv_open(start, UCS_INTERNAL);
		istate->translate_out = UT_iconv_open(UCS_INTERNAL, start);
	}
}

/***************************************************************************/

ISpellChecker::ISpellChecker()
  : deftflag(-1), prefstringchar(-1), m_bSuccessfulInit(false)
{
	m_pISpellState = NULL;
}

ISpellChecker::~ISpellChecker()
{
	if (!m_pISpellState)
		return;

	lcleanup(m_pISpellState);

	if(UT_iconv_isValid (m_pISpellState->translate_in ))
		UT_iconv_close(m_pISpellState->translate_in);
	m_pISpellState->translate_in = (UT_iconv_t)-1;
	if(UT_iconv_isValid(m_pISpellState->translate_out))
		UT_iconv_close(m_pISpellState->translate_out);
	m_pISpellState->translate_out = (UT_iconv_t)-1;

	FREEP(m_pISpellState);
}

SpellChecker::SpellCheckResult
ISpellChecker::checkWord(const UT_UCSChar *word32, size_t length)
{
    SpellChecker::SpellCheckResult retVal;
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char  word8[INPUTWORDLEN + MAXAFFIXLEN];

    if (!m_bSuccessfulInit)
    {
        return SpellChecker::LOOKUP_FAILED;
    }

    if (!word32 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
		return SpellChecker::LOOKUP_FAILED;

	/* TODO get rid of this heuristic - for if we don't have
	 * iconv support for latin1 we've got bigger problems -
	 * if we convert to 8 bit this way for other encodings we're only
	 * generating garbage which may silently introduce errors
	 * and it will make it harder to find the cause
	 */
	if(!UT_iconv_isValid(m_pISpellState->translate_in))
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
        UT_iconv(m_pISpellState->translate_in, &In, &len_in, &Out, &len_out);
        *Out = '\0';
    }

	if( !strtoichar(m_pISpellState, iWord, word8, sizeof(iWord), 0) )
		if ( good(m_pISpellState, iWord, 0, 0, 1, 0) == 1 ||
		 compoundgood(m_pISpellState, iWord, 1 ) == 1 )
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

	if (!m_bSuccessfulInit)
		return 0;
	if (!word32 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
		return 0;
	if (!sgvec)
		return 0;

	/* TODO get rid of this heuristic - for if we don't have
	 * iconv support for latin1 we've got bigger problems -
	 * if we convert to 8 bit this way for other encodings we're only
	 * generating garbage which may silently introduce errors
	 * and it will make it harder to find the cause
	 */
	if(!UT_iconv_isValid(m_pISpellState->translate_in))
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
		UT_iconv(m_pISpellState->translate_in, &In, &len_in, &Out, &len_out);
		*Out = '\0';
    }

	if( !strtoichar(m_pISpellState, iWord, word8, sizeof(iWord), 0) )
		makepossibilities(m_pISpellState, iWord);

	for (c = 0; c < m_pISpellState->pcount; c++)
    {
		int l;

		l = strlen(m_pISpellState->possibilities[c]);

		UT_UCS4Char *theWord = (UT_UCS4Char*)malloc(sizeof(UT_UCS4Char) * (l + 1));
		if (theWord == NULL)
        {
		    // OOM, but return what we have so far
		    return sgvec;
        }

		if (m_pISpellState->translate_out == (iconv_t)-1)
        {
			/* copy to 16bit string and null terminate */
			register int x;

			for (x = 0; x < l; x++)
				theWord[x] = (unsigned char)m_pISpellState->possibilities[c][x];
			theWord[l] = 0;
        }
		else
        {
			/* convert to 16bit string and null terminate */
			/* TF CHANGE: Use the right types
			 unsigned int len_in, len_out;
			*/
			size_t len_in, len_out;
			const char *In = m_pISpellState->possibilities[c];
			char *Out = (char *)theWord;

			len_in = l;
			len_out = sizeof(UT_UCS4Char) * (l+1);
			UT_iconv(m_pISpellState->translate_out, &In, &len_in, &Out, &len_out);
			*((UT_UCS4Char *)Out) = 0;
		}

		sgvec->addItem((void *)theWord);
    }
	return sgvec;
}

static void
s_couldNotLoadDictionary ( const char * szLang )
{
	XAP_Frame           * pFrame = XAP_App::getApp()->getLastFocussedFrame ();

#if 0
	// this invariably happens at start up, when there is no frame yet,
	// and generates irritating assert ...
	UT_return_if_fail(pFrame && szLang);
#else
	UT_return_if_fail(szLang);
#endif

	if(pFrame)
	{

		const XAP_StringSet * pSS    = XAP_App::getApp()->getStringSet ();

		const char * text = pSS->getValue (XAP_STRING_ID_DICTIONARY_CANTLOAD);
		pFrame->showMessageBox (UT_String_sprintf(text, szLang).c_str(),
							XAP_Dialog_MessageBox::b_O,
							XAP_Dialog_MessageBox::a_OK);
	}
	else
	{
		// TODO -- create a dialog not bound to a frame
		UT_DEBUGMSG(( "ispell_checker::s_couldNotLoadDictionary: could not load dictionary for %s\n", szLang ));
	}
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
	if (linit(m_pISpellState, const_cast<char*>(hashname)) < 0)
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
	if (linit(m_pISpellState, const_cast<char*>(hashname)) < 0)
	{
		FREEP( hashname );
		return(NULL);
	}
	return(hashname);
}


/*!
 * Load ispell dictionary hash file for given language.
 * 
 * \param szLang -  The language tag ("en-US") we want to use
 * \return The name of the dictionary file
 */
char *
ISpellChecker::loadDictionaryForLanguage ( const char * szLang )
{
	char *hashname = NULL;
	const char * szFile = NULL ;
#ifdef HAVE_CURL
	UT_sint32 ret;
#endif

	/* TODO
	 * Add support for "deterministic dictionary names"
	 * As well as the dictionary name mapping table, we should
	 * look for dictionaries with names of the form: en-US.hash
	 * This makes it trivial to add new spelling support
	 * without modifying the code and recompiling.
	 * Which should have priority when both exist?
	 */
	for (UT_uint32 i = 0; i < (sizeof (m_mapping) / sizeof (m_mapping[0])); i++)
	{
		if (!strcmp (szLang, m_mapping[i].lang))
		{
			szFile = m_mapping[i].dict;
			break;
		}
	}

	if ( szFile == NULL )
		return NULL ;

	m_pISpellState = alloc_ispell_struct();

	if (!(hashname = loadGlobalDictionary(szFile)))
	{
		if (!(hashname = loadLocalDictionary(szFile)))
		{
#ifdef HAVE_CURL
			AP_HashDownloader *hd = (AP_HashDownloader *)XAP_App::getApp()->getHashDownloader();
			XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();

			setUserSaidNo(0);
			  
			if (!hd || ((ret = hd->suggestDownload(pFrame, szLang)) != 1)
			  || (!(hashname = loadGlobalDictionary(szFile))
			  && !(hashname = loadLocalDictionary(szFile))) )
			{
				if (hd && ret == 0)
					setUserSaidNo(1);
				return NULL;
			}
#else
			return NULL;
#endif
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
#ifdef HAVE_CURL
		/* 
		 * Don't show this message if user said no or canceled
		 * The information (could not load dictionary) has already been given to
		 * the user in xap_HashDownloader
		 */
		if (!getUserSaidNo())
#endif
			s_couldNotLoadDictionary ( szLang );
		return false ;
	}

	m_bSuccessfulInit = true;

	/* Test for utf8 first */
	/* TODO get rid of heuristic - use only *-encoding file - more deterministic, less
	 * entropy and confusion */
	prefstringchar = findfiletype(m_pISpellState, "utf8", 1, deftflag < 0 ? &deftflag : (int *) NULL);
	if (prefstringchar >= 0)
	{
		m_pISpellState->translate_in = UT_iconv_open("utf-8", UCS_INTERNAL);
		m_pISpellState->translate_out = UT_iconv_open(UCS_INTERNAL, "utf-8");

	}

	/* Test for "latinN" */
	/* TODO get rid of heuristic - use only *-encoding file - more deterministic, less
	 * entropy and confusion */
	if(!UT_iconv_isValid(m_pISpellState->translate_in))
	{
		UT_String teststring;
		int n1;

		/* Look for "altstringtype" names from latin1 to latin15 */
		for(n1 = 1; n1 <= 15; n1++)
		{
			UT_String_sprintf(teststring, "latin%u", n1);
			prefstringchar = findfiletype(m_pISpellState, teststring.c_str(), 1, deftflag < 0 ? &deftflag : (int *) NULL);
			if (prefstringchar >= 0)
			{
				m_pISpellState->translate_in = UT_iconv_open(teststring.c_str(), UCS_INTERNAL);
				m_pISpellState->translate_out = UT_iconv_open(UCS_INTERNAL, teststring.c_str());
				break;
			}
		}
	}

	/* Get Hash encoding from hash's accompanying *-encoding file */
	/* TODO this should be the only method of finding the hash's encoding- more deterministic, less
	 * entropy and confusion */
	s_try_autodetect_charset(m_pISpellState, const_cast<char*>(hashname));

	/* Test for known "hashname"s */
	/* TODO get rid of heuristic - use only *-encoding file - more deterministic, less
	 * entropy and confusion */
	if(!UT_iconv_isValid(m_pISpellState->translate_in))
	{
		if( strstr( hashname, "russian.hash" ))
		{
			/* ISO-8859-5, CP1251 or KOI8-R */
			m_pISpellState->translate_in = UT_iconv_open("KOI8-R", UCS_INTERNAL);
			m_pISpellState->translate_out = UT_iconv_open(UCS_INTERNAL, "KOI8-R");
		}
	}

	/* If nothing found, use latin1 */
	/* TODO get rid of fallback - use only *-encoding file - more deterministic, less
	 * entropy and confusion */
	if(!UT_iconv_isValid(m_pISpellState->translate_in))
	{
		m_pISpellState->translate_in = UT_iconv_open("latin1", UCS_INTERNAL);
		m_pISpellState->translate_out = UT_iconv_open(UCS_INTERNAL, "latin1");
	}

	if (prefstringchar < 0)
		m_pISpellState->defdupchar = 0;
	else
		m_pISpellState->defdupchar = prefstringchar;

	FREEP(hashname);
	return true;
}
