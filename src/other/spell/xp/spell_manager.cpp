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
#include "spell_manager.h"

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
	
	if (checker->requestDictionary (szLang))
    {     
		m_map.insert (szLang, static_cast<void *>(checker));
		checker->setLanguage (szLang);
		m_lastDict = checker;
		m_nLoadedDicts++;
		return checker;
    }
	else
    {
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

SpellChecker *	SpellManager::getInstance()  const 
{
	return  new SpellCheckerClass ();
}

bool SpellChecker::addToCustomDict (const UT_UCSChar *word, size_t len)
{
  // TODO: make this support language tags, subclass this for pspell
  return XAP_App::getApp()->addWordToDict (word, len);
}
