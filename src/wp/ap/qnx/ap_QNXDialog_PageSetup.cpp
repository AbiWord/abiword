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

#include "ut_assert.h"
#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"
#include "ap_Strings.h"

#include "ap_QNXDialog_PageSetup.h"
#include "ut_qnxHelper.h"
#include "ut_Xpm2Bitmap.h"

// include the 3 pixmaps that we use in this dialog
#include "orient-vertical.xpm"
#include "orient-horizontal.xpm"
#include "margin.xpm"

/*********************************************************************************/

// static helper functions
static bool label_with_pixmap(PtWidget_t *widget, const char **pIconData, UT_uint32 sizeofIconData)
{
	PhImage_t *pImage = NULL;

	if (UT_Xpm2Bitmap(pIconData, sizeofIconData, &pImage) == false) {
		return false;
	}

	if (!pImage) {
		return false;
	}

	PtSetResource(widget, Pt_ARG_DIM, &pImage->size, 0);
	PtSetResource(widget, Pt_ARG_LABEL_DATA, pImage, sizeof(*pImage));
	PtSetResource(widget, Pt_ARG_LABEL_TYPE, Pt_TEXT_IMAGE, 0);
//	PtSetArg(button, Pt_ARG_BALLOON_POSITION, Pt_BALLOON_BOTTOM, 0);

	return true;
}


/*********************************************************************************/

// static event callbacks

static int s_ok_clicked (PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_PageSetup *dlg = (AP_QNXDialog_PageSetup *)data;
	UT_ASSERT (dlg);
	dlg->event_OK ();
	return Pt_CONTINUE;
}

static int s_cancel_clicked (PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_PageSetup *dlg = (AP_QNXDialog_PageSetup *)data;
	UT_ASSERT (dlg);
	dlg->event_Cancel ();
	return Pt_CONTINUE;
}

static int s_delete_clicked (PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_PageSetup *dlg = (AP_QNXDialog_PageSetup *)data;
	UT_ASSERT (dlg);
	dlg->event_WindowDelete ();
	return Pt_CONTINUE;
}

static int s_page_size_changed (PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_PageSetup *dlg = (AP_QNXDialog_PageSetup *)data;
	UT_ASSERT(w && dlg);
	if (info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return Pt_CONTINUE;
	}
	dlg->event_PageSizeChanged ();
	return Pt_CONTINUE;
}

static int s_page_units_changed (PtWidget_t * w, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_PageSetup *dlg = (AP_QNXDialog_PageSetup *)data;
	UT_ASSERT(w && dlg);
	if (info->reason_subtype != Pt_LIST_SELECTION_FINAL) {
		return Pt_CONTINUE;
	}
	dlg->event_PageUnitsChanged ();
	return Pt_CONTINUE;
}

/*********************************************************************************/

void AP_QNXDialog_PageSetup::event_OK (void)
{

	int index, *flags;
	double *d;

	index = UT_QNXComboGetPos(m_optionPageSize);
	fp_PageSize pd = (fp_PageSize::Predefined)((int)m_vecsize.getNthItem(index - 1));
	
	if(pd.Width(DIM_IN) < 1.0 || pd.Height(DIM_IN) < 1.0)
	{
		if (!done++) {
			setAnswer (a_CANCEL);
		}
		return;
	}
	setPageSize (pd);

	setAnswer (a_OK);
	
	index = UT_QNXComboGetPos(m_optionMarginUnits);
	UT_Dimension mu = (UT_Dimension)((int)m_vecunits.getNthItem(index - 1));
	setMarginUnits (mu);

	index = UT_QNXComboGetPos(m_optionPageUnits);
	UT_Dimension pu = (UT_Dimension)((int)m_vecunits.getNthItem(index - 1));
	setPageUnits (pu);

	PtGetResource(m_radioPagePortrait, Pt_ARG_FLAGS, &flags, 0);
	setPageOrientation((*flags & Pt_SET) ? PORTRAIT : LANDSCAPE);

	PtGetResource(m_spinPageScale, Pt_ARG_NUMERIC_VALUE, &d, 0);
	setPageScale ((int)*d);

	PtGetResource(m_spinMarginTop, Pt_ARG_NUMERIC_VALUE, &d, 0);
	setMarginTop (*d);
	PtGetResource(m_spinMarginBottom, Pt_ARG_NUMERIC_VALUE, &d, 0);
	setMarginBottom (*d);
	PtGetResource(m_spinMarginLeft, Pt_ARG_NUMERIC_VALUE, &d, 0);
	setMarginLeft (*d);
	PtGetResource(m_spinMarginRight, Pt_ARG_NUMERIC_VALUE, &d, 0);
	setMarginRight (*d);
	PtGetResource(m_spinMarginHeader, Pt_ARG_NUMERIC_VALUE, &d, 0);
	setMarginHeader (*d);
	PtGetResource(m_spinMarginFooter, Pt_ARG_NUMERIC_VALUE, &d, 0);
	setMarginFooter (*d);

	done++;
}

