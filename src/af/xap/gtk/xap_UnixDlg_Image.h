/* AbiWord
 * Copyright (C) 2001-2002 Dom Lachowicz
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

#ifndef XAP_UNIXDIALOG_IMAGE_H
#define XAP_UNIXDIALOG_IMAGE_H

#include "xap_Dlg_Image.h"

class XAP_Frame;

/*****************************************************************/

class XAP_UnixDialog_Image: public XAP_Dialog_Image
{
 public:
	XAP_UnixDialog_Image(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~XAP_UnixDialog_Image(void);

	virtual void			runModal(XAP_Frame * pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);
	void                    setPositionToGUI(void);
	void                    setWrappingGUI(void);
 protected:
	void _constructWindowContents (GtkWidget * container);
	virtual GtkWidget * _constructWindow ();
	void _connectSignals ();

 private:

	typedef enum
	  {
	    BUTTON_OK = GTK_RESPONSE_OK,
	    BUTTON_CANCEL = GTK_RESPONSE_CANCEL
	  } ResponseId ;

	void event_Ok ();
	void event_Cancel ();
	void doHeightSpin(void);
	void doWidthSpin(void);
	void doHeightEntry(void);
	void doWidthEntry(void);
	void setHeightEntry(void);
	void setWidthEntry(void);
	void adjustHeightForAspect(void);
	void adjustWidthForAspect(void);
	void aspectCheckbox();
	void wrappingChanged(void);
	void wrapTypeChanged(void);

	static void s_HeightSpin_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg) ;

	static void s_WidthSpin_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg) ;

	static void s_HeightEntry_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg) ;
	static gboolean s_HeightEntry_FocusOut(GtkWidget * widget, GdkEvent  *event, XAP_UnixDialog_Image *dlg);
	static gboolean s_WidthEntry_FocusOut(GtkWidget * widget, GdkEvent  *event, XAP_UnixDialog_Image *dlg);

	static void s_WidthEntry_changed(GtkWidget * widget, XAP_UnixDialog_Image *dlg) ;

	static void s_aspect_clicked(GtkWidget * widget, XAP_UnixDialog_Image * dlg) ;
	static void s_wrapping_changed(GtkWidget * widget, XAP_UnixDialog_Image * dlg) ;
	static void s_wrapType_changed(GtkWidget * widget, XAP_UnixDialog_Image * dlg) ;

	GtkWidget * mMainWindow;
  	GtkWidget * m_wAspectCheck;
  	GtkWidget * m_wHeightSpin;
  	GtkWidget * m_wHeightEntry;
  	GtkWidget * m_wWidthSpin;
  	GtkWidget * m_wWidthEntry;
	GtkWidget * m_wTitleEntry;
	GtkWidget * m_wDescriptionEntry;
	GtkWidget * m_wrbInLine;
	GtkWidget * m_wrbNone;
	GtkWidget * m_wrbWrappedRight;
	GtkWidget * m_wrbWrappedLeft;
	GtkWidget * m_wrbWrappedBoth;
	GtkWidget * m_wrbPlaceParagraph;
	GtkWidget * m_wrbPlaceColumn;
	GtkWidget * m_wrbPlacePage;
	GtkWidget * m_wrbSquareWrap;
	GtkWidget * m_wrbTightWrap;



	GtkAdjustment * m_oHeightSpin_adj;
	GtkAdjustment * m_oWidthSpin_adj;
	guint m_iHeightID;
	guint m_iWidthID;
	UT_sint32 m_iHeight;
	UT_sint32 m_iWidth;
	bool m_bAspect;
	double m_dHeightWidth;
};

#endif /* XAP_UNIXDIALOG_IMAGE_H */
