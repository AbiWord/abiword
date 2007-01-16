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

#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_vector.h"
#include "ut_hash.h"

#include "gr_QNXGraphics.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_Insert_Symbol.h"
#include "xap_QNXDlg_Insert_Symbol.h"
#include "xap_Draw_Symbol.h"
#include "ut_qnxHelper.h"


/*****************************************************************/
static UT_UCSChar m_CurrentSymbol;
static UT_UCSChar m_PreviousSymbol;

XAP_Dialog * XAP_QNXDialog_Insert_Symbol::static_constructor(XAP_DialogFactory * pFactory,
															  XAP_Dialog_Id id)
{
	XAP_QNXDialog_Insert_Symbol * p = new XAP_QNXDialog_Insert_Symbol(pFactory,id);
	return p;
}

XAP_QNXDialog_Insert_Symbol::XAP_QNXDialog_Insert_Symbol(XAP_DialogFactory * pDlgFactory,
														   XAP_Dialog_Id id)
	: XAP_Dialog_Insert_Symbol(pDlgFactory,id)
{
	m_windowMain = NULL;

	m_qnxGraphics = NULL;
	m_qnxarea = NULL;
	
	m_buttonOK = NULL;
	m_buttonCancel = NULL;

	m_SymbolMap = 	NULL;

	m_areaCurrentSym = NULL;
}

XAP_QNXDialog_Insert_Symbol::~XAP_QNXDialog_Insert_Symbol(void)
{
	DELETEP(m_qnxGraphics);
	DELETEP(m_qnxarea);
}


/*****************************************************************/

static int s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

static int s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

static int s_sym_SymbolMap_exposed(PtWidget_t * w, PhTile_t * damage) 
{
   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtCalcCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	XAP_QNXDialog_Insert_Symbol *pQNXDlg;
	PtGetResource(w, Pt_ARG_POINTER, &pQNXDlg,0);

	UT_ASSERT(pQNXDlg);
	pQNXDlg->SymbolMap_exposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_Symbolarea_exposed(PtWidget_t * w, PhTile_t * damage) 
{

   	PhRect_t rect;
   	PtSuperClassDraw(PtBasic, w, damage);
   	PtCalcCanvas(w, &rect);
	//clip to our basic canvas (it's only polite).
    PtClipAdd( w, &rect );

	XAP_QNXDialog_Insert_Symbol *pQNXDlg;
	PtGetResource(w, Pt_ARG_POINTER, &pQNXDlg,0);

	UT_ASSERT(pQNXDlg);
	pQNXDlg->Symbolarea_exposed();

    PtClipRemove();
	return Pt_CONTINUE;
}

static int s_SymbolMap_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->SymbolMap_clicked(info);
	return Pt_CONTINUE;
}

static int s_CurrentSymbol_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->CurrentSymbol_clicked(info);
	return Pt_CONTINUE;
}


static int s_new_font(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	if(info->reason_subtype == Pt_LIST_SELECTION_FINAL){
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(widget && dlg);
	dlg->New_Font();
	}
	return Pt_CONTINUE;
}

static int s_keypressed(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	dlg->Key_Pressed( info->event);

	return Pt_CONTINUE;
}

static int s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{

	XAP_QNXDialog_Insert_Symbol *dlg = (XAP_QNXDialog_Insert_Symbol *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();

	return Pt_CONTINUE;
}

/*****************************************************************/

void XAP_QNXDialog_Insert_Symbol::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(0);	//DEPRECATED
}

void XAP_QNXDialog_Insert_Symbol::activate(void)
{
	UT_ASSERT(m_windowMain);
	ConstructWindowName();
	PtSetResource(m_windowMain, Pt_ARG_WINDOW_TITLE, m_WindowName, 0);
	PtWindowFocus(m_windowMain);
}

void XAP_QNXDialog_Insert_Symbol::destroy(void)
{
	if (!m_windowMain) {
		return;
	}

	modeless_cleanup();

	PtWidget_t *win = m_windowMain;
	m_windowMain = NULL;
	PtDestroyWidget(win);
}

