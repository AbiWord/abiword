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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef ENCHANT_CHECKER_H
#define ENCHANT_CHECKER_H

#include "spell_manager.h"
#ifdef _MSC_VER
typedef long ssize_t;
#endif
#include <enchant.h>

class ABI_EXPORT EnchantChecker : public SpellChecker
{
	friend class SpellManager;

public:

	virtual ~EnchantChecker();

	virtual bool addToCustomDict (const UT_UCSChar *word, size_t len);
	virtual void correctWord (const UT_UCSChar *toCorrect, size_t toCorrectLen,
							  const UT_UCSChar *correct, size_t correctLen);

	virtual void ignoreWord (const UT_UCSChar *toCorrect, size_t toCorrectLen);
	virtual bool isIgnored (const UT_UCSChar * pWord, size_t len) const;

protected:

	EnchantChecker();

private:

	bool _requestDictionary (const char * szLang);
	SpellChecker::SpellCheckResult _checkWord (const UT_UCSChar * word, size_t len);
	UT_GenericVector<UT_UCSChar*> * _suggestWord (const UT_UCSChar * word, size_t len);

	EnchantDict *m_dict;
};

#endif
