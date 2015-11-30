/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (c) 2009 Hubert Figuiere
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>
#include <math.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"
#include "xap_GtkSignalBlocker.h"
#include "xap_Gtk2Compat.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixFrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Columns.h"
#include "ap_UnixDialog_Columns.h"
#include "gr_UnixCairoGraphics.h"

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


//
//--------------------------------------------------------------------------
//
// Code to make pixmaps for gtk buttons
//
// findIconDataByName stolen from ap_Toolbar_Icons.cpp
//
bool findIconDataByName(const char * szName, const char *** pIconData, UT_uint32 * pSizeofData)
{
	// This is a static function.

	if (!szName || !*szName || (g_ascii_strcasecmp(szName,"NoIcon")==0))
		return false;

	UT_uint32 kLimit = G_N_ELEMENTS(s_itTable);
	UT_uint32 k;
	xxx_UT_DEBUGMSG(("SEVIOR: Looking for %s \n",szName));
	for (k=0; k < kLimit; k++)
	{
		xxx_UT_DEBUGMSG(("SEVIOR: examining %s \n",s_itTable[k].m_name));
		if (g_ascii_strcasecmp(szName,s_itTable[k].m_name) == 0)
		{
			*pIconData = s_itTable[k].m_staticVariable;
			*pSizeofData = s_itTable[k].m_sizeofVariable;
			return true;
		}
	}
	return false;
}

bool label_button_with_abi_pixmap( GtkWidget * button, const char * szIconName)
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
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_xpm_data(pIconData);
	GtkWidget * wpixmap = gtk_image_new_from_pixbuf(pixbuf);
	g_object_unref(pixbuf);
	if (!wpixmap)
		return false;
	gtk_widget_show(wpixmap);
	UT_DEBUGMSG(("SEVIOR: Adding pixmap to button now \n"));
	gtk_container_add (GTK_CONTAINER (button), wpixmap);
	return true;
}


/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Columns::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Columns(pFactory,id);
}

AP_UnixDialog_Columns::AP_UnixDialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_wlineBetween = NULL;
	m_wtoggleOne = NULL;
	m_wtoggleTwo = NULL;
	m_wpreviewArea = NULL;
	m_pPreviewWidget = NULL;
	m_wtoggleThree = NULL;
	m_wSpin = NULL;
	m_spinHandlerID = 0;
	m_windowMain = NULL;
	m_iSpaceAfter = 0;
	m_iSpaceAfterID =0;
	m_wSpaceAfterSpin = NULL;
	m_iMaxColumnHeight = 0;
	m_iMaxColumnHeightID = 0;
	m_wMaxColumnHeightSpin = NULL;
	m_checkOrder = NULL;
}

AP_UnixDialog_Columns::~AP_UnixDialog_Columns(void)
{
	DELETEP (m_pPreviewWidget);
}

/*****************************************************************/

static void s_one_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_Toggle(1);
}

static void s_two_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_Toggle(2);
}


static void s_three_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->event_Toggle(3);
}

static void s_spin_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->readSpin();
}


static void s_HeightSpin_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doHeightSpin();
}


static void s_SpaceAfterSpin_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doSpaceAfterSpin();
}


static void s_SpaceAfterEntry_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doSpaceAfterEntry();
}


static void s_MaxHeightEntry_changed(GtkWidget * widget, AP_UnixDialog_Columns *dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->doMaxHeightEntry();
}


static void s_line_clicked(GtkWidget * widget, AP_UnixDialog_Columns * dlg)
{
	UT_return_if_fail(widget && dlg);
	dlg->checkLineBetween();
}

static gboolean s_preview_draw(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Columns * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_previewExposed();
	return FALSE;
}

static gboolean s_window_draw(GtkWidget * widget, gpointer /* data */, AP_UnixDialog_Columns * dlg)
{
	UT_return_val_if_fail(widget && dlg, FALSE);
	dlg->event_previewExposed();
	return FALSE;
}



/*****************************************************************/