void XAP_QNXDialog_Insert_Symbol::notifyActiveFrame(XAP_Frame *pFrame) {
	activate();
}

void XAP_QNXDialog_Insert_Symbol::notifyCloseFrame(XAP_Frame *pFrame) {
}

void XAP_QNXDialog_Insert_Symbol::runModeless(XAP_Frame * pFrame)
{
	unsigned short w, h;

	// First see if the dialog is already running
	UT_sint32 sid =(UT_sint32)  getDialogId();
	
	// Build the window's widgets and arrange them
	PtWidget_t * mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// Save dialog the ID number and pointer to the widget
	m_pApp->rememberModelessId( sid,  (XAP_Dialog_Modeless *) m_pDialog);

	//This magic command displays the frame that characters will be
	//inserted into.
	connectFocusModeless(mainWindow, m_pApp);

	// *** this is how we add the gc for symbol table ***
	// attach a new graphics context to the drawing area
	XAP_QNXApp * app;
	app = (XAP_QNXApp *) (m_pApp);
	UT_ASSERT(app);

	// make a new QNX GC
	DELETEP (m_qnxGraphics);
	//m_qnxGraphics = new GR_QNXGraphics(mainWindow, m_SymbolMap, m_pApp);
	GR_QNXAllocInfo ai(mainWindow, m_SymbolMap, m_pApp);
	m_qnxGraphics = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai);

	// let the widget materialize TODO: get a real size!
	UT_QNXGetWidgetArea(m_SymbolMap, NULL, NULL, &w, &h);
	_createSymbolFromGC(m_qnxGraphics, (UT_uint32) w, (UT_uint32) h);

	// make a new QNX GC
	DELETEP (m_qnxarea);

	//m_qnxarea = new GR_QNXGraphics(mainWindow, m_areaCurrentSym, m_pApp);
	GR_QNXAllocInfo ai_(mainWindow, m_areaCurrentSym, m_pApp);
	m_qnxarea = (GR_QNXGraphics*) XAP_App::getApp()->newGraphics(ai_);

	// let the widget materialize
	UT_QNXGetWidgetArea(m_areaCurrentSym, NULL, NULL, &w, &h);
	_createSymbolareaFromGC(m_qnxarea, (UT_uint32) w, (UT_uint32) h);

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);

		iDrawSymbol->setSelectedFont("Symbol");

	// Show the Previously selected symbol
	m_PreviousSymbol = m_CurrentSymbol;
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	PtRealizeWidget(mainWindow);
	PgFlush();
}

void XAP_QNXDialog_Insert_Symbol::event_OK(void)
{
	m_answer = XAP_Dialog_Insert_Symbol::a_OK;
	m_Inserted_Symbol = m_CurrentSymbol;
	_onInsertButton();
}

void XAP_QNXDialog_Insert_Symbol::event_Cancel(void)
{
	m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;

	destroy();	//Calls modeless cleanup and destroy for us
	done++;
}

void XAP_QNXDialog_Insert_Symbol::SymbolMap_exposed(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->draw();
	/* 
     Need this to see the blue square after an expose event???
	*/
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

void XAP_QNXDialog_Insert_Symbol::Symbolarea_exposed(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

//
// This function allows the symbol to be selected via the keyboard
//

void XAP_QNXDialog_Insert_Symbol::Key_Pressed(void * e)
{
	int move = 0;
	PhKeyEvent_t *ev=(PhKeyEvent_t* )PhGetData((PhEvent_t*)e);

if(!(ev->key_flags & Pk_KF_Sym_Valid)) return;

	switch (ev->key_sym)
	{
	case Pk_Up:
		move = -32;
		break;
	case Pk_Down:
		move = 32;
		break;
	case Pk_Left:
		move = -1;
		break;
	case Pk_Right:
		move = 1;
		break;
	}

	if (move != 0)
	{
		if ((m_CurrentSymbol + move) >= 32 && (m_CurrentSymbol + move) <= 255)
		{ 
			XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
			UT_ASSERT(iDrawSymbol);
			m_PreviousSymbol = m_CurrentSymbol;
			m_CurrentSymbol = m_CurrentSymbol + move;
			iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
		}
	}
}

void XAP_QNXDialog_Insert_Symbol::SymbolMap_clicked( PtCallbackInfo_t * e)
{
	PhPointerEvent_t *ptrevent;
	PhRect_t         *rect;
	UT_uint32 		 x, y;

	ptrevent = (PhPointerEvent_t *)PhGetData(e->event);

	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);

	rect = PhGetRects(e->event);
  	x = iDrawSymbol->getGraphics()->tlu(rect->ul.x);
  	y = iDrawSymbol->getGraphics()->tlu(rect->ul.y);


	m_PreviousSymbol = m_CurrentSymbol;
	m_CurrentSymbol = iDrawSymbol->calcSymbol(x, y);
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);

	/* Double clicking should also insert the symbol */
	if (ptrevent->click_count > 1) {
		event_OK();
	}
}

