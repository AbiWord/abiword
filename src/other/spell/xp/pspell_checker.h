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

#include <pspell/pspell.h>
#include "spell_manager.h"

class PSpellChecker : public SpellChecker
{
	friend class SpellManager;

public:
	~PSpellChecker();

	virtual SpellChecker::SpellCheckResult checkWord (const UT_UCSChar * word, size_t len);
	virtual UT_Vector * suggestWord (const UT_UCSChar * word, size_t len);

protected:
	virtual bool requestDictionary (const char * szLang);
	PSpellChecker();

private:
	PspellManager *spell_manager;
};

#endif /* PSPELL_CHECKER_H */
