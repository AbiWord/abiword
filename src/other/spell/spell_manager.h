/*
 * Code copyright Dom Lachowicz <cinamod@hotmail.com> 2001
 * Copyrighted under the GNU GPL version 2.0
 * http://www.fsf.org/
 */

#ifndef SPELL_MANAGER_H
#define SPELL_MANAGER_H

#include "ut_types.h"
#include "ut_string.h"
#include "ut_vector.h"
#include "ut_hash.h"

// forward declaration
class SpellManager;

class SpellChecker
{
	friend class SpellManager;
	
public:
	
	typedef enum _SpellCheckResult {
		LOOKUP_SUCCEEDED = 0, // looking up the word succeeded
		LOOKUP_FAILED = 1,    // could not find the word
		LOOKUP_ERROR = 2      // internal error
	} SpellCheckResult;
	
	virtual SpellCheckResult checkWord (const UT_UCSChar * word, size_t len) = 0;
	virtual UT_Vector * suggestWord (const UT_UCSChar * word, size_t len) = 0;
	
protected:
	virtual bool requestDictionary (const char * szLang) = 0;
	
protected:
    SpellChecker ();
    virtual ~SpellChecker ();
};


class SpellManager
{
public:
	
	static SpellManager * instance (void);
	
	virtual ~SpellManager ();
	
	virtual SpellChecker * requestDictionary (const char * szLang);
	virtual SpellChecker * lastDictionary (void) const;

private:
	SpellManager ();

private:
	UT_HashTable m_map;
	SpellChecker * m_lastDict;
};

#endif /* SPELL_MANAGER_H */
