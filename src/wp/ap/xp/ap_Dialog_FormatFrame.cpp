/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
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
#include <stdio.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pt_Types.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_ContainerObject.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fl_BlockLayout.h"
#include "fl_DocLayout.h"
#include "ut_timer.h"
#include "fg_GraphicRaster.h"
#include "fg_GraphicVector.h"
#include "ap_Dialog_FormatFrame.h"
#include "ut_png.h"

AP_Dialog_FormatFrame::AP_Dialog_FormatFrame(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogformattable"),
	  m_borderColor(0,0,0),
	  m_lineStyle(LS_NORMAL),
	  m_bgFillStyle(NULL),
	
	  m_answer(a_OK),
	  m_pFormatFramePreview(NULL),
	  m_bSettingsChanged(false),
	  m_pAutoUpdaterMC(NULL),
	  m_borderToggled(false),
	  m_bDestroy_says_stopupdating(false),
	  m_bAutoUpdate_happening_now(false),
	  m_iOldPos(0),
	  m_sImagePath(""),
	  m_iGraphicType(0),
	  m_pImage(NULL),
	  m_pGraphic(NULL)
{
	if(m_vecProps.getItemCount() > 0)
		m_vecProps.clear();
	  
	if(m_vecPropsAdjRight.getItemCount() > 0)
		m_vecPropsAdjRight.clear();
	  
	if(m_vecPropsAdjBottom.getItemCount() > 0)
		m_vecPropsAdjBottom.clear();
}

AP_Dialog_FormatFrame::~AP_Dialog_FormatFrame(void)
{
	stopUpdater();
	DELETEP(m_pFormatFramePreview);
	DELETEP(m_pGraphic);
	DELETEP(m_pImage);
}

AP_Dialog_FormatFrame::tAnswer AP_Dialog_FormatFrame::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_FormatFrame::setActiveFrame(XAP_Frame *pFrame)
{
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_FormatFrame::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * tmp = NULL;
	UT_uint32 title_width = 26;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_FormatFrameTitle));
	BuildWindowName(static_cast<char *>(m_WindowName),static_cast<char*>(tmp),title_width);
	FREEP(tmp);
}

void AP_Dialog_FormatFrame::startUpdater(void)
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;
	GR_Graphics *pG = NULL;
	m_pAutoUpdaterMC =  UT_Timer::static_constructor(autoUpdateMC,this,pG);
	m_pAutoUpdaterMC->set(100); // use a fast time, so the dialogs behaviour looks "snappy"
	m_pAutoUpdaterMC->start();
}

void AP_Dialog_FormatFrame::stopUpdater(void)
{
	if(m_pAutoUpdaterMC == NULL)
	{
		return;
	}
	m_bDestroy_says_stopupdating = true;
	while (m_bAutoUpdate_happening_now == true) 
	  ;
	m_pAutoUpdaterMC->stop();
	DELETEP(m_pAutoUpdaterMC);
	m_pAutoUpdaterMC = NULL;
}
/*!
 Autoupdater of the dialog.
 */
