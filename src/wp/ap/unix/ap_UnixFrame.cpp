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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <gtk/gtk.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "xap_ViewListener.h"
#include "ap_FrameData.h"
#include "ap_Frame.h"
#include "ev_UnixToolbar.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "pd_Document.h"
#include "gr_UnixGraphics.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_UnixFrame.h"
#include "ap_UnixFrameImpl.h"
#include "xap_UnixApp.h"
#include "ap_UnixTopRuler.h"
#include "ap_UnixLeftRuler.h"
#include "xap_UnixFontManager.h"
#include "ap_UnixStatusBar.h"
#include "ap_UnixViewListener.h"
#include "xap_UnixDialogHelper.h"
#if 1
#include "ev_UnixMenuBar.h"
#include "ev_Menu_Layouts.h"
#include "ev_Menu_Labels.h"
#include "ev_Menu_Actions.h"
#endif

/*****************************************************************/

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/
#include "ap_UnixApp.h"

void AP_UnixFrame::setZoomPercentage(UT_uint32 iZoom)
{
	_showDocument(iZoom);
}

UT_uint32 AP_UnixFrame::getZoomPercentage(void)
{
	return ((AP_FrameData*)m_pData)->m_pG->getZoomPercentage();
}

UT_Error AP_UnixFrame::_showDocument(UT_uint32 iZoom)
{
	if (!m_pDoc)
	{
		UT_DEBUGMSG(("Can't show a non-existent document\n"));
		return UT_IE_FILENOTFOUND;
	}

	if (!((AP_FrameData*)m_pData))
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_IE_IMPORTERROR;
	}

        if(static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->getShowDocLocked())
        {
                UT_DEBUGMSG(("Evil race condition detected. Fix this!!! \n"));
                UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
                return  UT_IE_ADDLISTENERERROR;
        }                                                                       
	static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->setShowDocLocked(true);

	GR_UnixGraphics * pG = NULL;
	FL_DocLayout * pDocLayout = NULL;
	AV_View * pView = NULL;
	AV_ScrollObj * pScrollObj = NULL;
	ap_ViewListener * pViewListener = NULL;
	AD_Document * pOldDoc = NULL;
	ap_Scrollbar_ViewListener * pScrollbarViewListener = NULL;
	AV_ListenerId lid;
	AV_ListenerId lidScrollbarViewListener;

	xxx_UT_DEBUGMSG(("_showDocument: Initial m_pView %x \n",m_pView));
	gboolean bFocus;
	XAP_UnixFontManager * fontManager = ((XAP_UnixApp *) getApp())->getFontManager();
	gtk_widget_show(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_dArea);
	pG = new GR_UnixGraphics(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_dArea->window, fontManager, getApp());
	ENSUREP(pG);

	pG->setZoomPercentage(iZoom);

	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	ENSUREP(pDocLayout);  

	pView = new FV_View(getApp(), this, pDocLayout);
	ENSUREP(pView);

	bFocus=GPOINTER_TO_INT(g_object_get_data(G_OBJECT(static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->getTopLevelWindow()),
						 "toplevelWindowFocus"));
	pView->setFocus(bFocus && (gtk_grab_get_current()==NULL || gtk_grab_get_current()==static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->getTopLevelWindow()) ? AV_FOCUS_HERE : !bFocus && gtk_grab_get_current()!=NULL && isTransientWindow(GTK_WINDOW(gtk_grab_get_current()),GTK_WINDOW(static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->getTopLevelWindow())) ?  AV_FOCUS_NEARBY : AV_FOCUS_NONE);
	// The "AV_ScrollObj pScrollObj" receives
	// send{Vertical,Horizontal}ScrollEvents
	// from both the scroll-related edit methods
	// and from the UI callbacks.
	// 
	// The "ap_ViewListener pViewListener" receives
	// change notifications as the document changes.
	// This ViewListener is responsible for keeping
	// the title-bar up to date (primarily title
	// changes, dirty indicator, and window number).
	// ON UNIX ONLY: we subclass this with ap_UnixViewListener
	// ON UNIX ONLY: so that we can deal with X-Selections.
	//
	// The "ap_Scrollbar_ViewListener pScrollbarViewListener"
	// receives change notifications as the doucment changes.
	// This ViewListener is responsible for recalibrating the
	// scrollbars as pages are added/removed from the document.
	//
	// Each Toolbar will also get a ViewListener so that
	// it can update toggle buttons, and other state-indicating
	// controls on it.
	//
	// TODO we ***really*** need to re-do the whole scrollbar thing.
	// TODO we have an addScrollListener() using an m_pScrollObj
	// TODO and a View-Listener, and a bunch of other widget stuff.
	// TODO and its very confusing.
	
	pScrollObj = new AV_ScrollObj(this,_scrollFuncX,_scrollFuncY);
	ENSUREP(pScrollObj);
	pViewListener = new ap_UnixViewListener(this);
	ENSUREP(pViewListener);
	pScrollbarViewListener = new ap_Scrollbar_ViewListener(this,pView);
	ENSUREP(pScrollbarViewListener);

	if (!pView->addListener(static_cast<AV_Listener *>(pViewListener),&lid))
		goto Cleanup;

	if (!pView->addListener(static_cast<AV_Listener *>(pScrollbarViewListener),
							&lidScrollbarViewListener))
		goto Cleanup;
	
	static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->_bindToolbars(pView);

	_replaceView(pG, pDocLayout, pView, pScrollObj, pViewListener, pOldDoc, 
		     pScrollbarViewListener, lid, lidScrollbarViewListener, iZoom);

	setXScrollRange();
	setYScrollRange();

	m_pView->draw();

	if ( ((AP_FrameData*)m_pData)->m_bShowRuler  ) 
	{
		if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
			((AP_FrameData*)m_pData)->m_pTopRuler->draw(NULL);

		if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
			((AP_FrameData*)m_pData)->m_pLeftRuler->draw(NULL);
	}
	if(isStatusBarShown())
	{
		((AP_FrameData*)m_pData)->m_pStatusBar->notify(m_pView, AV_CHG_ALL);
	}
	if(m_pView)
	{
		m_pView->notifyListeners(AV_CHG_ALL);
		m_pView->focusChange(AV_FOCUS_HERE);
	}

	static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->setShowDocLocked(false);

	return UT_OK;