void AP_UnixDialog_Columns::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	setViewAndDoc(pFrame);

	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	XAP_UnixFrameImpl *pUnixFrameImpl = static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl());
	GtkWidget *parentWindow = pUnixFrameImpl->getTopLevelWindow();
	if (GTK_IS_WINDOW(parentWindow) != TRUE)
		parentWindow = gtk_widget_get_parent(parentWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));
	gtk_window_set_position(GTK_WINDOW(mainWindow), GTK_WIN_POS_CENTER_ON_PARENT);    

	// ***show*** before creating gc's
	gtk_widget_show ( mainWindow ) ;

	// Populate the window's data items
	_populateWindowData();

    {
		XAP_GtkSignalBlocker b(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
		gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
	}

	{
		XAP_GtkSignalBlocker b(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
		gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
	}

	// *** this is how we add the gc for Column Preview ***
	// attach a new graphics context to the drawing area
	UT_return_if_fail(m_wpreviewArea && gtk_widget_get_window(m_wpreviewArea));

	// make a new Unix GC
	DELETEP (m_pPreviewWidget);
	GR_UnixCairoAllocInfo ai(m_wpreviewArea);
	m_pPreviewWidget =
	    (GR_UnixCairoGraphics*) XAP_App::getApp()->newGraphics(ai);

	// Todo: we need a good widget to query with a probable
	// Todo: non-white (i.e. gray, or a similar bgcolor as our parent widget)
	// Todo: background. This should be fine
	m_pPreviewWidget->init3dColors(m_wpreviewArea);

	// let the widget materialize

	GtkAllocation alloc;
	gtk_widget_get_allocation(m_wpreviewArea, &alloc);
	_createPreviewFromGC(m_pPreviewWidget,
						 (UT_uint32) alloc.width,
						 (UT_uint32) alloc.height);

	setLineBetween(getLineBetween());
	if(getLineBetween()==true)
	{
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wlineBetween),TRUE);
	}
	// Now draw the columns

	event_Toggle(getColumns());

	// Run into the GTK event loop for this window.

	switch( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, BUTTON_CANCEL, false ) )
	{
		case BUTTON_OK:
			event_OK () ; break ;
		default:
			event_Cancel () ; break ;
	}
	
	setColumnOrder (gtk_toggle_button_get_active(
												 GTK_TOGGLE_BUTTON(m_checkOrder)));

	_storeWindowData();
	DELETEP (m_pPreviewWidget);

	abiDestroyWidget(mainWindow);
}

void AP_UnixDialog_Columns::checkLineBetween(void)
{
  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (m_wlineBetween)))
      setLineBetween(true);
  else
      setLineBetween(false);
}

void AP_UnixDialog_Columns::doHeightSpin(void)
{
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wMaxColumnHeightSpin));
	UT_DEBUGMSG(("SEVIOR: spin Height %d old value = %d \n",val,m_iMaxColumnHeight));
	if (val == m_iMaxColumnHeight)
		return;
	if(val < m_iMaxColumnHeight)
		bIncrement = false;
	m_iMaxColumnHeight = val;
	incrementMaxHeight(bIncrement);
	//g_signal_handler_block(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
	gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
	//g_signal_handler_unblock(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
}

void  AP_UnixDialog_Columns::doSpaceAfterSpin(void)
{
	UT_DEBUGMSG(("SEVIOR: In do Space After Spin \n"));
	bool bIncrement = true;
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wSpaceAfterSpin));
	UT_DEBUGMSG(("MARCM: spin Height %d old value = %d \n",val,m_iSpaceAfter));
	if (val == m_iSpaceAfter)
		return;
	else if(val < m_iSpaceAfter)
		bIncrement = false;
	m_iSpaceAfter = val;
	incrementSpaceAfter(bIncrement);
	//g_signal_handler_block(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
	gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
	//g_signal_handler_unblock(G_OBJECT(m_wSpaceAfterEntry),m_iSpaceAfterID);
}

void AP_UnixDialog_Columns::readSpin(void)
{
	UT_sint32 val = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(m_wSpin));
	if(val < 1)
		return;
	if(val < 4)
	{
		event_Toggle(val);
		return;
	}
	{
		XAP_GtkSignalBlocker b1(G_OBJECT(m_wtoggleOne), m_oneHandlerID);
		XAP_GtkSignalBlocker b2(G_OBJECT(m_wtoggleTwo), m_twoHandlerID);
		XAP_GtkSignalBlocker b3(G_OBJECT(m_wtoggleThree), m_threeHandlerID);

		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),FALSE);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),FALSE);
	}
	setColumns( val );
	m_pColumnsPreview->draw();
}

