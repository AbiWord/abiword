/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "ut_types.h"
#include "ap_Frame.h"
#include "xap_App.h"

#include "ap_Features.h"

#include "ap_FrameData.h"
#include "ap_Strings.h"
#include "fv_View.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "pd_Document.h"
#include "xap_ViewListener.h"
#include "xap_Scrollbar_ViewListener.h"
#include "ap_TopRuler.h"
#include "ap_LeftRuler.h"
#include "ap_StatusBar.h"
#include "xap_Dlg_Zoom.h"

/*****************************************************************/

#define ENSUREP_C(p)		do { UT_ASSERT_HARMLESS(p); if (!p) goto Cleanup; } while (0)

/*****************************************************************/

void AP_Frame::setZoomPercentage(UT_uint32 iZoom)
{
	XAP_Frame::setZoomPercentage(iZoom);
	_showDocument(iZoom);
}

/*!
 * This zooms an existing document in a frame by just changing the zoom
 * in the graphics class.
 */
void AP_Frame::quickZoom(UT_uint32 iZoom)
{
	bool bChanged = (getZoomPercentage() != iZoom);
	XAP_Frame::setZoomPercentage(iZoom);
	FV_View * pView = static_cast<FV_View *>(getCurrentView());
	if(!pView)
		return;
	if (bChanged) 
	{
		FL_DocLayout * pDocLayout = pView->getLayout();
		pDocLayout->incrementGraphicTick();
		GR_Graphics * pOldGraphics = pView->getGraphics();
		pOldGraphics->setZoomPercentage(iZoom);
		pOldGraphics->clearFont();

		if(pView->getViewMode() == VIEW_WEB)
		{
			UT_sint32 iAdjustZoom = pView->calculateZoomPercentForPageWidth();
			UT_Dimension orig_ut = DIM_IN;
			orig_ut = pDocLayout->m_docViewPageSize.getDims();
			double orig_width = pDocLayout->getDocument()->m_docPageSize.Width(orig_ut);
			double orig_height = pDocLayout->getDocument()->m_docPageSize.Height(orig_ut);
			double rat = static_cast<double>(iAdjustZoom)/static_cast<double>(iZoom) ;
			double new_width = orig_width*rat;
			UT_DEBUGMSG(("AP_Frame zoom VIEW_WEB old width %f new width %f old height %f \n",orig_width,new_width,orig_height));
			bool p = pDocLayout->m_docViewPageSize.isPortrait();
			pDocLayout->m_docViewPageSize.Set(new_width,orig_height,orig_ut);
			pDocLayout->m_docViewPageSize.Set(fp_PageSize::psCustom,orig_ut);
			if(p)
			{
				pDocLayout->m_docViewPageSize.setPortrait();
			}
			else
			{
				pDocLayout->m_docViewPageSize.setLandscape();
			}
			fl_SectionLayout* pSL = pDocLayout->getFirstSection();
			while (pSL)
			{
				pSL->lookupMarginProperties();
				pSL = static_cast<fl_SectionLayout *>(pSL->getNext());
			}
			pView->rebuildLayout();
			pDocLayout->formatAll();

		}
		AP_TopRuler * pTop = pView->getTopRuler();
		if(pTop)
		{
			pTop->setZoom(iZoom);
		}
		AP_LeftRuler * pLeft = pView->getLeftRuler();
		if(pLeft)
		{
			pLeft->setZoom(iZoom);
		}
		pView->calculateNumHorizPages();
		setYScrollRange();
		setXScrollRange();
		if(pTop && !pTop->isHidden())
		{
			pTop->queueDraw();
		}
		if(pLeft && !pLeft->isHidden())
		{
			pLeft->queueDraw();
		}
//
// Redraw the entire screen
//
		pView->setPoint(pView->getPoint()); // place the cursor correctly
		pView->ensureInsertionPointOnScreen(); // on the screen
		pView->updateScreen(false);
	}
	else
	{
//
// Redraw the entire screen
//
		pView->updateScreen(false);
	}

	// Notify listeners of new zoom (to update the zoom combo in toolbar)
	// We do this regardless of bChanged since a change from 100% zoom to
	// "Page Width" zoom may not change the logical zoom level.
	pView->notifyListeners(AV_CHG_ALL);
}