Cleanup:
	// clean up anything we created here
	DELETEP(pG);
	DELETEP(pDocLayout);
	DELETEP(pView);
	DELETEP(pViewListener);
	DELETEP(pScrollObj);
	DELETEP(pScrollbarViewListener);

	// change back to prior document
	UNREFP(m_pDoc);
	m_pDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();
	static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->setShowDocLocked(false);
	return UT_IE_ADDLISTENERERROR;
}

void AP_UnixFrame::setXScrollRange(void)
{
	int width = ((AP_FrameData*)m_pData)->m_pDocLayout->getWidth();
	int windowWidth = GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_dArea)->allocation.width;

	int newvalue = ((m_pView) ? m_pView->getXScrollOffset() : 0);
	int newmax = width - windowWidth; /* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	bool bDifferentPosition = (newvalue != (int)static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pHadj->value);
	bool bDifferentLimits = ((width-windowWidth) != (int)(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pHadj->upper-
							      static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pHadj->page_size));
	

	static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->_setScrollRange(apufi_scrollX, newvalue, (gfloat)width, (gfloat)windowWidth);

	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendHorizontalScrollEvent(newvalue, (int)(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pHadj->upper-
								   static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pHadj->page_size));
}

void AP_UnixFrame::setYScrollRange(void)
{
	int height = ((AP_FrameData*)m_pData)->m_pDocLayout->getHeight();
	int windowHeight = GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_dArea)->allocation.height;

	int newvalue = ((m_pView) ? m_pView->getYScrollOffset() : 0);
	int newmax = height - windowHeight;	/* upper - page_size */
	if (newmax <= 0)
		newvalue = 0;
	else if (newvalue > newmax)
		newvalue = newmax;

	bool bDifferentPosition = (newvalue != (int)static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pVadj->value);
	bool bDifferentLimits ((height-windowHeight) != (int)(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pVadj->upper-
							      static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pVadj->page_size));
	
	static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->_setScrollRange(apufi_scrollY, newvalue, (gfloat)height, (gfloat)windowHeight);

	if (m_pView && (bDifferentPosition || bDifferentLimits))
		m_pView->sendVerticalScrollEvent(newvalue, (int)(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pVadj->upper -
								 static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_pVadj->page_size));
}


AP_UnixFrame::AP_UnixFrame(XAP_UnixApp * pApp)
	: AP_Frame(new AP_UnixFrameImpl(this, pApp), pApp)
{
	m_pData = NULL;
	static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->setShowDocLocked(false);

}

AP_UnixFrame::AP_UnixFrame(AP_UnixFrame * f)
	: AP_Frame(static_cast<AP_Frame *>(f))
{
	m_pFrameImpl = new AP_UnixFrameImpl(this, static_cast<XAP_UnixApp *>(f->m_pApp));
	m_pData = NULL;
}

AP_UnixFrame::~AP_UnixFrame()
{
	killFrameData();
}

