#ifndef PSPELL_CHECKER_H
#define PSPELL_CHECKER_H

#include <pspell/pspell.h>
#include "spell_manager.h"

class PSpellChecker : public SpellChecker
{
	friend class SpellManager;

public:
	~PSpellChecker();

	virtual SpellCheckResult checkWord (const UT_UCSChar * word, size_t len);
	virtual UT_Vector * suggestWord (const UT_UCSChar * word, size_t len);

protected:
	virtual bool requestDictionary (const char * szLang);
	PSpellChecker();

private:
	PspellManager *spell_manager;
};

#endif /* PSPELL_CHECKER_H */
