/* AbiSource Application Framework
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include <stdio.h>

// This header defines some functions for QNX dialogs,
// like centering them, measuring them, etc.
#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_QNXDlg_WindowMore.h"
#include "ut_qnxHelper.h"

/*****************************************************************/

XAP_Dialog * XAP_QNXDialog_WindowMore::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_QNXDialog_WindowMore * p = new XAP_QNXDialog_WindowMore(pFactory,id);
	return p;
}

XAP_QNXDialog_WindowMore::XAP_QNXDialog_WindowMore(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_WindowMore(pDlgFactory,id)
{
}

XAP_QNXDialog_WindowMore::~XAP_QNXDialog_WindowMore(void)
{
}

/*****************************************************************/
// These are all static callbacks, bound to GTK or GDK events.

static int s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_WindowMore *dlg = (XAP_QNXDialog_WindowMore *)data;
	UT_ASSERT(widget && dlg);

	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_WindowMore *dlg = (XAP_QNXDialog_WindowMore *)data;
	UT_ASSERT(widget && dlg);

	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_clist_event(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_WindowMore *dlg = (XAP_QNXDialog_WindowMore *)data;
	UT_ASSERT(widget && dlg);

	// Only respond to double clicks
	dlg->event_DoubleClick(((PtListCallback_t *)info->cbdata)->item_pos);
	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_WindowMore *dlg = (XAP_QNXDialog_WindowMore *)data;
	UT_ASSERT(dlg);

	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}
/*****************************************************************/

void XAP_QNXDialog_WindowMore::runModal(XAP_Frame * pFrame)
{
	// Initialize member so we know where we are now
	m_ndxSelFrame = m_pApp->findFrame(pFrame);
	UT_ASSERT(m_ndxSelFrame >= 0);

	// Build the window's widgets and arrange them
	PtWidget_t * windowMain = _constructWindow();
	UT_ASSERT(windowMain);

	// Populate the window's data items
	_populateWindowData();
	
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

	// Run into the GTK event loop for this window.
	gtk_main();

	gtk_widget_destroy(mainWindow);
#endif

   printf("Running the more windows main window loop \n");
   PtRealizeWidget(windowMain);
   int count = PtModalStart();
   done = 0;
   while(!done) {
           PtProcessEvent();
   }
   PtModalEnd(MODAL_END_ARG(count));	
}

void XAP_QNXDialog_WindowMore::event_OK(void)
{
	//m_ndxSelFrame = (UT_uint32) row;
	m_answer = XAP_Dialog_WindowMore::a_OK;
	done = 1;
}

void XAP_QNXDialog_WindowMore::event_Cancel(void)
{
	m_answer = XAP_Dialog_WindowMore::a_CANCEL;
	done = 1;
}

void XAP_QNXDialog_WindowMore::event_DoubleClick(int index)
{
	m_ndxSelFrame = index -1;
	event_OK();
}

void XAP_QNXDialog_WindowMore::event_WindowDelete(void)
{
	m_answer = XAP_Dialog_WindowMore::a_CANCEL;	
	done = 1;
}

/*****************************************************************/

PtWidget_t * XAP_QNXDialog_WindowMore::_constructWindow(void)
{
	// This is the top level GTK widget, the window.
	// It's created with a "dialog" style.
	PtWidget_t *windowMain;

	// This is the top level organization widget, which packs
	// things vertically
	PtWidget_t *vboxMain;

	// The top item in the vbox is a simple label
	PtWidget_t *labelActivate;

	// The second item in the vbox is a scrollable area
	PtWidget_t *scrollWindows;
	
	// The child of the scrollable area is our list of windows
	PtWidget_t *clistWindows;
	
	// The third (and bottom) item in the vbox is a horizontal
	// button box, which holds our two action buttons.
	PtWidget_t *buttonboxAction;

	// These are the buttons.
	PtWidget_t *buttonOK;
	PtWidget_t *buttonCancel;
	PtArg_t    args[10];
	int 		n;
	PhArea_t	area;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

#define WIDGET_WIDTH  200
#define LABEL_HEIGHT   10
#define LIST_HEIGHT   100
#define BUTTON_WIDTH   80
#define V_SPACER	   15
#define H_SPACER	   15

	// Create the new top level window.
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, pSS->getValue(XAP_STRING_ID_DLG_MW_MoreWindows), 0);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
    PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowMain = PtCreateWidget(PtWindow, NULL, n, args);
	PtAddCallback(windowMain, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_MW_Activate), 0);
	area.pos.x = H_SPACER; area.pos.y = V_SPACER;
	area.size.w = WIDGET_WIDTH; area.size.h = LABEL_HEIGHT;
	PtSetArg(&args[n++], Pt_ARG_AREA, &area, 0);
	labelActivate = PtCreateWidget(PtList, windowMain, n, args);

	n = 0;
	area.pos.x += area.size.h + V_SPACER;
	area.size.h = LIST_HEIGHT;
	clistWindows = PtCreateWidget(PtList, windowMain, n, args);
	PtAddCallback(windowMain, Pt_CB_SELECTION, s_clist_event, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_OK), 0);
	buttonOK = PtCreateWidget(PtButton, windowMain, n, args);
	PtAddCallback(windowMain, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, pSS->getValue(XAP_STRING_ID_DLG_Cancel), 0);
	buttonCancel = PtCreateWidget(PtButton, windowMain, n, args);
	PtAddCallback(windowMain, Pt_CB_ACTIVATE, s_cancel_clicked, this);

#if 0
	gtk_signal_connect(GTK_OBJECT(buttonOK),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_ok_clicked),
					   (gpointer) this);
	
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
					   "clicked",
					   GTK_SIGNAL_FUNC(s_cancel_clicked),
					   (gpointer) this);

	gtk_signal_connect(GTK_OBJECT(clistWindows),
					   "button_press_event",
					   GTK_SIGNAL_FUNC(s_clist_event),
					   (gpointer) this);

	// Since each modal dialog is raised through gtk_main(),
	// we need to bind a callback to the top level window's
	// "delete" event to make sure we actually exit
	// with gtk_main_quit(), for the case that the user used
	// a window manager to close us.

	gtk_signal_connect_after(GTK_OBJECT(windowMain),
							 "delete_event",
							 GTK_SIGNAL_FUNC(s_delete_clicked),
							 (gpointer) this);

	gtk_signal_connect_after(GTK_OBJECT(windowMain),
							 "destroy",
							 NULL,
							 NULL);
#endif

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	m_windowMain = windowMain;
	m_clistWindows = clistWindows;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	return windowMain;
}

void XAP_QNXDialog_WindowMore::_populateWindowData(void)
{
	// We just do one thing here, which is fill the list with
	// all the windows.

	for (UT_uint32 i = 0; i < m_pApp->getFrameCount(); i++)
	{
		XAP_Frame * f = m_pApp->getFrame(i);
		UT_ASSERT(f);
		const char * s = f->getTitle(128);	// TODO: chop this down more? 
		
		PtListAddItems(m_clistWindows, &s, 1, 0);
	} 
	PtListSelectPos(m_clistWindows, 1);
	m_ndxSelFrame = 0;
}
