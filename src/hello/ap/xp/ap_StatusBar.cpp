/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ap_StatusBar.h"
#include "gr_Graphics.h"
#include "xap_Frame.h"
#include "xav_View.h"
#include "ap_Strings.h"
#include "ap_View.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_sb_Field
{
public:
	ap_sb_Field(AP_StatusBar * pSB);
	virtual ~ap_sb_Field(void);
	
	void				setLeftOrigin(UT_uint32 left);

	virtual UT_uint32	getDesiredWidth(void) = 0;
	virtual void		draw(void) = 0;
	virtual void		notify(AV_View * pView, const AV_ChangeMask mask) = 0;
	
protected:
	void				_draw3D(void);
	
	AP_StatusBar *		m_pSB;
	UT_Rect				m_rect3d;
};

ap_sb_Field::ap_sb_Field(AP_StatusBar * pSB)
{
	m_pSB = pSB;
	memset(&m_rect3d,0,sizeof(m_rect3d));
}

ap_sb_Field::~ap_sb_Field(void)
{
}

void ap_sb_Field::setLeftOrigin(UT_uint32 left)
{
	m_rect3d.left	= left;
	m_rect3d.width	= getDesiredWidth();
	m_rect3d.top	= 3;

	UT_uint32 barHeight = m_pSB->getHeight();
	m_rect3d.height	= barHeight - 2*m_rect3d.top;
}

