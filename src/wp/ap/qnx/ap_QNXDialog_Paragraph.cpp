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

#include <stdlib.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include <stdio.h>

// This header defines some functions for QNX dialogs,
// like centering them, measuring them, etc.
#include "gr_QNXGraphics.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "ap_Dialog_Id.h"

#include "ap_Strings.h"

#include "ap_Preview_Paragraph.h"
#include "ap_QNXDialog_Paragraph.h"

/*****************************************************************/

#define WIDGET_MENU_PARENT_ID_TAG	"parentmenu"
#define WIDGET_MENU_VALUE_TAG		"menuvalue"
#define WIDGET_DIALOG_TAG 			"dialog"
#define WIDGET_ID_TAG				"id"

/*****************************************************************/

XAP_Dialog * AP_QNXDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_QNXDialog_Paragraph * p = new AP_QNXDialog_Paragraph(pFactory,id);
	return p;
}

AP_QNXDialog_Paragraph::AP_QNXDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Paragraph(pDlgFactory,id)
{
	m_unixGraphics = NULL;
	m_bEditChanged = UT_FALSE;
}

AP_QNXDialog_Paragraph::~AP_QNXDialog_Paragraph(void)
{
	DELETEP(m_unixGraphics);
}

/*****************************************************************/
/* These are static callbacks for dialog widgets                 */
/*****************************************************************/

static int s_ok_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t *info)
{ UT_ASSERT(widget && data); AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data; dlg->event_OK(); return Pt_CONTINUE; }

static int s_cancel_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{ UT_ASSERT(widget && data); AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data; dlg->event_Cancel(); return Pt_CONTINUE; }

static int s_tabs_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{ UT_ASSERT(widget && dlg); AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;	dlg->event_Tabs(); return Pt_CONTINUE; }