void AP_UnixDialog_Columns::event_Toggle( UT_uint32 icolumns)
{
	checkLineBetween();
	g_signal_handler_block(G_OBJECT(m_wtoggleOne),
							 m_oneHandlerID);
	g_signal_handler_block(G_OBJECT(m_wtoggleTwo),
							 m_twoHandlerID);
	g_signal_handler_block(G_OBJECT(m_wtoggleThree),
							 m_threeHandlerID);
	{
		// DOM: TODO: rewrite me
		XAP_GtkSignalBlocker b(G_OBJECT(m_wSpin),
						   m_spinHandlerID);
		gtk_spin_button_set_value( GTK_SPIN_BUTTON(m_wSpin), (gfloat) icolumns);
	}
	switch (icolumns)
	{
	case 1:
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),TRUE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),FALSE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),FALSE);
		 break;
	case 2:
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),FALSE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),TRUE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),FALSE);
		 break;
	case 3:
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleOne),FALSE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleTwo),FALSE);
		 gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_wtoggleThree),TRUE);
		 break;
	default:
		break;
		// TODO: make these insenstive and update a spin control

	}
	g_signal_handler_unblock(G_OBJECT(m_wtoggleOne),
							   m_oneHandlerID);
	g_signal_handler_unblock(G_OBJECT(m_wtoggleTwo),
							   m_twoHandlerID);
	g_signal_handler_unblock(G_OBJECT(m_wtoggleThree),
							   m_threeHandlerID);
	setColumns( icolumns );
	m_pColumnsPreview->draw();
}


void AP_UnixDialog_Columns::event_OK(void)
{
	// TODO save out state of radio items
	m_answer = AP_Dialog_Columns::a_OK;
}


void AP_UnixDialog_Columns::doMaxHeightEntry(void)
{
	const char * szHeight = gtk_entry_get_text(GTK_ENTRY(m_wMaxColumnHeightEntry));
	if(UT_determineDimension(szHeight,DIM_none) != DIM_none)
	{
		setMaxHeight(szHeight);

		XAP_GtkSignalBlocker b(G_OBJECT(m_wMaxColumnHeightEntry), m_iMaxColumnHeightID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wMaxColumnHeightEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wMaxColumnHeightEntry),getHeightString() );
		gtk_editable_set_position(GTK_EDITABLE(m_wMaxColumnHeightEntry), pos);
	}
}

void AP_UnixDialog_Columns::doSpaceAfterEntry(void)
{
	const char * szAfter = gtk_entry_get_text(GTK_ENTRY(m_wSpaceAfterEntry));
	if(UT_determineDimension(szAfter,DIM_none) != DIM_none)
	{
		setSpaceAfter(szAfter);

		XAP_GtkSignalBlocker b(G_OBJECT(m_wSpaceAfterEntry), m_iSpaceAfterID);
		int pos = gtk_editable_get_position(GTK_EDITABLE(m_wSpaceAfterEntry));
		gtk_entry_set_text( GTK_ENTRY(m_wSpaceAfterEntry),getSpaceAfterString() );
		gtk_editable_set_position(GTK_EDITABLE(m_wSpaceAfterEntry), pos);
	}
}

void AP_UnixDialog_Columns::event_Cancel(void)
{
	m_answer = AP_Dialog_Columns::a_CANCEL;
}

void AP_UnixDialog_Columns::event_previewExposed(void)
{
        if(m_pColumnsPreview)
	       m_pColumnsPreview->draw();
}

/*****************************************************************/

GtkWidget * AP_UnixDialog_Columns::_constructWindow(void)
{

	GtkWidget * windowColumns;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	//	gchar * unixstr = NULL;	// used for conversions
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_ColumnTitle,s);
	
	windowColumns = abiDialogNew ( "column dialog", TRUE, s.c_str() ) ;
	gtk_window_set_resizable(GTK_WINDOW(windowColumns), FALSE);

	_constructWindowContents(gtk_dialog_get_content_area(GTK_DIALOG(windowColumns)));

	pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, s);
	abiAddButton(GTK_DIALOG(windowColumns), s, BUTTON_CANCEL);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_OK, s);
	abiAddButton(GTK_DIALOG(windowColumns), s, BUTTON_OK);

	_connectsignals();
	return windowColumns;
}

