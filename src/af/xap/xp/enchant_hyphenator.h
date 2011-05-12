/* AbiSuite
 * Copyright (C) 2003 ChenXiajian
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

#ifndef ENCHANT_HYPENATOR_H
#define ENCHANT_HYPENATOR_H

#include "hyphenate_manager.h"
#ifdef _MSC_VER
typedef long ssize_t;
#endif
#include <enchant.h>

class ABI_EXPORT EnchantHypenator : public Hyphenator
{
	friend class HypenatorManager;

public:

	virtual ~EnchantHypenator();
	virtual bool addToCustomDict (const UT_UCSChar *word, size_t len){return true;}


protected:

	EnchantHypenator();

private:

	bool _requestDictionary (const char * szLang){return true;}	
	EnchantDict *m_dict;
};

#endif
