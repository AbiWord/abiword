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

/***********************************************************************/
/***********************************************************************/

// some arbitrary number for how many language buckets to have by default
#define NBUCKETS 5

/*!
 * Protected constructor
 *
 * This (Singleton) class is responsible for creating, handing out, 
 * and destroying instances of the ISpellChecker class
 */
/* private */ SpellManager::SpellManager ()
	: m_map (NBUCKETS), m_lastDict(0)
{
}

/*!
 * Destructor
 */
SpellManager::~SpellManager ()
{
	UT_HASH_PURGEDATA (SpellCheckerClass *, m_map);
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
	
	// first look up the entry in the hashtable
	UT_HashEntry * pEntry = m_map.findEntry (szLang);
	if (pEntry)
		return (SpellCheckerClass *)pEntry->pData;
	
	// not found, so insert it
	checker = new SpellCheckerClass ();
	
	if (checker->requestDictionary (szLang))
    {      
		m_map.addEntry (szLang, 0, checker);
		m_lastDict = checker;
		return checker;
    }
	else
    {
		m_map.addEntry (szLang, 0, 0); // add a null entry for this lang
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