UT_uint32 AP_Frame::getZoomPercentage(void)
{
	// !m_pData can happen when calling dlgZoom from AbiCommand; 
	// !m_pG can happen when called from an uninitialized abiwidget
	if(m_pData && static_cast<AP_FrameData*>(m_pData)->m_pG)
		return static_cast<AP_FrameData*>(m_pData)->m_pG->getZoomPercentage();

	return 100; // 100 seems like the sanest default
}

AP_Frame::~AP_Frame()
{
}


bool AP_Frame::initFrameData()
{
	UT_ASSERT_HARMLESS(!static_cast<AP_FrameData*>(m_pData));

	AP_FrameData* pData = new AP_FrameData();

	m_pData = static_cast<void*>(pData);
	return (pData ? true : false);
}

void AP_Frame::killFrameData()
{
	AP_FrameData* pData = static_cast<AP_FrameData*>(m_pData);
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
	if(XAP_App::getApp()->findFrame(this) < 0)
	{
		XAP_App::getApp()->rememberFrame(this);
	}
	AD_Document * pNewDoc = new PD_Document();
	UT_return_val_if_fail (pNewDoc, UT_ERROR);
	UT_Error errorCode = UT_OK;

	if (!szFilename || !*szFilename)
	{
		pNewDoc->newDocument();
		m_iUntitled = _getNextUntitledNumber();
		goto ReplaceDocument;
	}
	errorCode = pNewDoc->readFromFile(szFilename, ieft);
	if (UT_IS_IE_SUCCESS(errorCode))
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

		UT_DEBUGMSG(("Could not open the document - create new istead error code is %d \n", errorCode));
	    if ( UT_IE_FILENOTFOUND == errorCode ||  UT_INVALIDFILENAME == errorCode  )
		{
			UT_DEBUGMSG(("File NOT found!! Create new doc \n"));
			if( UT_IE_FILENOTFOUND == errorCode)
			{
				errorCode = pNewDoc->saveAs(szFilename, ieft);
			}
			else
			{
				errorCode = 0;
			}
			UT_DEBUGMSG(("errocode after save is %d\n",errorCode));
		}
	  }
	if (!errorCode)
	  goto ReplaceDocument;
	
	UT_DEBUGMSG(("ap_Frame: could not open the file [%s]\n",szFilename));
	UNREFP(pNewDoc);
	return errorCode;

ReplaceDocument:
	XAP_App::getApp()->forgetClones(this);
	UT_DEBUGMSG(("Doing replace document \n"));
	
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return errorCode;
}

UT_Error AP_Frame::_loadDocument(GsfInput * input, IEFileType ieft)
{
	UT_return_val_if_fail (input != NULL, UT_ERROR);

	// are we replacing another document?
	if (m_pDoc)
	{
		// yep.  first make sure it's OK to discard it, 
		// TODO: query user if dirty...
	}

	// load a document into the current frame.
	// if no filename, create a new document.
	if(XAP_App::getApp()->findFrame(this) < 0)
	{
		XAP_App::getApp()->rememberFrame(this);
	}
	AD_Document * pNewDoc = new PD_Document();
	UT_return_val_if_fail (pNewDoc, UT_ERROR);
	
	UT_Error errorCode;
	errorCode = static_cast<PD_Document*>(pNewDoc)->readFromFile(input, ieft);
	if (errorCode)
		{
			UT_DEBUGMSG(("ap_Frame: could not open the file\n"));
			UNREFP(pNewDoc);
			return errorCode;
		}

	XAP_App::getApp()->forgetClones(this);
	UT_DEBUGMSG(("Doing replace document \n"));
	
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

	AD_Document * pNewDoc = new PD_Document();
	UT_return_val_if_fail (pNewDoc, UT_ERROR);

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
	XAP_App::getApp()->forgetClones(this);

	m_iUntitled = _getNextUntitledNumber();

	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pNewDoc;
	return UT_OK;
}

XAP_Frame * AP_Frame::buildFrame(XAP_Frame * pF)
{
	UT_Error error = UT_OK;
	AP_Frame * pClone = static_cast<AP_Frame *>(pF);
	XAP_Frame::tZoomType iZoomType = pF->getZoomType();
	setZoomType(iZoomType);
	UT_uint32 iZoom = XAP_Frame::getZoomPercentage();
	ENSUREP_C(pClone);
	if (!pClone->initialize())
		goto Cleanup;

	// we remember the view of the parent frame ...
	static_cast<AP_FrameData*>(pClone->m_pData)->m_pRootView = m_pView;
	
	error = pClone->_showDocument(iZoom);
	if (error)
		goto Cleanup;

	pClone->show();
	return static_cast<XAP_Frame *>(pClone);

 Cleanup:
	// clean up anything we created here
	if (pClone)
	{
		XAP_App::getApp()->forgetFrame(pClone);
		delete pClone;
	}
	return NULL;
}

