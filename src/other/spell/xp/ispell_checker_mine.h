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
	
protected:
/***************************************************************************/
/* these used to be the ispell globals, they need to be implemented as members*/
/***************************************************************************/
	UT_sint32				numhits;
	struct success			hits[MAX_HITS];

	struct flagptr			pflagindex[SET_SIZE + MAXSTRINGCHARS];/*prefix index*/
	struct flagent          *pflaglist; /* prefix flag control list */
	UT_sint32				numpflags;  /* Number of prefix flags in table*/
	struct flagptr			sflagindex[SET_SIZE + MAXSTRINGCHARS];/*suffix index*/
	struct flagent          *sflaglist; /* suffix flag control list */
	UT_sint32				numsflags;  /* Number of prefix flags in table*/

	struct hashheader       hashheader; /* Header of hash table */
	UT_sint32				hashsize;   /* Size of main hash table */
	char					*hashstrings = NULL; /* Strings in hash table */
	struct dent             *hashtbl;    /* Main hash table, for dictionary */

	struct strchartype      *chartypes;  /* String character type collection */
	UT_sint32				defdupchar;   /* Default duplicate string type */
	UT_uint32				laststringch; /* Number of last string character */


	char     possibilities[MAXPOSSIBLE][INPUTWORDLEN + MAXAFFIXLEN];
                                /* Table of possible corrections */
	UT_sint32		pcount;         /* Count of possibilities generated */
	UT_sint32		maxposslen;     /* Length of longest possibility */
	UT_sint32		easypossibilities; /* Number of "easy" corrections found */
                                /* ..(defined as those using legal affixes) */

	UT_sint32		deftflag = -1;              /* NZ for TeX mode by default */
	UT_sint32		prefstringchar = -1;        /* Preferred string character type */
	iconv_t			translate_in = (iconv_t)-1; /* Selected translation from/to Unicode */
	iconv_t			translate_out = (iconv_t)-1;

/*
 * The following array contains a list of characters that should be tried
 * in "missingletter."  Note that lowercase characters are omitted.
 */
	UT_sint32		Trynum;         /* Size of "Try" array */
	ichar_t			Try[SET_SIZE + MAXSTRINGCHARS];

/*this is used for converting form unsigned short to UCS-2*/
	UT_uint16  ucs2[INPUTWORDLEN + MAXAFFIXLEN];
	
	UT_sint32 g_bSuccessfulInit = 0;


	UT_sint32 good (ichar_t * w, UT_sint32 ignoreflagbits, UT_sint32 allhits, UT_sint32 pfxopts, UT_sint32 sfxopts);
#ifndef NO_CAPITALIZATION_SUPPORT	
	UT_sint32 cap_ok (ichar_t *word, struct success *hit, UT_sint32 len);
#endif
};

#endif /* ISPELL_CHECKER_H */
