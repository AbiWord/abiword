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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "gr_BeOSGraphics.h"
#include "ap_BeOSStatusBar.h"
#include "xap_BeOSFrame.h"

//////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

class StatusBarDrawView: public be_GRDrawView {
public:
 	StatusBarDrawView(AP_BeOSStatusBar *pBar, AV_View *pView, 
			 BRect frame, const char *name,
                         uint32 resizeMask, uint32 flags);
	virtual void FrameResized(float new_width, float new_height);
	virtual void Draw(BRect invalid);
	
private:
	float				m_fOldWidth;
	float				m_fOldHeight;
	AP_BeOSStatusBar  *m_pAPStatusBar;

};


StatusBarDrawView::StatusBarDrawView(AP_BeOSStatusBar *pBar, AV_View *pView, 
				   BRect frame, const char *name,
                                   uint32 resizeMask, uint32 flags) 
		: be_GRDrawView(pView, frame, name, resizeMask, flags) {

	m_pAPStatusBar = pBar;
	m_pAPStatusBar->setHeight(STATUS_BAR_HEIGHT); //this line is the key 
	m_pAPStatusBar->setWidth((int)frame.IntegerWidth()+1);
}

void StatusBarDrawView::FrameResized(float new_width, float new_height) {
	//m_pAPStatusBar->setHeight((int)new_height);
      m_pAPStatusBar->setWidth((int)new_width);

	//TODO does this goes well?? umm.. we'll see later
	BRect r;
	if (new_width > m_fOldWidth)
	{
		r.left=m_fOldWidth-5;
		r.right=new_width-1;
		r.top=Bounds().top;
		r.bottom=Bounds().bottom-1;
		UT_DEBUGMSG(("Actually invalidating StatusBar\n"));
		Invalidate(r);
	}
	m_fOldWidth=new_width;

}

void StatusBarDrawView::Draw(BRect invalid) {

	m_pAPStatusBar->draw();

}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_BeOSStatusBar::AP_BeOSStatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_wStatusBar = NULL;
	m_pG = NULL;
}

AP_BeOSStatusBar::~AP_BeOSStatusBar(void)
{
	DELETEP(m_pG);
}

be_GRDrawView * AP_BeOSStatusBar::createWidget(BRect r)
{

	UT_ASSERT(!m_pG && !m_wStatusBar);
	m_wStatusBar = NULL;
	m_wStatusBar = new StatusBarDrawView(this,m_pView,r,"StatusBar",B_FOLLOW_LEFT_RIGHT | B_FOLLOW_BOTTOM, B_WILL_DRAW | B_FRAME_EVENTS);
 	return m_wStatusBar;

}

void AP_BeOSStatusBar::setView(AV_View * pView)
{
	// We really should allocate m_pG in createWidget(), but
	// unfortunately, the actual window (m_wStatusBar->window)
	// is not created until the frame's top-level window is
	// shown.

	DELETEP(m_pG);	
	UT_ASSERT(m_wStatusBar);

	GR_BeOSGraphics *pG = new GR_BeOSGraphics(m_wStatusBar , m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);
 
	be_Window *pBWin = (be_Window*)((XAP_BeOSFrame *)m_pFrame)->getTopLevelWindow();
	pBWin->Lock();
	m_wStatusBar->SetViewColor(pG->Get3DColor(GR_Graphics::CLR3D_Background));
	pBWin->Unlock();
	
//	pG->init3dColors();
	
	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	// Now that we've initialized the graphics context and
	// installed the GUI font, let the base class do it's
	// think and layout the fields.
	
	AP_StatusBar::setView(pView);
}
