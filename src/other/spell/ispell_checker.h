#ifndef ISPELL_CHECKER_H
#define ISPELL_CHECKER_H

#include "spell_manager.h"

class ISpellChecker : public SpellChecker
{
	friend class SpellManager;

public:
	~ISpellChecker();

	virtual SpellCheckResult	checkWord(const UT_UCSChar* word, size_t len);
	virtual UT_Vector*			suggestWord(const UT_UCSChar* word, size_t len);

protected:
	virtual bool requestDictionary (const char * szLang);
	ISpellChecker();

private:
	ISpellChecker(const ISpellChecker&);	// no impl
	void operator=(const ISpellChecker&);	// no impl
};

#endif /* ISPELL_CHECKER_H */
