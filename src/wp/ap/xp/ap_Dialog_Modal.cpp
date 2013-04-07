/* AbiWord
 * Copyright (C) 2011 AbiSource, Inc.
 * Copyright (C) 2011 Ben Martin
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
#include "ut_std_string.h"
#include "ut_debugmsg.h"
#include "ap_Dialog_Modal.h"
#include "ap_Strings.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"
#include "fl_BlockLayout.h"
#include "xap_Frame.h"
#include "pd_Document.h"



AP_Dialog_Modal::AP_Dialog_Modal( XAP_DialogFactory * pDlgFactory,
                                  XAP_Dialog_Id id,
                                  const char * helpUrl )
	: XAP_Dialog_NonPersistent( pDlgFactory, id, helpUrl )
{
}

AP_Dialog_Modal::~AP_Dialog_Modal()
{
}


void
AP_Dialog_Modal::maybeClosePopupPreviewBubbles()
{
    closePopupPreviewBubbles();
}

FV_View*
AP_Dialog_Modal::getView() const
{
    XAP_Frame * pFrame = m_pApp->getLastFocussedFrame();
	if(pFrame == (XAP_Frame *) NULL)
	{
		pFrame = m_pApp->getFrame(0);
	}
    
    if( !pFrame )
        return 0;

    FV_View* pView = (FV_View *)pFrame->getCurrentView();
    return pView;
}


void
AP_Dialog_Modal::closePopupPreviewBubbles()
{
  	FV_View* view = getView();
    m_bubbleBlocker = view->getBubbleBlocker();
}

void
AP_Dialog_Modal::maybeReallowPopupPreviewBubbles()
{
    m_bubbleBlocker = FV_View_BubbleBlocker();
    UT_DEBUGMSG(("AP_Dialog_Modal::maybeReallowPopupPreviewBubbles()\n"));
}

