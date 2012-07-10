/* AbiSource
 *
 * Copyright (C) 2012 Tanya Guza <tanya.guza@gmail.com>
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


#include "ODe_ThumbnailsWriter.h"

// Internal includes
#include "ODe_Common.h"

// Abiword includes
#include <ut_types.h>
#include <ut_string_class.h>
#include "ut_std_string.h"
#include <pd_Document.h>
#include <xap_App.h>
#include <xap_Frame.h>
#include <fl_DocLayout.h>
#include <fp_Page.h>
#include <fv_View.h>
#include <gr_Graphics.h>
#include <gr_DrawArgs.h>
#include <gr_Painter.h>

// External includes
#include <gsf/gsf-output-stdio.h>
#include <gsf/gsf-outfile.h>

bool ODe_ThumbnailsWriter::writeThumbnails(PD_Document* /*pDoc*/, GsfOutfile* oo) {

	GsfOutput* thumbnailsDir = gsf_outfile_new_child (oo, "Thumbnails", TRUE);
	if(thumbnailsDir == NULL){
		return false;
	}

	GsfOutput* thumbnail = gsf_outfile_new_child(GSF_OUTFILE(thumbnailsDir),
			 "thumbnail.png", FALSE);
	if(thumbnail == NULL){
		return false;
	}

    XAP_Frame *pFrame = XAP_App::getApp()->getLastFocussedFrame();
    FV_View* pView = static_cast<FV_View*>(pFrame->getCurrentView());

    GR_Graphics* pVG = pView->getGraphics();

    UT_sint32 iWidth = pView->getWindowWidth();
	UT_sint32 iHeight = pView->getWindowHeight();
    UT_Rect rect(0, 0, iWidth, iHeight);

	GR_Painter painter(pVG);
	GR_Image * pImage = painter.genImageFromRectangle(rect);

	if(pImage == NULL){
		gsf_output_close(thumbnail);
		gsf_output_close(thumbnailsDir);
		return false;
	}

	UT_ByteBuf * pBuf = NULL;
	pImage->convertToBuffer(&pBuf);

	gsf_output_write(thumbnail, pBuf->getLength(),
			pBuf->getPointer(0));

	DELETEP(pBuf);
	DELETEP(pImage);

	gsf_output_close(thumbnail);
	gsf_output_close(thumbnailsDir);

	return true;
}
