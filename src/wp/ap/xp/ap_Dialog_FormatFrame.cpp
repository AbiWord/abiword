/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Marc Maurer
 * Copyright (C) 2009 Hubert Figuiere
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

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_debugmsg.h"
#include "ut_locale.h"

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
#include "ap_Dialog_FormatFrame.h"
#include "ap_Strings.h"
#include "ut_png.h"
#include "gr_Painter.h"


GR_Image * AP_Dialog_FormatFrame::_makeImageForRaster(const std::string & name, 
                              GR_Graphics * pGraphics,
                              const FG_Graphic * pG)
{
    GR_Image* pImage;
    const UT_ByteBuf * pBB = pG->getBuffer();
	if(pG->getType() == FGT_Raster)
	{
		pImage = pGraphics->createNewImage( name.c_str(),
								pBB, pG->getMimeType(),
								pG->getWidth(),
								pG->getHeight(),
								GR_Image::GRT_Raster);
	}
	else
	{
		pImage = pGraphics->createNewImage( name.c_str(),
                                pBB, pG->getMimeType(),
								m_pFormatFramePreview->getWindowWidth()-2,
								m_pFormatFramePreview->getWindowHeight()-2,
								GR_Image::GRT_Vector);
	}
    return pImage;
}


AP_Dialog_FormatFrame::AP_Dialog_FormatFrame(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogformattable"),
	  m_borderColor(0,0,0),
	  m_lineStyle(LS_NORMAL),
	
	  m_answer(a_OK),
	  m_pFormatFramePreview(NULL),
	  m_bSettingsChanged(false),

	  m_borderColorRight(0,0,0),
	  m_borderColorLeft(0,0,0),
	  m_borderColorTop(0,0,0),
	  m_borderColorBottom(0,0,0),

	  m_borderLineStyleRight(LS_NORMAL),
	  m_borderLineStyleLeft(LS_NORMAL),
	  m_borderLineStyleTop(LS_NORMAL),
	  m_borderLineStyleBottom(LS_NORMAL),

	  m_borderThicknessRight(1.0f),
	  m_borderThicknessLeft(1.0f),
	  m_borderThicknessTop(1.0f),
	  m_borderThicknessBottom(1.0f),

	  m_sBorderThickness("1.00pt"),

	  m_sBorderThicknessRight("1.00pt"),
	  m_sBorderThicknessLeft("1.00pt"),
	  m_sBorderThicknessTop("1.00pt"),
	  m_sBorderThicknessBottom("1.00pt"),

	  m_pAutoUpdaterMC(NULL),
	  m_bDestroy_says_stopupdating(false),
	  m_bAutoUpdate_happening_now(false),
	  m_iOldPos(0),
	  m_sImagePath(""),
	  m_iGraphicType(0),
	  m_pImage(NULL),
	  m_pGraphic(NULL),
	  m_bSensitive(false),
	  m_bSetWrapping(false),
	  m_bLineToggled(false),
	  m_iFramePositionTo(FL_FRAME_POSITIONED_TO_BLOCK)
{
	if(m_vecProps.getItemCount() > 0)
		m_vecProps.clear();
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

void AP_Dialog_FormatFrame::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_FormatFrame::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	gchar * tmp = NULL;
	UT_uint32 title_width = 26;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_FormatFrameTitle));
	BuildWindowName(static_cast<char *>(m_WindowName),static_cast<char*>(tmp),title_width);
	FREEP(tmp);
}

void AP_Dialog_FormatFrame::startUpdater(void)
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;
	m_pAutoUpdaterMC =  UT_Timer::static_constructor(autoUpdateMC,this);
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
	m_pAutoUpdaterMC->stop();
	DELETEP(m_pAutoUpdaterMC);
	m_pAutoUpdaterMC = NULL;
}
/*!
 Autoupdater of the dialog.
 */
