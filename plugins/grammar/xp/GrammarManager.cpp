#include <vector>
#include <string>

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "grammar_manager.h"


static AccidenceBroker * accidence_broker = 0;

static UT_UCS4Char *
char_to_ucs4(const char *sent)
{
	UT_UCS4Char * ucs4 = 0;
	UT_UCS4_cloneString (&ucs4, UT_UCS4String (sent).ucs4_str());
	return ucs4;
}

#if 0
char * 
ucs4_to_char(const UT_UCSChar *wword)
{
	UT_UCS4String ucs4(wword);
    return g_strdup(ucs4.utf8_str());
}
#endif

GrammarChecker::GrammarChecker()
	: m_checker(0), m_vecMistake(0)
{
	if (!accidence_broker)
		accidence_broker = accidence_broker_init();
}

GrammarChecker::~GrammarChecker()
{
	UT_return_if_fail (accidence_broker);

	/* TODO
	if (m_checker)
		accidence_broker_free_checker (accidence_broker, m_checker, m_sLanguage);
	 */
	
}

GrammarChecker::GrammarCheckResult
GrammarChecker::checkSentence (const UT_UCSChar * ucszSentence, size_t len)
{
	UT_return_val_if_fail (m_checker, GrammarChecker::CHECK_ERROR);
	UT_return_val_if_fail (ucszSentence, GrammarChecker::CHECK_ERROR);
	UT_return_val_if_fail (len, GrammarChecker::CHECK_ERROR);

	accidence_check_status check_status;
	UT_UTF8String utf8Sentence (ucszSentence, len);

	check_status = accidence_checker_check(m_checker, utf8Sentence.utf8_str());

	if (check_status == ACCIDENCE_CHECK_OK)
		return GrammarChecker::CHECK_SUCCEEDED;
		
	else if (check_status == ACCIDENCE_CHECK_MISTAKES)
	{	
		_getMistakes();
		return GrammarChecker::CHECK_MISTAKES;
	}

	return GrammarChecker::CHECK_ERROR;
}


void 
GrammarChecker::_getMistakes ()
{	
	AccidenceMistake* mistakes;
	mistakes = accidence_checker_get_mistakes(m_checker);
	
	m_vecMistake = new UT_GenericVector<AccidenceMistake*>();
	while (mistakes)
	{
		UT_DEBUGMSG(("added!!!\n"));
		m_vecMistake->addItem(mistakes);
		mistakes = mistakes->next;
	}
}

UT_uint32 
GrammarChecker::getNMistakes()
{
	if (!m_vecMistake)
		return 0;
	return m_vecMistake->size();
}

GrammarMistake*
GrammarChecker::getMistake(UT_uint32 nMistake)
{
	UT_return_val_if_fail (m_vecMistake, 0);

	AccidenceMistake *mistake = m_vecMistake->getNthItem(nMistake);
	GrammarMistake *mstk = new GrammarMistake();
	
	mstk->iStart = mistake->pos_start;
	mstk->iEnd = mistake->pos_end;
	mstk->pExplanation = char_to_ucs4(mistake->message);
	mstk->m_pSuggestion = NULL;

	if (mistake->num_suggestions)
	{
		mstk->m_pSuggestion = new UT_GenericVector<UT_UCSChar*>();
		for (size_t i=0; i < mistake->num_suggestions; i++)
		{
			UT_UCS4Char *sugg = NULL;
			sugg = char_to_ucs4(mistake->suggestions[i]);
			if (sugg) 
			{
				mstk->m_pSuggestion->addItem(sugg);
			}
		}
	}

	return mstk;
}


void GrammarChecker::purgeMistakes()
	{
	UT_return_if_fail (m_vecMistake);
	UT_return_if_fail (m_checker);

	if (m_vecMistake->size() < 1)
		return;	

	AccidenceMistake *init_mstk = m_vecMistake->getNthItem(0);
	accidence_checker_free_mistakes(m_checker, init_mstk);

	m_vecMistake->clear();
	}

bool 
GrammarChecker::requestChecker(const char * szLang)
{
	UT_return_val_if_fail(szLang, false);
	UT_return_val_if_fail(accidence_broker, false);

	m_checker = accidence_broker_request_checker(accidence_broker, szLang);

	return (m_checker != NULL);
}

//
//
//

// some arbitrary number for how many language buckets to have by default
#define NBUCKETS 3

GrammarManager::GrammarManager ()
  : m_map (NBUCKETS), m_lastChecker(0), m_nLoadedCheckers(0)
{
	m_missingHashs += "-none-";
}

/*!
 * Destructor
 */
GrammarManager::~GrammarManager ()
{
	UT_GenericVector<void const *> *pVec = m_map.enumerate();
	UT_ASSERT(pVec);
	UT_VECTOR_PURGEALL (GrammarChecker *, (*pVec));
	DELETEP(pVec);
}

/*!
 * If an instance already exists it will be returned for you.
 * If not, one is created and returned for you
 * \return A valid instance of the GrammarManager class
 */
/* static */ GrammarManager &
GrammarManager::instance ()
{
	// Singleton implementation
	static GrammarManager s_instance;
	return s_instance;
}

/*!
 * Request an instance of a dictionary capable of checking the language
 * Represented by szLang
 *
 * \param szLang -  The language tag ("en-US") we want to use
 * \return A valid GrammarChecker for 'szLang' on success, or 0 on failure
 */
GrammarChecker *
GrammarManager::requestChecker (const char * szLang)
{
	GrammarChecker * checker = 0;

	// Don't try to load hashes we know are missing
	if (strstr(m_missingHashs.c_str(), szLang))
		return 0;

	// first look up the entry in the hashtable
	if (m_map.contains (szLang, 0))
	{
		return static_cast<GrammarChecker *>(const_cast<void *>(m_map.pick (szLang)));
	}
	
	// not found, so insert it
	checker = new GrammarChecker ();

	checker->setLanguage (szLang);

	if (checker->requestChecker (szLang))
    {
		m_map.insert (szLang, static_cast<void *>(checker));
		m_lastChecker = checker;
		m_nLoadedCheckers++;
		//checker->setDictionaryFound(true);
    }
	else
    {
		//checker->setDictionaryFound(false);
		m_missingHashs += szLang;
		delete checker;
		checker = NULL;
    }

	return checker;
}

/*!
 * Returns the last used dictionary, or 0
 * \return The last valid GrammarChecker or 0
 */
GrammarChecker *
GrammarManager::lastChecker () const
{
	return m_lastChecker;
}

GrammarChecker *
GrammarManager::getInstance() const
{
	return new GrammarChecker ();
}
