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

#ifndef RL_TABLELAYOUT_H
#define RL_TABLELAYOUT_H

#include "rl_Layout.h"
#include "pt_Types.h"
#include "ut_vector.h"
#include "rl_SectionLayout.h"
#include "rp_Table.h"

class ABI_EXPORT rl_TableLayout : public rl_SectionLayout
{
public:
	rl_TableLayout(PTStruxType type, PL_StruxDocHandle sdh, rl_ContainerLayout* pParent, rl_DocLayout* pDocLayout);
	virtual ~rl_TableLayout() {}
		
protected:
	virtual	rp_Object* _createObject() { return new rp_Table(this); }
};

class ABI_EXPORT rl_CellLayout : public rl_SectionLayout
{
public:
	rl_CellLayout(PTStruxType type, PL_StruxDocHandle sdh, rl_ContainerLayout* pParent, rl_DocLayout* pDocLayout);
	virtual ~rl_CellLayout() {}
		
protected:
	virtual	rp_Object* _createObject() { return new rp_Cell(this); }
};

#endif /* RL_TABLELAYOUT_H */
