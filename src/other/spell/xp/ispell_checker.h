#ifndef ISPELL_CHECKER_H
#define ISPELL_CHECKER_H

#include "ispell.h"
#include "spell_manager.h"
#include "ut_string_class.h"
#include "ut_vector.h"




class ISpellChecker : public SpellChecker
{
	friend class SpellManager;

public:
	~ISpellChecker();

	virtual SpellCheckResult	checkWord(const UT_UCSChar* word, size_t len);
	virtual UT_Vector*			suggestWord(const UT_UCSChar* word, size_t len);

	// vector of DictionaryMapping*
	virtual	UT_Vector & getMapping();
	virtual bool  doesDictionaryExist (const char * szLang);

protected:
	virtual bool requestDictionary (const char * szLang);
	ISpellChecker();

#ifdef HAVE_CURL
	void		setUserSaidNo(UT_uint32 flag) { m_userSaidNo = flag; }
	UT_uint32	getUserSaidNo(void) { return(m_userSaidNo); }
#endif

private:
	ISpellChecker(const ISpellChecker&);	// no impl
	void operator=(const ISpellChecker&);	// no impl

	char * loadGlobalDictionary ( const char *szHash );
	char * loadLocalDictionary ( const char *szHash ) ;

	bool   loadDictionaryForLanguage ( const char * szLang );
	void   setDictionaryEncoding ( const char * hashname, const char * enc ) ;

	/*this is used for converting form unsigned short to UCS-4*/

	int deftflag;              /* NZ for TeX mode by default */
	int prefstringchar;        /* Preferred string character type */
	bool m_bSuccessfulInit;

	ispell_state_t	*m_pISpellState;


#ifdef HAVE_CURL
	UT_uint32		m_userSaidNo;
#endif
};

#endif /* ISPELL_CHECKER_H */
