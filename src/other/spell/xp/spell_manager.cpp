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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Strings.h"
#include "spell_manager.h"
#include "ut_debugmsg.h"

// we either use an ispell or pspell based backend

#ifdef HAVE_PSPELL
#include "pspell_checker.h"
typedef PSpellChecker SpellCheckerClass;
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
	SpellChecker::SpellCheckResult ret;

	m_bIsBarbarism = false;
	m_bIsDictionaryWord = false;

    if (m_BarbarismChecker.checkWord (word, len))
	{
		UT_DEBUGMSG(("SPELL:  spell %lx %s barb \"%s\"\n", this, getLanguage().c_str(), UT_UTF8String (word, len).utf8_str()));
		m_bIsBarbarism = true;
	}

	ret = _checkWord(word, len);

	if (ret == SpellChecker::LOOKUP_SUCCEEDED && m_bIsBarbarism)
		ret = SpellChecker::LOOKUP_FAILED;

	return ret;
}

UT_Vector *SpellChecker::suggestWord(const UT_UCSChar* word, size_t len)
{
	UT_Vector *pvSugg = _suggestWord(word, len);

	m_BarbarismChecker.suggestWord(word, len, pvSugg);

	return pvSugg;
}

bool SpellChecker::addToCustomDict (const UT_UCSChar *word, size_t len)
{
	// TODO: make this support language tags, subclass this for pspell
	return XAP_App::getApp()->addWordToDict (word, len);
}

void SpellChecker::correctWord (const UT_UCSChar *toCorrect, size_t toCorrectLen,
								const UT_UCSChar *correct, size_t correctLen)
{
}

/* static */ void SpellChecker::couldNotLoadDictionary ( const char * szLang )
{
	XAP_App             * pApp   = XAP_App::getApp ();
	XAP_Frame           * pFrame = pApp->getLastFocussedFrame ();

	UT_String buf (UT_String_sprintf(pApp->getStringSet ()->getValue (XAP_STRING_ID_SPELL_CANTLOAD_DICT),
									 szLang));
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
	UT_Vector * pVec = m_map.enumerate();
	UT_ASSERT(pVec);
	UT_VECTOR_PURGEALL (SpellCheckerClass *, (*pVec));
	DELETEP(pVec);
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
		return checker;
    }
	else
    {
		checker->setDictionaryFound(false);
		m_missingHashs += szLang;
		delete checker;
		return 0;
    }
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