static int s_delete_clicked(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{ UT_ASSERT(dlg);  AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data; dlg->event_WindowDelete(); return Pt_CONTINUE; }

static int s_spin_focus_out(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{
	AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;
	dlg->event_SpinFocusOut(widget);
	
	// do NOT let GTK do its own update (which would erase the text we just
	// put in the entry area
	return Pt_CONTINUE;
}

static int s_spin_changed(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{
	// notify the dialog that an edit has changed
	AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;
	dlg->event_SpinChanged(widget);
	return Pt_CONTINUE;
}

static int s_menu_item_activate(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{
	UT_ASSERT(widget && data);
	
	AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;
	dlg->event_MenuChanged(widget);
	return Pt_CONTINUE;
}

static int s_check_toggled(PtWidget_t * widget, void *data, PtCallbackInfo_t * info)
{
	UT_ASSERT(widget && data);
	AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;
	dlg->event_CheckToggled(widget);
	return Pt_CONTINUE;
}

static int s_preview_exposed(PtWidget_t *widget, void *data, PtCallbackInfo_t * info)
{
	UT_ASSERT(data);
	AP_QNXDialog_Paragraph * dlg = (AP_QNXDialog_Paragraph *)data;
	dlg->event_PreviewAreaExposed();
	return Pt_CONTINUE;
}

/*****************************************************************/

void AP_QNXDialog_Paragraph::runModal(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Populate the window's data items
	_populateWindowData();

	// Attach signals (after data settings, so we don't trigger
	// updates yet)
	_connectCallbackSignals();

#if 0
	// To center the dialog, we need the frame of its parent.
	XAP_QNXFrame * pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(pQNXFrame);
	
	// Get the GtkWindow of the parent frame
	PtWidget_t * parentWindow = pQNXFrame->getTopLevelWindow();
	UT_ASSERT(parentWindow);
	
	// Center our new dialog in its parent and make it a transient
	// so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
	gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

	// Show the top level dialog,
	gtk_widget_show(mainWindow);

	// Make it modal, and stick it up top
	gtk_grab_add(mainWindow);

	// *** this is how we add the gc ***
	{
		// attach a new graphics context to the drawing area
		XAP_QNXApp * unixapp = static_cast<XAP_QNXApp *> (m_pApp);
		UT_ASSERT(unixapp);

		UT_ASSERT(m_drawingareaPreview && m_drawingareaPreview->window);

		// make a new QNX GC
		m_unixGraphics = new GR_QNXGraphics(m_drawingareaPreview->window, unixapp->getFontManager());
		
		// let the widget materialize
		_createPreviewFromGC(m_unixGraphics,
							 (UT_uint32) m_drawingareaPreview->allocation.width,
							 (UT_uint32) m_drawingareaPreview->allocation.height);
	}

	// sync all controls once to get started
	// HACK: the first arg gets ignored
	_syncControls(id_MENU_ALIGNMENT, UT_TRUE);

	// Run into the GTK event loop for this window.
	gtk_main();

	gtk_widget_destroy(mainWindow);
#endif

	printf("Running the paragraph main window loop \n");
	PtRealizeWidget(mainWindow);
	int count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(count);

	PtDestroyWidget(mainWindow);
	//m_answer = AP_Dialog_Paragraph::a_CANCEL;
}

/*****************************************************************/

void AP_QNXDialog_Paragraph::event_OK(void)
{
	m_answer = AP_Dialog_Paragraph::a_OK;
	done = 1;
}

void AP_QNXDialog_Paragraph::event_Cancel(void)
{
	m_answer = AP_Dialog_Paragraph::a_CANCEL;
	done = 1;
}

void AP_QNXDialog_Paragraph::event_Tabs(void)
{
	m_answer = AP_Dialog_Paragraph::a_TABS;
	done = 1;
}

void AP_QNXDialog_Paragraph::event_WindowDelete(void)
{
	if (!done) {
		m_answer = AP_Dialog_Paragraph::a_CANCEL;	
	}
	done = 1;
}

void AP_QNXDialog_Paragraph::event_MenuChanged(PtWidget_t * widget)
{
#if 0
	UT_ASSERT(widget);

	tControl id = (tControl) gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_MENU_PARENT_ID_TAG);

	UT_uint32 value = (UT_uint32) gtk_object_get_data(GTK_OBJECT(widget),
													  WIDGET_MENU_VALUE_TAG);

	_setMenuItemValue(id, value);
#endif
}

void AP_QNXDialog_Paragraph::event_SpinIncrement(PtWidget_t * widget)
{
	UT_ASSERT(widget);
}

void AP_QNXDialog_Paragraph::event_SpinDecrement(PtWidget_t * widget)
{
	UT_ASSERT(widget);
}

void AP_QNXDialog_Paragraph::event_SpinFocusOut(PtWidget_t * widget)
{
#if 0
	tControl id = (tControl) gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_ID_TAG);

	if (m_bEditChanged)
	{
		// this function will massage the contents for proper
		// formatting for spinbuttons that need it.  for example,
		// line spacing can't be negative.
		_setSpinItemValue(id, (const XML_Char *)
						  gtk_entry_get_text(GTK_ENTRY(widget)));

		// to ensure the massaged value is reflected back up
		// to the screen, we repaint from the member variable
		_syncControls(id);
		
		m_bEditChanged = UT_FALSE;
	}
#endif
}

void AP_QNXDialog_Paragraph::event_SpinChanged(PtWidget_t * widget)
{
#if 0
	tControl id = (tControl) gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_ID_TAG);
	m_bEditChanged = UT_TRUE;
#endif
}
	   
void AP_QNXDialog_Paragraph::event_CheckToggled(PtWidget_t * widget)
{
#if 0
	UT_ASSERT(widget);

	tControl id = (tControl) gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_ID_TAG);

	gboolean state = gtk_toggle_button_get_active(
		GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(widget)));

	tCheckState cs;

	// TODO : handle tri-state boxes !!!
	if (state == TRUE)
		cs = check_TRUE;
	else
		cs = check_FALSE;
	
	_setCheckItemValue(id, cs);
#endif
}

void AP_QNXDialog_Paragraph::event_PreviewAreaExposed(void)
{
	if (m_paragraphPreview)
		m_paragraphPreview->draw();
}

/*****************************************************************/

