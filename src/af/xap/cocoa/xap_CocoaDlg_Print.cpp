/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003-2009 Hubert Figuiere
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#import <Cocoa/Cocoa.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "pd_Document.h"
#include "gr_CocoaCairoGraphics.h"
#include "xap_Dialog_Id.h"
#include "xap_CocoaDlg_Print.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"
#import "xap_PrintingNSView.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "ap_Strings.h"
#include "ap_CocoaFrame.h"
#include "ap_PrintingDelegate.h"

/*****************************************************************/

XAP_Dialog * XAP_CocoaDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id dlgid)
{
	XAP_CocoaDialog_Print * p = new XAP_CocoaDialog_Print(pFactory,dlgid);
	return p;
}

XAP_CocoaDialog_Print::XAP_CocoaDialog_Print(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id dlgid)
	: XAP_Dialog_Print(pDlgFactory,dlgid),
		m_pPrintGraphics(NULL)
{
}

XAP_CocoaDialog_Print::~XAP_CocoaDialog_Print(void)
{
//	DELETEP(m_pPrintGraphics);
}

void XAP_CocoaDialog_Print::useStart(void)
{
	XAP_Dialog_Print::useStart();

}

void XAP_CocoaDialog_Print::useEnd(void)
{
	XAP_Dialog_Print::useEnd();
}

GR_Graphics * XAP_CocoaDialog_Print::getPrinterGraphicsContext(void)
{
	if (m_pPrintGraphics == NULL) {
		NSSize size = [[NSPrintInfo sharedPrintInfo] paperSize];	// TODO get the size from a real data
		XAP_PrintingNSView* printingView = [[XAP_PrintingNSView  alloc] initWithFrame:NSMakeRect(0,0,size.width,size.height)];
		GR_CocoaCairoAllocInfo ai(printingView);
		m_pPrintGraphics = (GR_CocoaCairoGraphics*)XAP_App::getApp()->newGraphics(ai);
		m_pPrintGraphics->setIsPrinting(true);
	}
	return m_pPrintGraphics;
}


void XAP_CocoaDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	UT_ASSERT(pGraphics == m_pPrintGraphics);
	DELETEP(pGraphics);
}

/*****************************************************************/

void XAP_CocoaDialog_Print::runModal(XAP_Frame * /*pFrame*/)
{
	UT_ASSERT_NOT_REACHED();
}


void XAP_CocoaDialog_Print::runPrint(XAP_Frame * /*pFrame*/, FV_View * pPrintView,
							UT_sint32 /*iWidth*/, UT_sint32 /*iHeight*/)
{
	UT_ASSERT(m_pPrintGraphics);
	XAP_PrintingNSView* printingView = (XAP_PrintingNSView*)m_pPrintGraphics->getView();
	[printingView setPrintingDelegate:(new AP_PrintingDelegate(pPrintView))];
	fp_PageSize ps = pPrintView->getPageSize();	  
	bool orient = ps.isPortrait ();
	m_pPrintGraphics->setPortrait (orient);

	
	NSPrintOperation * op = [NSPrintOperation printOperationWithView:printingView];
	if (m_bBypassActualDialog) {
		[op setShowsPrintPanel:NO];
	}
	[op runOperation];
}



/*****************************************************************/


bool s_doPrint(FV_View * pView, bool bTryToSuppressDialog,bool bPrintDirectly)
{
	XAP_Frame * pFrame = static_cast<XAP_Frame *> ( pView->getParentData());
	UT_ASSERT(pFrame);

	pFrame->raise();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_Print * pDialog
		= static_cast<XAP_Dialog_Print *>(pDialogFactory->requestDialog(bPrintDirectly? XAP_DIALOG_ID_PRINT_DIRECTLY: XAP_DIALOG_ID_PRINT));
	UT_ASSERT(pDialog);
	if (!pDialog)
		return false;

	FL_DocLayout* pLayout = pView->getLayout();
	PD_Document * doc = pLayout->getDocument();

	pDialog->setPaperSize (pView->getPageSize().getPredefinedName());
	pDialog->setDocumentTitle(pFrame->getNonDecoratedTitle());
	pDialog->setDocumentPathname(!doc->getFilename().empty()
				     ? doc->getFilename().c_str()
				     : pFrame->getNonDecoratedTitle());
	pDialog->setEnablePageRangeButton(true,1,pLayout->countPages());
	pDialog->setEnablePrintSelection(false);	// TODO change this when we know how to do it.
	pDialog->setEnablePrintToFile(true);
	pDialog->setTryToBypassActualDialog(bTryToSuppressDialog);

//
// Turn on Wait cursor
//
	pView->setCursorWait();
//		s_pLoadingFrame = pFrame;
//		s_pLoadingDoc = static_cast<AD_Document *>(doc);

	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	std::string msg;
	pSS->getValueUTF8(AP_STRING_ID_MSG_PrintingDoc, msg);

	pFrame->setStatusMessage (msg);

	// TODO these are here temporarily to make printing work.  We'll fix the hack later.
	// BUGBUG assumes all pages are same size and orientation
	UT_sint32 iWidth = pLayout->getWidth();
	UT_sint32 iHeight = pLayout->getHeight() / pLayout->countPages();

	GR_Graphics * pGraphics = dynamic_cast<XAP_CocoaDialog_Print*>(pDialog)->getPrinterGraphicsContext();
	UT_ASSERT(pGraphics->queryProperties(GR_Graphics::DGP_PAPER));

	FL_DocLayout * pDocLayout = new FL_DocLayout(doc,pGraphics);
	FV_View * pPrintView = new FV_View(XAP_App::getApp(),0,pDocLayout);
	pPrintView->getLayout()->fillLayouts();
	pPrintView->getLayout()->formatAll();


//	const char *pDocName = ((doc->getFilename()) ? doc->getFilename() : pFrame->getNonDecoratedTitle());

	dynamic_cast<XAP_CocoaDialog_Print*>(pDialog)->runPrint(pFrame, pPrintView, iWidth, iHeight);

//	s_actuallyPrint(doc, pGraphics, pPrintView, pDocName, nCopies, bCollate,
//			iWidth,  iHeight, nToPage, nFromPage);

	delete pDocLayout;
	delete pPrintView;

	pDialog->releasePrinterGraphicsContext(pGraphics);

//
// Turn off wait cursor
//
	pView->clearCursorWait();
//	s_pLoadingFrame = NULL;


	XAP_Dialog_Print::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_Print::a_OK);


	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}
