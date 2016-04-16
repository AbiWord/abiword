/* AbiWord
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include "ap_Features.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ap_StatusBar.h"
#include "ap_FrameData.h"
#include "gr_Graphics.h"
#include "xap_App.h"
#include "xap_Frame.h"
#include "xav_View.h"
#include "ap_Strings.h"
#include "fl_DocLayout.h"
#include "fv_View.h"
#include "xap_EncodingManager.h"
#include "ut_timer.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#define AP_STATUSBAR_STATUSMESSAGE_REPRESENTATIVE_STRING "MMMMMMMMMMMMMMMMMMMMMMMMMMMM"
#define AP_STATUSBAR_INPUTMODE_REP_STRING "MMMMMMMM"
#define AP_STATUSBAR_INSERTMODE_REP_STRING "MMMMMMM"

#define AP_STATUSBAR_MAX_PAGES 9999

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_StatusBarField::AP_StatusBarField(AP_StatusBar * pSB)
{
    m_pSB = pSB;
    m_pStatusBarFieldListener = NULL;
    m_fillMethod = MAX_POSSIBLE;
}

AP_StatusBarField::~AP_StatusBarField(void)
{
    DELETEP(m_pStatusBarFieldListener);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_StatusBarField_TextInfo::AP_StatusBarField_TextInfo(AP_StatusBar *pSB) 
    : AP_StatusBarField(pSB)
{ 
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
class ABI_EXPORT ap_sbf_PageInfo : public AP_StatusBarField_TextInfo
{
public:
    ap_sbf_PageInfo(AP_StatusBar * pSB);
	~ap_sbf_PageInfo();

    virtual void		notify(AV_View * pView, const AV_ChangeMask mask);

private:
    UT_uint32			m_pageNr;
    UT_uint32			m_nrPages;

    const gchar *	m_szFormat;
};

ap_sbf_PageInfo::ap_sbf_PageInfo(AP_StatusBar * pSB)
    : AP_StatusBarField_TextInfo(pSB)
{
    m_pageNr = 0;
    m_nrPages = 0;
    std::string s;
    XAP_App::getApp()->getStringSet()->getValueUTF8(AP_STRING_ID_PageInfoField,s);
    m_szFormat = g_strdup(s.c_str());
    m_fillMethod = REPRESENTATIVE_STRING;
    m_alignmentMethod = LEFT;
    UT_UTF8String_sprintf(m_sRepresentativeString,m_szFormat,AP_STATUSBAR_MAX_PAGES,AP_STATUSBAR_MAX_PAGES);
}

void ap_sbf_PageInfo::notify(AV_View * pavView, const AV_ChangeMask mask)
{
    FV_View * pView = static_cast<FV_View *>(pavView);
	
    bool bNeedNewString = false;

    if (mask & (AV_CHG_MOTION | AV_CHG_PAGECOUNT))
    {
		UT_uint32 currentPage = pView->getCurrentPageNumForStatusBar(); 
		UT_uint32 newPageCount = pView->getLayout()->countPages();

		if (newPageCount != m_nrPages || m_pageNr != currentPage)
		{
			bNeedNewString = true;
			m_nrPages = newPageCount;
			m_pageNr = currentPage; 
		}
    }

    if (bNeedNewString)
    {
		UT_UTF8String_sprintf(m_sBuf, m_szFormat, m_pageNr, m_nrPages);
		
		if (getListener())
			getListener()->notify();

    }
}

ap_sbf_PageInfo::~ap_sbf_PageInfo()
{
	if (m_szFormat)
		g_free(const_cast<gchar *>(m_szFormat));

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT ap_sbf_StatusMessage : public AP_StatusBarField_TextInfo
{
public:
    ap_sbf_StatusMessage(AP_StatusBar * pSB);

    virtual void		notify(AV_View * pView, const AV_ChangeMask mask);
    void update(const UT_UTF8String &sMessage); // for receiving messages from the status bar itself
};

ap_sbf_StatusMessage::ap_sbf_StatusMessage(AP_StatusBar * pSB)
    : AP_StatusBarField_TextInfo(pSB)
      
{
    m_fillMethod = MAX_POSSIBLE;
    m_alignmentMethod = LEFT;
    m_sRepresentativeString = AP_STATUSBAR_STATUSMESSAGE_REPRESENTATIVE_STRING;
}

void ap_sbf_StatusMessage::notify(AV_View * /*pView*/, const AV_ChangeMask /*mask*/)
{    
    m_sBuf = m_pSB->getStatusMessage();

    if (getListener())
	getListener()->notify();
}

