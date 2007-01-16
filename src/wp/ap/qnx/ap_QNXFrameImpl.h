/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 Johan Björk 
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

#ifndef AP_QNXFRAMEIMPL_H
#define AP_QNXFRAMEIMPL_H
#include <Pt.h>
#include "xap_Frame.h"
#include "ap_Frame.h"
#include "xap_QNXFrameImpl.h"
#include "ap_QNXFrame.h"
#include "ie_types.h"

class XAP_QNXApp;
class AP_QNXFrame;

enum apufi_ScrollType { apufi_scrollX, apufi_scrollY }; // can we use namespaces yet? this is quite ugly

class AP_QNXFrameImpl : public XAP_QNXFrameImpl
{
 public:
	AP_QNXFrameImpl(AP_QNXFrame *pQNXFrame, XAP_QNXApp *pQNXApp); 
	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame, XAP_App *pApp);
	void setDocumentFocus();
	void _reflowLayout(int loweradj,int upperadj,int topruleradj,int leftruleradj);	
	virtual UT_RGBColor getColorSelBackground () const;

 protected:
	friend class AP_QNXFrame;
	void _showOrHideStatusbar(void);
	void _showOrHideToolbars(void);

	virtual void _refillToolbarsInFrameData();
	void _bindToolbars(AV_View * pView);
	void _createWindow();

	virtual PtWidget_t * _createDocumentWindow();
	virtual PtWidget_t * _createStatusBarWindow();

	virtual void _setWindowIcon();
	void _setScrollRange(apufi_ScrollType scrollType, int iValue, float fUpperLimit, float fSize);

	
	PtWidget_t *			m_dArea;
	PtWidget_t *			m_dAreaGroup;
	PtWidget_t *			m_hScroll;
	PtWidget_t *			m_vScroll;
	PtWidget_t *			m_topRuler;
	PtWidget_t *			m_leftRuler;
};
#endif
