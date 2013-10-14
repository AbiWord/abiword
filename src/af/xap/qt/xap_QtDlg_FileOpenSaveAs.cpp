/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* 
 * Copyright (C) 2013 Serhat Kiyak <serhatkiyak91@gmail.com>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_path.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_QtDlg_FileOpenSaveAs.h"
#include "xap_QtApp.h"
#include "xap_Frame.h"
#include "xap_QtFrameImpl.h"
#include "xap_Strings.h"
#include "xap_Prefs.h"
#include "ut_debugmsg.h"
#include "ut_string_class.h"
#include "ut_png.h"
#include "ut_svg.h"
#include "ut_misc.h"
#include "fg_Graphic.h"
#include "fg_GraphicRaster.h"
#include "ie_impGraphic.h"

#include "gr_Painter.h"
#include "ut_bytebuf.h"

#include <sys/stat.h>

#include "../../../wp/impexp/xp/ie_types.h"
#include "../../../wp/impexp/xp/ie_imp.h"
#include "../../../wp/impexp/xp/ie_exp.h"
#include "../../../wp/impexp/xp/ie_impGraphic.h"

#define PREVIEW_WIDTH  100
#define PREVIEW_HEIGHT 100



/*****************************************************************/
XAP_Dialog * XAP_QtDialog_FileOpenSaveAs::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id id)
{
	XAP_QtDialog_FileOpenSaveAs * p = new XAP_QtDialog_FileOpenSaveAs(pFactory,id);
	return p;
}

XAP_QtDialog_FileOpenSaveAs::XAP_QtDialog_FileOpenSaveAs(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id id)
  : XAP_Dialog_FileOpenSaveAs(pDlgFactory,id), m_preview(0), m_bSave(true)
{
	m_szFinalPathnameCandidate = NULL;
}

XAP_QtDialog_FileOpenSaveAs::~XAP_QtDialog_FileOpenSaveAs(void)
{
	FREEP(m_szFinalPathnameCandidate);
}

/*****************************************************************/

bool XAP_QtDialog_FileOpenSaveAs::_run_gtk_main(XAP_Frame * pFrame,
													 QWidget * filetypes_pulldown)
{
	return true;
}

bool XAP_QtDialog_FileOpenSaveAs::_askOverwrite_YesNo(XAP_Frame * pFrame, const char * fileName)
{
	return true;
}

void XAP_QtDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame, XAP_String_Id sid)
{
}

void XAP_QtDialog_FileOpenSaveAs::_notifyError_OKOnly(XAP_Frame * pFrame,
							XAP_String_Id sid,
							const char * sz1)
{
}

void XAP_QtDialog_FileOpenSaveAs::fileTypeChanged(QWidget * w)
{
}

void XAP_QtDialog_FileOpenSaveAs::onDeleteCancel() 
{
}

/*****************************************************************/

void XAP_QtDialog_FileOpenSaveAs::runModal(XAP_Frame * pFrame)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	std::string szTitle;
	std::string szFileTypeLabel;

	switch (m_id)
	{
		case XAP_DIALOG_ID_INSERT_PICTURE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_IP_Title, szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel, szFileTypeLabel);
				m_bSave = false;    
				break;
			}
		case XAP_DIALOG_ID_FILE_OPEN:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_OpenTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_FILE_IMPORT:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ImportTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_INSERTMATHML:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertMath,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileInsertMath,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_INSERTOBJECT:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertObject,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileInsertObject,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_INSERT_FILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_InsertTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileOpenTypeLabel,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		case XAP_DIALOG_ID_FILE_SAVEAS:
		case XAP_DIALOG_ID_FILE_SAVE_IMAGE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_SaveAsTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel,szFileTypeLabel);
				m_bSave = true;
				break;
			}
		case XAP_DIALOG_ID_FILE_EXPORT:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ExportTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FileSaveTypeLabel,szFileTypeLabel);
				m_bSave = true;
				break;
			}
		case XAP_DIALOG_ID_PRINTTOFILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_PrintToFileTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_FilePrintTypeLabel,szFileTypeLabel);
				m_bSave = true;
				break;
			}
		case XAP_DIALOG_ID_RECORDTOFILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_RecordToFileTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_RecordToFileLabel,szFileTypeLabel);
				m_bSave = true;
				break;
			}
		case XAP_DIALOG_ID_REPLAYFROMFILE:
			{
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ReplayFromFileTitle,szTitle);
				pSS->getValueUTF8(XAP_STRING_ID_DLG_FOSA_ReplayFromFileLabel,szFileTypeLabel);
				m_bSave = false;
				break;
			}
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			m_bSave = false;
			break;
	}

	// Get the QWindow of the parent frame
	XAP_QtFrameImpl * pQtFrameImpl = static_cast<XAP_QtFrameImpl *>(pFrame->getFrameImpl());
	QMainWindow * parent = pQtFrameImpl->getTopLevel();

	QString str = szTitle.c_str();
	m_fileDialog = new QFileDialog(parent, str);
	if(m_bSave)
	{
		m_fileDialog->setAcceptMode(QFileDialog::AcceptSave);
		m_fileDialog->setFileMode(QFileDialog::AnyFile);
	}
	else
	{
		m_fileDialog->setAcceptMode(QFileDialog::AcceptOpen);
		m_fileDialog->setFileMode(QFileDialog::ExistingFile);
	}

	QStringList fileNames;
	if(m_fileDialog->exec())
	{
		fileNames = m_fileDialog->selectedFiles();
	}

	if(fileNames.size() != 1)
	{
		m_answer = a_CANCEL;
		return;
	}

	m_answer = a_OK;
	FREEP(m_szFinalPathname);
	m_szFinalPathname = g_strdup(fileNames.at(0).toLocal8Bit().data());
	UT_DEBUGMSG(("SERHAT: file path/name [%s] \n", m_szFinalPathname));	
}

gint XAP_QtDialog_FileOpenSaveAs::previewPicture (void)
{
	// TODO
	return 0;
}

QPixmap *  XAP_QtDialog_FileOpenSaveAs::_loadXPM(UT_ByteBuf * pBB)
{
	// TODO
	return NULL;
}

QPixmap *  XAP_QtDialog_FileOpenSaveAs::pixmapForByteBuf (UT_ByteBuf * pBB)
{
	// TODO
	return NULL;
}