void ap_sbf_StatusMessage::update(const UT_UTF8String &sMessage)
{
    m_sBuf = sMessage;

    if (getListener())
	getListener()->notify();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT ap_sbf_InputMode : public AP_StatusBarField_TextInfo
{
public:
    ap_sbf_InputMode(AP_StatusBar * pSB);
	
    virtual void		notify(AV_View * pView, const AV_ChangeMask mask);
};

ap_sbf_InputMode::ap_sbf_InputMode(AP_StatusBar * pSB)
    : AP_StatusBarField_TextInfo(pSB)
      
{
    UT_UTF8String sInputMode(XAP_App::getApp()->getInputMode(), XAP_App::getApp()->getDefaultEncoding());
    m_sBuf = sInputMode;

    m_fillMethod = REPRESENTATIVE_STRING;
    m_alignmentMethod = LEFT;
    m_sRepresentativeString = AP_STATUSBAR_INPUTMODE_REP_STRING;
}

void ap_sbf_InputMode::notify(AV_View * /*pavView*/, const AV_ChangeMask mask)
{
    if (mask & (AV_CHG_INPUTMODE))
    {
	UT_UTF8String sInputMode(XAP_App::getApp()->getInputMode(), XAP_App::getApp()->getDefaultEncoding());
	m_sBuf = sInputMode;

	if (getListener())
	    getListener()->notify();
    }
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT ap_sbf_InsertMode : public AP_StatusBarField_TextInfo
{
public:
    ap_sbf_InsertMode(AP_StatusBar * pSB);
	
    virtual void        notify(AV_View * pView, const AV_ChangeMask mask);

private:
    std::string m_sInsertMode[2];
    bool m_bInsertMode;
};

ap_sbf_InsertMode::ap_sbf_InsertMode(AP_StatusBar * pSB)
    : AP_StatusBarField_TextInfo(pSB)
      

{
    m_bInsertMode = true;
    
    const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

    std::string s1, s2;
    pSS->getValueUTF8(AP_STRING_ID_InsertModeFieldINS, s1);
    pSS->getValueUTF8(AP_STRING_ID_InsertModeFieldOVR, s2);

    m_sInsertMode[0] = s2; // m_bInsertMode == false
    m_sInsertMode[1] = s1; // m_bInsertMode == true

    m_fillMethod = REPRESENTATIVE_STRING;
    m_alignmentMethod = CENTER;
    m_sRepresentativeString = AP_STATUSBAR_INSERTMODE_REP_STRING;
}

void ap_sbf_InsertMode::notify(AV_View * /*pavView*/, const AV_ChangeMask mask)
{
    if (mask & (AV_CHG_INSERTMODE))
    {
	AP_FrameData * pData = static_cast<AP_FrameData *>(m_pSB->getFrame()->getFrameData());
	if (pData) {
	    m_bInsertMode = pData->m_bInsertMode;
	    m_sBuf = m_sInsertMode[m_bInsertMode];
	}

	if (getListener())
	    getListener()->notify();
    }
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT ap_sbf_Language : public AP_StatusBarField_TextInfo
{
public:
    ap_sbf_Language(AP_StatusBar * pSB);

    virtual void notify(AV_View * pView, const AV_ChangeMask mask);
};

ap_sbf_Language::ap_sbf_Language(AP_StatusBar * pSB)
    : AP_StatusBarField_TextInfo(pSB)      
{
    m_fillMethod = REPRESENTATIVE_STRING;
    m_alignmentMethod = CENTER;
    m_sRepresentativeString = "mm-MM";
}

void ap_sbf_Language::notify(AV_View * pavView, const AV_ChangeMask /*mask*/)
{
    // TODO do we want our own bit for language change?
    //if (mask & (AV_CHG_INSERTMODE))
    {
	PP_PropertyVector props_in;
	if (pavView && static_cast<FV_View *>(pavView)->getCharFormat(props_in))
	{
	    std::string lang = PP_getAttribute("lang", props_in);

	    m_sBuf = lang;
	}

	if (getListener())
	    getListener()->notify();
    }
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// PROGRESSBAR. Implemented for GTK. Needs implementing for Win and OSX

AP_StatusBarField_ProgressBar::AP_StatusBarField_ProgressBar(AP_StatusBar * pSB)
    : AP_StatusBarField(pSB)
{
    m_ProgressStartPoint = 0;
    m_ProgressFlags = 0;
    m_ProgressTimer = NULL;
    m_fillMethod = PROGRESS_BAR;
}

AP_StatusBarField_ProgressBar::~AP_StatusBarField_ProgressBar(void)
{
}

#if 0
static void updateProgress(UT_Worker * pWorker)
{
    UT_return_if_fail (pWorker);

    AP_StatusBarField_ProgressBar *pfspb;
    pfspb = static_cast<AP_StatusBarField_ProgressBar *>(pWorker->getInstanceData());
    UT_return_if_fail (pfspb);

    if(pfspb->getListener())
	pfspb->getListener()->notify();
}
#endif

void AP_StatusBarField_ProgressBar::notify(AV_View * /*pView*/, const AV_ChangeMask /*mask*/)
{
  if(getListener())
    getListener()->notify();
}

void AP_StatusBarField_ProgressBar::setStatusProgressType(int start, int end, int flags)
{
    m_ProgressStart = m_ProgressValue = start;
    m_ProgressEnd = end;
    m_ProgressFlags = flags;
    m_ProgressStartPoint = 0;

#if 0
    DELETEP(m_ProgressTimer);

    if (m_ProgressStart == m_ProgressEnd &&
	(m_ProgressFlags & PROGRESS_CMD_MASK) == PROGRESS_STARTBAR) {  
	m_ProgressTimer = UT_Timer::static_constructor(updateProgress, this);
	m_ProgressTimer->stop();
	m_ProgressTimer->set(50);	//Milliseconds
    }
#endif
}

void AP_StatusBarField_ProgressBar::setStatusProgressValue(int value)
{
    UT_sint32 prev =  m_ProgressValue;
    m_ProgressValue = value;
    if(getListener() && (prev < value))
      getListener()->notify();
}

double AP_StatusBarField_ProgressBar::getFraction(void)
{
  double denom = static_cast<double>(m_ProgressEnd) - static_cast<double>(m_ProgressStart);
  if(denom <= 0.0001)
  {
    return 0.0;
  }
  return static_cast<double>( m_ProgressValue)/denom;
}

bool AP_StatusBarField_ProgressBar::isDefinate(void)
{
  return (m_ProgressFlags != PROGRESS_INDEFINATE);
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_StatusBar::AP_StatusBar(XAP_Frame * pFrame)
    :       m_pFrame(pFrame),
	    m_pView(NULL),
	    m_bInitFields(false),
	    m_pStatusMessageField(NULL),
	    m_pStatusProgressField(NULL),
	    m_sStatusMessage("")
{

#define DclField(type,var)								\
		type * var = new type(this);					\
		UT_return_if_fail ((var));								\
		m_vecFields.addItem((var));						\
		
    DclField(ap_sbf_PageInfo, pf1);
    DclField(ap_sbf_StatusMessage, pf2);

    m_pStatusMessageField = pf2;	// its in the vector, but we remember it explicitly
    // so that setStatusMessage() can do its thing.
    DclField(AP_StatusBarField_ProgressBar,pf3);
    m_pStatusProgressField = pf3;

    DclField(ap_sbf_InsertMode, pf4);
    DclField(ap_sbf_InputMode, pf5);
		
    DclField(ap_sbf_Language, pf6);
    // TODO add other fields

#undef DclField
}

AP_StatusBar::~AP_StatusBar(void)
{
    UT_VECTOR_PURGEALL(AP_StatusBarField *, m_vecFields);
}

XAP_Frame * AP_StatusBar::getFrame(void) const
{
    return m_pFrame;
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
		
	m_bInitFields = true;
    }

    // force a full notify of all fields so that they all
    // completely update themselves.
	
    notify(pView,AV_CHG_ALL);
	
    return;
}

bool AP_StatusBar::notify(AV_View * pView, const AV_ChangeMask mask)
{
    // Handle AV_Listener events on the view.	

    // We choose to clear any status message we may have,
    // since it's a pain for the code which set the message
    // to hang around and clear it at some point in the future.
    // This way, message will get cleared any time the user does
    // something with the window.
    if(getFrame()->getFrameMode() != XAP_NormalFrame)
    {
		return true;
    }
//
// High order masks are uninteresting right now
//
	if( ((mask & 0x4FFF) == 0))
	{
		return true;
	}
    setStatusMessage(static_cast<UT_UCSChar *>(NULL));
	
    // Let each field on the status bar update itself accordingly.
	
    UT_ASSERT_HARMLESS(pView==m_pView);
    UT_uint32 kLimit = m_vecFields.getItemCount();
    UT_uint32 k;

    for (k=0; k<kLimit; k++)
    {
	AP_StatusBarField * pf = static_cast<AP_StatusBarField *>(m_vecFields.getNthItem(k));
	if(pf)
	{
	    pf->notify(pView,mask);
	}
    }

    return true;
}

void AP_StatusBar::setStatusMessage(UT_UCSChar * pBufUCS, int /*redraw*/)
{
    if(getFrame()->getFrameMode() != XAP_NormalFrame)
    {
	return;
    }
    m_sStatusMessage.clear();

    if (pBufUCS && *pBufUCS)
	m_sStatusMessage.appendUCS4(pBufUCS);
	
    ap_sbf_StatusMessage * pf = static_cast<ap_sbf_StatusMessage *>(m_pStatusMessageField);
    if(pf)
	pf->update(m_sStatusMessage);
}

void AP_StatusBar::setStatusMessage(const char * pBuf, int /*redraw*/)
{
    if(getFrame()->getFrameMode() != XAP_NormalFrame)
    {
	return;
    }

    if (pBuf && *pBuf)
    {
	UT_UTF8String sBuf(pBuf, XAP_App::getApp()->getDefaultEncoding());
	m_sStatusMessage = sBuf;
    }
	else
	m_sStatusMessage.clear();

    ap_sbf_StatusMessage * pf = static_cast<ap_sbf_StatusMessage *>(m_pStatusMessageField);
    if(pf)
	pf->update(m_sStatusMessage);
}

const UT_UTF8String & AP_StatusBar::getStatusMessage(void) const
{
    return m_sStatusMessage;
}

void AP_StatusBar::setStatusProgressType(int start, int end, int flags) 
{
  if(!m_pStatusProgressField)
  {
      m_pStatusProgressField = new AP_StatusBarField_ProgressBar(this);
  }
  if(m_pStatusProgressField)
  {
      m_pStatusProgressField->setStatusProgressType(start, end, flags);
  }
}

void AP_StatusBar::setStatusProgressValue(int value ) 
{
  if(m_pStatusProgressField)
  {
      m_pStatusProgressField->setStatusProgressValue(value);
  }
}
