/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz, William Lachance and others
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
#include "ap_Frame.h"

#if defined(XP_UNIX_TARGET_GTK) || (defined(__APPLE__) && defined(__MACH__)) || defined(WIN32)
#include "ap_FrameData.h"
#include "fv_View.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "pd_Document.h"
#include "xap_ViewListener.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_StatusBar.h"

/*****************************************************************/

#define ENSUREP_C(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

AP_Frame::~AP_Frame()
{
}


bool AP_Frame::initFrameData()
{
	UT_ASSERT(!((AP_FrameData*)m_pData));

	AP_FrameData* pData = new AP_FrameData(static_cast<XAP_App *>(m_pApp));

	m_pData = (void*)pData;
	return (pData ? true : false);
}

void AP_Frame::killFrameData()
{
	AP_FrameData* pData = (AP_FrameData*) m_pData;
	DELETEP(pData);
	m_pData = NULL;
}

UT_Error AP_Frame::_loadDocument(const char * szFilename, IEFileType ieft,
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
	
UT_Error AP_Frame::_importDocument(const char * szFilename, int ieft,
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

XAP_Frame * AP_Frame::buildFrame(XAP_Frame * pF)
{
	UT_Error error = UT_OK;
	AP_Frame * pClone = static_cast<AP_Frame *>(pF);
	ENSUREP_C(pClone);
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

UT_Error AP_Frame::loadDocument(const char * szFilename, int ieft, bool createNew)
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
			AP_Frame * pFrame = (AP_Frame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

UT_Error AP_Frame::loadDocument(const char * szFilename, int ieft)
{
  return loadDocument(szFilename, ieft, false);
}

UT_Error AP_Frame::importDocument(const char * szFilename, int ieft, bool markClean)
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
			AP_Frame * pFrame = (AP_Frame *) vClones.getNthItem(i);
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
				pApp->rememberFrame(pFrame, this);
			}
		}
	}

	return _showDocument();
}

UT_Error AP_Frame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = REFP(pDoc);

	return _showDocument();
}

UT_Error AP_Frame::_showDocument(UT_uint32 iZoom)
{
	if (!m_pDoc)
	{
		UT_DEBUGMSG(("Can't show a non-existent document\n"));
		return UT_IE_FILENOTFOUND;
	}
	if(isFrameLocked())
	{
		UT_DEBUGMSG(("_showDocument: Nasty race bug, please fix me!! \n"));
		UT_ASSERT(0);
		return  UT_IE_ADDLISTENERERROR;
	}
	setFrameLocked(true);
	if (!((AP_FrameData*)m_pData))
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		setFrameLocked(false);
		return UT_IE_IMPORTERROR;
	}

// 	static_cast<XAP_FrameImpl *>(m_pFrameImpl)->setShowDocLocked(true);

	GR_Graphics * pG = NULL;
	FL_DocLayout * pDocLayout = NULL;
	AV_View * pView = NULL;
	AV_ScrollObj * pScrollObj = NULL;
	ap_ViewListener * pViewListener = NULL;
	AD_Document * pOldDoc = NULL;
	ap_Scrollbar_ViewListener * pScrollbarViewListener = NULL;
	AV_ListenerId lid;
	AV_ListenerId lidScrollbarViewListener;

	xxx_UT_DEBUGMSG(("_showDocument: Initial m_pView %x \n",m_pView));

	if (!_createViewGraphics(pG, iZoom))
		goto Cleanup;

	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	ENSUREP_C(pDocLayout);  

	pView = new FV_View(getApp(), this, pDocLayout);
	ENSUREP_C(pView);

	_setViewFocus(pView);

	if (!_createScrollBarListeners(pView, pScrollObj, pViewListener, pScrollbarViewListener,
				       lid, lidScrollbarViewListener))
		goto Cleanup;

	_bindToolbars(pView);	

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
		if (((AP_FrameData*)m_pData)->m_pStatusBar)
			((AP_FrameData*)m_pData)->m_pStatusBar->notify(m_pView, AV_CHG_ALL);
	}

	m_pView->notifyListeners(AV_CHG_ALL);
	m_pView->focusChange(AV_FOCUS_HERE);

	//static_cast<XAP_FrameImpl *>(m_pFrameImpl)->setShowDocLocked(false);
	setFrameLocked(false);
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
	setFrameLocked(false);
	UT_return_val_if_fail(((AP_FrameData*)m_pData)->m_pDocLayout, UT_IE_ADDLISTENERERROR);
	m_pDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();
	//static_cast<XAP_FrameImpl *>(m_pFrameImpl)->setShowDocLocked(false);
	return UT_IE_ADDLISTENERERROR;
}

