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

 #ifndef RL_DOCLAYOUT_H
 #define RL_DOCLAYOUT_H

#include "pd_Document.h"
#include "gr_Graphics.h"
#include "rl_ContainerLayout.h"

class rl_DocListener;
class rv_View;
class AV_View;

class ABI_EXPORT rl_DocLayout
{
public:
	rl_DocLayout(PD_Document* doc, GR_Graphics* pG);
	~rl_DocLayout();

	GR_Graphics* getGraphics() { return m_pG; }

	void fillLayouts(void);
	void formatAll(void);

	void add(rl_Layout *pLayout);
	void closeContainer(PTStruxType type, bool backtrack);
	void closeAll();

	rl_ContainerLayout* getCurrentContainer() { return m_pCurrentCL; }
	
	UT_uint32 getNumContainers() { return m_vecContainers.size(); }
	rl_ContainerLayout * getNthContainer(UT_uint32 i) { return static_cast<rl_ContainerLayout*>(m_vecContainers.getNthItem(i)); }
	
	PD_Document* getDocument() { return m_pDoc; }
	rv_View* getView() { return m_pView; }
	void setView(rv_View* pView) { m_pView = pView; }
	
	AV_View* getAvView() { return m_pAvView; }
	void setAvView(AV_View* pView) { m_pAvView = pView; }
	
private:
	PD_Document* m_pDoc;
	GR_Graphics* m_pG;
	rv_View* m_pView;
	AV_View* m_pAvView;
	rl_DocListener* m_pDocListener;

	/** List of all the rl_ContainerLayouts the document consists of */
	UT_GenericVector<rl_Layout *> m_vecContainers;
	rl_ContainerLayout* m_pCurrentCL;
	PL_ListenerId m_lid;
};

#endif /* RL_DOCLAYOUT_H */
