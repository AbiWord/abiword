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

#include "ap_RevealCodes.h"
#include "gr_Graphics.h"
#include "pd_Document.h"
#include "rv_View.h"
#include "ap_FrameData.h"

AP_RevealCodes::AP_RevealCodes(XAP_Frame* pFrame) :
	m_pFrame(pFrame)
{
}

void AP_RevealCodes::initialize()
{
	GR_Graphics* pRcG = NULL;
	rl_DocLayout* pRcDocLayout = NULL;
	rv_View* pRcView = NULL;
	
	_createViewGraphics(m_pFrame, pRcG, 100);
		
	pRcDocLayout = new rl_DocLayout(static_cast<PD_Document *>(m_pFrame->getCurrentDoc()), pRcG);
	
	pRcView = new rv_View(m_pFrame->getApp(), this, pRcDocLayout);
	
	// FIXME: store m_pRcG in ap_Frame!! - MARCM
	static_cast<AP_FrameData*>(m_pFrame->getFrameData())->m_pRcDocLayout = pRcDocLayout;
	m_pFrame->setRcView(pRcView);
	pRcDocLayout->setAvView(m_pFrame->getCurrentView());
	
	pRcDocLayout->fillLayouts();
	pRcDocLayout->formatAll();
	
	pRcView->draw();
}