void AP_Dialog_FormatFrame::autoUpdateMC(UT_Worker * pTimer)
{
	UT_ASSERT(pTimer);
	
	// get the dialog pointer from the timer instance data
	AP_Dialog_FormatFrame * pDialog = static_cast<AP_Dialog_FormatFrame *>(pTimer->getInstanceData());

	// Handshaking code
	if( pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->setAllSensitivities();
		pDialog->setCurCellProps();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

/*!
 * Import graphic for cell background.
 */
void AP_Dialog_FormatFrame::askForGraphicPathName(void)
{
	XAP_Frame * pFrame = m_pApp->getLastFocussedFrame();

	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_INSERT_PICTURE));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(NULL);
	pDialog->setSuggestFilename(false);

	// to fill the file types popup list, we need to convert AP-level
	// ImpGraphic descriptions, suffixes, and types into strings.

	UT_uint32 filterCount = IE_ImpGraphic::getImporterCount();

	const char ** szDescList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	const char ** szSuffixList = static_cast<const char **>(UT_calloc(filterCount + 1, sizeof(char *)));
	IEGraphicFileType * nTypeList = (IEGraphicFileType *)
		 UT_calloc(filterCount + 1,	sizeof(IEGraphicFileType));
	UT_uint32 k = 0;

	while (IE_ImpGraphic::enumerateDlgLabels(k, &szDescList[k], &szSuffixList[k], &nTypeList[k]))
		k++;

	pDialog->setFileTypeList(szDescList, szSuffixList, static_cast<const UT_sint32 *>(nTypeList));
	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
	{
		m_sImagePath = pDialog->getPathname();
		UT_sint32 type = pDialog->getFileType();

		// If the number is negative, it's a special type.
		// Some operating systems which depend solely on filename
		// suffixes to identify type (like Windows) will always
		// want auto-detection.
		if (type < 0)
			switch (type)
			{
			case XAP_DIALOG_FILEOPENSAVEAS_FILE_TYPE_AUTO:
				// do some automagical detecting
				m_iGraphicType = IEGFT_Unknown;
				break;
			default:
				// it returned a type we don't know how to handle
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			}
		else
			m_iGraphicType = static_cast<IEGraphicFileType>(pDialog->getFileType());
	}

	FREEP(szDescList);
	FREEP(szSuffixList);
	FREEP(nTypeList);

	pDialogFactory->releaseDialog(pDialog);
	if(m_sImagePath.size() == 0)
	{
		return;
	}

	IE_ImpGraphic *pIEG = NULL;
	FG_Graphic* pFG = NULL;

	UT_Error errorCode;

	errorCode = IE_ImpGraphic::constructImporter(m_sImagePath.c_str(), m_iGraphicType, &pIEG);
	if(errorCode != UT_OK)
	{
		ShowErrorBox(m_sImagePath, errorCode);
		return;
	}

	errorCode = pIEG->importGraphic( m_sImagePath.c_str(), &pFG);
	if(errorCode != UT_OK || !pFG)
	{
		ShowErrorBox(m_sImagePath, errorCode);
		DELETEP(pIEG);
		return;
	}

	DELETEP(pIEG);
	DELETEP(m_pGraphic);
	DELETEP(m_pImage);
	m_pGraphic = pFG->clone();
	GR_Graphics * pG = m_pFormatFramePreview->getGraphics();
	if(m_pGraphic->getType() == FGT_Raster)
	{
		UT_sint32 iImageWidth;
		UT_sint32 iImageHeight;
		UT_ByteBuf * pBB = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();
		UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
		m_pImage = static_cast<GR_Image *>(
			pG->createNewImage( m_sImagePath.c_str(),
								pBB,
								iImageWidth,
								iImageHeight,
								GR_Image::GRT_Raster));
	}
	else
	{
		m_pImage = static_cast<GR_Image *>(
			pG->createNewImage( m_sImagePath.c_str(),
								static_cast<FG_GraphicVector *>(pFG)->getVector_SVG(),
								m_pFormatFramePreview->getWindowWidth()-2,
								m_pFormatFramePreview->getWindowHeight()-2,
								GR_Image::GRT_Vector));
	}
	
	// draw the preview with the changed properties
	if(m_pFormatFramePreview)
		m_pFormatFramePreview->draw();

}

void AP_Dialog_FormatFrame::ShowErrorBox(UT_String & sFile, UT_Error errorCode)
{
	XAP_String_Id String_id;
	XAP_Frame * pFrame = m_pApp->getLastFocussedFrame();
	switch (errorCode)
	  {
	  case -301:
		String_id = AP_STRING_ID_MSG_IE_FileNotFound;
		break;

	  case -302:
		String_id = AP_STRING_ID_MSG_IE_NoMemory;
		break;

	  case -303:
		String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
		//AP_STRING_ID_MSG_IE_UnknownType;
		break;

	  case -304:
		String_id = AP_STRING_ID_MSG_IE_BogusDocument;
		break;

	  case -305:
		String_id = AP_STRING_ID_MSG_IE_CouldNotOpen;
		break;

	  case -306:
		String_id = AP_STRING_ID_MSG_IE_CouldNotWrite;
		break;

	  case -307:
		String_id = AP_STRING_ID_MSG_IE_FakeType;
		break;

	  case -311:
		String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
		break;

	  default:
		String_id = AP_STRING_ID_MSG_ImportError;
	  }

	pFrame->showMessageBox(String_id,
						   XAP_Dialog_MessageBox::b_O,
						   XAP_Dialog_MessageBox::a_OK,
						   sFile.c_str());
}



