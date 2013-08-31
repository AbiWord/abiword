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
	//TODO
	UT_DEBUGMSG (("SERHAT: runModal\n"));
}

gint XAP_QtDialog_FileOpenSaveAs::previewPicture (void)
{
}

QPixmap *  XAP_QtDialog_FileOpenSaveAs::_loadXPM(UT_ByteBuf * pBB)
{
}

QPixmap *  XAP_QtDialog_FileOpenSaveAs::pixmapForByteBuf (UT_ByteBuf * pBB)
{
}

