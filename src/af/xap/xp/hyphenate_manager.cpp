/* AbiSuite
 * Copyright (C) 2001 ChenXiajian <chenxiajian1985@gmail.com> 
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Strings.h"
#include "spell_manager.h"
#include "ut_debugmsg.h"
#include "ut_Language.h"

#ifdef WITH_ENCHANT
#include "enchant_Hyphenation.h"
typedef EnchantHyphenation HyphenationClass;
#else
#include "ispell_checker.h"
typedef IHyphenation HyphenationClass;
#endif

/*!
 * Abstract constructor
 */
/* protected */ Hyphenation::Hyphenation ()
{
	// not used, abstract base class
}

/*!
 * Abstract destructor
 */
/* protected */ Hyphenation::~Hyphenation ()
{
	// not used, abstract base class
}

bool Hyphenation::requestDictionary (const char * szLang)
{
	bool bSuccess = _requestDictionary(szLang);

	return bSuccess;
}


UT_GenericVector<UT_UCSChar*>* Hyphenation::hyphenateWord(const UT_UCSChar* word, size_t len)
{
	UT_GenericVector<UT_UCSChar*> *pvSugg = __hyphenateWord(word, len);

	return pvSugg;
}

bool Hyphenation::addToCustomDict (const UT_UCSChar *word, size_t len)
{
	return XAP_App::getApp()->addWordToDict (word, len);
}