void XAP_QNXDialog_Insert_Symbol::CurrentSymbol_clicked( PtCallbackInfo_t * e)
{
	//Single clicks will insert the symbol
	event_OK();
}


void XAP_QNXDialog_Insert_Symbol::New_Font(void )
{
	XAP_Draw_Symbol * iDrawSymbol = _getCurrentSymbolMap();
	UT_ASSERT(iDrawSymbol);
	
/*
  Extract the new font string from the combo box, update the current symbol
  font and display the new set of symbols to choose from.

  The text extraction code was stolen from ev_GnomeQNXToolbar.
*/
	//TODO: Get the list from the combo box
	char **buffer;
	unsigned short *item;
	PtGetResource(m_fontcombo,Pt_ARG_ITEMS,&buffer,0);
	PtGetResource(m_fontcombo,Pt_ARG_CBOX_SEL_ITEM,&item,0);
	iDrawSymbol->setSelectedFont(buffer[*item-1]);
	iDrawSymbol->draw();
	iDrawSymbol->drawarea(m_CurrentSymbol, m_PreviousSymbol);
}

void XAP_QNXDialog_Insert_Symbol::event_WindowDelete(void)
{
	if (!done++) {
		m_answer = XAP_Dialog_Insert_Symbol::a_CANCEL;	

		destroy();
	}
}


/*****************************************************************/