UT_Error AP_Frame::loadDocument(AD_Document* pDoc) {
	bool bUpdateClones;
	UT_GenericVector<XAP_Frame*> vClones;
	XAP_App * pApp = XAP_App::getApp();
	UT_sint32 j = 0;
	if(pApp->findFrame(this) < 0)
	{
			pApp->rememberFrame(this);
	}
	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	for(j=0; j<vClones.getItemCount();j++)
	{
		AP_Frame * pFrame = static_cast<AP_Frame *>(vClones.getNthItem(j));
		if(pApp->findFrame(pFrame) < 0)
		{
			pFrame->_replaceDocument(pDoc);
		}
	}

	return _replaceDocument(pDoc);
}

UT_Error AP_Frame::loadDocument(GsfInput * input, int ieft)
{
	bool bUpdateClones;
	UT_GenericVector<XAP_Frame*> vClones;
	XAP_App * pApp = XAP_App::getApp();
	UT_sint32 j = 0;
	if(pApp->findFrame(this) < 0)
	{
			pApp->rememberFrame(this);
	}
	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	for(j=0; j<vClones.getItemCount();j++)
	{
		XAP_Frame * pFrame = vClones.getNthItem(j);
		if(pApp->findFrame(pFrame) < 0)
		{
			pApp->rememberFrame(pFrame,this);
		}
	}
	UT_Error errorCode;
	errorCode =  _loadDocument(input, static_cast<IEFileType>(ieft));
	if (!UT_IS_IE_SUCCESS(errorCode))
	{
		// we could not load the document.
		// we cannot complain to the user here, we don't know
		// if the app is fully up yet.  we force our caller
		// to deal with the problem.
		return errorCode;
	}
	XAP_Frame::tZoomType iZoomType;
	UT_uint32 iZoom = getNewZoom(&iZoomType);
	setZoomType(iZoomType);
	if(pApp->findFrame(this) < 0)
	{
		pApp->rememberFrame(this);
	}
	if (bUpdateClones)
	{
		for (UT_sint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_Frame * pFrame = static_cast<AP_Frame*>(vClones.getNthItem(i));
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
			}
		}
	}

	return _showDocument(iZoom);
}

UT_Error AP_Frame::loadDocument(const char * szFilename, int ieft, bool createNew)
{
	bool bUpdateClones;
	UT_GenericVector<XAP_Frame*> vClones;
	XAP_App * pApp = XAP_App::getApp();
	UT_sint32 j = 0;
	if(pApp->findFrame(this) < 0)
	{
			pApp->rememberFrame(this);
	}
	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	for(j=0; j<vClones.getItemCount();j++)
	{
		XAP_Frame * pFrame = vClones.getNthItem(j);
		if(pApp->findFrame(pFrame) < 0)
		{
			pApp->rememberFrame(pFrame,this);
		}
	}
	UT_Error errorCode;
	errorCode =  _loadDocument(szFilename, static_cast<IEFileType>(ieft), createNew);
	if (!UT_IS_IE_SUCCESS(errorCode))
	{
		// we could not load the document.
		// we cannot complain to the user here, we don't know
		// if the app is fully up yet.  we force our caller
		// to deal with the problem.
		return errorCode;
	}
	XAP_Frame::tZoomType iZoomType;
	UT_uint32 iZoom = getNewZoom(&iZoomType);
	setZoomType(iZoomType);
	if(pApp->findFrame(this) < 0)
	{
		pApp->rememberFrame(this);
	}
	if (bUpdateClones)
	{
		for (UT_sint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_Frame * pFrame = static_cast<AP_Frame*>(vClones.getNthItem(i));
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
			}
		}
	}

	UT_Error errorCode2 =  _showDocument(iZoom);
    if((errorCode2 == UT_OK) && (errorCode == UT_IE_TRY_RECOVER)) 
    {
        return errorCode;
    }
    return errorCode2;
}

