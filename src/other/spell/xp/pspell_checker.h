/* AbiSuite
 * Copyright (C) 2001 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef PSPELL_CHECKER_H
#define PSPELL_CHECKER_H

#if defined(WIN32)
#include <windows.h>
#endif

#include <pspell/pspell.h>
#include "spell_manager.h"

class PSpellChecker : public SpellChecker
{
	friend class SpellManager;

public:
	virtual ~PSpellChecker();

	virtual bool addToCustomDict (const UT_UCSChar *word, size_t len);
	virtual void correctWord (const UT_UCSChar *toCorrect, size_t toCorrectLen,
							  const UT_UCSChar *correct, size_t correctLen);

protected:
	PSpellChecker();

private:

	bool _requestDictionary (const char * szLang);
	SpellChecker::SpellCheckResult _checkWord (const UT_UCSChar * word, size_t len);
	UT_Vector * _suggestWord (const UT_UCSChar * word, size_t len);

#if defined(WIN32)
	// Aspell DLL stubs so we can dynamically link without the stub .lib
	// (which I couldn't get to work for me anyway)
	struct AspellConfig *new_aspell_config()
	{
		typedef struct AspellConfig *(*MYPROC)();
		MYPROC ProcAdd;
		struct AspellConfig *r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "new_aspell_config")))
			r = (ProcAdd) ();
		return r;
	}

	int aspell_config_replace(struct AspellConfig *ths, const char *key, const char *value)
	{
		typedef int (*MYPROC)(struct AspellConfig *ths, const char *key, const char *value);
		MYPROC ProcAdd;
		int r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_config_replace")))
			r = (ProcAdd)(ths, key, value);
		return r;
	}

	void delete_aspell_speller(struct AspellSpeller *ths)
	{
		typedef void (*MYPROC)(struct AspellSpeller *ths);
		MYPROC ProcAdd;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "delete_aspell_speller")))
			(ProcAdd)(ths);
	}

	struct AspellSpeller *to_aspell_speller(struct AspellCanHaveError *obj)
	{
		typedef AspellSpeller *(*MYPROC)(struct AspellCanHaveError *obj);
		MYPROC ProcAdd;
		struct AspellSpeller *r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "to_aspell_speller")))
			r = (ProcAdd)(obj);
		return r;
	}

	const char *aspell_error_message(const struct AspellCanHaveError *ths)
	{
		typedef const char *(*MYPROC)(const struct AspellCanHaveError *ths);
		MYPROC ProcAdd;
		const char *r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_error_message")))
			r = (ProcAdd)(ths);
		return r;
	}

	unsigned int aspell_error_number(const struct AspellCanHaveError *ths)
	{
		typedef int (*MYPROC)(const struct AspellCanHaveError *ths);
		MYPROC ProcAdd;
		int r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_error_number")))
			r = (ProcAdd)(ths);
		return r;
	}

	void delete_aspell_config(struct AspellConfig *ths)
	{
		typedef void (*MYPROC)(struct AspellConfig *ths);
		MYPROC ProcAdd;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "delete_aspell_config")))
			(ProcAdd)(ths);
	}

	struct AspellCanHaveError *new_aspell_speller(struct AspellConfig *config)
	{
		typedef struct AspellCanHaveError *(*MYPROC)(struct AspellConfig *config);
		MYPROC ProcAdd;
		struct AspellCanHaveError *r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "new_aspell_speller")))
			r = (ProcAdd)(config);
		return r;
	}

	int aspell_speller_check(struct AspellSpeller *ths, const char *word, int word_size)
	{
		typedef int (*MYPROC)(struct AspellSpeller *ths, const char *word, int word_size);
		MYPROC ProcAdd;
		int r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_speller_check")))
			r = (ProcAdd)(ths, word, word_size);
		return r;
	}

	void delete_aspell_string_enumeration(struct AspellStringEnumeration *ths)
	{
		typedef void (*MYPROC)(struct AspellStringEnumeration *ths);
		MYPROC ProcAdd;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "delete_aspell_string_enumeration")))
			(ProcAdd)(ths);
	}

	const char *aspell_string_enumeration_next(struct AspellStringEnumeration *ths)
	{
		typedef const char *(*MYPROC)(struct AspellStringEnumeration *ths);
		MYPROC ProcAdd;
		const char *r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_string_enumeration_next")))
			r = (ProcAdd)(ths);
		return r;
	}

	unsigned int aspell_word_list_size(const struct AspellWordList *ths)
	{
		typedef unsigned int (*MYPROC)(const struct AspellWordList *ths);
		MYPROC ProcAdd;
		unsigned int r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_word_list_size")))
			r = (ProcAdd)(ths);
		return r;
	}

	struct AspellStringEnumeration *aspell_word_list_elements(const struct AspellWordList *ths)
	{
		typedef AspellStringEnumeration * (*MYPROC)(const struct AspellWordList *ths);
		MYPROC ProcAdd;
		AspellStringEnumeration *r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_word_list_elements")))
			r = (ProcAdd)(ths);
		return r;
	}

	const struct AspellWordList *aspell_speller_suggest(struct AspellSpeller *ths, const char *word, int word_size)
	{
		typedef const struct AspellWordList *(*MYPROC)(struct AspellSpeller *ths, const char *word, int word_size);
		MYPROC ProcAdd;
		const struct AspellWordList *r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_speller_suggest")))
			r = (ProcAdd)(ths, word, word_size);
		return r;
	}

	int aspell_speller_add_to_personal(struct AspellSpeller *ths, const char *word, int word_size)
	{
		typedef int (*MYPROC)(struct AspellSpeller *ths, const char *word, int word_size);
		MYPROC ProcAdd;
		int r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_speller_add_to_personal")))
			r = (ProcAdd)(ths, word, word_size);
		return r;
	}

	int aspell_speller_store_replacement(struct AspellSpeller *ths, const char *mis, int mis_size, const char *cor, int cor_size)
	{
		typedef int (*MYPROC)(struct AspellSpeller *ths, const char *mis, int mis_size, const char *cor, int cor_size);
		MYPROC ProcAdd;
		int r = 0;
		if (ProcAdd = reinterpret_cast<MYPROC>(GetProcAddress(sm_hinstLib, "aspell_speller_store_replacement")))
			r = (ProcAdd)(ths, mis, mis_size, cor, cor_size);
		return r;
	}

	static HINSTANCE sm_hinstLib;
	static int sm_nDllUseCount;
#endif

	// Note this is a Pspell class unrelated to AbiWord's SpellManager class
	PspellManager *m_pPSpellManager;
};

#endif /* PSPELL_CHECKER_H */