PtWidget_t * XAP_QNXDialog_Insert_Symbol::_constructWindow(void)
{
	PtWidget_t * windowInsertS;

	PtWidget_t * vboxInsertS;

	PtWidget_t * fontcombo;
	PtWidget_t * SymbolMap;

	PtWidget_t * areaCurrentSym;

	PtWidget_t * hboxInsertS;
	PtWidget_t * buttonOK;
	PtWidget_t * buttonCancel;


	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtArg_t args[10];
	int     n;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE,m_WindowName, 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	PtSetArg(&args[n++],Pt_ARG_WIDTH,0,0);
	PtSetArg(&args[n++],Pt_ARG_HEIGHT,0,0);
	windowInsertS = PtCreateWidget(PtWindow, NULL, n, args);
	SetupContextHelp(windowInsertS,this);
	PtAddHotkeyHandler(windowInsertS,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	PtAddCallback(windowInsertS, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
	//Create a vertical group to contain the font selector, 
	// raw drawing area and then a horizontal group of buttons
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_HORZ_ALIGN, Pt_GROUP_HORZ_CENTER, 0);
 	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 10, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0); 
	PtSetArg(&args[n++], Pt_ARG_FLAGS,Pt_FALSE,Pt_GETS_FOCUS);
	vboxInsertS = PtCreateWidget(PtGroup, windowInsertS, n, args);

	// First put in a combo box so the user can select fonts
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS, 0, Pt_EDITABLE);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, 3 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS,Pt_FALSE,Pt_GETS_FOCUS);
	fontcombo = PtCreateWidget(PtComboBox, vboxInsertS, n, args);
	PtAddCallback(fontcombo, Pt_CB_SELECTION, s_new_font, this);
	FontDetails *font_list;
	int alloc = PfQueryFonts(PHFONT_ALL_SYMBOLS,PHFONT_SCALABLE, NULL, 0);

	if (!(font_list = (FontDetails *)g_try_malloc(alloc * sizeof(*font_list)))) {
		fprintf(stderr, "ERROR GETTING FONT LIST \n");
		return false;
	}
	memset(font_list, 0, alloc * sizeof(*font_list));

	PfQueryFonts(PHFONT_ALL_SYMBOLS,PHFONT_SCALABLE, font_list, alloc);
	for(int i=0;i<alloc;i++) {
	const char *sz = font_list[i].desc;
	PtListAddItems(fontcombo, &sz, 1, 0);
	}
	PtSetResource(fontcombo,Pt_ARG_CBOX_SEL_ITEM,1,0);
	g_free(font_list);
	// Then put the main symbol area in the center vertically 
	// *** Code Stolen from the preview widget ***
	{	
		// TODO: 32 * x (19) = 608, 7 * y (21) = 147  FIXME!
		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, 608, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, 147, 0);
		PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			Pt_GROUP_STRETCH_HORIZONTAL, Pt_GROUP_STRETCH_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_FLAGS,Pt_FALSE,Pt_GETS_FOCUS);
		PtWidget_t *symgroup = PtCreateWidget(PtGroup, vboxInsertS, n, args);

		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, 608, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, 147, 0);
		PtSetArg(&args[n++], Pt_ARG_POINTER, this, 0); 
		PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_sym_SymbolMap_exposed, 1); 
  	PtSetArg(&args[n++], Pt_ARG_FLAGS,Pt_TRUE,Pt_GETS_FOCUS);
		SymbolMap = PtCreateWidget(PtRaw, symgroup, n, args);
		PtAddEventHandler(SymbolMap, Ph_EV_BUT_PRESS/* | Ph_EV_BUT_RELEASE */, 
							s_SymbolMap_clicked, this);
		PtAddEventHandler(SymbolMap,Ph_EV_KEY,s_keypressed,this);
   	}
	
	// Then horizontally group the OK, Preview, Cancel widgets
	n = 0;
 	PtSetArg(&args[n++], Pt_ARG_GROUP_VERT_ALIGN, Pt_GROUP_VERT_CENTER, 0);
 	PtSetArg(&args[n++], Pt_ARG_GROUP_HORZ_ALIGN, Pt_GROUP_HORZ_CENTER, 0);
 	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 20, 0);
	hboxInsertS = PtCreateWidget(PtGroup, vboxInsertS, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonOK = PtCreateWidget(PtButton, hboxInsertS, n, args);
	localizeLabel(buttonOK,pSS,XAP_STRING_ID_DLG_OK);
	PtAddCallback(buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	// *** Code Stolen from the preview widget again! ***
	{
		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, 60, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, 45, 0);
		PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			Pt_GROUP_STRETCH_HORIZONTAL, Pt_GROUP_STRETCH_HORIZONTAL);
		PtWidget_t *symgroup = PtCreateWidget(PtGroup, hboxInsertS, n, args);

		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, 60, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, 45, 0);
		PtSetArg(&args[n++], Pt_ARG_POINTER, this, 0); 
		PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_Symbolarea_exposed, 1); 
		areaCurrentSym = PtCreateWidget(PtRaw, symgroup, n, args);
		PtAddEventHandler(areaCurrentSym, Ph_EV_BUT_PRESS /* | Ph_EV_BUT_RELEASE */, s_CurrentSymbol_clicked, this);
   	}

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonCancel = PtCreateWidget(PtButton, hboxInsertS, n, args);
	localizeLabel(buttonCancel,pSS,XAP_STRING_ID_DLG_Cancel);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

	// Update member variables with the important widgets that
	// might need to be queried or altered later.
	m_windowMain = windowInsertS;

	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;

	m_SymbolMap = 	SymbolMap;
	m_fontcombo = fontcombo;
	m_areaCurrentSym = areaCurrentSym;
	return windowInsertS;
}