UT_Error AP_Frame::loadDocument(const char * szFilename, int ieft)
{
  return loadDocument(szFilename, ieft, false);
}

UT_Error AP_Frame::importDocument(const char * szFilename, int ieft, bool markClean)
{
	bool bUpdateClones;
	UT_GenericVector<XAP_Frame*> vClones;
	XAP_App * pApp = XAP_App::getApp();

	bUpdateClones = (getViewNumber() > 0);
	if (bUpdateClones)
	{
		pApp->getClones(&vClones, this);
	}
	UT_Error errorCode;
	errorCode =  _importDocument(szFilename, static_cast<IEFileType>(ieft), markClean);
	if (!UT_IS_IE_SUCCESS(errorCode))
	{
		return errorCode;
	}

	if (bUpdateClones)
	{
		for (UT_sint32 i = 0; i < vClones.getItemCount(); i++)
		{
			AP_Frame * pFrame = static_cast<AP_Frame *>(vClones.getNthItem(i));
			if(pFrame != this)
			{
				pFrame->_replaceDocument(m_pDoc);
			}
		}
	}
	XAP_Frame::tZoomType iZoomType;
	UT_uint32 iZoom = getNewZoom(&iZoomType);
	setZoomType(iZoomType);
	UT_Error errorCode2 =  _showDocument(iZoom);
    if((errorCode2 == UT_OK) && (errorCode == UT_IE_TRY_RECOVER)) 
    {
        return errorCode;
    }
    return errorCode2;
}

/*!
 * This method returns the zoomPercentage of the new frame.
 * Logic goes like this.
 * If there is a valid last focussed frame return the zoom and zoom type 
 * for that.
 * Otherwise use the preference value.
 */
UT_uint32 AP_Frame::getNewZoom(XAP_Frame::tZoomType * tZoom)
{
	UT_GenericVector<XAP_Frame*> vecClones;
	XAP_Frame *pF = NULL;
	XAP_App * pApp = XAP_App::getApp();
	UT_return_val_if_fail (pApp, 0);
	XAP_Frame * pLastFrame = pApp->getLastFocussedFrame();
	UT_uint32 iZoom = 100;
	if(pLastFrame == NULL)
	{
		std::string sZoom;
		pApp->getPrefsValue(XAP_PREF_KEY_ZoomType, sZoom);
		*tZoom = getZoomType();
		if( (g_ascii_strcasecmp( sZoom.c_str(), "Width" ) == 0 ) || (g_ascii_strcasecmp( sZoom.c_str(), "Page" ) == 0 ))
		{
			iZoom = 100;
		}
		else
		{
			iZoom =  atoi( sZoom.c_str() );
		}
		return iZoom;
	}
	else
	{
		UT_uint32 nFrames = getViewNumber();

		if(nFrames == 0)
		{
			iZoom = pLastFrame->getZoomPercentage();
			*tZoom = pLastFrame->getZoomType();
			return iZoom;
		}
		XAP_App::getApp()->getClones(&vecClones,this);
		UT_sint32 i =0;
		bool bMatch = false;
		for (i=0; !bMatch && (i< vecClones.getItemCount()); i++)
		{
			pF = static_cast<XAP_Frame *>(vecClones.getNthItem(i));
			bMatch = (pF == pLastFrame);
		}
		if(bMatch)
		{
			iZoom = pLastFrame->getZoomPercentage();
			*tZoom = pLastFrame->getZoomType();
			return iZoom;
		}
	}
	iZoom = pF->getZoomPercentage();
	*tZoom = pF->getZoomType();
	return iZoom;
}