bool AP_UnixFrame::initialize(XAP_FrameMode frameMode)
{
	UT_DEBUGMSG(("AP_UnixFrame::initialize!!!! \n"));

	setFrameMode(frameMode);
	static_cast<XAP_UnixFrameImpl *>(m_pFrameImpl)->setShowDocLocked(false);

	if (!initFrameData())
		return false;
	UT_DEBUGMSG(("AP_UnixFrame:: Initializing base classes!!!! \n"));

	if (!XAP_Frame::initialize(AP_PREF_KEY_KeyBindings,AP_PREF_DEFAULT_KeyBindings,
				   AP_PREF_KEY_MenuLayout, AP_PREF_DEFAULT_MenuLayout,
				   AP_PREF_KEY_StringSet, AP_PREF_KEY_StringSet,
				   AP_PREF_KEY_ToolbarLayouts, AP_PREF_DEFAULT_ToolbarLayouts,
				   AP_PREF_KEY_StringSet, AP_PREF_DEFAULT_StringSet))
		return false;

	UT_DEBUGMSG(("AP_UnixFrame:: Creating Toplevel Window!!!! \n"));	
	static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->_createWindow();

	return true;
}

/*****************************************************************/

bool AP_UnixFrame::initFrameData()
{
	UT_ASSERT(!((AP_FrameData*)m_pData));

	AP_FrameData* pData = new AP_FrameData(static_cast<XAP_App *>(m_pApp));

	m_pData = (void*)pData;
	return (pData ? true : false);
}

void AP_UnixFrame::killFrameData()
{
	AP_FrameData* pData = (AP_FrameData*) m_pData;
	DELETEP(pData);
	m_pData = NULL;
}

UT_Error AP_UnixFrame::_loadDocument(const char * szFilename, IEFileType ieft,
				     bool createNew)
{
#ifdef DEBUG
	if (szFilename) {
		UT_DEBUGMSG(("DOM: trying to load %s (%d, %d)\n", szFilename, ieft, createNew));
	}
	else {
		UT_DEBUGMSG(("DOM: trying to load %s (%d, %d)\n", "(NULL)", ieft, createNew));
	}
#endif

	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document(getApp());
	UT_ASSERT(pNewDoc);
	
	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		m_iUntitled = _getNextUntitledNumber();
		goto ReplaceDocument;
	}
	UT_Error errorCode;
	errorCode = pNewDoc->readFromFile(szFilename, ieft);
	if (!errorCode)
		goto ReplaceDocument;

	if (createNew)
	  {
	    // we have a file name but couldn't load it
	    pNewDoc->newDocument();

	    // here, we want to open a new document if it doesn't exist.
	    // errorCode could also take several other values, indicating
	    // that the document exists but for some reason we could not
	    // open it. in those cases, we do not wish to overwrite the
	    // existing documents, but instead open a new blank document. 
	    // this fixes bug 1668 - DAL
	    if ( UT_IE_FILENOTFOUND == errorCode )
	      errorCode = pNewDoc->saveAs(szFilename, ieft);
	  }
	if (!errorCode)
	  goto ReplaceDocument;
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	UNREFP(pNewDoc);
	return errorCode;

ReplaceDocument:
	getApp()->forgetClones(this);

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}
	
UT_Error AP_UnixFrame::_importDocument(const char * szFilename, int ieft,
									  bool markClean)
{
	UT_DEBUGMSG(("DOM: trying to import %s (%d, %d)\n", szFilename, ieft, markClean));

	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.

	AD_Document * pNewDoc = new PD_Document(getApp());
	UT_ASSERT(pNewDoc);

	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		goto ReplaceDocument;
	}
	UT_Error errorCode;
	errorCode = pNewDoc->importFile(szFilename, ieft, markClean);
	if (!errorCode)
		goto ReplaceDocument;

	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));

	UNREFP(pNewDoc);
	return errorCode;

ReplaceDocument:
	getApp()->forgetClones(this);

	m_iUntitled = _getNextUntitledNumber();

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}

XAP_Frame * AP_UnixFrame::cloneFrame()
{
	AP_UnixFrame * pClone = new AP_UnixFrame(this);
	ENSUREP(pClone);
	return static_cast<XAP_Frame *>(pClone);

 Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		static_cast<XAP_App *>(m_pApp)->forgetFrame(pClone);
		delete pClone;
	}
	return NULL;
}


