/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz and others
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

#ifndef AP_FRAME_H
#define AP_FRAME_H

#if defined(ANY_UNIX) || (defined(__APPLE__) && defined(__MACH__))
#include "xap_Frame.h"
#include "fv_View.h"
#include "fl_DocLayout.h"

class AP_Frame : public XAP_Frame
{
 public:
	AP_Frame(XAP_App *pApp) : XAP_Frame(pApp) {}
	AP_Frame(AP_Frame *pFrame) : XAP_Frame(static_cast<XAP_Frame *>(pFrame)) {}
	virtual ~AP_Frame();
  
 protected:
	void _replaceView(GR_Graphics * pG, FL_DocLayout *pDocLayout,
			  AV_View *pView, AV_ScrollObj * pScrollObj,
			  ap_ViewListener *pViewListener, AD_Document *pOldDoc,
			  ap_Scrollbar_ViewListener *pScrollbarViewListener,
			  AV_ListenerId lid, AV_ListenerId lidScrollbarViewListener,
			  UT_uint32 iZoom);

 private:
	void _resetInsertionPoint();
};
#else 
class AP_Frame
{
 public:

  virtual ~AP_Frame () ;

 private:
};
#endif
#endif // AP_FRAME_H