void AP_Dialog_FormatFrame::addOrReplaceVecProp(UT_Vector &vec,
												const XML_Char * pszProp,
												const XML_Char * pszVal)
{
	UT_sint32 iCount = vec.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		char * prop = NULL;
		char * val = NULL;
		CLONEP(prop, pszProp);
		CLONEP(val, pszVal);
		vec.addItem(static_cast<void *>(prop));
		vec.addItem(static_cast<void *>(val));
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = reinterpret_cast<const XML_Char *>(vec.getNthItem(i));
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		char * pVal = static_cast<char *>(vec.getNthItem(i+1));
		FREEP(pVal);
		char * val = NULL;
		CLONEP(val, pszVal);
		vec.setNthItem(i+1, static_cast<void *>(val), NULL);
	}
	else
	{
		char * prop = NULL;
		char * val = NULL;
		CLONEP(prop, pszProp);
		CLONEP(val, pszVal);
		vec.addItem(static_cast<void *>(prop));
		vec.addItem(static_cast<void *>(val));
	}
	return;
}

void AP_Dialog_FormatFrame::getVecProp(UT_Vector &vec,
									   const XML_Char * pszProp,
									   const XML_Char * &pszVal)
{
	UT_sint32 iCount = vec.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = reinterpret_cast<const XML_Char *>(vec.getNthItem(i));
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		pszVal = reinterpret_cast<const XML_Char *>(vec.getNthItem(i+1));
	}
	return;
}

/*!
 Removes the key,value pair  (pszProp,pszVal) given by pszProp
 from the Vector of all properties of the current format.
 If the Property does not exists nothing happens
 \param UT_Vector &vec the vector to remove the pair from
 \param const XML_Char * pszProp the property name
*/
void AP_Dialog_FormatFrame::removeVecProp(UT_Vector &vec, const XML_Char * pszProp)
{
	UT_sint32 iCount = vec.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = reinterpret_cast<const XML_Char *>(vec.getNthItem(i));
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		char * pSP = static_cast<char *>(vec.getNthItem(i));
		char * pSV = static_cast<char *>(vec.getNthItem(i+1));
		FREEP(pSP);
		FREEP(pSV);
		vec.deleteNthItem(i+1);
		vec.deleteNthItem(i);
	}
	return;
}

/*! 
 Sets the sensitivity of the radio buttons to top/bottom/left/right line buttons
 Call this right after contructing the widget and before dropping into the main loop.
 */
void AP_Dialog_FormatFrame::setAllSensitivities(void)
{
    FV_View * pView = static_cast<FV_View *>(m_pApp->getLastFocussedFrame()->getCurrentView());
	setSensitivity(pView->isInTable());
}

