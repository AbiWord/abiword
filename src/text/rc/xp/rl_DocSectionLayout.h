/* AbiWord
 * Copyright (C) 2004 Marc Maurer
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

#ifndef RL_DOCSECTIONLAYOUT_H
#define RL_DOCSECTIONLAYOUT_H

#include "rl_Layout.h"
#include "pt_Types.h"
#include "ut_vector.h"
#include "rl_SectionLayout.h"
#include "rp_Section.h"

class ABI_EXPORT rl_DocSectionLayout : public rl_SectionLayout
{
public:
	rl_DocSectionLayout(PTStruxType type, PL_StruxDocHandle sdh, PT_AttrPropIndex api, rl_ContainerLayout* pParent, rl_DocLayout *pDocLayout);
	virtual ~rl_DocSectionLayout() {}
		
protected:
	virtual	rp_Object* _createObject() { return new rp_Section(this); }
};

#endif /* RL_DOCSECTIONLAYOUT_H */