PtWidget_t * AP_QNXDialog_Paragraph::_constructWindow(void)
{
	// grab the string set
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtWidget_t * windowParagraph;
	PtWidget_t * vboxMain;
	PtWidget_t * fixedMain;
	PtWidget_t * tabMain;
	PtWidget_t * fixedSpacing;
	PtWidget_t * listAlignment;
	PtWidget_t * listAlignment_menu;
	PtWidget_t * glade_menuitem;
	PtWidget_t * spinbuttonLeft;
	PtWidget_t * spinbuttonRight;
	PtWidget_t * listSpecial;
	PtWidget_t * listSpecial_menu;
	PtWidget_t * spinbuttonBy;
	PtWidget_t * spinbuttonBefore;
	PtWidget_t * spinbuttonAfter;
	PtWidget_t * listLineSpacing;
	PtWidget_t * listLineSpacing_menu;
	PtWidget_t * spinbuttonAt;
	PtWidget_t * labelAlignment;
	PtWidget_t * labelBy;
	PtWidget_t * labelIndentation;
	PtWidget_t * labelLeft;
	PtWidget_t * labelRight;
	PtWidget_t * labelSpecial;
	PtWidget_t * hseparator3;
	PtWidget_t * labelSpacing;
	PtWidget_t * labelAfter;
	PtWidget_t * labelLineSpacing;
	PtWidget_t * labelAt;
	PtWidget_t * labelPreview;

	PtWidget_t * framePreview;
	PtWidget_t * drawingareaPreview;
	PtWidget_t * drawingAreaPreview2;

	PtWidget_t * hseparator4;
	PtWidget_t * hseparator1;
	PtWidget_t * labelBefore;
	PtWidget_t * labelIndents;
	PtWidget_t * fixedBreaks;
	PtWidget_t * labelPagination;
	PtWidget_t * hseparator5;
	PtWidget_t * hseparator7;
	PtWidget_t * labelPreview2;
	PtWidget_t * checkbuttonWindowOrphan;
	PtWidget_t * checkbuttonKeepLines;
	PtWidget_t * checkbuttonPagebreak;
	PtWidget_t * checkbuttonSuppress;
	PtWidget_t * checkbuttonHyphenate;
	PtWidget_t * hseparator6;
	PtWidget_t * checkbuttonKeepNext;
	PtWidget_t * labelBreaks;
	PtWidget_t * hbox1;
	PtWidget_t * hbuttonboxLeft;
	PtWidget_t * buttonTabs;
	PtWidget_t * hbox2;
	PtWidget_t * hbuttonboxRight;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;
	PtWidget_t * panelGroup;

	XML_Char * unixstr = NULL;
	const char *litem[1];

	litem[0] = unixstr;

	PtArg_t args[10];
	int     n;

#define WIN_WIDTH  550	
#define WIN_HEIGHT 400	
	n = 0;
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ParaTitle));
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, unixstr, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 550, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 400, 0);
	PtSetParentWidget(NULL);
	windowParagraph = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowParagraph, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
	FREEP(unixstr);

	n = 0;
	PhPoint_t pos;