void AP_Dialog_FormatFrame::setCurCellProps(void)
{
	FV_View * pView = static_cast<FV_View *>(m_pApp->getLastFocussedFrame()->getCurrentView());

	if (m_bSettingsChanged || 
		m_iOldPos == pView->getPoint()) // comparing the actual cell pos would be even better; but who cares :)
		return;
	
	m_iOldPos = pView->getPoint();

	XML_Char * bgColor = NULL;
	if (pView->getCellBGColor (bgColor))
	{
		addOrReplaceVecProp(m_vecProps, "background-color", bgColor);
	}
	else
	{
		removeVecProp(m_vecProps, "background-color");
	}
	if(pView->isImageAtStrux(m_iOldPos,PTX_SectionCell))
	{
		if(pView->isInTable())
		{
			fl_BlockLayout * pBL = pView->getCurrentBlock();
			fl_CellLayout * pCell = static_cast<fl_CellLayout *>(pBL->myContainingLayout());
			if(pCell->getContainerType() != FL_CONTAINER_CELL)
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				DELETEP(m_pGraphic);
				DELETEP(m_pImage);
				m_sImagePath.clear();
			}
			else
			{
				FG_Graphic * pFG = FG_GraphicRaster::createFromStrux(pCell);
				if(pFG)
				{
					DELETEP(m_pGraphic);
					DELETEP(m_pImage);
					m_sImagePath.clear();
					m_pGraphic = pFG;
					m_sImagePath = pFG->getDataId();
					GR_Graphics * pG = m_pFormatFramePreview->getGraphics();
					if(m_pGraphic->getType() == FGT_Raster)
					{
						UT_sint32 iImageWidth;
						UT_sint32 iImageHeight;
						UT_ByteBuf * pBB = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();
						UT_PNG_getDimensions(pBB, iImageWidth, iImageHeight);
						m_pImage = static_cast<GR_Image *>(
							pG->createNewImage( m_sImagePath.c_str(),
												pBB,
												iImageWidth,
												iImageHeight,
												GR_Image::GRT_Raster));
					}
					else
					{
						m_pImage = static_cast<GR_Image *>(
							pG->createNewImage( m_sImagePath.c_str(),
												static_cast<FG_GraphicVector *>(pFG)->getVector_SVG(),
												m_pFormatFramePreview->getWindowWidth()-2,
												m_pFormatFramePreview->getWindowHeight()-2,
												GR_Image::GRT_Vector));
					}
				}
			}
		}
		else
		{
			DELETEP(m_pGraphic);
			DELETEP(m_pImage);
			m_sImagePath.clear();
		}
	}
	else
	{
		DELETEP(m_pGraphic);
		DELETEP(m_pImage);
		m_sImagePath.clear();
	}

	UT_String bstmp = UT_String_sprintf("%d", FS_FILL);
    addOrReplaceVecProp(m_vecProps, "bg-style", bstmp.c_str());
	
	// draw the preview with the changed properties
	if(m_pFormatFramePreview)
		m_pFormatFramePreview->draw();
}

void AP_Dialog_FormatFrame::applyChanges()
{
	UT_DEBUGMSG(("Doing apply changes number props %d \n",m_vecProps.getItemCount()));
	if (m_vecProps.getItemCount() == 0)
		return;

    FV_View * pView = static_cast<FV_View *>(m_pApp->getLastFocussedFrame()->getCurrentView());
	const XML_Char ** propsArray  = new const XML_Char * [m_vecProps.getItemCount()+1];
	propsArray[m_vecProps.getItemCount()] = NULL;
	
	UT_sint32 i = m_vecProps.getItemCount();
	UT_sint32 j;
	for(j= 0; j<i; j=j+2)
	{
		propsArray[j] = static_cast<XML_Char *>(m_vecProps.getNthItem(j));
		propsArray[j+1] = static_cast<XML_Char *>(m_vecProps.getNthItem(j+1));
	}

	delete [] propsArray;
	m_bSettingsChanged = false;
}

void AP_Dialog_FormatFrame::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

/*!
 Set the color and style of the toggled button
 */