void AP_Dialog_FormatFrame::autoUpdateMC(UT_Worker * pTimer)
{
	UT_return_if_fail (pTimer);
	
	// get the dialog pointer from the timer instance data
	AP_Dialog_FormatFrame * pDialog = static_cast<AP_Dialog_FormatFrame *>(pTimer->getInstanceData());

	// Handshaking code
	if( pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->setCurFrameProps();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

/*!
 * Import graphic for cell background.
 */
void AP_Dialog_FormatFrame::askForGraphicPathName(void)
{
	UT_return_if_fail(m_pApp);
	XAP_Frame * pFrame = m_pApp->getLastFocussedFrame();

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
	GR_Graphics * pG = m_pFormatFramePreview->getGraphics();

	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	UT_return_if_fail(pView && pView->getDocument());

	UT_uint32 uid = pView->getDocument()->getUID(UT_UniqueId::Image); //see Bug 10851
	m_sImagePath.clear();
	m_sImagePath = UT_std_string_sprintf("%d",uid);

    m_pImage = _makeImageForRaster(m_sImagePath, pG, m_pGraphic);

	// draw the preview with the changed properties
	if(m_pFormatFramePreview)
		m_pFormatFramePreview->draw();

}

void AP_Dialog_FormatFrame::ShowErrorBox(const std::string & sFile, UT_Error errorCode)
{
	XAP_String_Id String_id;
	XAP_Frame * pFrame = XAP_App::getApp()->getLastFocussedFrame();
	switch (errorCode)
	  {
	  case UT_IE_FILENOTFOUND:
		String_id = AP_STRING_ID_MSG_IE_FileNotFound;
		break;

	  case UT_IE_NOMEMORY:
		String_id = AP_STRING_ID_MSG_IE_NoMemory;
		break;

	  case UT_IE_UNKNOWNTYPE:
		String_id = AP_STRING_ID_MSG_IE_UnsupportedType;
		//AP_STRING_ID_MSG_IE_UnknownType;
		break;

	  case UT_IE_BOGUSDOCUMENT:
		String_id = AP_STRING_ID_MSG_IE_BogusDocument;
		break;

	  case UT_IE_COULDNOTOPEN:
		String_id = AP_STRING_ID_MSG_IE_CouldNotOpen;
		break;

	  case UT_IE_COULDNOTWRITE:
		String_id = AP_STRING_ID_MSG_IE_CouldNotWrite;
		break;

	  case UT_IE_FAKETYPE:
		String_id = AP_STRING_ID_MSG_IE_FakeType;
		break;

	  case UT_IE_UNSUPTYPE:
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
void AP_Dialog_FormatFrame::setAllSensitivities(void)
{
	XAP_Frame * frame = m_pApp->getLastFocussedFrame();
	if (frame) {
		FV_View * pView = static_cast<FV_View *>(frame->getCurrentView());
		if (pView) {
			bool bInFrame = m_bSensitive; // pView->isInFrame(pView->getPoint());
			setSensitivity(bInFrame);
		}
		else {
			setSensitivity(false);
		}
	}
	else {
		setSensitivity(false);
	}
}

void AP_Dialog_FormatFrame::setCurFrameProps(void)
{
	XAP_Frame * frame = m_pApp->getLastFocussedFrame();
	if (!frame) {
		if (m_bSensitive) {
			m_bSensitive = false;
			setAllSensitivities();
		}
		return;
	}

	FV_View * pView = static_cast<FV_View *>(frame->getCurrentView());
	if (!pView) {
		if (m_bSensitive) {
			m_bSensitive = false;
			setAllSensitivities();
		}
		return;
	}

	PT_DocPosition pos = pView->getPoint();

	if (/* m_bSettingsChanged || */ m_iOldPos == pos) {
		// comparing the actual cell pos would be even better; but who cares :)
		return;
	}
	m_iOldPos = pos;

	if (!pView->isInFrame(pos)) {
		if (m_bSensitive) {
			m_bSensitive = false;
			setAllSensitivities();
		}
		return;
	}

	m_bLineToggled = false;

	m_borderLineStyleRight  = LS_NORMAL;
	m_borderLineStyleLeft   = LS_NORMAL;
	m_borderLineStyleTop    = LS_NORMAL;
	m_borderLineStyleBottom = LS_NORMAL;

	UT_RGBColor black(0,0,0);
	UT_RGBColor white(255,255,255);

	m_borderColor = black;

	m_borderColorRight  = black;
	m_borderColorLeft   = black;
	m_borderColorTop    = black;
	m_borderColorBottom = black;

	m_borderThicknessRight  = 1.0f;
	m_borderThicknessLeft   = 1.0f;
	m_borderThicknessTop    = 1.0f;
	m_borderThicknessBottom = 1.0f;

	m_sBorderThickness = "1.00pt",
 
	m_sBorderThicknessRight  = "1.00pt";
	m_sBorderThicknessLeft   = "1.00pt";
	m_sBorderThicknessTop    = "1.00pt";
	m_sBorderThicknessBottom = "1.00pt";

	m_backgroundColor = white;

	m_bSetWrapping = false;

	m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;

	PD_Document * pDoc = pView->getDocument();

	PL_StruxDocHandle sdh;

	m_bSensitive = false;

	if (pDoc->getStruxOfTypeFromPosition(pos, PTX_SectionFrame, &sdh))
		if (PT_AttrPropIndex api = pDoc->getAPIFromSDH(sdh)) {
			const PP_AttrProp * pAP = 0;
			if (pDoc->getAttrProp(api, &pAP))
				if (pAP) {
					m_bSensitive = true;

#define REPLACE_CELL_PROPERTY(X) \
	do { \
		const gchar * prop = 0; \
		if (pAP->getProperty(X, prop)) \
			m_vecProps.addOrReplaceProp(X, prop); \
		else \
			m_vecProps.removeProp(X); \
	} while (0)

					REPLACE_CELL_PROPERTY("right-style");
					REPLACE_CELL_PROPERTY("left-style");
					REPLACE_CELL_PROPERTY("top-style");
					REPLACE_CELL_PROPERTY("bot-style");

					REPLACE_CELL_PROPERTY("right-thickness");
					REPLACE_CELL_PROPERTY("left-thickness");
					REPLACE_CELL_PROPERTY("top-thickness");
					REPLACE_CELL_PROPERTY("bot-thickness");

					REPLACE_CELL_PROPERTY("right-color");
					REPLACE_CELL_PROPERTY("left-color");
					REPLACE_CELL_PROPERTY("top-color");
					REPLACE_CELL_PROPERTY("bot-color");

					REPLACE_CELL_PROPERTY("background-color");

					REPLACE_CELL_PROPERTY("wrap-mode");
					REPLACE_CELL_PROPERTY("position-to");
				}
		}
	if (!m_bSensitive) {
		setAllSensitivities();
		return;
	}

	if(pView->isImageAtStrux(m_iOldPos,PTX_SectionFrame))
	{
		if(true /* pView->isInFrame(pView->getPoint()) */)
		{
			fl_BlockLayout * pBL = pView->getCurrentBlock();
			fl_FrameLayout * pFrame = static_cast<fl_FrameLayout *>(pBL->myContainingLayout());
			if(pFrame->getContainerType() != FL_CONTAINER_FRAME)
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				DELETEP(m_pGraphic);
				DELETEP(m_pImage);
				m_sImagePath.clear();
			}
			else
			{
				FG_Graphic * pFG = FG_GraphicRaster::createFromStrux(pFrame);
				if(pFG)
				{
					DELETEP(m_pGraphic);
					DELETEP(m_pImage);
					m_sImagePath.clear();
					m_pGraphic = pFG;
					m_sImagePath = pFG->getDataId();

					GR_Graphics * pG = m_pFormatFramePreview->getGraphics();
                    m_pImage = _makeImageForRaster(m_sImagePath, pG, m_pGraphic);
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
	m_vecProps.addOrReplaceProp("bg-style", bstmp.c_str()); // FIXME ??
		
	const gchar * pszStyle = 0;
	m_vecProps.getProp("background-color", pszStyle);
	if (pszStyle) {
		m_backgroundColor.setColor(pszStyle);
	}

	/* update border properties
	 */
	long linestyle;

	pszStyle = 0;
	m_vecProps.getProp("right-style", pszStyle);
	if (pszStyle) {
		linestyle = LS_NORMAL;
		sscanf(pszStyle, "%ld", &linestyle);
		m_borderLineStyleRight = linestyle;
	}
	pszStyle = 0;
	m_vecProps.getProp("left-style", pszStyle);
	if (pszStyle) {
		linestyle = LS_NORMAL;
		sscanf(pszStyle, "%ld", &linestyle);
		m_borderLineStyleLeft = linestyle;
	}
	pszStyle = 0;
	m_vecProps.getProp("top-style", pszStyle);
	if (pszStyle) {
		linestyle = LS_NORMAL;
		sscanf(pszStyle, "%ld", &linestyle);
		m_borderLineStyleTop = linestyle;
	}
	pszStyle = 0;
	m_vecProps.getProp("bot-style", pszStyle);
	if (pszStyle) {
		linestyle = LS_NORMAL;
		sscanf(pszStyle, "%ld", &linestyle);
		m_borderLineStyleBottom = linestyle;
	}

	pszStyle = 0;
	m_vecProps.getProp("right-color", pszStyle);
	if (pszStyle) {
		m_borderColorRight.setColor(pszStyle);
	}
	pszStyle = 0;
	m_vecProps.getProp("left-color", pszStyle);
	if (pszStyle) {
		m_borderColorLeft.setColor(pszStyle);
	}
	pszStyle = 0;
	m_vecProps.getProp("top-color", pszStyle);
	if (pszStyle) {
		m_borderColorTop.setColor(pszStyle);
	}
	pszStyle = 0;
	m_vecProps.getProp("bot-color", pszStyle);
	if (pszStyle) {
		m_borderColorBottom.setColor(pszStyle);
	}

	UT_UTF8String thickness;

	pszStyle = 0;
	m_vecProps.getProp("right-thickness", pszStyle);
	if (pszStyle) {
		thickness = pszStyle;
		setBorderThicknessRight(thickness);
	}
	pszStyle = 0;
	m_vecProps.getProp("left-thickness", pszStyle);
	if (pszStyle) {
		thickness = pszStyle;
		setBorderThicknessLeft(thickness);
	}
	pszStyle = 0;
	m_vecProps.getProp("top-thickness", pszStyle);
	if (pszStyle) {
		thickness = pszStyle;
		setBorderThicknessTop(thickness);
	}
	pszStyle = 0;
	m_vecProps.getProp("bot-thickness", pszStyle);
	if (pszStyle) {
		thickness = pszStyle;
		setBorderThicknessBottom(thickness);
	}

	/* update wrap properties
	 */
	pszStyle = 0;
	m_vecProps.getProp("wrap-mode", pszStyle);
	if (pszStyle) {
		if (strcmp (pszStyle, "wrapped-both") == 0) {
			m_bSetWrapping = true;
		}
	}

	/* update position properties
	 */
	pszStyle = 0;
	m_vecProps.getProp("position-to", pszStyle);
	if (pszStyle) {
		if (strcmp (pszStyle, "block-above-text") == 0) {
			m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
		}
		else if (strcmp (pszStyle, "column-above-text") == 0) {
			m_iFramePositionTo = FL_FRAME_POSITIONED_TO_COLUMN;
		}
		else if (strcmp (pszStyle, "page-above-text") == 0) {
			m_iFramePositionTo = FL_FRAME_POSITIONED_TO_PAGE;
		}
	}

	/* draw the preview with the changed properties
	 */
	if(m_pFormatFramePreview) {
		m_pFormatFramePreview->draw();
	}

	m_bSettingsChanged = false;
	setActiveFrame(frame); // this is just to trigger the subclass to update the dialog
}

void AP_Dialog_FormatFrame::setPositionMode(FL_FrameFormatMode mode)
{
	switch (mode) {
	default:
	case FL_FRAME_POSITIONED_TO_BLOCK:
		m_iFramePositionTo = FL_FRAME_POSITIONED_TO_BLOCK;
		m_vecProps.addOrReplaceProp("position-to", "block-above-text");
		break;

	case FL_FRAME_POSITIONED_TO_COLUMN:
		m_iFramePositionTo = FL_FRAME_POSITIONED_TO_COLUMN;
		m_vecProps.addOrReplaceProp("position-to", "column-above-text");
		break;

	case FL_FRAME_POSITIONED_TO_PAGE:
		m_iFramePositionTo = FL_FRAME_POSITIONED_TO_PAGE;
		m_vecProps.addOrReplaceProp("position-to", "page-above-text");
		break;
	}
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setWrapping(bool bWrap)
{
	m_bSetWrapping = bWrap;

	if (m_bSetWrapping)
		m_vecProps.addOrReplaceProp("wrap-mode", "wrapped-both");
	else
		m_vecProps.addOrReplaceProp("wrap-mode", "above-text");

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::applyChanges()
{
	UT_sint32 count = m_vecProps.getItemCount();

	UT_DEBUGMSG(("Doing apply changes number props %d \n", count));
	if (!count)
		return;

	FV_View * pView = static_cast<FV_View *>(m_pApp->getLastFocussedFrame()->getCurrentView());
	if (!pView)
		return;

	const gchar ** propsArray  = new const gchar * [count + 2];

	for (UT_sint32 j = 0; j < count; j = j + 2)
	{
		propsArray[j  ] = static_cast<gchar *>(m_vecProps.getNthItem(j));
		propsArray[j+1] = static_cast<gchar *>(m_vecProps.getNthItem(j+1));
	}
	propsArray[count  ] = 0;
	propsArray[count+1] = 0;

	pView->setFrameFormat(propsArray, m_pGraphic, m_sImagePath);
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
			m_borderLineStyleLeft = enabled ? LS_NORMAL : LS_OFF;
			m_borderColorLeft = m_borderColor;
			setBorderThicknessLeft(m_sBorderThickness);
			m_vecProps.addOrReplaceProp("left-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("left-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("left-thickness",m_sBorderThickness.utf8_str());
		}
		break;

		case toggle_right:
		{	
			m_borderLineStyleRight = enabled ? LS_NORMAL : LS_OFF;
			m_borderColorRight = m_borderColor;
			setBorderThicknessRight(m_sBorderThickness);
			m_vecProps.addOrReplaceProp("right-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("right-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("right-thickness",m_sBorderThickness.utf8_str());
		}
		break;

		case toggle_top:
		{			
			m_borderLineStyleTop = enabled ? LS_NORMAL : LS_OFF;
			m_borderColorTop = m_borderColor;
			setBorderThicknessTop(m_sBorderThickness);
			m_vecProps.addOrReplaceProp("top-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("top-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("top-thickness",m_sBorderThickness.utf8_str());
		}
		break;

		case toggle_bottom:
		{			
			m_borderLineStyleBottom = enabled ? LS_NORMAL : LS_OFF;
			m_borderColorBottom = m_borderColor;
			setBorderThicknessBottom(m_sBorderThickness);
			m_vecProps.addOrReplaceProp("bot-style", sTmp.c_str());
			m_vecProps.addOrReplaceProp("bot-color", cTmp.c_str());
			m_vecProps.addOrReplaceProp("bot-thickness",m_sBorderThickness.utf8_str());
		}
		break;

	default:
		// should not happen
		break;
	}
	m_bLineToggled = true;
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderLineStyleRight (UT_sint32 linestyle)
{
	char buf[16];
	sprintf (buf, "%ld", static_cast<unsigned long>(linestyle));

	m_vecProps.addOrReplaceProp("right-style", buf);

	m_borderLineStyleRight = linestyle;
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderLineStyleLeft (UT_sint32 linestyle)
{
	char buf[16];
	sprintf (buf, "%ld", static_cast<unsigned long>(linestyle));

	m_vecProps.addOrReplaceProp("left-style", buf);

	m_borderLineStyleLeft = linestyle;
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderLineStyleTop (UT_sint32 linestyle)
{
	char buf[16];
	sprintf (buf, "%ld", static_cast<unsigned long>(linestyle));

	m_vecProps.addOrReplaceProp("top-style", buf);

	m_borderLineStyleTop = linestyle;
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderLineStyleBottom (UT_sint32 linestyle)
{
	char buf[16];
	sprintf (buf, "%ld", static_cast<unsigned long>(linestyle));

	m_vecProps.addOrReplaceProp("bot-style", buf);

	m_borderLineStyleBottom = linestyle;
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderThickness(const UT_UTF8String & sThick)
{
	m_sBorderThickness = sThick;

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderThicknessAll(const UT_UTF8String & sThick)
{
	setBorderThicknessRight(sThick);
	setBorderThicknessLeft(sThick);
	setBorderThicknessTop(sThick);
	setBorderThicknessBottom(sThick);

	m_bSettingsChanged = true;
}

static UT_UTF8String s_canonical_thickness (float thickness)
{
	UT_UTF8String sThick;

	if (thickness < 0.01) {
		sThick = "0.01pt";
	}
	else if (thickness > 99.99) {
		sThick = "99.99pt";
	}
	else {
		char buf[16];
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		sprintf(buf, "%.2fpt", thickness);
		sThick = buf;
	}
	return sThick;
}

static UT_UTF8String s_canonical_thickness (const UT_UTF8String & sThickness, float & thickness)
{
	thickness = static_cast<float>(UT_convertToPoints(sThickness.utf8_str()));

	UT_UTF8String sThick;

	if (thickness < 0.01) {
		thickness = 0.01f;
		sThick = "0.01pt";
	}
	else if (thickness > 99.99) {
		thickness = 99.99f;
		sThick = "99.99pt";
	}
	else {
		char buf[16];
		UT_LocaleTransactor t(LC_NUMERIC, "C");
		sprintf(buf, "%.2fpt", thickness);
		sThick = buf;
	}
	return sThick;
}

void AP_Dialog_FormatFrame::setBorderThicknessRight (const UT_UTF8String & sThick)
{
	m_sBorderThicknessRight = s_canonical_thickness(sThick, m_borderThicknessRight);

	m_vecProps.addOrReplaceProp("right-thickness", m_sBorderThicknessRight.utf8_str());

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderThicknessLeft (const UT_UTF8String & sThick)
{
	m_sBorderThicknessLeft = s_canonical_thickness(sThick, m_borderThicknessLeft);

	m_vecProps.addOrReplaceProp("left-thickness", m_sBorderThicknessLeft.utf8_str());

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderThicknessTop (const UT_UTF8String & sThick)
{
	m_sBorderThicknessTop = s_canonical_thickness(sThick, m_borderThicknessTop);

	m_vecProps.addOrReplaceProp("top-thickness", m_sBorderThicknessTop.utf8_str());

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderThicknessBottom (const UT_UTF8String & sThick)
{
	m_sBorderThicknessBottom = s_canonical_thickness(sThick, m_borderThicknessBottom);

	m_vecProps.addOrReplaceProp("bot-thickness", m_sBorderThicknessBottom.utf8_str());

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderThicknessAll (float thickness)
{
	setBorderThicknessAll(s_canonical_thickness(thickness));
}

void AP_Dialog_FormatFrame::setBorderThicknessRight (float thickness)
{
	setBorderThicknessRight(s_canonical_thickness(thickness));
}

void AP_Dialog_FormatFrame::setBorderThicknessLeft (float thickness)
{
	setBorderThicknessLeft(s_canonical_thickness(thickness));
}

void AP_Dialog_FormatFrame::setBorderThicknessTop (float thickness)
{
	setBorderThicknessTop(s_canonical_thickness(thickness));
}

void AP_Dialog_FormatFrame::setBorderThicknessBottom (float thickness)
{
	setBorderThicknessBottom(s_canonical_thickness(thickness));
}

void AP_Dialog_FormatFrame::setBorderColor(UT_RGBColor clr)
{
	m_borderColor = clr;

	if (!m_bLineToggled)
		{
			setBorderColorAll(clr);
		}
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderColorAll(UT_RGBColor clr)
{
	setBorderColorRight(clr);
	setBorderColorLeft(clr);
	setBorderColorTop(clr);
	setBorderColorBottom(clr);
	
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderColorRight (const UT_RGBColor & clr)
{
	m_borderColorRight = clr;
	
	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	

	m_vecProps.addOrReplaceProp("right-color", s.c_str());
	
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderColorLeft (const UT_RGBColor & clr)
{
	m_borderColorLeft = clr;
	
	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	

	m_vecProps.addOrReplaceProp("left-color", s.c_str());
	
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderColorTop (const UT_RGBColor & clr)
{
	m_borderColorTop = clr;
	
	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	

	m_vecProps.addOrReplaceProp("top-color", s.c_str());
	
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::setBorderColorBottom (const UT_RGBColor & clr)
{
	m_borderColorBottom = clr;
	
	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	

	m_vecProps.addOrReplaceProp("bot-color", s.c_str());
	
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
	m_backgroundColor = clr;

	UT_String bgcol = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);

	m_vecProps.removeProp ("bg-style");
	m_vecProps.removeProp ("bgcolor");

	if (clr.isTransparent ())
		m_vecProps.removeProp ("background-color");
	else
		m_vecProps.addOrReplaceProp ("background-color", bgcol.c_str ());

	m_bSettingsChanged = true;
}

void AP_Dialog_FormatFrame::_createPreviewFromGC(GR_Graphics * gc,
											     UT_uint32 width,
											     UT_uint32 height)
{
	UT_return_if_fail (gc);

	DELETEP(m_pFormatFramePreview);
	m_pFormatFramePreview = new AP_FormatFrame_preview(gc,this);
	UT_return_if_fail (m_pFormatFramePreview);

	m_pFormatFramePreview->setWindowSize(width, height);
}

bool AP_Dialog_FormatFrame::_getToggleButtonStatus(const char * lineStyle)
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
	if(m_pFormatFrame->getImage())
	{
		GR_Image * pImg = m_pFormatFrame->getImage();
		FG_Graphic * pFG = m_pFormatFrame->getGraphic();
		const char * szName = pFG->getDataId();
        const UT_ByteBuf * pBB = static_cast<FG_GraphicRaster *>(pFG)->getBuffer();
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
		m_pFormatFrame->getPropVector().getProp(static_cast<const gchar *>("background-color"), pszBGCol);
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
	// right border
	if (m_pFormatFrame->getRightToggled())
	{
		UT_sint32 linestyle = m_pFormatFrame->borderLineStyleRight();
		if (linestyle == LS_DOTTED)
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_DOTTED);
		else if (linestyle == LS_DASHED)
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_ON_OFF_DASH);
		else
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_SOLID);

		m_gc->setColor(m_pFormatFrame->borderColorRight());

		UT_sint32 iRightThickness = UT_convertToLogicalUnits(m_pFormatFrame->getBorderThicknessRight().utf8_str());
		m_gc->setLineWidth(iRightThickness);

		painter.drawLine(pageRect.left + pageRect.width - border, pageRect.top + border,
						 pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}

	// left border
	if (m_pFormatFrame->getLeftToggled())
	{
		UT_sint32 linestyle = m_pFormatFrame->borderLineStyleLeft();
		if (linestyle == LS_DOTTED)
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_DOTTED);
		else if (linestyle == LS_DASHED)
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_ON_OFF_DASH);
		else
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_SOLID);

		m_gc->setColor(m_pFormatFrame->borderColorLeft());

		UT_sint32 iLeftThickness = UT_convertToLogicalUnits(m_pFormatFrame->getBorderThicknessLeft().utf8_str());
		m_gc->setLineWidth(iLeftThickness);

		painter.drawLine(pageRect.left + border, pageRect.top + border,
						 pageRect.left + border, pageRect.top + pageRect.height - border);
	}

	// top border
	if (m_pFormatFrame->getTopToggled())
	{
		UT_sint32 linestyle = m_pFormatFrame->borderLineStyleTop();
		if (linestyle == LS_DOTTED)
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_DOTTED);
		else if (linestyle == LS_DASHED)
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_ON_OFF_DASH);
		else
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_SOLID);

		m_gc->setColor(m_pFormatFrame->borderColorTop());		
		UT_sint32 iTopThickness = UT_convertToLogicalUnits(m_pFormatFrame->getBorderThicknessTop().utf8_str());
		m_gc->setLineWidth(iTopThickness);

		painter.drawLine(pageRect.left + border, pageRect.top + border,
						 pageRect.left + pageRect.width - border, pageRect.top + border);
	}

	// bottom border
	if (m_pFormatFrame->getBottomToggled())
	{
		UT_sint32 linestyle = m_pFormatFrame->borderLineStyleBottom();
		if (linestyle == LS_DOTTED)
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_DOTTED);
		else if (linestyle == LS_DASHED)
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_ON_OFF_DASH);
		else
			m_gc->setLineProperties(1, GR_Graphics::JOIN_MITER, GR_Graphics::CAP_BUTT, GR_Graphics::LINE_SOLID);

		m_gc->setColor(m_pFormatFrame->borderColorBottom());

		UT_sint32 iBottomThickness = UT_convertToLogicalUnits(m_pFormatFrame->getBorderThicknessBottom().utf8_str());
		m_gc->setLineWidth(iBottomThickness);

		painter.drawLine(pageRect.left + border, pageRect.top + pageRect.height - border,
						 pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
}
