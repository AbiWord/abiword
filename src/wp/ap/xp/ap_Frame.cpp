/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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
 

//#include <string.h>
//#include <stdio.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
//#include "ut_vector.h"
//#include "ap_App.h"
#include "ap_Frame.h"
//#include "ap_ViewListener.h"
//#include "ev_EditBinding.h"
//#include "ev_EditEventMapper.h"
//#include "ev_EditMethod.h"
//#include "ev_Menu_Layouts.h"
//#include "ev_Menu_Labels.h"
//#include "ap_LoadBindings.h"
//#include "ap_Menu_Layouts.h"
//#include "ap_Menu_LabelSet.h"
//#include "gr_Graphics.h"
//#include "av_View.h"
//#include "fl_DocLayout.h"
#include "ad_Document.h"
#include "pd_Document.h"


#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/

UT_Bool AP_Frame::loadDocument(const char * szFilename)
{
	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document();
	UT_ASSERT(pNewDoc);
	
	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		s_iUntitled++;
		m_iUntitled = s_iUntitled;
		goto ReplaceDocument;
	}

	if (pNewDoc->readFromFile(szFilename))
		goto ReplaceDocument;
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	delete pNewDoc;
	return UT_FALSE;

ReplaceDocument:
	// NOTE: prior document is bound to m_pDocLayout, which gets discarded by subclass
	m_pDoc = pNewDoc;
	return UT_TRUE;
}

