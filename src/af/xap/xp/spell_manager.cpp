/* AbiSuite
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com> 
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Strings.h"
#include "spell_manager.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"

#ifdef WITH_ENCHANT
#include "enchant_checker.h"
typedef EnchantChecker SpellCheckerClass;
#else
#include "ispell_checker.h"
typedef ISpellChecker SpellCheckerClass;
#endif

/*!
 * Abstract constructor
 */
/* protected */ SpellChecker::SpellChecker ()
{
	// not used, abstract base class
}

/*!
 * Abstract destructor
 */
/* protected */ SpellChecker::~SpellChecker ()
{
	// not used, abstract base class
}

bool SpellChecker::requestDictionary (const char * szLang)
{
	bool bSuccess = _requestDictionary(szLang);

	m_BarbarismChecker.load(szLang);

	return bSuccess;
}

SpellChecker::SpellCheckResult SpellChecker::checkWord(const UT_UCSChar* word, size_t len)
{
	UT_return_val_if_fail( word, SpellChecker::LOOKUP_SUCCEEDED );
	
	SpellChecker::SpellCheckResult ret = SpellChecker::LOOKUP_FAILED;

	m_bIsBarbarism = false;
	m_bIsDictionaryWord = false;
	
    if (m_BarbarismChecker.checkWord (word, len))
	{
		UT_DEBUGMSG(("SPELL:  spell %p %s barb \"%s\"\n",
					 this, getLanguage().c_str(), UT_UTF8String (word, len).utf8_str()));
		m_bIsBarbarism = true;
		return SpellChecker::LOOKUP_FAILED;
	}

	// handle hyphenated constructions
	// we split each construction into the constituent words and check each word
	// individually -- if they all pass, we consider the whole phrase good, if any fails,
	// we try the entire hyphenated phrase
	
	const UT_uint32 iMaxWords = 10;
	const UT_UCS4Char * pWords[iMaxWords];
	size_t iWordLengths[iMaxWords];
	UT_uint32 iWordCount = 0;
	const UT_UCS4Char * p;
	UT_uint32 i;

	// initialise the first word to the start of the string we were given
	pWords[0] = word;
	
	for(i = 0, p = word; i < len; ++i, ++p)
	{
		if(*p == '-')
		{
			// calculate langth of this word
			iWordLengths[iWordCount] = p - pWords[iWordCount];

			// store the start of next word
			iWordCount++;
			pWords[iWordCount] = p+1;
		}

		if(iWordCount >= iMaxWords - 1)
			break;
	}

	// compute the length of the last word
	iWordLengths[iWordCount] = len - (pWords[iWordCount] - word);
	
	// NB iWordCount is really 'word count' - 1 after the loop above, hence the <=
	for(i = 0; i <= iWordCount; ++i)
	{
		ret = _checkWord(pWords[i], iWordLengths[i]);
		if(ret == SpellChecker::LOOKUP_FAILED)
			break;
	}

	if(ret == SpellChecker::LOOKUP_SUCCEEDED)
		return ret;

	// try the whole hyphenated phrase ...
	ret = _checkWord(word, len);

	return ret;
}

UT_GenericVector<UT_UCSChar*> *SpellChecker::suggestWord(const UT_UCSChar* word, size_t len)
{
	UT_GenericVector<UT_UCSChar*> *pvSugg = _suggestWord(word, len);

	m_BarbarismChecker.suggestWord(word, len, pvSugg);

	return pvSugg;
}

bool SpellChecker::addToCustomDict (const UT_UCSChar *word, size_t len)
{
	return XAP_App::getApp()->addWordToDict (word, len);
}

void SpellChecker::correctWord (const UT_UCSChar * /*toCorrect*/, size_t /*toCorrectLen*/,
								const UT_UCSChar * /*correct*/, size_t /*correctLen*/)
{
}

/* static */ void SpellChecker::couldNotLoadDictionary ( const char * szLang )
{
	XAP_App             * pApp   = XAP_App::getApp ();
	XAP_Frame           * pFrame = pApp->getLastFocussedFrame ();
	char				szLangName[255];
	UT_Language			lang;
	
	UT_uint32 id = lang.getIndxFromCode(szLang);
	const gchar* pLang  = lang.getNthLangName(id);	
	sprintf(szLangName, "%s [%s]", pLang, szLang); // language name [language_code]

	UT_String buf (UT_String_sprintf(pApp->getStringSet ()->getValue (XAP_STRING_ID_SPELL_CANTLOAD_DICT),
									 szLangName));
	if (pFrame)
		pFrame->showMessageBox (buf.c_str(),
								XAP_Dialog_MessageBox::b_O,
								XAP_Dialog_MessageBox::a_OK);
	else
	{
		UT_DEBUGMSG(("SpellChecker::could not load dictionary for %s\n", szLang));
	}
}

/***********************************************************************/
/***********************************************************************/

// some arbitrary number for how many language buckets to have by default
#define NBUCKETS 3

/*!
 * Protected constructor
 *
 * This (Singleton) class is responsible for creating, handing out,
 * and destroying instances of the ISpellChecker class
 */
/* private */ SpellManager::SpellManager ()
  : m_map (NBUCKETS), m_lastDict(0), m_nLoadedDicts(0)
{
	m_missingHashs += "-none-";
}

/*!
 * Destructor
 */
SpellManager::~SpellManager ()
{
	UT_GenericVector<void const *> *pVec = m_map.enumerate();
	UT_ASSERT(pVec);
	if (pVec) {
		UT_VECTOR_PURGEALL (SpellCheckerClass *, (*pVec));
		DELETEP(pVec);
	}
}

/*!
 * If an instance already exists it will be returned for you.
 * If not, one is created and returned for you
 * \return A valid instance of the SpellManager class
 */
/* static */ SpellManager &
SpellManager::instance ()
{
	// Singleton implementation
	static SpellManager s_instance;
	return s_instance;
}

/*!
 * Request an instance of a dictionary capable of checking the language
 * Represented by szLang
 *
 * \param szLang -  The language tag ("en-US") we want to use
 * \return A valid SpellChecker for 'szLang' on success, or 0 on failure
 */
SpellChecker *
SpellManager::requestDictionary (const char * szLang)
{
	SpellCheckerClass * checker = 0;

	// Don't try to load hashes we know are missing
	if (strstr(m_missingHashs.c_str(), szLang))
		return 0;

	// first look up the entry in the hashtable
	if (m_map.contains (szLang, 0))
	{
		return static_cast<SpellCheckerClass *>(const_cast<void *>(m_map.pick (szLang)));
	}
	
	// not found, so insert it
	checker = new SpellCheckerClass ();

	checker->setLanguage (szLang);

	if (checker->requestDictionary (szLang))
    {
		m_map.insert (szLang, static_cast<void *>(checker));
		m_lastDict = checker;
		m_nLoadedDicts++;
		checker->setDictionaryFound(true);
    }
	else
    {
		checker->setDictionaryFound(false);
		m_missingHashs += szLang;
		delete checker;
		checker = NULL;
    }

	return checker;
}

/*!
 * Returns the last used dictionary, or 0
 * \return The last valid SpellChecker or 0
 */
SpellChecker *
SpellManager::lastDictionary () const
{
	return m_lastDict;
}

SpellChecker *	SpellManager::getInstance() const
{
	return new SpellCheckerClass ();
}
