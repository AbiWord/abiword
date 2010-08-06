/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2010 Maleesh Prasan
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

#ifndef AP_UNIXDIALOG_BORDER_SHADING_H
#define AP_UNIXDIALOG_BORDER_SHADING_H

#include "ap_Dialog_Border_Shading.h"


class XAP_UnixFrame;
class GR_UnixCairoGraphics;

/*****************************************************************/

class AP_UnixDialog_Border_Shading: public AP_Dialog_Border_Shading
{
public:
	AP_UnixDialog_Border_Shading(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Border_Shading(void);

	virtual void			runModeless(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void			event_Close(void);
	void					event_previewExposed(void);
	void                    event_BorderThicknessChanged(void);
	void                    event_BorderStyleChanged(void);
	void                    event_ShadingOffsetChanged(void);

	virtual void           	setBorderThicknessInGUI(UT_UTF8String & sThick);
	virtual void			setShadingColorInGUI(UT_RGBColor clr);
	virtual void           	setSensitivity(bool bsens);
	virtual void           	destroy(void);
	virtual void            activate(void);
	virtual void            notifyActiveFrame(XAP_Frame * pFrame);
	const GtkWidget 	  * getWindow (void) const { return m_windowMain; }
	void 					loadLastKnownValues();
protected:
	typedef enum
	{
	  BUTTON_APPLY = GTK_RESPONSE_APPLY,
	  BUTTON_CLOSE = GTK_RESPONSE_CLOSE
	} ResponseId ;
		
	virtual GtkWidget *		_constructWindow(void);
	void					_populateWindowData(void);
	void					_storeWindowData(void);
	void					_connectSignals(void);
	
	GR_UnixCairoGraphics	* 		m_pPreviewWidget;	
	
	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;
	GtkWidget * m_wApplyButton;
	GtkWidget * m_wCloseButton;

	GtkWidget * m_wBorderColorButton;
	GtkWidget * m_wShadingColorButton;
	GtkWidget * m_wLineLeft;
	GtkWidget * m_wLineRight;
	GtkWidget * m_wLineTop;
	GtkWidget * m_wLineBottom;
	
	GtkWidget * m_wPreviewArea;
	GtkWidget * m_wBorderThickness;
	GtkWidget * m_wBorderStyle;
	GtkWidget * m_wShadingOffset;

	guint       m_iBorderThicknessConnect;
	guint       m_iBorderStyleConnect;
	guint       m_iShadingOffsetConnect;

	guint		m_iLastBorderThicknessIndex;
	guint		m_iLastBorderStyleIndex;
	guint		m_iLastShadingOffsetIndex;
};

#endif /* AP_UNIXDIALOG_BORDER_SHADING_H */
