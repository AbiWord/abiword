/* AbiWord
 * Copyright (C) 2005 Martin Sevior
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "ut_mbtowc.h"

#include "ap_Strings.h"
#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fl_DocLayout.h"
#include "fv_View.h"
#include "gr_EmbedManager.h"
#include "ap_Dialog_Latex.h"

AP_Dialog_Latex::AP_Dialog_Latex(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id)
{
	m_answer = a_OK;
	m_compact = false;
}

AP_Dialog_Latex::~AP_Dialog_Latex(void)
{
}

AP_Dialog_Latex::tAnswer AP_Dialog_Latex::getAnswer(void) const
{
	return m_answer;
}

/*!
 * Use the convert method of the embedmanager to convert from Latex to MathML
 */
bool AP_Dialog_Latex::convertLatexToMathML(void)
{
	UT_ByteBuf From,To;

	From.ins(0,reinterpret_cast<const UT_Byte *>(m_sLatex.utf8_str()),static_cast<UT_uint32>(m_sLatex.size()));
	XAP_Frame * pFrame = getActiveFrame();
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	FL_DocLayout * pLayout = pView->getLayout();
	GR_EmbedManager * pEmbed = pLayout->getEmbedManager("mathml");
	
	if (pEmbed->isDefault())
	{
		return false;
	}

	if (pEmbed->convert(0,From,To))
	{
		m_sMathML.clear();
		UT_UCS4_mbtowc myWC;
		m_sMathML.appendBuf(To, myWC);
		return true;
	}

	return false;
}

void AP_Dialog_Latex::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	gchar * tmp = NULL;
	const UT_uint32 title_width = 100;
	char wName[title_width];
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_Latex_LatexTitle));
        BuildWindowName(wName,static_cast<char*>(tmp),title_width);
	m_sWindowName = wName;
        FREEP(tmp);
	
}

void    AP_Dialog_Latex::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	notifyActiveFrame(getActiveFrame());
}

void  AP_Dialog_Latex::fillLatex(UT_UTF8String & sLatex)
{
        setLatex(sLatex);
	setLatexInGUI();
}

void AP_Dialog_Latex::insertIntoDoc(void)
{
	XAP_Frame * pFrame = getActiveFrame();
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	pView->cmdInsertLatexMath(m_sLatex,m_sMathML, m_compact);
}
