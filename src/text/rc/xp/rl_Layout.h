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

#ifndef RL_LAYOUT_H
#define RL_LAYOUT_H

#include "pt_Types.h"
#include "rp_Object.h"
#include "fl_Layout.h"

class rl_ContainerLayout;
class rl_DocLayout;

class ABI_EXPORT rl_Layout : public fl_Layout
{
public:
	rl_Layout(PTStruxType type, PL_StruxDocHandle sdh, rl_ContainerLayout* pParent, rl_DocLayout *pDocLayout);
	virtual ~rl_Layout() {}

	virtual bool isContainer() { return false; }
	rl_ContainerLayout* getParent() { return m_pParent; }
	rl_DocLayout* getDocLayout() { return m_pDocLayout; }
	
	virtual rp_Object* format() = 0;
	void setObject(rp_Object* pObject) { m_pObject = pObject; }
	rp_Object* getObject() { return m_pObject; } 
	
	bool breakAfter() { return m_bBreakAfter; }
	
	void setPrev(rl_Layout* pPrev) { m_pPrev = pPrev; }
	void setNext(rl_Layout* pNext) { m_pNext = pNext; }
	
	rl_Layout* getPrev() { return m_pPrev; }
	rl_Layout* getNext() { return m_pNext; }

	bool				getAttrProp(const PP_AttrProp ** ppAP, PP_RevisionAttr ** pRevisions,
									bool bShowRevisions, UT_uint32 iRevisionId,
									bool &bHiddenRevision) const;
	
protected:
	virtual	rp_Object* _createObject() = 0;
	
	bool					m_bBreakAfter;
	
	rl_Layout*				m_pPrev;
	rl_Layout*				m_pNext;
	
private:
	rl_ContainerLayout* m_pParent;
	rl_DocLayout* m_pDocLayout;

	rp_Object* m_pObject;
};

#endif /* RL_LAYOUT_H */