UT_Error AP_Frame::_replaceDocument(AD_Document * pDoc)
{
	// NOTE: prior document is discarded in _showDocument()
	m_pDoc = pDoc; // This was a REFP(pDoc) but it just a caused leak
	XAP_Frame::tZoomType iZoomType;
	UT_uint32 iZoom = getNewZoom(&iZoomType);
	setZoomType(iZoomType);
	UT_Error res = _showDocument(iZoom);
	// notify our listeners
	_signal(APF_ReplaceDocument);
	return res;
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
		UT_ASSERT_HARMLESS(0);
		return  UT_IE_ADDLISTENERERROR;
	}
	setFrameLocked(true);
	if (!static_cast<AP_FrameData*>(m_pData))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	if(iZoom < XAP_DLG_ZOOM_MINIMUM_ZOOM) 
		iZoom = 100;
	else if (iZoom > XAP_DLG_ZOOM_MAXIMUM_ZOOM) 
		iZoom = 100;
	UT_DEBUGMSG(("!!!!!!!!! _showdOCument: Initial izoom is %d \n",iZoom));

	if (!_createViewGraphics(pG, iZoom))
		goto Cleanup;

	pDocLayout = new FL_DocLayout(static_cast<PD_Document *>(m_pDoc), pG);
	ENSUREP_C(pDocLayout);  

	pView = new FV_View(XAP_App::getApp(), this, pDocLayout);
	ENSUREP_C(pView);
	
	if(getZoomType() == XAP_Frame::z_PAGEWIDTH)
	{
		iZoom = pView->calculateZoomPercentForPageWidth();
		pG->setZoomPercentage(iZoom);
	}
	else if(getZoomType() == XAP_Frame::z_WHOLEPAGE)
	{
		iZoom = pView->calculateZoomPercentForWholePage();
		pG->setZoomPercentage(iZoom);
	}
	UT_DEBUGMSG(("!!!!!!!!! _showdOCument: izoom is %d \n",iZoom));
	XAP_Frame::setZoomPercentage(iZoom);
	_setViewFocus(pView);

	if (!_createScrollBarListeners(pView, pScrollObj, pViewListener, pScrollbarViewListener,
				       lid, lidScrollbarViewListener))
		goto Cleanup;
	if(getFrameMode() ==XAP_NormalFrame)
	{
		_bindToolbars(pView);	
	}
	_replaceView(pG, pDocLayout, pView, pScrollObj, pViewListener, pOldDoc, 
		     pScrollbarViewListener, lid, lidScrollbarViewListener, iZoom);

	setXScrollRange();
	setYScrollRange();

	m_pView->draw();

	if ( static_cast<AP_FrameData*>(m_pData)->m_bShowRuler  ) 
	{
		if ( static_cast<AP_FrameData*>(m_pData)->m_pTopRuler )
		{
			static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->setZoom(iZoom);
			static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->queueDraw();
		}
		if ( static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler )
		{
			static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->setZoom(iZoom);
			static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->queueDraw();
		}
	}
	if(isStatusBarShown())
	{
		if (static_cast<AP_FrameData*>(m_pData)->m_pStatusBar)
			static_cast<AP_FrameData*>(m_pData)->m_pStatusBar->notify(m_pView, AV_CHG_ALL);
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
	UT_return_val_if_fail(static_cast<AP_FrameData*>(m_pData)->m_pDocLayout, UT_IE_ADDLISTENERERROR);
	m_pDoc = static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getDocument();
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
	bool holdsSelection = false, hadView = true;
	PD_DocumentRange range;
	PT_DocPosition inspt = 0;

	// we want to remember point/selection when that is appropriate, which is when the new view is a
	// view of the same doc as the old view, or if this frame is being cloned from an existing frame
	
	// these are the view and doc from which this frame is being cloned
	FV_View * pRootView = NULL; // this should really be const, but
								// getDocumentRangeOfCurrentSelection() is not
	const AD_Document * pRootDoc = NULL;
	
	if (m_pView && !m_pView->isSelectionEmpty ())
	{
		holdsSelection = true;
		static_cast<FV_View*>(m_pView)->getDocumentRangeOfCurrentSelection (&range);
	} else if (m_pView)
	{
		inspt = static_cast<FV_View*>(m_pView)->getInsPoint ();
	}
	else if(static_cast<AP_FrameData*>(m_pData)->m_pRootView)
	{
		pRootView = static_cast<FV_View*>(static_cast<AP_FrameData*>(m_pData)->m_pRootView);
		pRootDoc = pRootView->getDocument();

		if (!pRootView->isSelectionEmpty())
		{
			holdsSelection = true;
			pRootView->getDocumentRangeOfCurrentSelection (&range);
		} else if (pRootView)
		{
			inspt = pRootView->getInsPoint ();
		}
		else
			hadView = false;

		// we want to set m_pData->m_pRootView to NULL, since it has fullfilled its function and we
		// do not want any dead pointers hanging around
		static_cast<AP_FrameData*>(m_pData)->m_pRootView = NULL;
	}
	else
		hadView = false;

	// switch to new view, cleaning up previous settings
	if (static_cast<AP_FrameData*>(m_pData)->m_pDocLayout)
	{
		pOldDoc = (static_cast<AP_FrameData*>(m_pData)->m_pDocLayout->getDocument());
	}

	REPLACEP(static_cast<AP_FrameData*>(m_pData)->m_pG, pG);
	REPLACEP(static_cast<AP_FrameData*>(m_pData)->m_pDocLayout, pDocLayout);
	
	bool bSameDocument = false; // be cautious ...

	if(!pOldDoc)
	{
		// this is the case when this is a new frame unrelated to anything, or a frame cloned from
		// existing frame
		if(pRootDoc == m_pDoc)
		{
			// we have been cloned from a frame which related to the same document
			bSameDocument = true;
		}
	}
	else if (pOldDoc != m_pDoc)
	{
	        static_cast<PD_Document *>(pOldDoc)->changeConnectedDocument(static_cast<PD_Document *>(m_pDoc));
		UNREFP(pOldDoc);
	}
	else
	{
		// pOldDoc == m_pDoc
		bSameDocument = true;
	}

	AV_View * pReplacedView = m_pView;
	m_pView = pView;

	XAP_App::getApp()->setViewSelection(NULL);

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
	if ( static_cast<AP_FrameData*>(m_pData)->m_bShowRuler )
	{
		if ( static_cast<AP_FrameData*>(m_pData)->m_pTopRuler )
			static_cast<AP_FrameData*>(m_pData)->m_pTopRuler->setView(pView, iZoom);
		if ( static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler )
			static_cast<AP_FrameData*>(m_pData)->m_pLeftRuler->setView(pView, iZoom);
	}

	if ( static_cast<AP_FrameData*>(m_pData)->m_pStatusBar && (getFrameMode() != XAP_NoMenusWindowLess))
		static_cast<AP_FrameData*>(m_pData)->m_pStatusBar->setView(pView);
	static_cast<FV_View *>(m_pView)->setShowPara(static_cast<AP_FrameData*>(m_pData)->m_bShowPara);

	pView->setInsertMode((static_cast<AP_FrameData*>(m_pData)->m_bInsertMode));
	m_pView->setWindowSize(_getDocumentAreaWidth(), _getDocumentAreaHeight());

	updateTitle();
	XAP_App * pApp = XAP_App::getApp();
	if(pApp->findFrame(this) < 0)
	{
		pApp->rememberFrame(this);
	}
	//
	// Remove the old layout before we fill the layouts to prevent
	// stale pointers being accessed during the fill phase.
	//
	if(bSameDocument)
	{
		static_cast<PD_Document *>(m_pDoc)->disableListUpdates();
	}
	pDocLayout->fillLayouts();      
	if(bSameDocument)
	{
		static_cast<PD_Document *>(m_pDoc)->enableListUpdates();
		static_cast<PD_Document *>(m_pDoc)->updateDirtyLists();
	}

	// can only hold selection/point if the two views are for the same doc !!!
	if(bSameDocument)
	{
		FV_View * pFV_View = static_cast<FV_View*>(m_pView);
		if (holdsSelection)
			pFV_View->cmdSelect (range.m_pos1, range.m_pos2);
		else if (hadView)
			pFV_View->moveInsPtTo(inspt);
	}

	if (XAP_FrameImpl * pFrameImpl = getFrameImpl())
	{
		pFrameImpl->notifyViewChanged(m_pView);
	}
	DELETEP(pReplacedView);
	
	// notify our listeners
	_signal(APF_ReplaceView);
}

UT_sint32 AP_Frame::registerListener(AP_FrameListener* pListener)
{
	UT_return_val_if_fail(pListener, -1);
	m_listeners.push_back(pListener); // TODO: look for gaps that we can reuse, caused by unregister calls - MARCM
	return m_listeners.size()-1;
}

void AP_Frame::unregisterListener(UT_sint32 iListenerId)
{
	UT_return_if_fail(iListenerId >= 0);
	UT_return_if_fail(iListenerId >= static_cast<UT_sint32>(m_listeners.size()));	
	m_listeners[iListenerId] = NULL;
}

void AP_Frame::_signal(AP_FrameSignal sig)
{
	for (std::vector<AP_FrameListener*>::iterator it = m_listeners.begin(); it != m_listeners.end(); it++)
	{
		AP_FrameListener* pListener = *it;
		if (pListener)
			pListener->signalFrame(sig);
	}
}