#define PANEL_WIDTH 530
#define PANEL_HEIGHT 350
	pos.x = (WIN_WIDTH - PANEL_WIDTH) / 2;
	pos.y = (WIN_HEIGHT - PANEL_HEIGHT - 30) / 2;
	PtSetArg(&args[n++], Pt_ARG_POS, &pos, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 530, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, 380, 0);
	panelGroup = PtCreateWidget(PtPanelGroup, windowParagraph, n, args);	

	/* Create the Indent Tab */
	/* Code Fragment generated by PhAB200 */
	{
	static const PhArea_t area1 = { { 12, 13 }, { 534, 351 } };
	static const PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BORDER_WIDTH, 1, 0 ),
		Pt_ARG( Pt_ARG_TITLE, "Indents and Spacing", 0 ),
		};

	static const PhArea_t area2 = { { 5, 43 }, { 79, 20 } };
	static const PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Indentation", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area3 = { { 92, 47 }, { 429, 12 } };
	static const PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		};

	static const PhArea_t area4 = { { 91, 12 }, { 104, 26 } };
	static const PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE ),
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		};

	static const PhArea_t area5 = { { 6, 15 }, { 70, 19 } };
	static const PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Alignment:", 0 ),
		};

	static const PhArea_t area6 = { { 26, 70 }, { 35, 19 } };
	static const PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Left:", 0 ),
		};

	static const PhArea_t area7 = { { 26, 102 }, { 40, 19 } };
	static const PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Right:", 0 ),
		};

	static const PhArea_t area8 = { { 91, 68 }, { 87, 24 } };
	static const PtArg_t args8[] = {
		Pt_ARG( Pt_ARG_AREA, &area8, 0 ),
		};

	static const PhArea_t area9 = { { 91, 99 }, { 87, 24 } };
	static const PtArg_t args9[] = {
		Pt_ARG( Pt_ARG_AREA, &area9, 0 ),
		};

	static const PhArea_t area10 = { { 250, 95 }, { 109, 26 } };
	static const PtArg_t args10[] = {
		Pt_ARG( Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE ),
		Pt_ARG( Pt_ARG_AREA, &area10, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "(none)", 0 ),
		};

	static const PhArea_t area11 = { { 398, 95 }, { 87, 24 } };
	static const PtArg_t args11[] = {
		Pt_ARG( Pt_ARG_AREA, &area11, 0 ),
		};

	static const PhArea_t area12 = { { 396, 75 }, { 35, 19 } };
	static const PtArg_t args12[] = {
		Pt_ARG( Pt_ARG_AREA, &area12, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "By:", 0 ),
		};

	static const PhArea_t area13 = { { 248, 73 }, { 51, 19 } };
	static const PtArg_t args13[] = {
		Pt_ARG( Pt_ARG_AREA, &area13, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Special:", 0 ),
		};

	static const PhArea_t area14 = { { 64, 138 }, { 455, 12 } };
	static const PtArg_t args14[] = {
		Pt_ARG( Pt_ARG_AREA, &area14, 0 ),
		};

	static const PhArea_t area15 = { { 3, 134 }, { 76, 20 } };
	static const PtArg_t args15[] = {
		Pt_ARG( Pt_ARG_AREA, &area15, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Spacing", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area16 = { { 248, 165 }, { 84, 19 } };
	static const PtArg_t args16[] = {
		Pt_ARG( Pt_ARG_AREA, &area16, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Line spacing:", 0 ),
		};

	static const PhArea_t area17 = { { 396, 167 }, { 35, 19 } };
	static const PtArg_t args17[] = {
		Pt_ARG( Pt_ARG_AREA, &area17, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "At:", 0 ),
		};

	static const PhArea_t area18 = { { 91, 160 }, { 87, 24 } };
	static const PtArg_t args18[] = {
		Pt_ARG( Pt_ARG_AREA, &area18, 0 ),
		};

	static const PhArea_t area19 = { { 26, 162 }, { 56, 19 } };
	static const PtArg_t args19[] = {
		Pt_ARG( Pt_ARG_AREA, &area19, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Before:", 0 ),
		};

	static const PhArea_t area20 = { { 398, 188 }, { 87, 24 } };
	static const PtArg_t args20[] = {
		Pt_ARG( Pt_ARG_AREA, &area20, 0 ),
		};

	static const PhArea_t area21 = { { 250, 188 }, { 109, 26 } };
	static const PtArg_t args21[] = {
		Pt_ARG( Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE ),
		Pt_ARG( Pt_ARG_AREA, &area21, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "(none)", 0 ),
		};

	static const PhArea_t area22 = { { 91, 192 }, { 87, 24 } };
	static const PtArg_t args22[] = {
		Pt_ARG( Pt_ARG_AREA, &area22, 0 ),
		};

	static const PhArea_t area23 = { { 26, 195 }, { 42, 19 } };
	static const PtArg_t args23[] = {
		Pt_ARG( Pt_ARG_AREA, &area23, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "After:", 0 ),
		};

	static const PhArea_t area24 = { { 65, 223 }, { 455, 12 } };
	static const PtArg_t args24[] = {
		Pt_ARG( Pt_ARG_AREA, &area24, 0 ),
		};

	static const PhArea_t area25 = { { 4, 219 }, { 76, 20 } };
	static const PtArg_t args25[] = {
		Pt_ARG( Pt_ARG_AREA, &area25, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Preview", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area26 = { { 19, 243 }, { 493, 96 } };
	static const PtArg_t args26[] = {
		Pt_ARG( Pt_ARG_AREA, &area26, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BORDER_WIDTH, 1, 0 ),
		};

	PtCreateWidget( PtPane, panelGroup, sizeof(args1) / sizeof(PtArg_t), args1 );

	labelIndentation = PtCreateWidget( PtLabel, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	listAlignment = PtCreateWidget( PtComboBox, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignLeft)); litem[0]= unixstr;
	PtListAddItems(listAlignment, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignCentered)); litem[0]= unixstr;
	PtListAddItems(listAlignment, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignRight)); litem[0]= unixstr;
	PtListAddItems(listAlignment, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignJustified)); litem[0]= unixstr;
	PtListAddItems(listAlignment, litem, 1, 0); FREEP(unixstr);

	labelAlignment = PtCreateWidget( PtLabel, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );

	labelLeft = PtCreateWidget( PtLabel, NULL, sizeof(args6) / sizeof(PtArg_t), args6 );

	labelRight = PtCreateWidget( PtLabel, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );

	spinbuttonLeft = PtCreateWidget( PtNumericInteger, NULL, sizeof(args8) / sizeof(PtArg_t), args8 );

	spinbuttonRight = PtCreateWidget( PtNumericInteger, NULL, sizeof(args9) / sizeof(PtArg_t), args9 );

	listSpecial = PtCreateWidget( PtComboBox, NULL, sizeof(args10) / sizeof(PtArg_t), args10 );
	PtListDeleteAllItems(listSpecial);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialNone)); litem[0]= unixstr;
	PtListAddItems(listSpecial, litem, 1, 0); FREEP(unixstr);

	spinbuttonBy = PtCreateWidget( PtNumericInteger, NULL, sizeof(args11) / sizeof(PtArg_t), args11 );

	labelBy = PtCreateWidget( PtLabel, NULL, sizeof(args12) / sizeof(PtArg_t), args12 );

	labelSpecial = PtCreateWidget( PtLabel, NULL, sizeof(args13) / sizeof(PtArg_t), args13 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args14) / sizeof(PtArg_t), args14 );

	PtCreateWidget( PtLabel, NULL, sizeof(args15) / sizeof(PtArg_t), args15 );

	labelLineSpacing = PtCreateWidget( PtLabel, NULL, sizeof(args16) / sizeof(PtArg_t), args16 );

	labelAt = PtCreateWidget( PtLabel, NULL, sizeof(args17) / sizeof(PtArg_t), args17 );

	spinbuttonBefore = PtCreateWidget( PtNumericInteger, NULL, sizeof(args18) / sizeof(PtArg_t), args18 );

	labelBefore = PtCreateWidget( PtLabel, NULL, sizeof(args19) / sizeof(PtArg_t), args19 );

	spinbuttonAt = PtCreateWidget( PtNumericInteger, NULL, sizeof(args20) / sizeof(PtArg_t), args20 );

	listLineSpacing = PtCreateWidget( PtComboBox, NULL, sizeof(args21) / sizeof(PtArg_t), args21 );
	PtListDeleteAllItems(listLineSpacing);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingSingle)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingHalf)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingDouble)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingAtLeast)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingExactly)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingMultiple)); litem[0]= unixstr;
	PtListAddItems(listLineSpacing, litem, 1, 0); FREEP(unixstr);

	spinbuttonAfter = PtCreateWidget( PtNumericInteger, NULL, sizeof(args22) / sizeof(PtArg_t), args22 );

	labelAfter = PtCreateWidget( PtLabel, NULL, sizeof(args23) / sizeof(PtArg_t), args23 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args24) / sizeof(PtArg_t), args24 );

	labelPreview = PtCreateWidget( PtLabel, NULL, sizeof(args25) / sizeof(PtArg_t), args25 );

	drawingareaPreview = PtCreateWidget( PtRaw, NULL, sizeof(args26) / sizeof(PtArg_t), args26 );


	}
	/* Code Fragment complete */

	/* Create the Break tab */ 
	/* Code Fragment generated by PhAB200 */
	{
	static const PhArea_t area1 = { { 10, 13 }, { 533, 352 } };
	static const PtArg_t args1[] = {
		Pt_ARG( Pt_ARG_AREA, &area1, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BORDER_WIDTH, 1, 0 ),
		Pt_ARG( Pt_ARG_TITLE, "Line and Page Breaks", 0 ),
		};

	static const PhArea_t area2 = { { 6, 8 }, { 79, 20 } };
	static const PtArg_t args2[] = {
		Pt_ARG( Pt_ARG_AREA, &area2, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Pagination", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area3 = { { 88, 12 }, { 434, 12 } };
	static const PtArg_t args3[] = {
		Pt_ARG( Pt_ARG_AREA, &area3, 0 ),
		};

	static const PhArea_t area4 = { { 23, 30 }, { 164, 24 } };
	static const PtArg_t args4[] = {
		Pt_ARG( Pt_ARG_AREA, &area4, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Window/Orphan control", 0 ),
		};

	static const PhArea_t area5 = { { 23, 54 }, { 140, 24 } };
	static const PtArg_t args5[] = {
		Pt_ARG( Pt_ARG_AREA, &area5, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Keep lines together", 0 ),
		};

	static const PhArea_t area6 = { { 247, 29 }, { 112, 24 } };
	static const PtArg_t args6[] = {
		Pt_ARG( Pt_ARG_AREA, &area6, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Keep with next", 0 ),
		};

	static const PhArea_t area7 = { { 247, 50 }, { 132, 24 } };
	static const PtArg_t args7[] = {
		Pt_ARG( Pt_ARG_AREA, &area7, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Page break before", 0 ),
		};

	static const PhArea_t area8 = { { 24, 103 }, { 162, 24 } };
	static const PtArg_t args8[] = {
		Pt_ARG( Pt_ARG_AREA, &area8, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Suppress line numbers", 0 ),
		};

	static const PhArea_t area9 = { { 24, 125 }, { 131, 24 } };
	static const PtArg_t args9[] = {
		Pt_ARG( Pt_ARG_AREA, &area9, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Don't hyphenate", 0 ),
		};

	static const PhArea_t area10 = { { 10, 88 }, { 512, 12 } };
	static const PtArg_t args10[] = {
		Pt_ARG( Pt_ARG_AREA, &area10, 0 ),
		};

	static const PhArea_t area11 = { { 71, 224 }, { 452, 12 } };
	static const PtArg_t args11[] = {
		Pt_ARG( Pt_ARG_AREA, &area11, 0 ),
		};

	static const PhArea_t area12 = { { 7, 220 }, { 79, 20 } };
	static const PtArg_t args12[] = {
		Pt_ARG( Pt_ARG_AREA, &area12, 0 ),
		Pt_ARG( Pt_ARG_TEXT_STRING, "Preview", 0 ),
		Pt_ARG( Pt_ARG_TEXT_FONT, "TextFont10", 0 ),
		};

	static const PhArea_t area13 = { { 22, 244 }, { 493, 96 } };
	static const PtArg_t args13[] = {
		Pt_ARG( Pt_ARG_AREA, &area13, 0 ),
		Pt_ARG( Pt_ARG_FLAGS, 256,256 ),
		Pt_ARG( Pt_ARG_BORDER_WIDTH, 1, 0 ),
		};

	PtCreateWidget( PtPane, panelGroup, sizeof(args1) / sizeof(PtArg_t), args1 );

	labelPagination = PtCreateWidget( PtLabel, NULL, sizeof(args2) / sizeof(PtArg_t), args2 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args3) / sizeof(PtArg_t), args3 );

	checkbuttonWindowOrphan = PtCreateWidget( PtToggleButton, NULL, sizeof(args4) / sizeof(PtArg_t), args4 );

	checkbuttonKeepLines = PtCreateWidget( PtToggleButton, NULL, sizeof(args5) / sizeof(PtArg_t), args5 );

	checkbuttonKeepNext = PtCreateWidget( PtToggleButton, NULL, sizeof(args6) / sizeof(PtArg_t), args6 );

	checkbuttonPagebreak = PtCreateWidget( PtToggleButton, NULL, sizeof(args7) / sizeof(PtArg_t), args7 );

	checkbuttonSuppress = PtCreateWidget( PtToggleButton, NULL, sizeof(args8) / sizeof(PtArg_t), args8 );

	checkbuttonHyphenate = PtCreateWidget( PtToggleButton, NULL, sizeof(args9) / sizeof(PtArg_t), args9 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args10) / sizeof(PtArg_t), args10 );

	PtCreateWidget( PtSeparator, NULL, sizeof(args11) / sizeof(PtArg_t), args11 );

	labelPreview2 = PtCreateWidget( PtLabel, NULL, sizeof(args12) / sizeof(PtArg_t), args12 );

	drawingAreaPreview2 = PtCreateWidget( PtRaw, NULL, sizeof(args13) / sizeof(PtArg_t), args13 );


	}
	/* Code Fragment complete */

	/* Now at the bottom add in some buttons ... */
#define BUTTON_WIDTH 80
	PhDim_t dim;
	dim.w = BUTTON_WIDTH;
	dim.h = 24;
	n = 0;
	pos.y +=  PANEL_HEIGHT + 40;
	PtSetArg(&args[n++], Pt_ARG_POS, &pos, 0);
	PtSetArg(&args[n++], Pt_ARG_DIM, &dim, 0);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ButtonTabs));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	buttonTabs = PtCreateWidget(PtButton, windowParagraph, n, args);
	FREEP(unixstr);
	n = 0;
	pos.x = WIN_WIDTH - BUTTON_WIDTH - 10;
	PtSetArg(&args[n++], Pt_ARG_POS, &pos, 0);
	PtSetArg(&args[n++], Pt_ARG_DIM, &dim, 0);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	buttonCancel = PtCreateWidget(PtButton, windowParagraph, n, args);
	FREEP(unixstr);
	n = 0;
	pos.x -= BUTTON_WIDTH + 10;
	PtSetArg(&args[n++], Pt_ARG_POS, &pos, 0);
	PtSetArg(&args[n++], Pt_ARG_DIM, &dim, 0);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(XAP_STRING_ID_DLG_OK));
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, unixstr, 0);
	buttonOK = PtCreateWidget(PtButton, windowParagraph, n, args);
	FREEP(unixstr);

#if 0
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignLeft));
	FREEP(unixstr);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignCentered));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignRight));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_AlignJustified));
	FREEP(unixstr);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialNone));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialFirstLine));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpecialHanging));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingSingle));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingHalf));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingDouble));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingAtLeast));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingExactly));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_SpacingMultiple));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelAlignment));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelBy));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelIndentation));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelLeft));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelRight));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelSpecial));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelSpacing));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelAfter));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelLineSpacing));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelAt));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelPreview));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelBefore));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_TabLabelIndentsAndSpacing));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelPagination));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_LabelPreview));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushWidowOrphanControl));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushKeepLinesTogether));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushPageBreakBefore));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushSuppressLineNumbers));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushNoHyphenate));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_PushKeepWithNext));
	FREEP(unixstr);
	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_TabLabelLineAndPageBreaks));
	FREEP(unixstr);
