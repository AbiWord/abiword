//#include "ispell.h"
#include "ut_iconv.h"

#include "sp_spell.h"
#include "ispell_checker.h"
#include "ut_vector.h"
#include "ut_xml.h"

#include "xap_App.h"
#include "ut_string_class.h"

#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"

// for a silly messagebox
#include <stdio.h>

#ifdef HAVE_CURL
#include "ap_HashDownloader.h"
#endif

#define DICTIONARY_LIST_FILENAME "/dictionary/ispell_dictionary_list.xml"

/***************************************************************************/

class ABI_EXPORT DictionaryListener : public UT_XML::Listener
{
public:

	explicit DictionaryListener ( UT_Vector & wordList )
		: mList ( wordList )
	{
	}

	virtual void startElement (const XML_Char * name, const XML_Char ** atts)
	{
		if (!strcmp (name, "dictionary"))
		{
			DictionaryMapping * mapping = new DictionaryMapping ();

			for (UT_uint32 i = 0; atts[i] != 0; i+=2)
			{
				if (!strcmp(atts[i], "tag"))
					mapping->lang = atts[i+1];
				else if (!strcmp(atts[i], "name"))
					mapping->dict = atts[i+1];
				else if (!strcmp(atts[i], "encoding"))
					mapping->enc = atts[i+1];
			}

			UT_ASSERT(mapping->lang.size() != 0);
			UT_ASSERT(mapping->dict.size() != 0);
			if (mapping->enc.size() == 0)
				mapping->enc = "iso-8859-1";

		mList.push_back (mapping);
		}
	}

	virtual void endElement (const XML_Char * name)
	{
	}

	virtual void charData (const XML_Char * buffer, int length)
	{
	}

private:

	UT_Vector & mList;
};

/***************************************************************************/

void
ISpellChecker::try_autodetect_charset(const UT_String & inEncoding)
{
	if (inEncoding.size() > 0)
    {
		m_translate_in = UT_iconv_open(inEncoding.c_str(), UCS_INTERNAL);
		m_translate_out = UT_iconv_open(UCS_INTERNAL, inEncoding.c_str());
    }
}

/***************************************************************************/

// declare static data
static UT_Vector m_mapping; // vector of DictionaryMapping*
static UT_uint32 mRefCnt = 0;

UT_Vector & ISpellChecker::getMapping()
{
	return m_mapping;
}

/***************************************************************************/

ISpellChecker::ISpellChecker()
  : deftflag(-1),
	prefstringchar(-1),
	m_bSuccessfulInit(false),
	m_BC(NULL),
	m_cd(NULL),
	m_cl(NULL),
	m_cm(NULL),
	m_ho(NULL),
	m_nd(NULL),
	m_so(NULL),
	m_se(NULL),
	m_ti(NULL),
	m_te(NULL),
	m_hashstrings(NULL),
	m_hashtbl(NULL),
	m_pflaglist(NULL),
	m_sflaglist(NULL),
	m_chartypes(NULL),
	m_infile(NULL),
	m_outfile(NULL),
	m_askfilename(NULL),
	m_Trynum(0),
	m_translate_in(UT_ICONV_INVALID),
	m_translate_out(UT_ICONV_INVALID)
{
	memset(m_sflagindex,0,sizeof(m_sflagindex));
	memset(m_pflagindex,0,sizeof(m_pflagindex));
	if (mRefCnt == 0)
	{
		// load the dictionary list
		UT_String dictionary_list ( XAP_App::getApp()->getAbiSuiteLibDir() );
		dictionary_list += DICTIONARY_LIST_FILENAME;

		DictionaryListener listener(m_mapping);
		UT_XML parser;
		parser.setListener (&listener);
		parser.parse (dictionary_list.c_str());
	}

	mRefCnt++;
}

ISpellChecker::~ISpellChecker()
{
	mRefCnt--;
	if (mRefCnt == 0)
	{
		// free the elements
		UT_VECTOR_PURGEALL(DictionaryMapping*, m_mapping);
		m_mapping.clear ();
	}

	lcleanup();

	if (UT_iconv_isValid (m_translate_in ))
		UT_iconv_close(m_translate_in);
	m_translate_in = UT_ICONV_INVALID;
	if (UT_iconv_isValid(m_translate_out))
		UT_iconv_close(m_translate_out);
	m_translate_out = UT_ICONV_INVALID;
}

