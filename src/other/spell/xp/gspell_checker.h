/* AbiSuite
 * Copyright (C) 2003 Dom Lachowicz
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

#ifndef GSPELL_CHECKER_H
#define GSPELL_CHECKER_H

#include <bonobo.h>
#include <gnome.h>
#include "Spell.h"
#include "spell_manager.h"

class GSpellChecker : public SpellChecker
{
	friend class SpellManager;

public:
	virtual ~GSpellChecker();

	virtual SpellChecker::SpellCheckResult checkWord (const UT_UCSChar * word, size_t len);
	virtual UT_Vector * suggestWord (const UT_UCSChar * word, size_t len);
	virtual bool addToCustomDict (const UT_UCSChar *word, size_t len);
	virtual void correctWord (const UT_UCSChar *toCorrect, size_t toCorrectLen,
							  const UT_UCSChar *correct, size_t correctLen);

protected:
	virtual bool requestDictionary (const char * szLang);
	GSpellChecker();

private:
	GNOME_Spell_Dictionary m_dict;
};

#endif /* GSPELL_CHECKER_H */