#endif
	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowParagraph;

	m_listAlignment = listAlignment;

	m_spinbuttonLeft = spinbuttonLeft;
	
	m_spinbuttonRight = spinbuttonRight;
	m_listSpecial = listSpecial;
	m_listSpecial_menu = listSpecial_menu;
	m_spinbuttonBy = spinbuttonBy;
	m_spinbuttonBefore = spinbuttonBefore;
	m_spinbuttonAfter = spinbuttonAfter;
	m_listLineSpacing = listLineSpacing;
	m_listLineSpacing_menu = listLineSpacing_menu;
	m_spinbuttonAt = spinbuttonAt;

	m_drawingareaPreview = drawingareaPreview;

	m_checkbuttonWidowOrphan = checkbuttonWindowOrphan;
	m_checkbuttonKeepLines = checkbuttonKeepLines;
	m_checkbuttonPageBreak = checkbuttonPagebreak;
	m_checkbuttonSuppress = checkbuttonSuppress;
	m_checkbuttonHyphenate = checkbuttonHyphenate;
	m_checkbuttonKeepNext = checkbuttonKeepNext;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	m_buttonTabs = buttonTabs;

	return windowParagraph;
}

void AP_QNXDialog_Paragraph::_connectCallbackSignals(void)
{
	PtAddCallback(m_buttonTabs, Pt_CB_ACTIVATE, s_tabs_clicked, this);
	PtAddCallback(m_buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);
	PtAddCallback(m_buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);
#if 0
	// we have to handle the changes in values for spin buttons
	// to preserve units
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonLeft);
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonRight);
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonBy);
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonBefore);
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonAfter);	
	CONNECT_SPIN_SIGNAL_CHANGED(m_spinbuttonAt);
	
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonLeft);
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonRight);
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonBy);
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonBefore);
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonAfter);	
	CONNECT_SPIN_SIGNAL_FOCUS_OUT(m_spinbuttonAt);

	// connect to option menus
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemLeft);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemCentered);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemRight);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemJustified);

	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemNone);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemFirstLine);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemHanging);	
	
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemSingle);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemOneAndHalf);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemDouble);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemAtLeast);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemExactly);
	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(m_menuitemMultiple);
	
	// all the checkbuttons
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonWidowOrphan), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonKeepLines), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonPageBreak), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonSuppress), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonHyphenate), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	gtk_signal_connect(GTK_OBJECT(m_checkbuttonKeepNext), "toggled",
					   GTK_SIGNAL_FUNC(s_check_toggled), (gpointer) this);
	
	// the catch-alls
	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(m_windowMain),
							 "destroy",
							 NULL,
							 NULL);

	// the expose event off the preview
	gtk_signal_connect(GTK_OBJECT(m_drawingareaPreview),
					   "expose_event",
					   GTK_SIGNAL_FUNC(s_preview_exposed),
					   (gpointer) this);
