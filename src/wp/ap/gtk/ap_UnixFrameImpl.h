/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2002 William Lachance
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

#ifndef AP_UNIXFRAMEIMPL_H
#define AP_UNIXFRAMEIMPL_H
#include "xap_Frame.h"
#include "ap_Frame.h"
#include "ap_UnixFrame.h"
#include "ie_types.h"
#include "xap_UnixFrameImpl.h"
#include "ap_DocView.h"


class XAP_UnixApp;
class AP_UnixFrame;

enum apufi_ScrollType { apufi_scrollX, apufi_scrollY }; // can we use namespaces yet? this is quite ugly

class AP_UnixFrameImpl : public XAP_UnixFrameImpl
{
 public:
	AP_UnixFrameImpl(AP_UnixFrame *pUnixFrame);
	virtual XAP_FrameImpl * createInstance(XAP_Frame *pFrame);

	virtual UT_RGBColor getColorSelBackground () const;
	virtual UT_RGBColor getColorSelForeground () const;

	GtkShadowType getShadowType () { return gtk_frame_get_shadow_type (GTK_FRAME (m_wSunkenBox)); }
	void setShadowType (GtkShadowType shadow) { gtk_frame_set_shadow_type (GTK_FRAME (m_wSunkenBox), shadow); }

	GtkWidget * getDrawingArea() const {return m_dArea;}
	static gboolean ap_focus_in_event (GtkWidget * drawing_area, GdkEventCrossing *event, AP_UnixFrameImpl * me);
	static gboolean ap_focus_out_event (GtkWidget * drawing_area, GdkEventCrossing *event, AP_UnixFrameImpl * me);
	virtual GtkWidget * getViewWidget(void) const
	{ return m_dArea; }

 protected:
	friend class AP_UnixFrame;
	void _showOrHideStatusbar(void);
	void _showOrHideToolbars(void);

	virtual void _hideMenuScroll(bool bHideMenuScroll);


	virtual void _refillToolbarsInFrameData();
	void _bindToolbars(AV_View * pView);
	virtual void _createWindow();

	virtual GtkWidget * _createDocumentWindow();
	virtual GtkWidget * _createStatusBarWindow();

	virtual void _setWindowIcon();
	void _setScrollRange(apufi_ScrollType scrollType, int iValue, gfloat fUpperLimit, gfloat fSize);

	GtkWidget * m_dArea;
	GtkAdjustment *	m_pVadj;
	GtkAdjustment *	m_pHadj;
	GtkWidget * m_hScroll;
	GtkWidget * m_vScroll;
	GtkWidget * m_topRuler;
	GtkWidget * m_leftRuler;
	GtkWidget * m_grid;
	GtkWidget * m_innergrid;
	GtkWidget * m_wSunkenBox;
	gulong      m_iHScrollSignal;
	gulong      m_iVScrollSignal;
};
#endif