void AP_UnixDialog_Columns::_constructWindowContents(GtkWidget * windowColumns)
{
	GtkWidget *lbColFrame;
	GtkWidget *wToggleOne;
	GtkWidget *wLabelOne;
	GtkWidget *wToggleTwo;
	GtkWidget *wLabelTwo;
	GtkWidget *wToggleThree;
	GtkWidget *wLabelThree;
	GtkWidget *lbPrevFrame;
	GtkWidget *wPreviewArea;
	GtkAdjustment *SpinAdj;
	GtkWidget *Spinbutton;
	GtkWidget *SpinLabel;
	GtkWidget *wLineBtween;

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	std::string s;

	GtkWidget *grid = gtk_grid_new();
	g_object_set(G_OBJECT(grid),
	             "row-spacing", 6,
	             "column-spacing", 12,
	             "border-width", 5,
	             NULL);
	gtk_widget_show (grid);
	gtk_box_pack_start(GTK_BOX (windowColumns), grid, FALSE, FALSE, 6);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Number,s);
	s = "<b>" + s + "</b>";
	lbColFrame = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(lbColFrame), s.c_str());
	gtk_widget_show(lbColFrame);
	gtk_grid_attach(GTK_GRID(grid), lbColFrame, 0, 0, 2, 1);

	wToggleOne = gtk_toggle_button_new();
	gtk_widget_show(wToggleOne );
        label_button_with_abi_pixmap(wToggleOne, "tb_1column_xpm");
	gtk_widget_set_can_default(wToggleOne, true);
	gtk_widget_set_margin_start(wToggleOne, 18);
	gtk_grid_attach(GTK_GRID(grid), wToggleOne, 0, 1, 1, 1);
	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_One,s);
	wLabelOne = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                                    "xalign", 0.0, "yalign", 0.5, NULL);
	gtk_widget_show(wLabelOne );
	gtk_grid_attach(GTK_GRID(grid), wLabelOne, 1, 1, 1, 1);

	wToggleTwo = gtk_toggle_button_new ();
	gtk_widget_show(wToggleTwo);
        label_button_with_abi_pixmap(wToggleTwo, "tb_2column_xpm");
	gtk_widget_set_can_default(wToggleTwo, true);
	gtk_widget_set_margin_start(wToggleTwo, 18);
	gtk_grid_attach(GTK_GRID(grid), wToggleTwo, 0, 2, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Two,s);
	wLabelTwo = gtk_widget_new(GTK_TYPE_LABEL, "label", s.c_str(),
                                   "xalign", 0.0, "yalign", 0.5, NULL);
	gtk_widget_show(wLabelTwo );
	gtk_grid_attach(GTK_GRID(grid), wLabelTwo, 1, 2, 1, 1);

	wToggleThree = gtk_toggle_button_new ();
	gtk_widget_show(wToggleThree);
        label_button_with_abi_pixmap(wToggleThree, "tb_3column_xpm");
	gtk_widget_set_can_default(wToggleThree, true);
	gtk_widget_set_margin_start(wToggleThree, 18);
	gtk_grid_attach(GTK_GRID(grid), wToggleThree, 0, 3, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Three,s);
        
	wLabelThree = gtk_widget_new(GTK_TYPE_LABEL, "label", s.c_str(),
                                   "xalign", 0.0, "yalign", 0.5, NULL);
	gtk_widget_show(wLabelThree);
	gtk_grid_attach(GTK_GRID(grid), wLabelThree, 1, 3, 1, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Preview,s);
	s = "<b>" + s + "</b>";
	lbPrevFrame = gtk_label_new(NULL);
	gtk_label_set_markup(GTK_LABEL(lbPrevFrame), s.c_str());
	gtk_widget_show(lbPrevFrame);
	gtk_grid_attach(GTK_GRID(grid), lbPrevFrame, 3, 0, 2, 1);

	double width = getPageWidth();
	double height = getPageHeight();
	gint rat = 0;
	wPreviewArea = createDrawingArea ();
	if( height > 0.000001)
	{
		if(height > width)
		{
			rat = static_cast<gint>( 100.0 *height/width);
			gtk_widget_set_size_request (wPreviewArea, 100, -1); // was -2
		}
		else
		{
			rat = static_cast<gint>( 200.* height/width);
			gtk_widget_set_size_request (wPreviewArea, 200, rat);
		}
	}
	else
	{
		gtk_widget_set_size_request (wPreviewArea, 100, -1); // was -2
	}

	g_object_ref (wPreviewArea);
	g_object_set_data_full (G_OBJECT (windowColumns), "wPreviewArea", wPreviewArea,
							(GDestroyNotify) g_object_unref);
	gtk_widget_set_margin_start(wPreviewArea, 18);
	gtk_widget_show(wPreviewArea);
	gtk_grid_attach(GTK_GRID(grid), wPreviewArea, 3, 1, 2, 4);