#endif
}

void AP_QNXDialog_Paragraph::_populateWindowData(void)
{
#if 0
	// alignment option menu 
	UT_ASSERT(m_listAlignment);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listAlignment),
								(int) _getMenuItemValue(id_MENU_ALIGNMENT));

	// indent and paragraph margins
	UT_ASSERT(m_spinbuttonLeft);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonLeft),
					   (const gchar *) _getSpinItemValue(id_SPIN_LEFT_INDENT));

	UT_ASSERT(m_spinbuttonRight);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonRight),
					   (const gchar *) _getSpinItemValue(id_SPIN_RIGHT_INDENT));

	UT_ASSERT(m_spinbuttonBy);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy),
					   (const gchar *) _getSpinItemValue(id_SPIN_SPECIAL_INDENT));

	UT_ASSERT(m_listSpecial);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listSpecial),
								(int) _getMenuItemValue(id_MENU_SPECIAL_INDENT));

	// spacing
	UT_ASSERT(m_spinbuttonLeft);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBefore),
					   (const gchar *) _getSpinItemValue(id_SPIN_BEFORE_SPACING));

	UT_ASSERT(m_spinbuttonRight);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAfter),
					   (const gchar *) _getSpinItemValue(id_SPIN_AFTER_SPACING));

	UT_ASSERT(m_spinbuttonAt);
	gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt),
					   (const gchar *) _getSpinItemValue(id_SPIN_SPECIAL_SPACING));

	UT_ASSERT(m_listLineSpacing);
	gtk_option_menu_set_history(GTK_OPTION_MENU(m_listLineSpacing),
								(int) _getMenuItemValue(id_MENU_SPECIAL_SPACING));

	// set the check boxes
	// TODO : handle tri-state boxes !!!

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonWidowOrphan)),
								 (_getCheckItemValue(id_CHECK_WIDOW_ORPHAN) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonKeepLines)),
								 (_getCheckItemValue(id_CHECK_KEEP_LINES) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonPageBreak)),
								 (_getCheckItemValue(id_CHECK_PAGE_BREAK) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonSuppress)),
								 (_getCheckItemValue(id_CHECK_SUPPRESS) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonHyphenate)),
								 (_getCheckItemValue(id_CHECK_NO_HYPHENATE) == check_TRUE));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(GTK_CHECK_BUTTON(m_checkbuttonKeepNext)),
								 (_getCheckItemValue(id_CHECK_KEEP_NEXT) == check_TRUE));
