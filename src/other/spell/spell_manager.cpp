/*
 * Code copyright Dom Lachowicz <cinamod@hotmail.com> 2001
 * Copyrighted under the GNU GPL version 2.0
 * http://www.fsf.org/
 */

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

// Singleton implementation
static SpellManager * s_pInstance = 0;

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
	s_pInstance = 0;
}

/*!
 * If an instance already exists it will be returned for you.
 * If not, one is created and returned for you
 * \return A valid instance of the SpellManager class
 */
/* static */ SpellManager *
SpellManager::instance (void)
{
	if (!s_pInstance)
    {
		s_pInstance = new SpellManager ();
    }
	return s_pInstance;
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
		delete checker;
		return 0;
    }
}

/*!
 * Returns the last used dictionary, or 0
 * \return The last valid SpellChecker or 0
 */
SpellChecker *
SpellManager::lastDictionary (void) const
{
	return m_lastDict;
}