void AP_QNXDialog_PageSetup::event_Cancel (void)
{
	if (!done++) {
		setAnswer (a_CANCEL);
	}
}

void AP_QNXDialog_PageSetup::event_WindowDelete (void)
{
	event_Cancel();
}

void AP_QNXDialog_PageSetup::event_PageUnitsChanged (void)
{
	int index;

	index = UT_QNXComboGetPos(m_optionPageUnits);
	UT_Dimension pu = (UT_Dimension)((int)m_vecunits.getNthItem(index - 1));
	setPageUnits (pu);

	index = UT_QNXComboGetPos(m_optionPageSize);
	fp_PageSize::Predefined pd = (fp_PageSize::Predefined)((int)m_vecsize.getNthItem(index - 1));
	fp_PageSize ps(pd);

  	double width, height;
  
	// get values
	width  = (double)ps.Width(pu);
	height = (double)ps.Height(pu);
  
	// set values
	PtSetResource(m_entryPageWidth, Pt_ARG_NUMERIC_VALUE, &width, 0);
	PtSetResource(m_entryPageHeight, Pt_ARG_NUMERIC_VALUE, &height, 0);
}

void AP_QNXDialog_PageSetup::event_PageSizeChanged (void)
{
	int index;

	index = UT_QNXComboGetPos(m_optionPageSize);
	fp_PageSize::Predefined pd = (fp_PageSize::Predefined)((int)m_vecsize.getNthItem(index - 1));
	fp_PageSize ps(pd);
	setPageSize (ps);

	index = UT_QNXComboGetPos(m_optionPageUnits);
	UT_Dimension pu = (UT_Dimension)((int)m_vecunits.getNthItem(index - 1));

	double w, h;

	// get values
	w = (double)ps.Width(pu);
	h = (double)ps.Height(pu);

	// set values
	PtSetResource(m_entryPageWidth, Pt_ARG_NUMERIC_VALUE, &w, 0);
	PtSetResource(m_entryPageHeight, Pt_ARG_NUMERIC_VALUE, &h, 0);
}

void AP_QNXDialog_PageSetup::event_MarginUnitsChanged (void)
{
	int index;
	UT_Dimension last_margin_unit;

	last_margin_unit = getMarginUnits();

	index = UT_QNXComboGetPos(m_optionMarginUnits);
	UT_Dimension mu = (UT_Dimension)((int)m_vecunits.getNthItem(index - 1));
	setMarginUnits (mu);


	double *d, top, bottom, left, right, header, footer;

	PtGetResource(m_spinMarginTop, Pt_ARG_NUMERIC_VALUE, &d, 0);
	top = *d;
	PtGetResource(m_spinMarginBottom, Pt_ARG_NUMERIC_VALUE, &d, 0);
	bottom = *d;
	PtGetResource(m_spinMarginLeft, Pt_ARG_NUMERIC_VALUE, &d, 0);
	left = *d;
	PtGetResource(m_spinMarginRight, Pt_ARG_NUMERIC_VALUE, &d, 0);
	right = *d;
	PtGetResource(m_spinMarginHeader, Pt_ARG_NUMERIC_VALUE, &d, 0);
	header = *d;
	PtGetResource(m_spinMarginFooter, Pt_ARG_NUMERIC_VALUE, &d, 0);
	footer = *d;
/* //XXX: FIND OUT WHAT THIS IS..
	CONVERT_DIMENSIONS (top,    last_margin_unit, mu);
	CONVERT_DIMENSIONS (bottom, last_margin_unit, mu);
	CONVERT_DIMENSIONS (left,   last_margin_unit, mu);
	CONVERT_DIMENSIONS (right,  last_margin_unit, mu);
	CONVERT_DIMENSIONS (header, last_margin_unit, mu);
	CONVERT_DIMENSIONS (footer, last_margin_unit, mu);
*/
	PtSetResource(m_spinMarginTop, Pt_ARG_NUMERIC_VALUE, &top, 0);
	PtSetResource(m_spinMarginBottom, Pt_ARG_NUMERIC_VALUE, &bottom, 0);
	PtSetResource(m_spinMarginLeft, Pt_ARG_NUMERIC_VALUE, &left, 0);
	PtSetResource(m_spinMarginRight, Pt_ARG_NUMERIC_VALUE, &right, 0);
	PtSetResource(m_spinMarginHeader, Pt_ARG_NUMERIC_VALUE, &header, 0);
	PtSetResource(m_spinMarginFooter, Pt_ARG_NUMERIC_VALUE, &footer, 0);
}

