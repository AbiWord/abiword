#ifndef ISPELL_CHECKER_H
#define ISPELL_CHECKER_H

#include "ispell.h"
#include "spell_manager.h"


typedef struct {
  char * dict;
  char * lang;
} Ispell2Lang_t;

// please keep this ordered alphabetically by country-code
static const Ispell2Lang_t m_mapping[] = {
  { "catala.hash",     "ca-ES" },
  { "czech.hash",      "cs-CZ" },
  { "dansk.hash",      "da-DK" },
  { "swiss.hash",      "de-CH" },
  { "deutsch.hash",    "de-DE" },
  { "deutsch.hash",    "de-AT" },
  { "ellhnika.hash",   "el-GR" },
  { "british.hash",    "en-AU" },
  { "british.hash",    "en-CA" },
  { "british.hash",    "en-GB" },
  { "british.hash",    "en-IE" },
  { "british.hash",    "en-NZ" },
  { "american.hash",   "en-US" },
  { "british.hash",    "en-ZA" },
  { "esperanto.hash",  "eo"    },
  { "espanol.hash",    "es-ES" },
  { "espanol.hash",    "es-MX" },
  { "finnish.hash",    "fi-FI" },
  { "francais.hash",   "fr-BE" },
  { "francais.hash",   "fr-CA" },
  { "francais.hash",   "fr-CH" },
  { "francais.hash",   "fr-FR" },
  { "hungarian.hash",  "hu-HU" },
  { "irish.hash",      "ga-IE" },
  { "galician.hash",   "gl-ES" },
  { "italian.hash",    "it-IT" },
  { "mlatin.hash",     "la-IT" },
  { "lietuviu.hash",   "lt-LT" },
  { "nederlands.hash", "nl-NL" },
  { "norsk.hash",      "nb-NO" },
  { "nynorsk.hash",    "nn-NO" },
  { "polish.hash",     "pl-PL" },
  { "portugues.hash",  "pt-PT" },
  { "brazilian.hash",  "pt-BR" },
  { "russian.hash",    "ru-RU" },
  { "slovensko.hash",  "sl-SI" },
  { "svenska.hash",    "sv-SE" },
  { "ukrainian.hash",  "uk-UA" }
};


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

	char * loadGlobalDictionary ( const char *szHash );
	char * loadLocalDictionary ( const char *szHash );
	char * loadDictionaryForLanguage ( const char * szLang );

	/*this is used for converting form unsigned short to UCS-4*/

	int deftflag;              /* NZ for TeX mode by default */
	int prefstringchar;        /* Preferred string character type */
	bool g_bSuccessfulInit;

#if defined(DONT_USE_GLOBALS)
	ispell_state_t	*m_pISpellState;
#endif
};

#endif /* ISPELL_CHECKER_H */