void AP_Dialog_FormatFrame::toggleLineType(toggle_button btn, bool enabled)
{
	UT_String cTmp = UT_String_sprintf("%02x%02x%02x", m_borderColor.m_red, m_borderColor.m_grn, m_borderColor.m_blu);	
	UT_String sTmp = UT_String_sprintf("%d", (enabled ? m_lineStyle : LS_OFF));

	switch (btn)
	{
		case toggle_left:
		{
			addOrReplaceVecProp(m_vecProps, "left-style", sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "left-color", cTmp.c_str());
		}
		break;
		case toggle_right:
		{	
			addOrReplaceVecProp(m_vecProps, "right-style", sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "right-color", cTmp.c_str());
		}
		break;
		case toggle_top:
		{			
			addOrReplaceVecProp(m_vecProps, "top-style", sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "top-color", cTmp.c_str());
		}
		break;
		case toggle_bottom:
		{			
			addOrReplaceVecProp(m_vecProps, "bot-style", sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "bot-color", cTmp.c_str());
		}
		break;
	}
	
	m_borderToggled = true;
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderColor(UT_RGBColor clr)
{
	m_borderColor = clr;
	
	if (m_borderToggled)
		return;

	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	

	addOrReplaceVecProp(m_vecProps, "left-color", s.c_str());
	addOrReplaceVecProp(m_vecProps, "right-color", s.c_str());
	addOrReplaceVecProp(m_vecProps, "top-color", s.c_str());
	addOrReplaceVecProp(m_vecProps, "bot-color", s.c_str());
	
	addOrReplaceVecProp(m_vecPropsAdjRight, "left-color", s.c_str());
	addOrReplaceVecProp(m_vecPropsAdjBottom, "top-color", s.c_str());
	
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::clearImage(void)
{
	DELETEP(m_pGraphic);
	DELETEP(m_pImage);
	m_sImagePath.clear();
	// draw the preview with the changed properties
	if(m_pFormatFramePreview)
		m_pFormatFramePreview->draw();

}

void AP_Dialog_FormatFrame::setBGColor(UT_RGBColor clr)
{
	UT_String bgcol = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);

	removeVecProp (m_vecProps, "bg-style");
	removeVecProp (m_vecProps, "bgcolor");

	if (clr.isTransparent ())
		removeVecProp (m_vecProps, "background-color");
	else
		addOrReplaceVecProp (m_vecProps, "background-color", bgcol.c_str ());

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::_createPreviewFromGC(GR_Graphics * gc,
											     UT_uint32 width,
											     UT_uint32 height)
{
	UT_ASSERT(gc);

	m_pFormatFramePreview = new AP_FormatFrame_preview(gc,this);
	UT_ASSERT(m_pFormatFramePreview);

	m_pFormatFramePreview->setWindowSize(width, height);
}

bool AP_Dialog_FormatFrame::_getToggleButtonStatus(const char * lineStyle)
{
	const XML_Char * pszStyle = NULL;
	UT_String lsOff = UT_String_sprintf("%d", LS_OFF);	

	getVecProp(m_vecProps, lineStyle, pszStyle);

	if ((pszStyle && strcmp(pszStyle, lsOff.c_str())) || 
		!pszStyle)
		return true;
	else
		return false;
}

bool AP_Dialog_FormatFrame::getTopToggled()
{
	return _getToggleButtonStatus("top-style");
}

bool AP_Dialog_FormatFrame::getBottomToggled()
{
	return _getToggleButtonStatus("bot-style");
}

bool AP_Dialog_FormatFrame::getRightToggled()
{
	return _getToggleButtonStatus("right-style");
}

bool AP_Dialog_FormatFrame::getLeftToggled()
{
	return _getToggleButtonStatus("left-style");
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_FormatFrame_preview::AP_FormatFrame_preview(GR_Graphics * gc, AP_Dialog_FormatFrame * pFormatFrame)
	: XAP_Preview(gc)
{
	m_pFormatFrame = pFormatFrame;
}

AP_FormatFrame_preview::~AP_FormatFrame_preview()
{
}

void AP_FormatFrame_preview::draw(void)
{
	UT_sint32 iWidth = m_gc->tlu (getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu (getWindowHeight());
	UT_Rect pageRect(m_gc->tlu(7), m_gc->tlu(7), iWidth - m_gc->tlu(14), iHeight - m_gc->tlu(14));	
	
	m_gc->fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	m_gc->clearArea(pageRect.left, pageRect.top, pageRect.width, pageRect.height);	
	
	
	UT_RGBColor tmpCol;
	
	UT_RGBColor black(0, 0, 0);
	m_gc->setLineWidth(m_gc->tlu(1));
	
	int border = m_gc->tlu(20);
	int cornerLength = m_gc->tlu(5);

//
//  Draw the cell background
//
	
	const XML_Char * pszBGCol = NULL;
	if(m_pFormatFrame->getImage())
	{
		GR_Image * pImg = m_pFormatFrame->getImage();
		FG_Graphic * pFG = m_pFormatFrame->getGraphic();
		const char * szName = pFG->getDataId();
		if(pFG->getType() == FGT_Raster)
		{
			UT_ByteBuf * pBB = static_cast<FG_GraphicRaster *>(pFG)->getRaster_PNG();
			pImg = static_cast<GR_Image *>(
				m_gc->createNewImage( szName,
									pBB,
									pageRect.width - 2*border,
									pageRect.height - 2*border,
									GR_Image::GRT_Raster));
		}
		else
		{
			pImg = static_cast<GR_Image *>(
				m_gc->createNewImage( szName,
									static_cast<FG_GraphicVector *>(pFG)->getVector_SVG(),
									pageRect.width - 2*border,
									pageRect.height - 2*border,
									GR_Image::GRT_Vector));
		}

		UT_Rect rec(pageRect.left + border, pageRect.top + border, 
					pageRect.width - 2*border, pageRect.height - 2*border);
		m_gc->drawImage(pImg,pageRect.left + border, pageRect.top + border);
		delete pImg;
	}
	else
	{
		m_pFormatFrame->getVecProp (m_pFormatFrame->m_vecProps,
				    static_cast<const XML_Char *>("background-color"), pszBGCol);
		if (pszBGCol && *pszBGCol)
		{
			UT_parseColor(pszBGCol, tmpCol);
			m_gc->fillRect(tmpCol, pageRect.left + border, pageRect.top + border, pageRect.width - 2*border, pageRect.height - 2*border);
		}
	}

//
//  Draw the cell corners
//
	
	m_gc->setColor(UT_RGBColor(127,127,127));
	
	// top left corner
	m_gc->drawLine(pageRect.left + border - cornerLength, pageRect.top + border,
				   pageRect.left + border, pageRect.top + border);
	m_gc->drawLine(pageRect.left + border, pageRect.top + border  - cornerLength,
				   pageRect.left + border, pageRect.top + border);

	// top right corner
	m_gc->drawLine(pageRect.left + pageRect.width - border + cornerLength, pageRect.top + border,
				   pageRect.left + pageRect.width - border, pageRect.top + border);
	m_gc->drawLine(pageRect.left + pageRect.width - border, pageRect.top + border - cornerLength,
				   pageRect.left + pageRect.width - border, pageRect.top + border);

	// bottom left corner
	m_gc->drawLine(pageRect.left + border - cornerLength, pageRect.top + pageRect.height - border,
				   pageRect.left + border, pageRect.top + pageRect.height - border);
	m_gc->drawLine(pageRect.left + border, pageRect.top + pageRect.height - border + cornerLength,
				   pageRect.left + border, pageRect.top + pageRect.height - border);

	// bottom right corner
	m_gc->drawLine(pageRect.left + pageRect.width - border + cornerLength, pageRect.top + pageRect.height - border,
				   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	m_gc->drawLine(pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border + cornerLength,
				   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);

//
//  Draw the cell borders
//
	
	// top border
	if (m_pFormatFrame->getTopToggled())
	{
		const XML_Char * pszTopColor = NULL;
		m_pFormatFrame->getVecProp(m_pFormatFrame->m_vecProps, "top-color", pszTopColor);
		if (pszTopColor)
		{
			UT_parseColor(pszTopColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + border);
	}

	// left border
	if (m_pFormatFrame->getLeftToggled())
	{
		const XML_Char * pszLeftColor = NULL;
		m_pFormatFrame->getVecProp(m_pFormatFrame->m_vecProps, "left-color", pszLeftColor);
		if (pszLeftColor)
		{
			UT_parseColor(pszLeftColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + border, pageRect.top + pageRect.height - border);
	}

	// right border
	if (m_pFormatFrame->getRightToggled())
	{
		const XML_Char * pszRightColor = NULL;
		m_pFormatFrame->getVecProp(m_pFormatFrame->m_vecProps, "right-color", pszRightColor);
		if (pszRightColor)
		{
			UT_parseColor(pszRightColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + pageRect.width - border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
	
	// bottom border
	if (m_pFormatFrame->getBottomToggled())
	{
		const XML_Char * pszBottomColor = NULL;
		m_pFormatFrame->getVecProp(m_pFormatFrame->m_vecProps, "bot-color", pszBottomColor);
		if (pszBottomColor)
		{
			UT_parseColor(pszBottomColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + border, pageRect.top + pageRect.height - border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
}
