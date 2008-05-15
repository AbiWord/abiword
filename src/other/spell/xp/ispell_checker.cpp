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
#include "ut_hash.h"

// for a silly messagebox
#include <stdio.h>

/***************************************************************************/

class ABI_EXPORT DictionaryListener : public UT_XML::Listener
{
public:

	explicit DictionaryListener ( UT_Vector & wordList )
		: mList ( wordList )
	{
	}

	virtual void startElement (const gchar * name, const gchar ** atts)
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

	virtual void endElement (const gchar * name)
	{
	}

	virtual void charData (const gchar * buffer, int length)
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

void ISpellChecker::ignoreWord (const UT_UCSChar *pWord, size_t len)
{
	UT_ASSERT(m_pIgnoreList);

	char _key[150], *key = _key;
	if (len > 145) key = new char[len + 1];
	UT_UCSChar *copy = new UT_UCSChar[len + 1];

	for (UT_uint32 i = 0; i < len; i++)
	{
		UT_UCSChar currentChar;
		currentChar = pWord[i];
		// convert smart quote apostrophe to ASCII single quote
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		key[i] = static_cast<char>(currentChar);
		copy[i] = currentChar;
	}
	key[len] = 0;
	copy[len] = 0;

	if (!isIgnored(pWord, len))
	{
		// If it's already on the ignored word list, don't add it again.
		// This can happen if you are looking at a longish document.  You
		// "ignore all" a word, but spell-check doesn't get around to removing
		// the squiggles in the background for a while.  Then, you "ignore all"
		// that word (or another instance of it) again, and ka-bloom, the 
		// hash table stuff asserts on a duplicate entry.
		m_pIgnoreList->insert(key, static_cast<void *>(copy));
	}

	if (key != _key) DELETEPV(key);
}

bool ISpellChecker::isIgnored(const UT_UCSChar * pWord, size_t len) const
{
	UT_ASSERT(m_pIgnoreList);

	char _key[150], *key = _key;
	if (len > 145) key = new char[len + 1];

	for (UT_uint32 i = 0; i < len; i++)
	{
		UT_UCSChar currentChar;
		currentChar = pWord[i];
		// convert smart quote apostrophe to ASCII single quote
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		key[i] = static_cast<char>(currentChar);
	}
	key[len] = 0;

	const void * pHE = m_pIgnoreList->pick(key);

	if (key != _key) DELETEPV(key);

	if (pHE != NULL)
		return true;
	else 
		return false;  
}

void ISpellChecker::clearIgnores()
{
	UT_ASSERT(m_pIgnoreList);

	UT_GenericVector<void const *> *pVec = m_pIgnoreList->enumerate();
	UT_ASSERT(pVec);

	UT_uint32 size = pVec->size();

	for (UT_uint32 i = 0; i < size; i++)
	{
		UT_UCSChar * pData = static_cast<UT_UCSChar *>(const_cast<void*>(pVec->getNthItem(i)));
		DELETEPV(pData);
	}

	DELETEP(pVec);
	DELETEP(m_pIgnoreList);
   
	m_pIgnoreList = new UT_StringPtrMap(11);
	UT_ASSERT(m_pIgnoreList);
}

ISpellChecker::ISpellChecker()
  : m_pIgnoreList(new UT_StringPtrMap(11)),
	deftflag(-1),
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
		UT_String dictionary_list;
		if (XAP_App::getApp()->findAbiSuiteLibFile(dictionary_list,"ispell_dictionary_list.xml","dictionary"))
		{
			DictionaryListener listener(m_mapping);
			UT_XML parser;
			parser.setListener (&listener);
			parser.parse (dictionary_list.c_str());
		}
	}
	mRefCnt++;
}

ISpellChecker::~ISpellChecker()
{
	mRefCnt--;
	if (mRefCnt == 0)
	{
		// g_free the elements
		UT_VECTOR_PURGEALL(DictionaryMapping*, m_mapping);
		m_mapping.clear ();
	}

	if (m_bSuccessfulInit)
	{
		// only cleanup our mess if we were successfully initialized
		clearindex (m_pflagindex);
		clearindex (m_sflagindex);		
	}

	FREEP(m_hashtbl);
	FREEP(m_hashstrings);
	FREEP(m_sflaglist);
	FREEP(m_chartypes);	
	
	if (UT_iconv_isValid (m_translate_in ))
		UT_iconv_close(m_translate_in);
	m_translate_in = UT_ICONV_INVALID;
	if (UT_iconv_isValid(m_translate_out))
		UT_iconv_close(m_translate_out);
	m_translate_out = UT_ICONV_INVALID;

	clearIgnores();
	DELETEP(m_pIgnoreList);
}

