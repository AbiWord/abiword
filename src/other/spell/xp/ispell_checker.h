#ifndef ISPELL_CHECKER_H
#define ISPELL_CHECKER_H

#include "ispell.h"
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

	/*this is used for converting form unsigned short to UCS-2*/
	unsigned short  ucs2[INPUTWORDLEN + MAXAFFIXLEN];

	int deftflag;              /* NZ for TeX mode by default */
	int prefstringchar;        /* Preferred string character type */
	bool g_bSuccessfulInit;

#if defined(DONT_USE_GLOBALS)
	ispell_state_t	*m_pISpellState;
#endif
};

#endif /* ISPELL_CHECKER_H */