void AP_Frame::_replaceView(GR_Graphics * pG, FL_DocLayout *pDocLayout,
			    AV_View *pView, AV_ScrollObj * pScrollObj,
			    ap_ViewListener *pViewListener, AD_Document *pOldDoc,
			    ap_Scrollbar_ViewListener *pScrollbarViewListener,
			    AV_ListenerId lid, AV_ListenerId lidScrollbarViewListener,
			    UT_uint32 iZoom)
{
	// switch to new view, cleaning up previous settings
	if (((AP_FrameData*)m_pData)->m_pDocLayout)
	{
		pOldDoc = ((AP_FrameData*)m_pData)->m_pDocLayout->getDocument();
	}

	REPLACEP(((AP_FrameData*)m_pData)->m_pG, pG);
	REPLACEP(((AP_FrameData*)m_pData)->m_pDocLayout, pDocLayout);

	if (pOldDoc != m_pDoc)
	{
		UNREFP(pOldDoc);
	}

	REPLACEP(m_pView, pView);
        if(getApp()->getViewSelection())
	       getApp()->setViewSelection(pView);

	REPLACEP(m_pScrollObj, pScrollObj);
	REPLACEP(m_pViewListener, pViewListener);
	m_lid = lid;
	REPLACEP(m_pScrollbarViewListener, pScrollbarViewListener);
	m_lidScrollbarViewListener = lidScrollbarViewListener;

	m_pView->addScrollListener(m_pScrollObj);

	// Associate the new view with the existing TopRuler, LeftRuler.
	// Because of the binding to the actual on-screen widgets we do
	// not destroy and recreate the TopRuler, LeftRuler when we change
	// views, like we do for all the other objects.  We also do not
	// allocate the TopRuler, LeftRuler  here; that is done as the
	// frame is created.
	if ( ((AP_FrameData*)m_pData)->m_bShowRuler )
	{
		if ( ((AP_FrameData*)m_pData)->m_pTopRuler )
			((AP_FrameData*)m_pData)->m_pTopRuler->setView(pView, iZoom);
		if ( ((AP_FrameData*)m_pData)->m_pLeftRuler )
			((AP_FrameData*)m_pData)->m_pLeftRuler->setView(pView, iZoom);
	}

	if ( ((AP_FrameData*)m_pData)->m_pStatusBar && (getFrameMode() != XAP_NoMenusWindowLess))
		((AP_FrameData*)m_pData)->m_pStatusBar->setView(pView);
	((FV_View *) m_pView)->setShowPara(((AP_FrameData*)m_pData)->m_bShowPara);

	pView->setInsertMode(((AP_FrameData*)m_pData)->m_bInsertMode);
	m_pView->setWindowSize(_getDocumentAreaWidth(), _getDocumentAreaHeight());

	updateTitle();
	//pDocLayout->setView((FV_View *) m_pView);
	pDocLayout->fillLayouts();   
	
	_resetInsertionPoint();
}

// _resetInsertionPoint: sets the current insertion point in the document to be the end
// of the editable bound in the current view, if and only if the upper editable bound of 
// pView is less than the current insertion point. Resets it to the current position 
// otherwise. 
void AP_Frame::_resetInsertionPoint()
{
	UT_uint32 point = 0;

	if (m_pView != NULL) {
		point = (static_cast<FV_View *>(m_pView))->getPoint();
		PT_DocPosition posEOD;
		static_cast<FV_View *>(m_pView)->getEditableBounds(true, posEOD, false);
		if(point > posEOD)
			(static_cast<FV_View *>(m_pView))->moveInsPtTo(posEOD);
	}		
}
#else
AP_Frame::~AP_Frame()
{
}
#endif