void ap_sb_Field::_draw3D(void)
{
	GR_Graphics * pG = m_pSB->getGraphics();
	
	pG->fillRect(GR_Graphics::CLR3D_Background,
				 m_rect3d.left,m_rect3d.top,
				 m_rect3d.width,m_rect3d.height);

	UT_uint32 l = m_rect3d.left -1;
	UT_uint32 r = l + m_rect3d.width +2;
	UT_uint32 t = m_rect3d.top -1;
	UT_uint32 b = t + m_rect3d.height +2;
	
	pG->setColor3D(GR_Graphics::CLR3D_BevelDown);
	pG->drawLine(l,t, l,b);
	pG->drawLine(l,t, r,t);
	
	pG->setColor3D(GR_Graphics::CLR3D_BevelUp);
	pG->drawLine(l+1,b, r,b);
	pG->drawLine(r,b, r,t);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ap_sb_Field_StatusMessage : public ap_sb_Field
{
public:
	ap_sb_Field_StatusMessage(AP_StatusBar * pSB);
	virtual ~ap_sb_Field_StatusMessage(void);

	virtual UT_uint32	getDesiredWidth(void);
	virtual void		draw(void);
	virtual void		notify(AV_View * pView, const AV_ChangeMask mask);
};

ap_sb_Field_StatusMessage::ap_sb_Field_StatusMessage(AP_StatusBar * pSB)
	: ap_sb_Field(pSB)
{
}

ap_sb_Field_StatusMessage::~ap_sb_Field_StatusMessage(void)
{
}

UT_uint32 ap_sb_Field_StatusMessage::getDesiredWidth(void)
{
	return 300;							// TODO define this somewhere
}

void ap_sb_Field_StatusMessage::draw(void)
{
	_draw3D();

	const UT_UCSChar * szMsg = m_pSB->getStatusMessage();
	UT_uint32 len = UT_UCS_strlen(szMsg);

	if (len)
	{
		GR_Graphics * pG = m_pSB->getGraphics();
		UT_uint32 iFontHeight = pG->getFontHeight();

		UT_uint32 x = m_rect3d.left + 3;
		UT_uint32 y = m_rect3d.top + (m_rect3d.height-iFontHeight)/2;

		pG->setColor3D(GR_Graphics::CLR3D_Foreground);
	
		pG->setClipRect(&m_rect3d);
		pG->drawChars(szMsg,0,len,x,y);
		pG->setClipRect(NULL);
	}
}

void ap_sb_Field_StatusMessage::notify(AV_View * /*pView*/, const AV_ChangeMask /*mask*/)
{
	return;
}


AP_StatusBar::AP_StatusBar(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	m_pView = NULL;
	m_pG = NULL;

	m_dim = DIM_IN;

	m_iWidth = 0;
	m_iHeight = 0;

	m_bInitFields = false;
	
	// really this should be "static const x = 20;" in the
	// class declaration, but MSVC5 can't handle it....
	// (GCC can :-)
	
	s_iFixedHeight = 20;
}

AP_StatusBar::~AP_StatusBar(void)
{
	UT_VECTOR_PURGEALL(ap_sb_Field *, m_vecFields);
}

XAP_Frame * AP_StatusBar::getFrame(void) const
{
	return m_pFrame;
}

GR_Graphics * AP_StatusBar::getGraphics(void) const
{
	return m_pG;
}

void AP_StatusBar::setView(AV_View * pView)
{
	m_pView = pView;

	// Register the StatusBar as a ViewListener on the View.
	// This lets us receive notify events as the user interacts
	// with the document (cmdCharMotion, etc).  This will let
	// us update the display as we move from block to block and
	// from column to column.

	AV_ListenerId lidTopRuler;
	m_pView->addListener(static_cast<AV_Listener *>(this),&lidTopRuler);

	if (!m_bInitFields)
	{
		UT_uint32 xOrigin = 3;
		UT_uint32 xGap = 6;
		
#define DclField(type,var)								\
		type * var = new type(this);					\
		UT_ASSERT((var));								\
		m_vecFields.addItem((var));						\
		(var)->setLeftOrigin(xOrigin);					\
		xOrigin += (var)->getDesiredWidth() + xGap
		
		DclField(ap_sb_Field_StatusMessage, pf1);

		m_pStatusMessageField = pf1;	// its in the vector, but we remember it explicitly
										// so that setStatusMessage() can do its thing.

		// TODO add other fields

#undef DclField
		m_bInitFields = true;
	}

	// force a full notify of all fields so that they all
	// completely update themselves.
	
	notify(pView,AV_CHG_ALL);
	
	return;
}

void AP_StatusBar::setHeight(UT_uint32 iHeight)
{
	m_iHeight = MyMax(iHeight,s_iFixedHeight);
}

UT_uint32 AP_StatusBar::getHeight(void) const
{
	return m_iHeight;
}

void AP_StatusBar::setWidth(UT_uint32 iWidth)
{
	m_iWidth = iWidth;
}

UT_uint32 AP_StatusBar::getWidth(void) const
{
	return m_iWidth;
}

bool AP_StatusBar::notify(AV_View * pView, const AV_ChangeMask mask)
{
	// Handle AV_Listener events on the view.	

	// We choose to clear any status message we may have,
	// since it's a pain for the code which set the message
	// to hang around and clear it at somepoint in the future.
	// This way, message will get cleared anytime the user does
	// something with the window.

	if (*m_bufUCS)
		setStatusMessage((UT_UCSChar *)NULL);
	
	// Let each field on the status bar update itself accordingly.
	
	UT_ASSERT(pView==m_pView);
	UT_uint32 kLimit = m_vecFields.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
	{
		ap_sb_Field * pf = (ap_sb_Field *)m_vecFields.getNthItem(k);
		pf->notify(pView,mask);
	}

	return true;
}

void AP_StatusBar::draw(void)
{
	if (!m_pG)
		return;
	
	// draw the background

	m_pG->fillRect(GR_Graphics::CLR3D_Background,0,0,m_iWidth,m_iHeight);

	// draw the foreground
	
	_draw();
}

void AP_StatusBar::_draw(void)
{
	UT_uint32 kLimit = m_vecFields.getItemCount();
	UT_uint32 k;

	for (k=0; k<kLimit; k++)
	{
		ap_sb_Field * pf = (ap_sb_Field *)m_vecFields.getNthItem(k);
		pf->draw();
	}
}

void AP_StatusBar::setStatusMessage(UT_UCSChar * pBufUCS)
{
	memset(m_bufUCS,0,sizeof(m_bufUCS));

	if (pBufUCS && *pBufUCS)
	{
		UT_ASSERT(UT_UCS_strlen(pBufUCS) < AP_MAX_MESSAGE_FIELD);
		UT_UCS_strcpy(m_bufUCS,pBufUCS);
	}
	
	ap_sb_Field_StatusMessage * pf = (ap_sb_Field_StatusMessage *)m_pStatusMessageField;
	pf->draw();
}

void AP_StatusBar::setStatusMessage(const char * pBuf)
{
	UT_uint32 len = ((pBuf && *pBuf) ? strlen(pBuf) : 0);
	UT_ASSERT(len < AP_MAX_MESSAGE_FIELD);

	UT_UCSChar bufUCS[AP_MAX_MESSAGE_FIELD];

	if (len)
		UT_UCS_strcpy_char(bufUCS,pBuf);
	else
		memset(bufUCS,0,sizeof(bufUCS));

	setStatusMessage(bufUCS);
}

const UT_UCSChar * AP_StatusBar::getStatusMessage(void) const
{
	return m_bufUCS;
}

