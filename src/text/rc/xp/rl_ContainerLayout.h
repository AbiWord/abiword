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

#ifndef RL_CONTAINTERLAYOUT_H
#define RL_CONTAINTERLAYOUT_H

#include "rl_Layout.h"
#include "pt_Types.h"
#include "ut_vector.h"
#include "rp_ContainerObject.h"

class ABI_EXPORT rl_ContainerLayout : public rl_Layout
{
public:
	rl_ContainerLayout(PTStruxType type, PL_StruxDocHandle sdh, rl_ContainerLayout* pParent, rl_DocLayout *pDocLayout);
	virtual ~rl_ContainerLayout() {}

	void addChild(rl_Layout* pLayout);
	virtual bool isContainer() { return true; }
	
	virtual rp_Object* format(); // FIXME: make pure virtual - NO: make final!!
	virtual void breakAfter(rl_Layout* child);	
	
	rl_Layout* getLastChild();
	
protected:
	virtual	rp_Object* _createObject();

private:
	UT_GenericVector<rl_Layout*> m_vecChildren;
};

#endif /* RL_CONTAINTERLAYOUT_H */
