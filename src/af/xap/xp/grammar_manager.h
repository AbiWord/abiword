#ifndef GRAMMAR_MANAGER_H
#define GRAMMAR_MANAGER_H

#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_string_class.h"
#include "ut_vector.h"
#include "ut_hash.h"

#include <accidence.h>

class GrammarManager; // forward declaration

struct _GrammarMistake
{
	UT_uint32 iStart;
	UT_uint32 iEnd;
	UT_UCSChar * pExplanation;
	UT_GenericVector<UT_UCSChar*> * m_pSuggestion;
};
typedef struct _GrammarMistake GrammarMistake;

class ABI_EXPORT GrammarChecker
{
	friend class GrammarManager;
		
public:
	~GrammarChecker();

	enum GrammarCheckResult
	{
		CHECK_SUCCEEDED = 	0,
		CHECK_MISTAKES	=	1,
		CHECK_ERROR		=	2
	};

	GrammarCheckResult		checkSentence(const UT_UCSChar* sentence, size_t len);
	UT_uint32	getNMistakes();
	UT_GenericVector<UT_UCSChar*>*	getSuggestion(UT_uint32 nMistake);
	GrammarMistake* getMistake(UT_uint32 nMistake);
	void		purgeMistakes();
	bool 		requestChecker(const char * szLang);

protected:
	GrammarChecker();
	
	void setLanguage (const char * lang)
	{
		m_sLanguage = lang;
	}

	UT_String	m_sLanguage;
	
private:
	void _getMistakes();

	AccidenceChecker *m_checker;
	bool bSentChecked;
	UT_GenericVector<AccidenceMistake *>* m_vecMistake;
};

class ABI_EXPORT GrammarManager
{
public:
	static GrammarManager & instance (void);

	virtual ~GrammarManager ();

	virtual GrammarChecker *lastChecker (void) const;
	virtual GrammarChecker *requestChecker (const char *szLang);
	UT_uint32 numLoadedCheckers () const { return m_nLoadedCheckers; }

	GrammarChecker *	getInstance() const;

private:
	GrammarManager();

	UT_StringPtrMap m_map;
	UT_String m_missingHashs;
	GrammarChecker * m_lastChecker;
	UT_uint32 m_nLoadedCheckers;
	
};

#endif // GRAMMAR_MANAGER_H