//////////////////////////////////////////////////////
// Line Between
/////////////////////////////////////////////////////

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Line_Between,s);
	wLineBtween = gtk_check_button_new_with_label (s.c_str());
	gtk_widget_set_margin_top(wLineBtween, 12);
	gtk_widget_show(wLineBtween);
	gtk_grid_attach(GTK_GRID(grid), wLineBtween, 0, 5, 5, 1);

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_RtlOrder,s);
	GtkWidget * checkOrder = gtk_check_button_new_with_label (s.c_str());
	gtk_widget_show (checkOrder);
	gtk_grid_attach(GTK_GRID(grid), checkOrder, 0, 6, 5, 1);
	gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(checkOrder), getColumnOrder() );
	m_checkOrder = checkOrder;

/////////////////////////////////////////////////////////
// Spin Button for Columns
/////////////////////////////////////////////////////////

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Number_Cols,s);
	SpinLabel = gtk_widget_new (GTK_TYPE_LABEL, "label", s.c_str(),
                                    "xalign", 0.0, "yalign", 0.5, NULL);
	gtk_widget_show(SpinLabel);
	gtk_widget_set_margin_top(SpinLabel, 12);
	gtk_grid_attach(GTK_GRID(grid), SpinLabel, 0, 7, 2, 1);

	SpinAdj = (GtkAdjustment *) gtk_adjustment_new( 1.0, 1.0, 20., 1.0,10.0,0.0);
	Spinbutton = gtk_spin_button_new( SpinAdj, 1.0,0);
	gtk_widget_show(Spinbutton);
	gtk_widget_set_margin_top(Spinbutton, 12);
	gtk_grid_attach(GTK_GRID(grid), Spinbutton, 2, 7, 2, 1);

/////////////////////////////////////////////////////////
// Spin Button for Space After
/////////////////////////////////////////////////////////

	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Space_After,s);
	GtkWidget * SpinLabelAfter = gtk_widget_new (GTK_TYPE_LABEL,
                                                     "label", s.c_str(),
                                                     "xalign", 0.0,
                                                     "yalign", 0.5,
                                                     NULL);
	gtk_widget_show(SpinLabelAfter);
	gtk_grid_attach(GTK_GRID(grid), SpinLabelAfter, 0, 8, 2, 1);

	GtkAdjustment * SpinAfterAdj = (GtkAdjustment*)gtk_adjustment_new( 1, -1000, 1000, 1, 1, 10);
	GtkWidget * SpinAfter = gtk_entry_new();
	gtk_widget_show (SpinAfter);
	gtk_grid_attach(GTK_GRID(grid), SpinAfter, 2, 8, 2, 1);
	
	GtkWidget * SpinAfter_dum = gtk_spin_button_new( GTK_ADJUSTMENT(SpinAfterAdj), 1.0,0);
	gtk_widget_show(SpinAfter_dum);
	gtk_widget_set_size_request(SpinAfter_dum,14,-1);
	gtk_grid_attach(GTK_GRID(grid), SpinAfter_dum, 4, 8, 1, 1);
	
