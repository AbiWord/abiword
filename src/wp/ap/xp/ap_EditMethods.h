/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

#ifndef ap_EditMethodsDefined
#define ap_EditMethodsDefined

#include "ie_types.h"
#include "fv_View.h"

#include <set>

UT_Error fileOpen(XAP_Frame * pFrame, const char * pNewFile, IEFileType ieft);

// defined in ap_editmethods.cpp
bool s_actuallyPrint(PD_Document *doc,  GR_Graphics *pGraphics,
		     FV_View * pPrintView, const char *pDocName,
		     UT_uint32 nCopies, bool bCollate,
		     UT_sint32 inWidth,  UT_sint32 inHeight,
		     UT_sint32 nToPage, UT_sint32 nFromPage);

bool s_actuallyPrint(PD_Document *doc,  GR_Graphics *pGraphics,
		     FV_View * pPrintView, const char *pDocName,
		     UT_uint32 nCopies, bool bCollate,
		     UT_sint32 inWidth,  UT_sint32 inHeight,
		     const std::set<UT_sint32>& pages);


void s_getPageMargins(FV_View * inView,
					  double &margin_left,
					  double &margin_right,
					  double &page_margin_left,
					  double &page_margin_right,
					  double &page_margin_top,
					  double &page_margin_bottom );

bool s_doTabDlg(FV_View * pView);

#endif