#endif
}

void AP_QNXDialog_Paragraph::_syncControls(tControl changed, UT_Bool bAll /* = UT_FALSE */)
{
#if 0
	// let parent sync any member variables first
	AP_Dialog_Paragraph::_syncControls(changed, bAll);

	// sync the display

	// 1.  link the "hanging indent by" combo and spinner
	if (bAll || (changed == id_SPIN_SPECIAL_INDENT))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_INDENT) == indent_FIRSTLINE)
		{
			gtk_option_menu_set_history(GTK_OPTION_MENU(m_listSpecial),
										(int) _getMenuItemValue(id_MENU_SPECIAL_INDENT));
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_INDENT))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_INDENT))
		{
		case indent_NONE:
			// clear the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy), "");
			break;

		default:
			// set the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy), _getSpinItemValue(id_SPIN_SPECIAL_INDENT));			
			break;
		}
	}

	// 2.  link the "line spacing at" combo and spinner

	if (bAll || (changed == id_SPIN_SPECIAL_SPACING))
	{
		// typing in the control can change the associated combo
		if (_getMenuItemValue(id_MENU_SPECIAL_SPACING) == spacing_MULTIPLE)
		{
			gtk_option_menu_set_history(GTK_OPTION_MENU(m_listLineSpacing),
										(int) _getMenuItemValue(id_MENU_SPECIAL_SPACING));
		}
	}
	if (bAll || (changed == id_MENU_SPECIAL_SPACING))
	{
		switch(_getMenuItemValue(id_MENU_SPECIAL_SPACING))
		{
		case spacing_SINGLE:
		case spacing_ONEANDHALF:
		case spacing_DOUBLE:
			// clear the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt), "");
			break;

		default:
			// set the spin control
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt), _getSpinItemValue(id_SPIN_SPECIAL_SPACING));
			break;
		}
	}

	// 3.  move results of _doSpin() back to screen

	if (!bAll)
	{
		// spin controls only sync when spun
		switch (changed)
		{
		case id_SPIN_LEFT_INDENT:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonLeft), 	_getSpinItemValue(id_SPIN_LEFT_INDENT));
		case id_SPIN_RIGHT_INDENT:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonRight), 	_getSpinItemValue(id_SPIN_RIGHT_INDENT));
		case id_SPIN_SPECIAL_INDENT:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBy), 		_getSpinItemValue(id_SPIN_SPECIAL_INDENT));
		case id_SPIN_BEFORE_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonBefore), 	_getSpinItemValue(id_SPIN_BEFORE_SPACING));
		case id_SPIN_AFTER_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAfter), 	_getSpinItemValue(id_SPIN_AFTER_SPACING));
		case id_SPIN_SPECIAL_SPACING:
			gtk_entry_set_text(GTK_ENTRY(m_spinbuttonAt), 		_getSpinItemValue(id_SPIN_SPECIAL_SPACING));
		default:
			break;
		}
	}
#endif
}