/////////////////////////////////////////////////////////
// Spin Button for Column Height
/////////////////////////////////////////////////////////
	pSS->getValueUTF8(AP_STRING_ID_DLG_Column_Size,s);
	GtkWidget * SpinLabelColumnSize
          = gtk_widget_new(GTK_TYPE_LABEL,
                           "label", s.c_str(),
                           "xalign", 0.0, "yalign", 0.5,
                           NULL);
	gtk_widget_show(SpinLabelColumnSize);
	gtk_grid_attach(GTK_GRID(grid), SpinLabelColumnSize, 0, 9, 2, 1);

	GtkAdjustment * SpinSizeAdj = (GtkAdjustment*)gtk_adjustment_new( 1,-2000, 2000, 1, 1, 10);
	GtkWidget * SpinSize = gtk_entry_new();
	gtk_widget_show (SpinSize);
	gtk_grid_attach(GTK_GRID(grid), SpinSize, 2, 9, 2, 1);

	GtkWidget * SpinSize_dum = gtk_spin_button_new( GTK_ADJUSTMENT(SpinSizeAdj), 1.0,0);
	gtk_widget_show(SpinSize_dum);
	gtk_widget_set_size_request(SpinSize_dum,14,-1);
	gtk_grid_attach(GTK_GRID(grid), SpinSize_dum, 4, 9, 1, 1);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_wlineBetween = wLineBtween;
	m_wtoggleOne = wToggleOne;
	m_wtoggleTwo = wToggleTwo;
	m_wtoggleThree = wToggleThree;
	m_wpreviewArea = wPreviewArea;
	m_wSpin = Spinbutton;
	m_windowMain = windowColumns;
	m_wSpaceAfterSpin = SpinAfter_dum;
	m_wSpaceAfterEntry = SpinAfter;
	m_oSpaceAfter_adj =  SpinAfterAdj;
	m_iSpaceAfter = (UT_sint32) gtk_adjustment_get_value(SpinAfterAdj);
	m_wMaxColumnHeightSpin = SpinSize_dum;
	m_wMaxColumnHeightEntry = SpinSize;
	m_oSpinSize_adj = SpinSizeAdj;
	m_iSizeHeight = (UT_sint32) gtk_adjustment_get_value(SpinSizeAdj);
}

void AP_UnixDialog_Columns::_connectsignals(void)
{

	// the control buttons
	m_oneHandlerID = g_signal_connect(G_OBJECT(m_wtoggleOne),
					   "clicked",
					   G_CALLBACK(s_one_clicked),
					   reinterpret_cast<gpointer>(this));

	m_twoHandlerID = g_signal_connect(G_OBJECT(m_wtoggleTwo),
					   "clicked",
					   G_CALLBACK(s_two_clicked),
					   reinterpret_cast<gpointer>(this));

	m_threeHandlerID = g_signal_connect(G_OBJECT(m_wtoggleThree),
					   "clicked",
					   G_CALLBACK(s_three_clicked),
					   reinterpret_cast<gpointer>(this));


	m_spinHandlerID = g_signal_connect(G_OBJECT(m_wSpin),
					   "changed",
					   G_CALLBACK(s_spin_changed),
					   reinterpret_cast<gpointer>(this));


	g_signal_connect(G_OBJECT(m_wSpaceAfterSpin),
					   "changed",
					  G_CALLBACK(s_SpaceAfterSpin_changed),
					   reinterpret_cast<gpointer>(this));


	g_signal_connect(G_OBJECT(m_wMaxColumnHeightSpin),
					   "changed",
					  G_CALLBACK(s_HeightSpin_changed),
					   reinterpret_cast<gpointer>(this));

	m_iSpaceAfterID = g_signal_connect(G_OBJECT(m_wSpaceAfterEntry),
					   "changed",
					  G_CALLBACK(s_SpaceAfterEntry_changed),
					   reinterpret_cast<gpointer>(this));


	m_iMaxColumnHeightID = g_signal_connect(G_OBJECT(m_wMaxColumnHeightEntry),
					   "changed",
					  G_CALLBACK(s_MaxHeightEntry_changed),
					   reinterpret_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wlineBetween),
					   "clicked",
					   G_CALLBACK(s_line_clicked),
					   reinterpret_cast<gpointer>(this));

	// the expose event of the preview
	g_signal_connect(G_OBJECT(m_wpreviewArea),
			 "draw",
			 G_CALLBACK(s_preview_draw),
			 reinterpret_cast<gpointer>(this));

	g_signal_connect_after(G_OBJECT(m_windowMain),
			       "draw",
			       G_CALLBACK(s_window_draw),
			       reinterpret_cast<gpointer>(this));
}

void AP_UnixDialog_Columns::_populateWindowData(void)
{
	// We're a pretty stateless dialog, so we just set up
	// the defaults from our members.
}

void AP_UnixDialog_Columns::_storeWindowData(void)
{
}

void AP_UnixDialog_Columns::enableLineBetweenControl(bool /*bState*/)
{
}





