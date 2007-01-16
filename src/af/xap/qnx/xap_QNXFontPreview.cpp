/* AbiSource Application Framework
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#include <Pt.h>
#include "xap_Frame.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"
#include "ut_debugmsg.h"
#include "xap_QNXFontPreview.h"
#include "gr_QNXGraphics.h"

XAP_QNXFontPreview::XAP_QNXFontPreview(XAP_Frame * pFrame, UT_sint32 left, UT_uint32 top)
	: XAP_FontPreview()
{
	int n=0;
	PtArg_t args[5];
	PhArea_t area;

	m_left = left;
	m_top = top;
	area.size.w=m_width;
	area.size.h=m_height;
	area.pos.x=left;
	area.pos.y=top;

	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetArg(&args[n++],Pt_ARG_WINDOW_RENDER_FLAGS,Pt_FALSE,Pt_TRUE);	
	PtSetArg(&args[n++],Pt_ARG_AREA,&area,0);
	PtSetArg(&args[n++],Pt_ARG_FLAGS,Pt_FALSE,Pt_GETS_FOCUS);
	PtSetArg(&args[n++],Pt_ARG_WINDOW_MANAGED_FLAGS,Pt_FALSE,Ph_WM_FOCUS);
	m_pPreviewWindow = PtCreateWidget(PtWindow,parentWindow,n,args);

		n=0;
	PtSetArg(&args[n++],Pt_ARG_DIM,&area.size,0);
	PtSetArg(&args[n++],Pt_ARG_FLAGS,Pt_FALSE,Pt_GETS_FOCUS);
	m_pDrawingArea = PtCreateWidget(PtRaw,m_pPreviewWindow,n,args); 
	PtRealizeWidget(m_pPreviewWindow);

	XAP_App *pApp = pQNXFrameImpl->getFrame()->getApp();
	//m_gc = new GR_QNXGraphics(m_pPreviewWindow,m_pDrawingArea, pApp);
	GR_QNXAllocInfo ai(m_pPreviewWindow,m_pDrawingArea, pApp);
	m_gc = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);
	
	_createFontPreviewFromGC(m_gc, area.size.w,area.size.h);

}

XAP_QNXFontPreview::~XAP_QNXFontPreview(void)
{
	DELETEP(m_gc);
	PtDestroyWidget(m_pPreviewWindow);
}
