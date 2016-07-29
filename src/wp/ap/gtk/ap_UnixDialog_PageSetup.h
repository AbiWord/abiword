/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#ifndef AP_UnixDialog_PageSetup_H
#define AP_UnixDialog_PageSetup_H

#include <gtk/gtk.h>
#include "ap_Dialog_PageSetup.h"

class AP_UnixDialog_PageSetup : public AP_Dialog_PageSetup
{
public:
	AP_UnixDialog_PageSetup (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_PageSetup (void);

	virtual void runModal (XAP_Frame *pFrame);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	virtual void event_OK (void);
	virtual void event_Cancel (void);

	virtual void event_PageSizeChanged (fp_PageSize::Predefined pageSize);
	virtual void event_PageUnitsChanged ();
	virtual void event_MarginUnitsChanged ();
	void event_LandscapeChanged(void);

	void doWidthEntry(void);
	void doHeightEntry(void);

 protected:
	// construction functions
	virtual GtkWidget * _constructWindow (void);
	virtual void        _connectSignals (void);
	GtkWidget *         _getWidget(const char * szNameBase, UT_sint32 level=0);

	// pointers to all the action items
	GtkWidget * m_window;
	GtkWidget * m_wHelp;
	GtkBuilder * m_pBuilder;

	typedef enum
		{
			BUTTON_OK = GTK_RESPONSE_OK,
			BUTTON_CANCEL = GTK_RESPONSE_CANCEL
		} ResponseId ;

 private:
	// pointers to widgets that we may need to query
	GtkWidget * m_comboPageSize;
	GtkWidget * m_entryPageWidth;
	GtkWidget * m_entryPageHeight;
	GtkWidget * m_optionPageUnits;
	GtkWidget * m_radioPagePortrait;
	GtkWidget * m_radioPageLandscape;
	GtkWidget * m_spinPageScale;

	GtkWidget * m_optionMarginUnits;
	GtkWidget * m_spinMarginTop;
	GtkWidget * m_spinMarginBottom;
	GtkWidget * m_spinMarginLeft;
	GtkWidget * m_spinMarginRight;
	GtkWidget * m_spinMarginHeader;
	GtkWidget * m_spinMarginFooter;

	GtkWidget * customPreview;
	GtkWidget * m_PageHbox;
	GtkWidget * m_MarginHbox;


	guint m_iEntryPageWidthID;
	guint m_iEntryPageHeightID;
	guint m_iComboPageSizeListID;

    XAP_Frame * m_pFrame;

	void _setWidth(const char * buf);
	void _setHeight(const char * buf);

	// The parent field behaves unpredictably, so we declare our own.
	fp_PageSize m_PageSize;

	void _updatePageSizeList();
};

#endif // AP_UnixDialog_PageSetup_H