/*********************************************************************************/

XAP_Dialog *
AP_QNXDialog_PageSetup::static_constructor(XAP_DialogFactory * pFactory,
					    XAP_Dialog_Id id)
{
    AP_QNXDialog_PageSetup * p = new AP_QNXDialog_PageSetup(pFactory,id);
    return p;
}

AP_QNXDialog_PageSetup::AP_QNXDialog_PageSetup (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id) 
  : AP_Dialog_PageSetup (pDlgFactory, id)
{
  // nada
}

AP_QNXDialog_PageSetup::~AP_QNXDialog_PageSetup (void)
{
  // nada
}

void AP_QNXDialog_PageSetup::runModal (XAP_Frame *pFrame)
{
  	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow); 

    // Build the window's widgets and arrange them
    PtWidget_t * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

    connectFocus(mainWindow, pFrame);
		
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);
	PtRealizeWidget(mainWindow);

	int count;
	count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(MODAL_END_ARG(count));

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

PtWidget_t * AP_QNXDialog_PageSetup::_constructWindow (void)
{
#if 0
	PtArg_t	args[10];
	int		n;
	double  d, min, max;
	PhRect_t zero;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	PtWidget_t *hgroup;
	PtWidget_t *vgroup;
	PtWidget_t *vgroup2;

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, _(AP, DLG_PageSetup_Title), 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	m_window = PtCreateWidget(PtWindow, NULL, n, args);
	SetupContextHelp(m_window,this);
	PtAddHotkeyHandler(m_window,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);
	PtAddCallback(m_window,Pt_CB_WINDOW_CLOSING,s_delete_clicked,this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 10, 0);
	vgroup = PtCreateWidget(PtGroup, m_window, n, args);

#define _ANCHOR (Pt_LEFT_ANCHORED_LEFT | Pt_RIGHT_ANCHORED_RIGHT)
	memset(&zero, 0, sizeof(0));

	/*** Create the tabbed group ***/	
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtWidget_t *panelgroup = PtCreateWidget(PtPanelGroup, vgroup, n, args);

	/* Create the first tab (Page) */
	n = 0;
  PtSetArg(&args[n++], Pt_ARG_TITLE, _(AP, DLG_PageSetup_Page), 0);
	PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	PtWidget_t *panel = PtCreateWidget(PtPane, panelgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_TRUE, Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS,3,0);
	PtWidget_t *vpanel = PtCreateWidget(PtGroup, panel, n, args);

	/** Paper Section **/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION,Pt_GROUP_VERTICAL,0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS,3,0);
	hgroup = PtCreateWidget(PtGroup, vpanel, n, args);
	pretty_group(hgroup, _(AP, DLG_PageSetup_Paper));

	/* First column: padding, paper size */
	n = 0;
	PtCreateWidget(PtLabel, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Paper_Size), 0);
	PtCreateWidget(PtLabel, Pt_DEFAULT_PARENT, n, args);
	n=0;
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args);
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args); //Padding
	n=0;
	
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS,Pt_FALSE,Pt_EDITABLE);
	m_optionPageSize = PtCreateWidget(PtComboBox, Pt_DEFAULT_PARENT, n, args);
	for (int i = (int)fp_PageSize::_first_predefined_pagesize_;
		 i < (int)fp_PageSize::_last_predefined_pagesize_dont_use_; i++)
    {
		const char *itemname = fp_PageSize::PredefinedToName ((fp_PageSize::Predefined)i);
		PtListAddItems(m_optionPageSize, &itemname, 1, 0);
		m_vecsize.addItem((void *)i);
    }
	int current = (int) fp_PageSize::NameToPredefined (getPageSize ().getPredefinedName ());
	UT_QNXComboSetPos(m_optionPageSize, current + 1);
	PtAddCallback(m_optionPageSize, Pt_CB_SELECTION, s_page_size_changed, this);

	n=0;
	PtCreateWidget(PtLabel,Pt_DEFAULT_PARENT,n,args); //Padding

	/* Third column: Width, Height, Units */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Width), 0);
	PtCreateWidget(PtLabel, Pt_DEFAULT_PARENT, n, args);
	n = 0;

	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Height), 0);
	PtCreateWidget(PtLabel, Pt_DEFAULT_PARENT, n, args);
	n = 0;

	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Units), 0);
	PtCreateWidget(PtLabel, Pt_DEFAULT_PARENT, n, args);
	n = 0;

	d = getPageSize().Width (getPageUnits ());
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	min = 0;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MIN, &min, 0);
	max = 100;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MAX, &max, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH / 2, 0);
	m_entryPageWidth = PtCreateWidget(PtNumericFloat, Pt_DEFAULT_PARENT, n, args);

	n = 0;

	d = getPageSize().Height (getPageUnits ());
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	min = 0;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MIN, &min, 0);
	max = 100;
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MAX, &max, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH / 2, 0);
	m_entryPageHeight = PtCreateWidget(PtNumericFloat, Pt_DEFAULT_PARENT, n, args);

	n = 0;

	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS,Pt_FALSE,Pt_EDITABLE);
	m_optionPageUnits = PtCreateWidget(PtComboBox, Pt_DEFAULT_PARENT, n, args);
	{
		const char *itemname;
		itemname = 	_(XAP, DLG_Unit_inch);
		PtListAddItems(m_optionPageUnits, &itemname, 1, 0);
		m_vecunits.addItem((void *)DIM_IN);
		itemname = 	_(XAP, DLG_Unit_cm);
		PtListAddItems(m_optionPageUnits, &itemname, 1, 0);
		m_vecunits.addItem((void *)DIM_CM);
		itemname = 	_(XAP, DLG_Unit_mm);
		PtListAddItems(m_optionPageUnits, &itemname, 1, 0);
		m_vecunits.addItem((void *)DIM_MM);
	}
	PtAddCallback(m_optionPageUnits, Pt_CB_SELECTION, s_page_units_changed, this);
	UT_QNXComboSetPos(m_optionPageUnits, 1);
	
	/** Orientation Section **/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 2, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, 
			Pt_GROUP_EQUAL_SIZE_HORIZONTAL | Pt_GROUP_EXCLUSIVE, 
			Pt_GROUP_EQUAL_SIZE_HORIZONTAL | Pt_GROUP_EXCLUSIVE);
	hgroup = PtCreateWidget(PtGroup, vpanel, n, args);
	pretty_group(hgroup, _(AP, DLG_PageSetup_Orient));

	PtWidget_t *img;
	n = 0;
	img = PtCreateWidget(PtLabel, hgroup, n, args);
	label_with_pixmap(img, (const char **)&orient_vertical_xpm, sizeof(orient_vertical_xpm));
	n = 0;
	img = PtCreateWidget(PtLabel, hgroup, n, args);
	label_with_pixmap(img, (const char **)&orient_horizontal_xpm, sizeof(orient_horizontal_xpm));
	/* Put the radio buttons up */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Portrait), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, (getPageOrientation() == PORTRAIT) ? Pt_SET : 0, Pt_SET);
	m_radioPagePortrait = PtCreateWidget(PtToggleButton, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Landscape), 0);
	PtSetArg(&args[n++], Pt_ARG_INDICATOR_TYPE, Pt_TOGGLE_RADIO, 0);
	PtSetArg(&args[n++], Pt_ARG_FLAGS, (getPageOrientation() != PORTRAIT) ? Pt_SET : 0, Pt_SET);
	m_radioPageLandscape = PtCreateWidget(PtToggleButton, hgroup, n, args);

	/** Scale Section **/
	n = 0;
	PtSetArg(&args[n++],Pt_ARG_GROUP_ROWS_COLS,3,0);
	hgroup = PtCreateWidget(PtGroup, vpanel, n, args);
	pretty_group(hgroup, _(AP, DLG_PageSetup_Scale));
	n = 0;
	
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Adjust), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);
	n = 0;
	d = getPageScale();
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_PRECISION, 0, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_spinPageScale = PtCreateWidget(PtNumericFloat, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,_(AP, DLG_PageSetup_Percent), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);

	/*** Second Tab (Margin) ***/
	n = 0;
  PtSetArg(&args[n++], Pt_ARG_TITLE, _(AP, DLG_PageSetup_Margin), 0);
	PtSetArg(&args[n++], Pt_ARG_RESIZE_FLAGS,Pt_TRUE,Pt_RESIZE_XY_AS_REQUIRED);
	panel = PtCreateWidget(PtPane, panelgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ROWS_COLS, 3, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_HORIZONTAL, 
											 Pt_GROUP_EQUAL_SIZE_HORIZONTAL);
	hgroup = PtCreateWidget(PtGroup, panel, n, args);

	/** First Row: Units, Top, Header labels **/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Units), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Top), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Header), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);

	/** Second Row: Units combo, Top/Header spinners */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS,Pt_FALSE,Pt_EDITABLE);
	m_optionMarginUnits = PtCreateWidget(PtComboBox, hgroup, n, args);
	{
		const char *itemname;
		itemname = 	_(XAP, DLG_Unit_inch);
		PtListAddItems(m_optionMarginUnits, &itemname, 1, 0);
		itemname = 	_(XAP, DLG_Unit_cm);
		PtListAddItems(m_optionMarginUnits, &itemname, 1, 0);
		itemname = 	_(XAP, DLG_Unit_mm);
		PtListAddItems(m_optionMarginUnits, &itemname, 1, 0);
	}
	UT_QNXComboSetPos(m_optionMarginUnits, 1);
	n = 0;
	d = getMarginTop ();
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH / 2, 0);
	m_spinMarginTop = PtCreateWidget(PtNumericFloat, hgroup, n, args);
	n = 0;
	d = getMarginHeader ();
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH / 2, 0);
	m_spinMarginHeader = PtCreateWidget(PtNumericFloat, hgroup, n, args);

	/** Third Row: Left + Spinner, Image, Right + Spinner  **/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	vgroup2 = PtCreateWidget(PtGroup, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Left), 0);
	PtCreateWidget(PtLabel, vgroup2, n, args);
	n = 0;
	d = getMarginLeft ();
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH / 2, 0);
	m_spinMarginLeft = PtCreateWidget(PtNumericFloat, vgroup2, n, args);

	/* Preview */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, ABI_DEFAULT_BUTTON_WIDTH, 0);
	img = PtCreateWidget(PtLabel, hgroup, n, args);
    label_with_pixmap(img, (const char **)&margin_xpm, sizeof(margin_xpm));

	/* Right */
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	vgroup2 = PtCreateWidget(PtGroup, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Right), 0);
	PtCreateWidget(PtLabel, vgroup2, n, args);
	n = 0;
	d = getMarginRight ();
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH / 2, 0);
	m_spinMarginRight = PtCreateWidget(PtNumericFloat, vgroup2, n, args);

	/** Fourth Row: padding, Bottom, Footer **/
	n = 0;
	PtCreateWidget(PtLabel, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Bottom), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(AP, DLG_PageSetup_Footer), 0);
	PtCreateWidget(PtLabel, hgroup, n, args);


	/** Fifth Row: padding, Bottom, Footer spinners **/
	n = 0;
	PtCreateWidget(PtLabel, hgroup, n, args);
	n = 0;
	d = getMarginBottom ();
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH / 2, 0);
	m_spinMarginBottom = PtCreateWidget(PtNumericFloat, hgroup, n, args);
	n = 0;
	d = getMarginFooter ();
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_VALUE, &d, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH / 2, 0);
	m_spinMarginFooter = PtCreateWidget(PtNumericFloat, hgroup, n, args);

	/*** Create the bottom buttons ***/
	n = 0; 
	hgroup = PtCreateWidget(PtGroup, vgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(XAP, DLG_OK), 0);	
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_buttonOK = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(m_buttonOK, Pt_CB_ACTIVATE, s_ok_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, _(XAP, DLG_Cancel), 0);	
	PtSetArg(&args[n++], Pt_ARG_WIDTH, ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_buttonCancel = PtCreateWidget(PtButton, hgroup, n, args);
	PtAddCallback(m_buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);

  	return m_window;
#endif
}

  