SpellChecker::SpellCheckResult
ISpellChecker::_checkWord(const UT_UCSChar *ucszWord, size_t length)
{
    SpellChecker::SpellCheckResult retVal;
    ichar_t iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char szWord[INPUTWORDLEN + MAXAFFIXLEN];

	if (isIgnored (ucszWord, length))
		return SpellChecker::LOOKUP_SUCCEEDED;

    if (!m_bSuccessfulInit)
        return SpellChecker::LOOKUP_FAILED;

    if (!ucszWord || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
		return SpellChecker::LOOKUP_FAILED;

    if (!UT_iconv_isValid(m_translate_in))
		return SpellChecker::LOOKUP_FAILED;
    else
    {
        /* convert to 8bit string and null terminate */
        size_t len_in, len_out;
        const char *In = reinterpret_cast<const char *>(ucszWord);
        char *Out = szWord;

        len_in = length * sizeof(UT_UCSChar);
        len_out = sizeof( szWord ) - 1;
        UT_iconv(m_translate_in, &In, &len_in, &Out, &len_out);
        *Out = '\0';
    }

    if (!strtoichar(iWord, szWord, sizeof(iWord), 0))
	{
		if (good(iWord, 0, 0, 1, 0) == 1 ||
			compoundgood(iWord, 1) == 1)
		{
			m_bIsDictionaryWord = true;
			retVal = SpellChecker::LOOKUP_SUCCEEDED;
		}
		else
			retVal = SpellChecker::LOOKUP_FAILED;
	}
    else
		retVal = SpellChecker::LOOKUP_ERROR;

    // Look the word up in the dictionary, return if found
	// TODO user custom dictionary should be read into memory when the ispell
	// TODO hash is read into memory. This is the way the original ispell
	// TODO code works and it means the same algorithm is used for choosing
	// TODO suggestions from either dictionary
    if (retVal != SpellChecker::LOOKUP_SUCCEEDED)
	{
		if (XAP_App::getApp ()->isWordInDict(ucszWord, length))
		{
			m_bIsDictionaryWord = true;
			retVal = SpellChecker::LOOKUP_SUCCEEDED;
		}
	}

    return retVal; /* 0 - not found, 1 on found, -1 on error */
}

UT_GenericVector<UT_UCSChar*>*
ISpellChecker::_suggestWord(const UT_UCSChar *ucszWord, size_t length)
{
    ichar_t  iWord[INPUTWORDLEN + MAXAFFIXLEN];
    char word8[INPUTWORDLEN + MAXAFFIXLEN];
    int  c;

	if (!m_bSuccessfulInit)
		return 0;
	if (!ucszWord || length >= (INPUTWORDLEN + MAXAFFIXLEN) || length == 0)
		return 0;

	if (!UT_iconv_isValid(m_translate_in))
		return 0;
	else
	{
		/* convert to 8bit string and null terminate */

		size_t len_in, len_out;
		const char *In = reinterpret_cast<const char *>(ucszWord);
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

	UT_GenericVector<UT_UCSChar*>* sgvec = new UT_GenericVector<UT_UCSChar*>();

	// Normal spell checker suggests words second
	if (!m_bIsDictionaryWord)
	{
		for (c = 0; c < m_pcount; c++)
		{
			int l = strlen(m_possibilities[c]);

			UT_UCS4Char *ucszSugg = static_cast<UT_UCS4Char*>(g_try_malloc(sizeof(UT_UCS4Char) * (l + 1)));
			if (ucszSugg == NULL)
			{
				// OOM, but return what we have so far
				return sgvec;
			}

			if (!UT_iconv_isValid(m_translate_out))
			{
				/* copy to 32bit string and null terminate */
				register int x;

				for (x = 0; x < l; x++)
					ucszSugg[x] = static_cast<unsigned char>(m_possibilities[c][x]);
				ucszSugg[l] = 0;
			}
			else
			{
				/* convert to 32bit string and null terminate */

				size_t len_in, len_out;
				const char *In = m_possibilities[c];
				char *Out = reinterpret_cast<char *>(ucszSugg);

				len_in = l;
				len_out = sizeof(UT_UCS4Char) * (l+1);
				UT_iconv(m_translate_out, &In, &len_in, &Out, &len_out);
				*(reinterpret_cast<UT_UCS4Char *>(Out)) = 0;
			}

			sgvec->addItem(ucszSugg);
		}
	}

	return sgvec;
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
	UT_String hashname;
	UT_String encoding;
	UT_String szFile;

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
		DictionaryMapping * mapping = static_cast<DictionaryMapping *>(const_cast<void*>(m_mapping.getNthItem ( i )));
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

	if (XAP_App::getApp()->findAbiSuiteLibFile(hashname,szFile.c_str(),"dictionary"))
	{
		if (!(linit(const_cast<char*>(hashname.c_str())) < 0))
		{
			setDictionaryEncoding (hashname.c_str(), encoding.c_str() );
			return true;
		}
	}
	return false;
}

void
ISpellChecker::setDictionaryEncoding( const char * hashname, const char * encoding )
{
	/* Get Hash encoding from XML file. This should always work! */
	try_autodetect_charset(encoding);

	if (UT_iconv_isValid(m_translate_in) && UT_iconv_isValid(m_translate_out))
	{
		/* We still have to setup prefstringchar*/		
		prefstringchar = findfiletype("utf8", 1, deftflag < 0 ? &deftflag : static_cast<int *>(NULL));

		if (prefstringchar < 0)
		{				
			UT_String teststring;
			for(int n1 = 1; n1 <= 15; n1++)
			{
				UT_String_sprintf(teststring, "latin%u", n1);
				prefstringchar = findfiletype(teststring.c_str(), 1,
											  deftflag < 0 ? &deftflag : static_cast<int *>(NULL));

				if (prefstringchar >= 0) break;				
			}	
		}
		
		return; /* success */
	}

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
	UT_String hashname;
	UT_String szFile;

	FILE * in = 0;

	for (UT_uint32 i = 0; i < m_mapping.size(); i++)
	{
		DictionaryMapping * mapping = static_cast<DictionaryMapping *>(const_cast<void*>(m_mapping.getNthItem ( i )));
		if (mapping->lang.size() && !strcmp (szLang, mapping->lang.c_str()))
		{
			szFile = mapping->dict;
			break;
		}
	}

	if (szFile.size () == 0 )
		return false;

	if (XAP_App::getApp()->findAbiSuiteLibFile(hashname,szFile.c_str(),"dictionary"))
	{
		in = fopen(hashname.c_str(), "r");
		if (in) fclose (in);
	}
	return (in != 0);
}

bool
ISpellChecker::_requestDictionary(const char *szLang)
{
	if (!loadDictionaryForLanguage ( szLang ))
	{
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
