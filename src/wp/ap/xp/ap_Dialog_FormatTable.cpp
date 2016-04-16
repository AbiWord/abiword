/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (c) 2009 Hubert Figuiere
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
#include "ut_debugmsg.h"

#include "xap_App.h"
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
#include "ap_Dialog_FormatTable.h"
#include "ut_png.h"
#include "gr_Painter.h"
#include "ut_units.h"
#include "ap_Strings.h"

AP_Dialog_FormatTable::AP_Dialog_FormatTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogformattable"),
	  m_borderColor(0,0,0),
	  m_lineStyle(LS_NORMAL),
	  m_bgFillStyle(NULL),
	
	  m_answer(a_OK),
	  m_pFormatTablePreview(NULL),
	  m_bSettingsChanged(false),
	  m_pAutoUpdaterMC(NULL),
	  m_borderToggled(false),
	  m_ApplyTo(FORMAT_TABLE_SELECTION),
	  m_bDestroy_says_stopupdating(false),
	  m_bAutoUpdate_happening_now(false),
	  m_iOldPos(0),
	  m_sImagePath(""),
	  m_iGraphicType(0),
	  m_pImage(NULL),
	  m_pGraphic(NULL)
{
	//
	// These are hardwired into the GUI.
	//
	const char * sThickness[FORMAT_TABLE_NUMTHICKNESS] ={"0.25pt","0.5pt",
													   "0.75pt","1.0pt",
													   "1.5pt","2.25pt","3pt",
													   "4.5pt","6.0pt"};
	UT_sint32 i = 0;
	for(i=0; i< FORMAT_TABLE_NUMTHICKNESS ;i++)
	{
		m_dThickness[i] = UT_convertToInches(sThickness[i]);
	}

	if(m_vecProps.getItemCount() > 0)
		m_vecProps.clear();
	  
	if(m_vecPropsAdjRight.getItemCount() > 0)
		m_vecPropsAdjRight.clear();
	  
	if(m_vecPropsAdjBottom.getItemCount() > 0)
		m_vecPropsAdjBottom.clear();
}

AP_Dialog_FormatTable::~AP_Dialog_FormatTable(void)
{
	stopUpdater();
	DELETEP(m_pFormatTablePreview);
	DELETEP(m_pGraphic);
	DELETEP(m_pImage);
}

AP_Dialog_FormatTable::tAnswer AP_Dialog_FormatTable::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_FormatTable::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_FormatTable::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	gchar * tmp = NULL;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_FormatTableTitle));
	BuildWindowName(static_cast<char *>(m_WindowName),static_cast<char*>(tmp),sizeof(m_WindowName));
	FREEP(tmp);
}

void AP_Dialog_FormatTable::startUpdater(void)
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;
	m_pAutoUpdaterMC =  UT_Timer::static_constructor(autoUpdateMC,this);
	m_pAutoUpdaterMC->set(100); // use a fast time, so the dialogs behaviour looks "snappy"
	m_pAutoUpdaterMC->start();
}

void AP_Dialog_FormatTable::stopUpdater(void)
{
	if(m_pAutoUpdaterMC == NULL)
	{
		return;
	}
	m_bDestroy_says_stopupdating = true;
	m_pAutoUpdaterMC->stop();
	DELETEP(m_pAutoUpdaterMC);
	m_pAutoUpdaterMC = NULL;
}
/*!
 Autoupdater of the dialog.
 */
