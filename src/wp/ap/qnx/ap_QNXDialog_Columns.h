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

#ifndef AP_QNXDialog_Columns_H
#define AP_QNXDialog_Columns_H

#include "ap_Dialog_Columns.h"

class XAP_QNXFrame;

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

#define DefineToolbarIcon(name)		{ #name, (const char **) ##name, sizeof(##name)/sizeof(##name[0]) },

static struct _it s_itTable[] =
{

#include "ap_Toolbar_Icons_All.h"
	
};

#undef DefineToolbarIcon

// Some convience functions to make Abi's pixmaps easily available to dialogs
static UT_Bool findIconDataByName(const char * szName, const char *** pIconData, UT_uint32 * pSizeofData) ;
static UT_Bool label_button_with_abi_pixmap( PtWidget_t * button, const char * szIconName);



/*****************************************************************/

class AP_QNXDialog_Columns: public AP_Dialog_Columns
{
public:
	AP_QNXDialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_QNXDialog_Columns(void);

	virtual void			runModal(XAP_Frame * pFrame);
	virtual void			enableLineBetweenControl(UT_Bool bState = UT_TRUE);

	static XAP_Dialog *		static_constructor(XAP_DialogFactory *, XAP_Dialog_Id id);

	// callbacks can fire these events
	virtual void			event_OK(void);
	virtual void			event_Cancel(void);
	virtual void			event_WindowDelete(void);
	virtual void			event_previewExposed(void);
	virtual void			event_Toggle(PtWidget_t *w);

protected:

	// private construction functions
	virtual PtWidget_t * _constructWindow(void);
	void		_populateWindowData(void);
	void 		_storeWindowData(void);

	GR_QNXGraphics	*m_pPreviewWidget;
	
	// pointers to widgets we need to query/set
	PtWidget_t 	*m_windowMain;
	PtWidget_t 	*m_wlineBetween;
	PtWidget_t 	*m_wtoggleOne;
	PtWidget_t 	*m_wtoggleTwo;
	PtWidget_t 	*m_wtoggleThree;
	PtWidget_t 	*m_wpreviewArea;
	PtWidget_t 	*m_buttonOK;
	PtWidget_t 	*m_buttonCancel;
	int 		done;
};

#endif /* AP_QNXDialog_Columns_H */
