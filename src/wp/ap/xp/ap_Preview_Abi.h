/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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


#ifndef AP_PREVIEW_ABI_H
#define AP_PREVIEW_ABI_H

#include "ut_misc.h"
#include "ut_types.h"
#include "ut_vector.h"

#include "xap_Preview.h"
#include "xap_Frame.h"
#include "xap_App.h"
#include "fv_View.h"
#include "pd_Document.h"

class GR_Font;

class AP_Preview_Abi : public XAP_Preview
{
public:

	AP_Preview_Abi(GR_Graphics * gc, UT_uint32 iWidth,  UT_uint32 iHeight,
				   XAP_Frame * pFrame, PreViewMode previewMode);
	virtual ~AP_Preview_Abi(void);

	FV_View * getView(void) const;
	PD_Document * getDoc(void) const;
	XAP_App * getApp(void) const {return m_pApp;}
	virtual void draw(void);
	
protected:
	
private:
	XAP_Frame * m_pFrame;
	FV_View * m_pView;
	FL_DocLayout * m_pDocLayout;
    PD_Document * m_pDocument;
	XAP_App * m_pApp;
};

#endif /* AP_PREVIEW_ABI_H */