XAP_Frame * AP_UnixFrame::buildFrame(XAP_Frame * pF)
{
	UT_Error error = UT_OK;
	AP_UnixFrame * pClone =	static_cast<AP_UnixFrame *>(pF);
	ENSUREP(pClone);
	if (!pClone->initialize())
		goto Cleanup;

	error = pClone->_showDocument();
	if (error)
		goto Cleanup;

	pClone->show();
	return static_cast<XAP_Frame *>(pClone);

 Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		static_cast<XAP_App *>(m_pApp)->forgetFrame(pClone);
		delete pClone;
	}
	return NULL;
}

UT_Error AP_UnixFrame::loadDocument(const char * szFilename, int ieft, bool createNew)
{
	bool bUpdateClones;
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	UT_Error errorCode;
	errorCode =  _loadDocument(szFilename, (IEFileType) ieft, createNew);
	if (errorCode)
	{
		// we could not load the document.
		// we cannot complain to the user here, we don't know
		// if the app is fully up yet.  we force our caller
		// to deal with the problem.
		return errorCode;
	}

	pApp->rememberFrame(this);
	if (bUpdateClones)
	{
		for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_UnixFrame * pFrame = (AP_UnixFrame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

UT_Error AP_UnixFrame::loadDocument(const char * szFilename, int ieft)
{
  return loadDocument(szFilename, ieft, false);
}

UT_Error AP_UnixFrame::importDocument(const char * szFilename, int ieft,
									  bool markClean)
{
	bool bUpdateClones;
	UT_Vector vClones;
	XAP_App * pApp = getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	UT_Error errorCode;
	errorCode =  _importDocument(szFilename, (IEFileType) ieft, markClean);
	if (errorCode)
	{
		return errorCode;
	}

	pApp->rememberFrame(this);
	if (bUpdateClones)
	{
		for (UT_uint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_UnixFrame * pFrame = (AP_UnixFrame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

// WL_REFACTOR: Put this in the helper
void AP_UnixFrame::_scrollFuncY(void * pData, UT_sint32 yoff, UT_sint32 /*yrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_UnixFrame * pUnixFrame = static_cast<AP_UnixFrame *>(pData);
	AV_View * pView = pUnixFrame->getCurrentView();
	
	// we've been notified (via sendVerticalScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.
	
	gfloat yoffNew = (gfloat)yoff;
	gfloat yoffMax = static_cast<AP_UnixFrameImpl *>(pUnixFrame->m_pFrameImpl)->m_pVadj->upper - 
		static_cast<AP_UnixFrameImpl *>(pUnixFrame->m_pFrameImpl)->m_pVadj->page_size;
	if (yoffMax <= 0)
		yoffNew = 0;
	else if (yoffNew > yoffMax)
		yoffNew = yoffMax;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(static_cast<AP_UnixFrameImpl *>(pUnixFrame->m_pFrameImpl)->m_pVadj),yoffNew);
	pView->setYScrollOffset((UT_sint32)yoffNew);
}

// WL_REFACTOR: Put this in the helper
void AP_UnixFrame::_scrollFuncX(void * pData, UT_sint32 xoff, UT_sint32 /*xrange*/)
{
	// this is a static callback function and doesn't have a 'this' pointer.
	
	AP_UnixFrame * pUnixFrame = static_cast<AP_UnixFrame *>(pData);
	AV_View * pView = pUnixFrame->getCurrentView();
	
	// we've been notified (via sendScrollEvent()) of a scroll (probably
	// a keyboard motion).  push the new values into the scrollbar widgets
	// (with clamping).  then cause the view to scroll.

	gfloat xoffNew = (gfloat)xoff;
	gfloat xoffMax = static_cast<AP_UnixFrameImpl *>(pUnixFrame->m_pFrameImpl)->m_pHadj->upper - 
		static_cast<AP_UnixFrameImpl *>(pUnixFrame->m_pFrameImpl)->m_pHadj->page_size;
	if (xoffMax <= 0)
		xoffNew = 0;
	else if (xoffNew > xoffMax)
		xoffNew = xoffMax;
	gtk_adjustment_set_value(GTK_ADJUSTMENT(static_cast<AP_UnixFrameImpl *>(pUnixFrame->m_pFrameImpl)->m_pHadj),xoffNew);
	pView->setXScrollOffset((UT_sint32)xoffNew);
}

void AP_UnixFrame::translateDocumentToScreen(UT_sint32 &x, UT_sint32 &y)
{
	UT_ASSERT_NOT_REACHED();
}

void AP_UnixFrame::setStatusMessage(const char * szMsg)
{
	((AP_FrameData *)m_pData)->m_pStatusBar->setStatusMessage(szMsg);
}

UT_Error AP_UnixFrame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}

void AP_UnixFrame::toggleTopRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);
		
	AP_UnixTopRuler * pUnixTopRuler = NULL;

	UT_DEBUGMSG(("AP_UnixFrame::toggleTopRuler %d, %d\n", 
		     bRulerOn, pFrameData->m_pTopRuler));

	if ( bRulerOn )
	{
		UT_ASSERT(!pFrameData->m_pTopRuler);

		pUnixTopRuler = new AP_UnixTopRuler(this);
		UT_ASSERT(pUnixTopRuler);
		static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_topRuler = pUnixTopRuler->createWidget();

		// get the width from the left ruler and stuff it into the 
		// top ruler.

		if (((AP_FrameData*)m_pData)->m_pLeftRuler)
		  pUnixTopRuler->setOffsetLeftRuler(((AP_FrameData*)m_pData)->m_pLeftRuler->getWidth());
		else
		  pUnixTopRuler->setOffsetLeftRuler(0);

		// attach everything	
		gtk_table_attach(GTK_TABLE(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_innertable), 
				 static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_topRuler, 
				 0, 2, 0, 1, 
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 (GtkAttachOptions)(GTK_FILL),
				 0, 0);

		pUnixTopRuler->setView(m_pView);
	}
	else
	  {
		// delete the actual widgets
		gtk_object_destroy( GTK_OBJECT(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_topRuler) );
		DELETEP(((AP_FrameData*)m_pData)->m_pTopRuler);
		static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_topRuler = NULL;
	  }

	((AP_FrameData*)m_pData)->m_pTopRuler = pUnixTopRuler;
}

void AP_UnixFrame::toggleLeftRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

	AP_UnixLeftRuler * pUnixLeftRuler = NULL;

	UT_DEBUGMSG(("AP_UnixFrame::toggleLeftRuler %d, %d\n", 
		     bRulerOn, pFrameData->m_pLeftRuler));

	if (bRulerOn)
	{
//
// if there is an old ruler just return.
//
		if(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_leftRuler)
		{
			return;
		}
		pUnixLeftRuler = new AP_UnixLeftRuler(this);
		UT_ASSERT(pUnixLeftRuler);
		static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_leftRuler = pUnixLeftRuler->createWidget();

		gtk_table_attach(GTK_TABLE(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_innertable), 
				 static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_leftRuler, 
				 0, 1, 1, 2,
				 (GtkAttachOptions)(GTK_FILL),
				 (GtkAttachOptions)(GTK_EXPAND | GTK_FILL),
				 0,0);
		pUnixLeftRuler->setView(m_pView);
		setYScrollRange();
	}
	else
	{
	    if (static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_leftRuler && 
		GTK_IS_OBJECT(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_leftRuler))
		gtk_object_destroy(GTK_OBJECT(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_leftRuler) );
	    
	    DELETEP(((AP_FrameData*)m_pData)->m_pLeftRuler);
	    static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_leftRuler = NULL;
	}

	((AP_FrameData*)m_pData)->m_pLeftRuler = pUnixLeftRuler;
}

void AP_UnixFrame::toggleRuler(bool bRulerOn)
{
	AP_FrameData *pFrameData = (AP_FrameData *)getFrameData();
	UT_ASSERT(pFrameData);

        toggleTopRuler(bRulerOn);
	toggleLeftRuler(bRulerOn && (pFrameData->m_pViewMode == VIEW_PRINT));
}

void AP_UnixFrame::toggleBar(UT_uint32 iBarNb, bool bBarOn)
{
	UT_DEBUGMSG(("AP_UnixFrame::toggleBar %d, %d\n", iBarNb, bBarOn));	

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	
	if (bBarOn)
		pFrameData->m_pToolbar[iBarNb]->show();
	else	// turning toolbar off
		pFrameData->m_pToolbar[iBarNb]->hide();
}

void AP_UnixFrame::toggleStatusBar(bool bStatusBarOn)
{
	UT_DEBUGMSG(("AP_UnixFrame::toggleStatusBar %d\n", bStatusBarOn));	

	AP_FrameData *pFrameData = static_cast<AP_FrameData *> (getFrameData());
	UT_ASSERT(pFrameData);
	
	if (bStatusBarOn)
		pFrameData->m_pStatusBar->show();
	else	// turning status bar off
		pFrameData->m_pStatusBar->hide();
}

UT_sint32 AP_UnixFrame::_getDocumentAreaWidth()
{
	return (UT_sint32) GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_dArea)->allocation.width;
}

UT_sint32 AP_UnixFrame::_getDocumentAreaHeight()
{
	return (UT_sint32) GTK_WIDGET(static_cast<AP_UnixFrameImpl *>(m_pFrameImpl)->m_dArea)->allocation.height;
}