SpellChecker::SpellCheckResult
ISpellChecker::checkWord(const UT_UCSChar *word32, size_t length)
{
    SpellChecker::SpellCheckResult retVal;
    ichar_t iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char  word8[INPUTWORDLEN + MAXAFFIXLEN];

    if (!m_bSuccessfulInit)
        return SpellChecker::LOOKUP_FAILED;

    if (!word32 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
		return SpellChecker::LOOKUP_FAILED;

    if (m_BarbarismChecker.checkWord (word32, length))
		return SpellChecker::LOOKUP_FAILED; // is a barbarism, should be squiggled

    if (!UT_iconv_isValid(m_translate_in))
		return SpellChecker::LOOKUP_FAILED;
    else
    {
        /* convert to 8bit string and null terminate */
        size_t len_in, len_out;
        const char *In = reinterpret_cast<const char *>(word32);
        char *Out = word8;

        len_in = length * sizeof(UT_UCSChar);
        len_out = sizeof( word8 ) - 1;
        UT_iconv(m_translate_in, &In, &len_in, &Out, &len_out);
        *Out = '\0';
    }

    if (!strtoichar(iWord, word8, sizeof(iWord), 0))
		if (good(iWord, 0, 0, 1, 0) == 1 ||
			compoundgood(iWord, 1) == 1)
		retVal = SpellChecker::LOOKUP_SUCCEEDED;
    else
		retVal = SpellChecker::LOOKUP_FAILED;
    else
		retVal = SpellChecker::LOOKUP_ERROR;

    // Look the word up in the dictionary, return if found
    if (retVal != SpellChecker::LOOKUP_SUCCEEDED)
		if (XAP_App::getApp ()->isWordInDict(word32, length))
			retVal = SpellChecker::LOOKUP_SUCCEEDED;

    return retVal; /* 0 - not found, 1 on found, -1 on error */
}

UT_Vector *
ISpellChecker::suggestWord(const UT_UCSChar *word32, size_t length)
{
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char word8[INPUTWORDLEN + MAXAFFIXLEN];
    int  c;

	if (!m_bSuccessfulInit)
		return 0;
	if (!word32 || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
		return 0;

	if (!UT_iconv_isValid(m_translate_in))
		return 0;
	else
	{
		/* convert to 8bit string and null terminate */

		size_t len_in, len_out;
		const char *In = reinterpret_cast<const char *>(word32);
		char *Out = word8;
		len_in = length * sizeof(UT_UCSChar);
		len_out = sizeof( word8 ) - 1;
		UT_iconv(m_translate_in, &In, &len_in, &Out, &len_out);
		*Out = '\0';
	}

	if (!strtoichar(iWord, word8, sizeof(iWord), 0))
		makepossibilities(iWord);
	else
		return NULL;

	UT_Vector * sgvec = new UT_Vector();

 	// Add suggestions if the word is a barbarism
 	m_BarbarismChecker.suggestWord(word32, length, sgvec);

	for (c = 0; c < m_pcount; c++)
	{
	    int l = strlen(m_possibilities[c]);

	    UT_UCS4Char *theWord = static_cast<UT_UCS4Char*>(malloc(sizeof(UT_UCS4Char) * (l + 1)));
	    if (theWord == NULL)
	    {
			// OOM, but return what we have so far
			return sgvec;
	    }

	    if (m_translate_out == UT_ICONV_INVALID)
	    {
			/* copy to 32bit string and null terminate */
			register int x;

			for (x = 0; x < l; x++)
				theWord[x] = static_cast<unsigned char>(m_possibilities[c][x]);
			theWord[l] = 0;
	    }
	    else
	    {
			/* convert to 32bit string and null terminate */

			size_t len_in, len_out;
			const char *In = m_possibilities[c];
			char *Out = reinterpret_cast<char *>(theWord);

			len_in = l;
			len_out = sizeof(UT_UCS4Char) * (l+1);
			UT_iconv(m_translate_out, &In, &len_in, &Out, &len_out);
			*(reinterpret_cast<UT_UCS4Char *>(Out)) = 0;
	    }

	    sgvec->addItem(static_cast<void *>(theWord));
	}

	return sgvec;
}

static char *
s_buildHashName ( const char * base, const char * dict )
{
	UT_String hName ( base );
	hName += "/dictionary/";
	hName += dict;
	return UT_strdup (hName.c_str());
}


char *
ISpellChecker::loadGlobalDictionary ( const char *szHash )
{
	char *hashname = NULL;
	hashname = s_buildHashName ( XAP_App::getApp()->getAbiSuiteLibDir(), szHash );
	if (linit(const_cast<char*>(hashname)) < 0)
	{
		FREEP( hashname );
		return(NULL);
	}

	m_BarbarismChecker.load(hashname);
	return(hashname);
}


char *
ISpellChecker::loadLocalDictionary ( const char *szHash )
{
	char *hashname = NULL;
	hashname = s_buildHashName ( XAP_App::getApp()->getUserPrivateDirectory(), szHash );
	if (linit(const_cast<char*>(hashname)) < 0)
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
bool
ISpellChecker::loadDictionaryForLanguage ( const char * szLang )
{
	char *hashname = NULL;

	UT_String encoding;
	UT_String szFile;
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
	for (UT_uint32 i = 0; i < m_mapping.size(); i++)
	{
		DictionaryMapping * mapping = static_cast<DictionaryMapping *>(m_mapping.getNthItem ( i ));
		if (mapping->lang.size() && !strcmp (szLang, mapping->lang.c_str()))
		{
			szFile = mapping->dict;
			encoding = mapping->enc;
			break;
		}
	}

	if ( szFile.size () == 0 )
		return false;

	alloc_ispell_struct();

	if (!(hashname = loadGlobalDictionary(szFile.c_str())))
	{
		if (!(hashname = loadLocalDictionary(szFile.c_str())))
		{
#ifdef HAVE_CURL
			AP_HashDownloader *hd = static_cast<AP_HashDownloader *>(XAP_App::getApp()->getHashDownloader());
			XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();

			setUserSaidNo(0);

			if (!hd || ((ret = hd->suggestDownload(pFrame, szLang)) != 1)
				|| (!(hashname = loadGlobalDictionary(szFile.c_str()))
				&& !(hashname = loadLocalDictionary(szFile.c_str()))) )
			{
				if (hd && ret == 0)
					setUserSaidNo(1);
				return false;
			}
#else
			return false;
#endif
		}
	}

	// one of the two above calls succeeded
	setDictionaryEncoding ( hashname, encoding.c_str() );
	return true;
}

void
ISpellChecker::setDictionaryEncoding( const char * hashname, const char * encoding )
{
	/* Get Hash encoding from XML file. This should always work! */
	try_autodetect_charset(encoding);

	if (UT_iconv_isValid(m_translate_in) && UT_iconv_isValid(m_translate_out))
		return; /* success */

	/* Test for UTF-8 first */
	prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag : static_cast<int *>(NULL));
	if (prefstringchar >= 0)
	{
		m_translate_in = UT_iconv_open("UTF-8", UCS_INTERNAL);
		m_translate_out = UT_iconv_open(UCS_INTERNAL, "UTF-8");
	}

	if (UT_iconv_isValid(m_translate_in) && UT_iconv_isValid(m_translate_out))
		return; /* success */

	/* Test for "latinN" */
	if (!UT_iconv_isValid(m_translate_in))
	{
		UT_String teststring;

		/* Look for "altstringtype" names from latin1 to latin15 */
		for(int n1 = 1; n1 <= 15; n1++)
		{
			UT_String_sprintf(teststring, "latin%u", n1);
			prefstringchar = findfiletype(teststring.c_str(), 1,
										  deftflag < 0 ? &deftflag : static_cast<int *>(NULL));
			if (prefstringchar >= 0)
			{
				m_translate_in = UT_iconv_open(teststring.c_str(), UCS_INTERNAL);
				m_translate_out = UT_iconv_open(UCS_INTERNAL, teststring.c_str());
				break;
			}
		}
	}

	/* If nothing found, use latin1 */
	if (!UT_iconv_isValid(m_translate_in))
	{
		m_translate_in = UT_iconv_open("latin1", UCS_INTERNAL);
		m_translate_out = UT_iconv_open(UCS_INTERNAL, "latin1");
	}
}

bool ISpellChecker::doesDictionaryExist (const char * szLang)
{
	char *hashname = NULL;
	UT_String szFile;

	for (UT_uint32 i = 0; i < m_mapping.size(); i++)
	{
		DictionaryMapping * mapping = static_cast<DictionaryMapping *>(m_mapping.getNthItem ( i ));
		if (mapping->lang.size() && !strcmp (szLang, mapping->lang.c_str()))
		{
			szFile = mapping->dict;
			break;
		}
	}

	if (szFile.size () == 0 )
		return false;

	hashname = s_buildHashName ( XAP_App::getApp()->getAbiSuiteLibDir(), szFile.c_str());
	FILE* in = fopen(hashname, "r");

	if (!in)
		return false;
	else
	{
		fclose(in);
		return true;
	}

}

bool
ISpellChecker::requestDictionary(const char *szLang)
{
	if (!loadDictionaryForLanguage ( szLang ))
	{
#ifdef HAVE_CURL
		/*
		 * Don't show this message if user said no or canceled
		 * The information (could not load dictionary) has already been given to
		 * the user in xap_HashDownloader
		 */
		if (!getUserSaidNo())
#endif
			couldNotLoadDictionary ( szLang );
		return false;
	}

	m_bSuccessfulInit = true;

	if (prefstringchar < 0)
		m_defdupchar = 0;
	else
		m_defdupchar = prefstringchar;

	return true;
}

