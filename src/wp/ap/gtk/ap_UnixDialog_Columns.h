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

#ifndef AP_UnixDialog_Columns_H
#define AP_UnixDialog_Columns_H

#include "ap_Dialog_Columns.h"

#include "ut_types.h"
#include "ut_string.h"


/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to load all of the icons.
** It is important that all of the ..._Icon_*.{h,xpm} files
** allow themselves to be included more than one time.
******************************************************************
*****************************************************************/
// This comes from ap_Toolbar_Icons.cpp
#include "xap_Toolbar_Icons.h"

#include "ap_Toolbar_Icons_All.h"

class GR_UnixCairoGraphics;
/*****************************************************************
******************************************************************
** Here we begin a little CPP magic to construct a table of
** the icon names and pointer to the data.
******************************************************************
*****************************************************************/

struct _it
{
	const char *				m_name;
	const char **				m_staticVariable;
	UT_uint32					m_sizeofVariable;
};

#define DefineToolbarIcon(name)		{ #name, (const char **) name, sizeof(name)/sizeof(name[0]) },

static struct _it s_itTable[] =
{

#include "ap_Toolbar_Icons_All.h"

};

#undef DefineToolbarIcon


// Some convience functions to make Abi's pixmaps easily available to unix
// dialogs

static bool findIconDataByName(const char * szName, const char *** pIconData, UT_uint32 * pSizeofData) ;

static bool label_button_with_abi_pixmap( GtkWidget * button, const char * szIconName);


//
//--------------------------------------------------------------------------
//
// Code to make pixmaps for gtk buttons
//
// findIconDataByName stolen from ap_Toolbar_Icons.cpp
//
static bool findIconDataByName(const char * szName, const char *** pIconData, UT_uint32 * pSizeofData)
{
	// This is a static function.

	if (!szName || !*szName || (g_ascii_strcasecmp(szName,"NoIcon")==0))
		return false;

	UT_uint32 kLimit = G_N_ELEMENTS(s_itTable);
	UT_uint32 k;
	UT_DEBUGMSG(("SEVIOR: Looking for %s \n",szName));
	for (k=0; k < kLimit; k++)
	{
		UT_DEBUGMSG(("SEVIOR: examining %s \n",s_itTable[k].m_name));
		if (g_ascii_strcasecmp(szName,s_itTable[k].m_name) == 0)
		{
			*pIconData = s_itTable[k].m_staticVariable;
			*pSizeofData = s_itTable[k].m_sizeofVariable;
			return true;
		}
	}
	return false;
}

static inline bool label_button_with_abi_pixmap( GtkWidget * button, const char * szIconName)
{
        const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	bool bFound = findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound)
	{
		UT_DEBUGMSG(("Could not find icon %s \n",szIconName));
		return false;
	}
	UT_DEBUGMSG(("SEVIOR: found icon name %s \n",szIconName));
	GdkBitmap * mask;
	GdkColormap * colormap =  gtk_widget_get_colormap (button);
	GdkPixmap * pixmap
		= gdk_pixmap_colormap_create_from_xpm_d(button->window,colormap,
							&mask, NULL,
							(char **)pIconData);
	if (!pixmap)
		return false;
	GtkWidget * wpixmap = gtk_image_new_from_pixmap(pixmap,mask);
	if (!wpixmap)
		return false;
	gtk_widget_show(wpixmap);
	UT_DEBUGMSG(("SEVIOR: Adding pixmap to button now \n"));
	gtk_container_add (GTK_CONTAINER (button), wpixmap);
	return true;
}
//----------------------------------------------------------------

class XAP_UnixFrame;

/*****************************************************************/

class AP_UnixDialog_Columns: public AP_Dialog_Columns
{
public:
	AP_UnixDialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_UnixDialog_Columns(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			enableLineBetweenControl(bool bState = true);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
    void                            doSpaceAfterEntry(void);
	void                            doMaxHeightEntry(void);
    void                            doHeightSpin(void);
	void                            doSpaceAfterSpin(void);
	void                            checkLineBetween(void);
	void                            readSpin(void);
	void                            event_Toggle( UT_uint32 icolumns);
	void                            event_previewExposed(void);
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
	GtkObject * m_oSpaceAfter_adj;
	UT_sint32 m_iMaxColumnHeight;
	guint m_iMaxColumnHeightID;
	GtkWidget * m_wMaxColumnHeightSpin;
	GtkWidget * m_wMaxColumnHeightEntry;
	GtkObject * m_oSpinSize_adj;
	UT_sint32 m_iSizeHeight;
    GtkWidget * m_checkOrder;
};

#endif /* AP_UnixDialog_Columns_H */






