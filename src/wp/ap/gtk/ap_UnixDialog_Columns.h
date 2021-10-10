/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2021 Hubert Figuière
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

#pragma once

#include "ap_Dialog_Columns.h"

#include "ut_types.h"
#include "ut_string.h"


class GR_UnixCairoGraphics;
/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to construct a table of
** the icon names and pointer to the data.
******************************************************************
*****************************************************************/

// Some convience functions to make Abi's pixmaps easily available to unix
// dialogs

bool findIconDataByName(const char * szName, const char *** pIconData, UT_uint32 * pSizeofData) ;

bool label_button_with_abi_pixmap( GtkWidget * button, const char * szIconName);


//----------------------------------------------------------------

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Columns: public AP_Dialog_Columns
{
public:
	AP_UnixDialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Columns(void);

	virtual void runModal(XAP_Frame * pFrame) override;
	virtual void enableLineBetweenControl(bool bState = true) override;

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
    void                            doSpaceAfterEntry(void);
	void                            doMaxHeightEntry(void);
    void                            doHeightSpin(void);
	void                            doSpaceAfterSpin(void);
	void                            checkLineBetween(void);
	void                            readSpin(void);
	void                            event_Toggle( UT_uint32 icolumns);
	// invalidate preview
	void event_previewInvalidate(void);
	// draw the preview
	void event_previewDraw(void);
	virtual void			event_OK(void);
	virtual void			event_Cancel(void);

protected:

	typedef enum
		{
			BUTTON_OK = GTK_RESPONSE_OK,
			BUTTON_CANCEL = GTK_RESPONSE_CANCEL
		} ResponseId ;

	// private construction functions
	virtual GtkWidget * _constructWindow(void);
	void            _constructWindowContents( GtkWidget * windowColumns);
	void		_populateWindowData(void);
	void 		_storeWindowData(void);
	void            _connectsignals(void);

	GR_UnixCairoGraphics	* 		m_pPreviewWidget;

	// pointers to widgets we need to query/set
	GtkWidget * m_windowMain;

	GtkWidget * m_wlineBetween;
	GtkWidget * m_wtoggleOne;
	GtkWidget * m_wtoggleTwo;
	GtkWidget * m_wtoggleThree;
	GtkWidget * m_wpreviewArea;
	GtkWidget * m_wSpin;

	guint m_oneHandlerID;
	guint m_twoHandlerID;
	guint m_threeHandlerID;
	guint m_spinHandlerID;
	UT_sint32 m_iSpaceAfter;
	guint m_iSpaceAfterID;
	GtkWidget * m_wSpaceAfterSpin;
	GtkWidget * m_wSpaceAfterEntry;
	GtkAdjustment * m_oSpaceAfter_adj;
	UT_sint32 m_iMaxColumnHeight;
	guint m_iMaxColumnHeightID;
	GtkWidget * m_wMaxColumnHeightSpin;
	GtkWidget * m_wMaxColumnHeightEntry;
	GtkAdjustment * m_oSpinSize_adj;
	UT_sint32 m_iSizeHeight;
    GtkWidget * m_checkOrder;
};