void AP_Dialog_FormatTable::autoUpdateMC(UT_Worker * pTimer)
{
	UT_return_if_fail (pTimer);
	
	// get the dialog pointer from the timer instance data
	AP_Dialog_FormatTable * pDialog = static_cast<AP_Dialog_FormatTable *>(pTimer->getInstanceData());

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
void AP_Dialog_FormatTable::askForGraphicPathName(void)
{
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();

	UT_return_if_fail(pFrame);
	XAP_DialogFactory * pDialogFactory
		= static_cast<XAP_DialogFactory *>(pFrame->getDialogFactory());

	UT_return_if_fail(pDialogFactory);
	XAP_Dialog_FileOpenSaveAs * pDialog
		= static_cast<XAP_Dialog_FileOpenSaveAs *>(pDialogFactory->requestDialog(XAP_DIALOG_ID_INSERT_PICTURE));
	UT_return_if_fail (pDialog);

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
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	FG_Graphic* pFG = NULL;

	UT_Error errorCode;

	errorCode = IE_ImpGraphic::loadGraphic(m_sImagePath.c_str(), m_iGraphicType, &pFG);
	if(errorCode != UT_OK || !pFG)
	{
		ShowErrorBox(m_sImagePath, errorCode);
		return;
	}

	DELETEP(m_pGraphic);
	DELETEP(m_pImage);
	m_pGraphic = pFG->clone();
	GR_Graphics * pG = m_pFormatTablePreview->getGraphics();

	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	UT_return_if_fail(pView && pView->getDocument());

	UT_uint32 uid = pView->getDocument()->getUID(UT_UniqueId::Image); //see Bug 10852
	m_sImagePath.clear();
	UT_String_sprintf(m_sImagePath,"%d",uid);

    const UT_ByteBuf * pBB = m_pGraphic->getBuffer();
	if(m_pGraphic->getType() == FGT_Raster)
	{
		m_pImage = static_cast<GR_Image *>(
			pG->createNewImage( m_sImagePath.c_str(),
								pBB, pFG->getMimeType(),
								pFG->getWidth(),
								pFG->getHeight(),
								GR_Image::GRT_Raster));
	}
	else
	{
		m_pImage = static_cast<GR_Image *>(
			pG->createNewImage( m_sImagePath.c_str(),
								pBB, pFG->getMimeType(),
								m_pFormatTablePreview->getWindowWidth()-2,
								m_pFormatTablePreview->getWindowHeight()-2,
								GR_Image::GRT_Vector));
	}
	
	// draw the preview with the changed properties
	if(m_pFormatTablePreview)
		m_pFormatTablePreview->queueDraw();

}

void AP_Dialog_FormatTable::ShowErrorBox(UT_String & sFile, UT_Error errorCode)
{
	XAP_String_Id String_id;
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
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


/*! 
 Sets the sensitivity of the radio buttons to top/bottom/left/right line buttons
 Call this right after contructing the widget and before dropping into the main loop.
 */
void AP_Dialog_FormatTable::setAllSensitivities(void)
{
	XAP_Frame *frame = XAP_App::getApp()->getLastFocussedFrame();
	if (frame) {
		FV_View * pView = static_cast<FV_View *>(frame->getCurrentView());
		setSensitivity(pView->isInTable(pView->getPoint()));
	}
	else {
		setSensitivity(false);
	}
}

void AP_Dialog_FormatTable::setCurCellProps(void)
{
	XAP_Frame *frame = XAP_App::getApp()->getLastFocussedFrame();
	if (frame) {
		FV_View * pView = static_cast<FV_View *>(frame->getCurrentView());

		if (m_bSettingsChanged || m_iOldPos == pView->getPoint()) 
		{
		    //comparing the actual cell pos would be even better; but who cares :)
		    return;
		}
		
		m_iOldPos = pView->getPoint();
		PT_DocPosition pos = 0;pView->getPoint();
		if (pView->getSelectionAnchor() > pView->getPoint())
		{
		    pos = pView->getPoint() + 2;
		}
		/*
		 * update the border colors
		 */
		
		gchar * color = NULL;
		
		if (pView->getCellProperty(pos, "left-color", color))
			m_vecProps.addOrReplaceProp("left-color", color);
		else
			m_vecProps.removeProp("left-color");

		if (pView->getCellProperty(pos, "right-color", color))
			m_vecProps.addOrReplaceProp("right-color", color);
		else
			m_vecProps.removeProp("right-color");

		if (pView->getCellProperty(pos, "top-color", color))
			m_vecProps.addOrReplaceProp("top-color", color);
		else
			m_vecProps.removeProp("top-color");
		
		if (pView->getCellProperty(pos, "bot-color", color))
			m_vecProps.addOrReplaceProp("bot-color", color);
		else
			m_vecProps.removeProp("bot-color");
		
		/*
		 * update the background color
		 */

		UT_RGBColor clr;
		gchar * bgColor = NULL;
		if (pView->getCellProperty(pos, "background-color", bgColor))
		{
			m_vecProps.addOrReplaceProp("background-color", bgColor);
			clr.setColor(bgColor);
			setBackgroundColorInGUI(clr);
		}
		else
		{
			m_vecProps.removeProp("background-color");
			setBackgroundColorInGUI(UT_RGBColor(255,255,255)); // No color == white for now - MARCM
		}
		
		
		if(pView->isImageAtStrux(m_iOldPos,PTX_SectionCell))
		{
			if(pView->isInTable())
			{
				fl_BlockLayout * pBL = pView->getCurrentBlock();
				fl_CellLayout * pCell = static_cast<fl_CellLayout *>(pBL->myContainingLayout());
				if(pCell->getContainerType() != FL_CONTAINER_CELL)
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
						GR_Graphics * pG = m_pFormatTablePreview->getGraphics();
                        const UT_ByteBuf * pBB = pFG->getBuffer();
						if(m_pGraphic->getType() == FGT_Raster)
						{
							m_pImage = static_cast<GR_Image *>(
								pG->createNewImage( m_sImagePath.c_str(),
													pBB, pFG->getMimeType(),
													pFG->getWidth(),
													pFG->getHeight(),
													GR_Image::GRT_Raster));
						}
						else
						{
							m_pImage = static_cast<GR_Image *>(
								pG->createNewImage( m_sImagePath.c_str(),
													pBB, pFG->getMimeType(),
													m_pFormatTablePreview->getWindowWidth()-2,
													m_pFormatTablePreview->getWindowHeight()-2,
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
		m_vecProps.addOrReplaceProp("bg-style", bstmp.c_str());
		
		// draw the preview with the changed properties
		if(m_pFormatTablePreview)
			m_pFormatTablePreview->queueDraw();
	}
}

void AP_Dialog_FormatTable::setApplyFormatTo(FormatTable applyTo)
{
	m_ApplyTo = applyTo;
}

void AP_Dialog_FormatTable::applyChanges()
{
	UT_DEBUGMSG(("Doing apply changes number props %d \n",m_vecProps.getItemCount()));
	if (m_vecProps.getItemCount() == 0)
		return;

	FV_View * pView = static_cast<FV_View *>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentView());
	PP_PropertyVector propsArray;

	UT_sint32 i = m_vecProps.getItemCount();
	UT_sint32 j;
	for(j= 0; j<i; j=j+2)
	{
		propsArray.push_back(m_vecProps.getNthItem(j));
		propsArray.push_back(m_vecProps.getNthItem(j+1));
	}

	pView->setCellFormat(propsArray, m_ApplyTo,m_pGraphic,m_sImagePath);
	m_bSettingsChanged = false;
}

void AP_Dialog_FormatTable::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

/*!
 Set the color and style and thickness of the toggled button
 */
void AP_Dialog_FormatTable::toggleLineType(toggle_button btn, bool enabled)
{
	UT_String cTmp = UT_String_sprintf("%02x%02x%02x", m_borderColor.m_red, m_borderColor.m_grn, m_borderColor.m_blu);	
	UT_String sTmp = UT_String_sprintf("%d", (enabled ? m_lineStyle : LS_OFF));

	switch (btn)
	{
		case toggle_left:
		{
			m_vecProps.addOrReplaceProp("left-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("left-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("left-thickness",m_sBorderThickness.utf8_str());
		}
		break;
		case toggle_right:
		{	
			m_vecProps.addOrReplaceProp("right-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("right-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("right-thickness",m_sBorderThickness.utf8_str());
		}
		break;
		case toggle_top:
		{			
			m_vecProps.addOrReplaceProp("top-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("top-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("top-thickness",m_sBorderThickness.utf8_str());
		}
		break;
		case toggle_bottom:
		{			
			m_vecProps.addOrReplaceProp("bot-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("bot-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("bot-thickness",m_sBorderThickness.utf8_str());
		}
		break;
	}
	
	m_borderToggled = true;
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatTable::setBorderThickness(UT_UTF8String & sThick)
{
	m_sBorderThickness = sThick;
	if(m_borderToggled)
		return;
	m_vecProps.addOrReplaceProp("left-thickness", m_sBorderThickness.utf8_str());
	m_vecProps.addOrReplaceProp("right-thickness",m_sBorderThickness.utf8_str());
	m_vecProps.addOrReplaceProp("top-thickness",m_sBorderThickness.utf8_str());
	m_vecProps.addOrReplaceProp("bot-thickness",m_sBorderThickness.utf8_str());
	
	m_bSettingsChanged = true;

}

void AP_Dialog_FormatTable::setBorderColor(UT_RGBColor clr)
{
	m_borderColor = clr;
	
	if (m_borderToggled)
		return;

	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	

	m_vecProps.addOrReplaceProp("left-color", s.c_str());
	m_vecProps.addOrReplaceProp("right-color", s.c_str());
	m_vecProps.addOrReplaceProp("top-color", s.c_str());
	m_vecProps.addOrReplaceProp("bot-color", s.c_str());
	
	m_vecPropsAdjRight.addOrReplaceProp("left-color", s.c_str());
	m_vecPropsAdjBottom.addOrReplaceProp("top-color", s.c_str());
	
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatTable::clearImage(void)
{
	DELETEP(m_pGraphic);
	DELETEP(m_pImage);
	m_sImagePath.clear();
	// draw the preview with the changed properties
	if(m_pFormatTablePreview)
		m_pFormatTablePreview->queueDraw();

}

void AP_Dialog_FormatTable::setBackgroundColor(UT_RGBColor clr)
{
	UT_String bgcol = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);

	m_vecProps.removeProp ("bg-style"); // Why do we remove this property?  We still use it in frames. -MG
	m_vecProps.removeProp ("bgcolor"); // this is only here for backward compatibility with AbiWord < 2.0. Could be removed as far as I can see - MARCM

	if (clr.isTransparent ())
		m_vecProps.removeProp ("background-color");
	else
		m_vecProps.addOrReplaceProp ("background-color", bgcol.c_str ());

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatTable::_createPreviewFromGC(GR_Graphics * gc,
											     UT_uint32 width,
											     UT_uint32 height)
{
	UT_return_if_fail (gc);

	delete m_pFormatTablePreview;
	m_pFormatTablePreview = new AP_FormatTable_preview(gc,this);
	UT_return_if_fail (m_pFormatTablePreview);

	m_pFormatTablePreview->setWindowSize(width, height);
}

bool AP_Dialog_FormatTable::_getToggleButtonStatus(const char * lineStyle)
{
	const gchar * pszStyle = NULL;
	UT_String lsOff = UT_String_sprintf("%d", LS_OFF);	

	m_vecProps.getProp(lineStyle, pszStyle);

	if ((pszStyle && strcmp(pszStyle, lsOff.c_str())) || 
		!pszStyle)
		return true;
	else
		return false;
}

bool AP_Dialog_FormatTable::getTopToggled()
{
	return _getToggleButtonStatus("top-style");
}

bool AP_Dialog_FormatTable::getBottomToggled()
{
	return _getToggleButtonStatus("bot-style");
}

bool AP_Dialog_FormatTable::getRightToggled()
{
	return _getToggleButtonStatus("right-style");
}

bool AP_Dialog_FormatTable::getLeftToggled()
{
	return _getToggleButtonStatus("left-style");
}


guint AP_Dialog_FormatTable::_findClosestThickness(const char *sthickness) const
{
	double thickness = UT_convertToInches(sthickness);
	guint i =0;
	guint closest = 0;
	double dClose = 100000000.;
	for(i=0; i<FORMAT_TABLE_NUMTHICKNESS; i++)
	{
		double diff = thickness - m_dThickness[i];
		if(diff < 0)
			diff = -diff;
		if(diff < dClose)
		{
			closest = i;
			dClose = diff;
		}
	}
	return closest;
}


//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_FormatTable_preview::AP_FormatTable_preview(GR_Graphics * gc, AP_Dialog_FormatTable * pFormatTable)
	: XAP_Preview(gc)
{
	m_pFormatTable = pFormatTable;
}

AP_FormatTable_preview::~AP_FormatTable_preview()
{
}

void AP_FormatTable_preview::draw(const UT_Rect *clip)
{
	UT_UNUSED(clip);
	GR_Painter painter(m_gc);

	UT_sint32 iWidth = m_gc->tlu (getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu (getWindowHeight());
	UT_Rect pageRect(m_gc->tlu(7), m_gc->tlu(7), iWidth - m_gc->tlu(14), iHeight - m_gc->tlu(14));	
	
	painter.fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	painter.clearArea(pageRect.left, pageRect.top, pageRect.width, pageRect.height);	
	
	
	UT_RGBColor tmpCol;
	
	UT_RGBColor black(0, 0, 0);
	m_gc->setLineWidth(m_gc->tlu(1));
	
	int border = m_gc->tlu(20);
	int cornerLength = m_gc->tlu(5);

//
//  Draw the cell background
//
	
	const gchar * pszBGCol = NULL;
	if(m_pFormatTable->getImage())
	{
		GR_Image * pImg = m_pFormatTable->getImage();
		FG_Graphic * pFG = m_pFormatTable->getGraphic();
		const char * szName = pFG->getDataId();
        const UT_ByteBuf * pBB = pFG->getBuffer();
		if(pFG->getType() == FGT_Raster)
		{
			pImg = static_cast<GR_Image *>(
				m_gc->createNewImage( szName,
									pBB, pFG->getMimeType(),
									pageRect.width - 2*border,
									pageRect.height - 2*border,
									GR_Image::GRT_Raster));
		}
		else
		{
			pImg = static_cast<GR_Image *>(
				m_gc->createNewImage( szName,
                                      pBB, pFG->getMimeType(),
									pageRect.width - 2*border,
									pageRect.height - 2*border,
									GR_Image::GRT_Vector));
		}

		UT_Rect rec(pageRect.left + border, pageRect.top + border, 
					pageRect.width - 2*border, pageRect.height - 2*border);
		painter.drawImage(pImg,pageRect.left + border, pageRect.top + border);
		delete pImg;
	}
	else
	{
		m_pFormatTable->getPropVector().getProp(static_cast<const gchar *>("background-color"), pszBGCol);
		if (pszBGCol && *pszBGCol)
		{
			UT_parseColor(pszBGCol, tmpCol);
			painter.fillRect(tmpCol, pageRect.left + border, pageRect.top + border, pageRect.width - 2*border, pageRect.height - 2*border);
		}
	}

//
//  Draw the cell corners
//
	
	m_gc->setColor(UT_RGBColor(127,127,127));
	
	// top left corner
	painter.drawLine(pageRect.left + border - cornerLength, pageRect.top + border,
				   pageRect.left + border, pageRect.top + border);
	painter.drawLine(pageRect.left + border, pageRect.top + border  - cornerLength,
				   pageRect.left + border, pageRect.top + border);

	// top right corner
	painter.drawLine(pageRect.left + pageRect.width - border + cornerLength, pageRect.top + border,
				   pageRect.left + pageRect.width - border, pageRect.top + border);
	painter.drawLine(pageRect.left + pageRect.width - border, pageRect.top + border - cornerLength,
				   pageRect.left + pageRect.width - border, pageRect.top + border);

	// bottom left corner
	painter.drawLine(pageRect.left + border - cornerLength, pageRect.top + pageRect.height - border,
				   pageRect.left + border, pageRect.top + pageRect.height - border);
	painter.drawLine(pageRect.left + border, pageRect.top + pageRect.height - border + cornerLength,
				   pageRect.left + border, pageRect.top + pageRect.height - border);

	// bottom right corner
	painter.drawLine(pageRect.left + pageRect.width - border + cornerLength, pageRect.top + pageRect.height - border,
				   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	painter.drawLine(pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border + cornerLength,
				   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);

//
//  Draw the cell borders
//
	
	// top border
	if (m_pFormatTable->getTopToggled())
	{
		const gchar * pszTopColor = NULL;
		m_pFormatTable->getPropVector().getProp("top-color", pszTopColor);
		if (pszTopColor)
		{
			UT_parseColor(pszTopColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		const gchar * pszTopThickness = NULL;
		m_pFormatTable->getPropVector().getProp("top-thickness", pszTopThickness);
		if(pszTopThickness)
		{
			UT_sint32 iTopThickness = UT_convertToLogicalUnits(pszTopThickness);
			m_gc->setLineWidth(iTopThickness);
		}
		else
		{
			m_gc->setLineWidth(m_gc->tlu(1));
		}

		painter.drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + border);
	}

	// left border
	if (m_pFormatTable->getLeftToggled())
	{
		const gchar * pszLeftColor = NULL;
		m_pFormatTable->getPropVector().getProp("left-color", pszLeftColor);
		if (pszLeftColor)
		{
			UT_parseColor(pszLeftColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		const gchar * pszLeftThickness = NULL;
		m_pFormatTable->getPropVector().getProp("left-thickness", pszLeftThickness);
		if(pszLeftThickness)
		{
			UT_sint32 iLeftThickness = UT_convertToLogicalUnits(pszLeftThickness);
			m_gc->setLineWidth(iLeftThickness);
		}
		else
		{
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + border, pageRect.top + pageRect.height - border);
	}

	// right border
	if (m_pFormatTable->getRightToggled())
	{
		const gchar * pszRightColor = NULL;
		m_pFormatTable->getPropVector().getProp("right-color", pszRightColor);
		if (pszRightColor)
		{
			UT_parseColor(pszRightColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		const gchar * pszRightThickness = NULL;
		m_pFormatTable->getPropVector().getProp("right-thickness", pszRightThickness);
		if(pszRightThickness)
		{
			UT_sint32 iRightThickness = UT_convertToLogicalUnits(pszRightThickness);
			m_gc->setLineWidth(iRightThickness);
		}
		else
		{
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + pageRect.width - border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
	
	// bottom border
	if (m_pFormatTable->getBottomToggled())
	{
		const gchar * pszBottomColor = NULL;
		m_pFormatTable->getPropVector().getProp("bot-color", pszBottomColor);
		if (pszBottomColor)
		{
			UT_parseColor(pszBottomColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		const gchar * pszBotThickness = NULL;
		m_pFormatTable->getPropVector().getProp("bot-thickness", pszBotThickness);
		if(pszBotThickness)
		{
			UT_sint32 iBotThickness = UT_convertToLogicalUnits(pszBotThickness);
			m_gc->setLineWidth(iBotThickness);
		}
		else
		{
			m_gc->setLineWidth(m_gc->tlu(1));
		}
		painter.drawLine(pageRect.left + border, pageRect.top + pageRect.height - border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
}
