/* AbiSuite
 * Copyright (C) 2012 chenxiajian <chenxiajian1985@gmail.com> 
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

#ifndef ENCHANT_Hyphenation_H
#define ENCHANT_Hyphenation_H

#include "hyphenate_manager.h"
#ifdef _MSC_VER
typedef long ssize_t;
#endif
#include <enchant.h>
//add two methods in enchant.h and implemented in dll
//ENCHANT_MODULE_EXPORT (int)
//enchant_dict_hyphenation (EnchantDict * dict, const char *const word, ssize_t len);
//ENCHANT_MODULE_EXPORT (char **)
//enchant_dict_hyphenationSuggest (EnchantDict * dict, const char *const word,
//								 ssize_t len, size_t * out_n_suggs);


class ABI_EXPORT EnchantHyphenation : public Hyphenation
{
	friend class HyphenationManager;

public:

	virtual ~EnchantHyphenation();
	virtual bool addToCustomDict (const UT_UCSChar *word, size_t len){return true;}
	Hyphenation::HyphenationResult _hyphenation (const UT_UCSChar * ucszWord, size_t len);
	UT_UCSChar* __hyphenateWord (const UT_UCSChar *ucszWord, size_t len);

protected:

	EnchantHyphenation();

private:

	bool _requestDictionary (const char * szLang){return true;}	
	EnchantDict *m_dict;
};

#endif
