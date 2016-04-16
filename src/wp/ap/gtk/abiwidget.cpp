/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */

/* The AbiWord Widget 
 *
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001,2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
 * Copyright (C) 2007 Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2007 One Laptop Per Child
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

#include <string.h>
#include <glib/gi18n.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "abiwidget.h"
#include <gsf/gsf-input.h>
#include <gsf/gsf-output.h>
#include <gsf/gsf-input-memory.h>
#include <gsf/gsf-output-memory.h>
#include <gsf/gsf-utils.h>

#include "gr_CairoGraphics.h"
#include "gr_Painter.h"

#include "ev_EditMethod.h"
#include "ut_assert.h"
#include "fv_View.h"
#include "ap_UnixFrame.h"
#include "ap_FrameData.h"
#include "ap_FrameListener.h"
#include "pd_Document.h"
#include "ie_imp.h"
#include "ie_exp.h"
#include "ie_impGraphic.h"
#include "xap_UnixDialogHelper.h"
#include "ap_UnixFrameImpl.h"
#include "ap_UnixApp.h"
#include "ut_sleep.h"
#include "ut_std_string.h"
#include "fv_View.h"
#include "fg_Graphic.h"
#include "fl_DocLayout.h"
#include "ut_go_file.h"
#include "ut_timer.h"
#include "ev_Toolbar_Actions.h"
#include "ap_Toolbar_Id.h"
#include "px_CR_Span.h"
#include "gr_UnixCairoGraphics.h"
#include "gr_UnixImage.h"
#include "gr_DrawArgs.h"
#include "fp_Page.h"

/**************************************************************************/
/**************************************************************************/

/*
 * The AbiWord Widget
 *    by Martin Sevior and Dom Lachowicz
 *
 * Four score and 3 months ago, we decided that it would be cool to have
 * AbiWord exposed as a GTK+ widget. After inventing the computer and GTK+,
 * Dom and Martin undertook the task of exposing AbiWord's functionality
 * in freaky-fun GTKWidget form. And to this day, it has been a horrible
 * success.
 *
 * Not stymied by the hoopla and fanfare surrounding their earlier 
 * achievements, the two decided to actually make AbiWord's main frame
 * an instance of this AbiWidget.
 *
 */

/**************************************************************************/
/**************************************************************************/

class AbiWidget_FrameListener;
class AbiWidget_ViewListener;

// Our widget's private storage data
// UnixApp and UnixFrame already properly inherit from either
// Their GTK+ or GNOME XAP equivalents, so we don't need to worry
// About that here

struct _AbiPrivData
{
public:
	_AbiPrivData()
		: m_pDoc(NULL),
		m_pFrame(NULL),
		m_bMappedToScreen(false),
		m_bUnlinkFileAfterLoad(false),
		m_pFrameListener(NULL),
		m_pViewListener(NULL),
		m_bShowMargin(false),
		m_bWordSelections(false),
		m_iContentLength(0),
		m_iSelectionLength(0),
		m_sSearchText(new UT_UCS4String(""))
	{}

	PD_Document*				m_pDoc;
	AP_UnixFrame*				m_pFrame;
	bool						m_bMappedToScreen;
    bool						m_bUnlinkFileAfterLoad;
	AbiWidget_FrameListener*	m_pFrameListener;
	AbiWidget_ViewListener*		m_pViewListener;
	bool						m_bShowMargin;
	bool						m_bWordSelections;
	gint						m_iContentLength;
	gint						m_iSelectionLength;
	UT_UCS4String *				m_sSearchText;
};

/**************************************************************************/
/**************************************************************************/

//
// Our widget's arguments. 
//
enum {
	ARG_0,
	CURSOR_ON,
	UNLINK_AFTER_LOAD,
	VIEWPARA,
	VIEWPRINTLAYOUT,
	VIEWNORMALLAYOUT,
	VIEWWEBLAYOUT,
	CONTENT,
	SELECTION,
	CONTENT_LENGTH,
	SELECTION_LENGTH,
	SHADOW_TYPE,
	ARG_LAST
};

// our parent class
static GtkBinClass * parent_class = 0;

static gboolean s_abi_widget_map_cb(GObject * w, gpointer p);

/**************************************************************************/
/**************************************************************************/

#define GET_CLASS(instance) G_TYPE_INSTANCE_GET_CLASS (instance, ABI_WIDGET_TYPE, AbiWidgetClass)

// Here, we have some macros that:
// 1) Define the desired name for an EditMethod (EM_NAME)
// 2) Actually define a static function which maps directly 
//    onto an abi EditMethod (the rest)
//
// All that these functions do is marshall data into and out of
// the AbiWord application

#define EM_NAME(n) _abi_em_##n
#define PUBLIC_EM_NAME(n) abi_widget_##n

#define EM_CHARPTR_INT_INT__BOOL(n, p) \
static gboolean EM_NAME(n) (AbiWidget * w, const char * str, gint32 x, gint32 y) \
{ \
return abi_widget_invoke_ex (w, #n, str, x, y); \
} \
extern "C" gboolean PUBLIC_EM_NAME(p) (AbiWidget * w, const char * str, gint32 x, gint32 y) \
{ \
return GET_CLASS (w)->p (w, str, x, y); \
}

#define EM_VOID__BOOL(n, p) \
static gboolean EM_NAME(n) (AbiWidget * w)\
{ \
return abi_widget_invoke_ex (w, #n, 0, 0, 0); \
}\
extern "C" gboolean PUBLIC_EM_NAME(p) (AbiWidget * w)\
{ \
return GET_CLASS (w)->p (w); \
}

#define EM_INT_INT__BOOL(n, p) \
static gboolean EM_NAME(n) (AbiWidget * w, gint32 x, gint32 y) \
{ \
return abi_widget_invoke_ex (w, #n, 0, x, y); \
} \
extern "C" gboolean PUBLIC_EM_NAME(p) (AbiWidget * w, gint32 x, gint32 y) \
{ \
return GET_CLASS (w)->p (w, x, y); \
}

#define EM_CHARPTR__BOOL(n, p) \
static gboolean EM_NAME(n) (AbiWidget * w, const char * str) \
{ \
return abi_widget_invoke_ex (w, #n, str, 0, 0); \
} \
extern "C" gboolean PUBLIC_EM_NAME(p) (AbiWidget * w, const char * str) \
{ \
return GET_CLASS (w)->p (w, str); \
}

/**************************************************************************/
/**************************************************************************/

// custom gclosure marchallers

void
g_cclosure_user_marshal_VOID__INT_INT_INT (GClosure     *closure,
                                           GValue       * /*return_value*/,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      /*invocation_hint*/,
                                           gpointer      marshal_data)
{
	typedef void (*GMarshalFunc_VOID__INT_INT_INT) (gpointer     data1,
	                                                gint         arg_1,
	                                                gint         arg_2,
	                                                gint         arg_3,
	                                                gpointer     data2);
	register GMarshalFunc_VOID__INT_INT_INT callback;
	register GCClosure *cc = (GCClosure*) closure;
	register gpointer data1, data2;

	UT_return_if_fail (n_param_values == 4);

	if (G_CCLOSURE_SWAP_DATA (closure))
	{
		data1 = closure->data;
		data2 = g_value_peek_pointer (param_values + 0);
	}
	else
	{
	    data1 = g_value_peek_pointer (param_values + 0);
	    data2 = closure->data;
	}
	callback = (GMarshalFunc_VOID__INT_INT_INT) (marshal_data ? marshal_data : cc->callback);

	callback (data1,
	        g_marshal_value_peek_int (param_values + 1),
	        g_marshal_value_peek_int (param_values + 2),
	        g_marshal_value_peek_int (param_values + 3),
	        data2);
}



/**************************************************************************/
/**************************************************************************/

// Here we define our EditMethods which will later be mapped back onto
// Our AbiWidgetClass' member functions

EM_VOID__BOOL(alignCenter, align_center)
EM_VOID__BOOL(alignLeft, align_left)
EM_VOID__BOOL(alignRight, align_right)
EM_VOID__BOOL(alignJustify, align_justify)

EM_VOID__BOOL(copy, copy)
EM_VOID__BOOL(cut, cut)
EM_VOID__BOOL(paste, paste)
EM_VOID__BOOL(pasteSpecial, paste_special)
EM_VOID__BOOL(selectAll, select_all)
EM_VOID__BOOL(selectBlock, select_block)
EM_VOID__BOOL(selectLine, select_line)
EM_VOID__BOOL(selectWord, select_word)

EM_VOID__BOOL(fileSave, file_save)
EM_VOID__BOOL(saveImmediate, save_immediate)

EM_VOID__BOOL(undo, undo)
EM_VOID__BOOL(redo, redo)

EM_CHARPTR__BOOL(insertData, insert_data)
EM_VOID__BOOL(insertSpace, insert_space)

EM_VOID__BOOL(delBOB, delete_bob)
EM_VOID__BOOL(delBOD, delete_bod)
EM_VOID__BOOL(delBOL, delete_bol)
EM_VOID__BOOL(delBOW, delete_bow)
EM_VOID__BOOL(delEOB, delete_eob)
EM_VOID__BOOL(delEOD, delete_eod)
EM_VOID__BOOL(delEOL, delete_eol)
EM_VOID__BOOL(delEOW, delete_eow)
EM_VOID__BOOL(delLeft, delete_left)
EM_VOID__BOOL(delRight, delete_right)

EM_VOID__BOOL(editHeader, edit_header)
EM_VOID__BOOL(editFooter, edit_footer)
EM_VOID__BOOL(removeHeader, remove_header)
EM_VOID__BOOL(removeFooter, remove_footer)

EM_VOID__BOOL(extSelBOB, select_bob)
EM_VOID__BOOL(extSelBOD, select_bod)
EM_VOID__BOOL(extSelBOL, select_bol)
EM_VOID__BOOL(extSelBOW, select_bow)
EM_VOID__BOOL(extSelEOB, select_eob)
EM_VOID__BOOL(extSelEOD, select_eod)
EM_VOID__BOOL(extSelEOL, select_eol)
EM_VOID__BOOL(extSelEOW, select_eow)
EM_VOID__BOOL(extSelLeft, select_left)
EM_VOID__BOOL(extSelNextLine, select_next_line)
EM_VOID__BOOL(extSelPageDown, select_page_down)
EM_VOID__BOOL(extSelPageUp, select_page_up)
EM_VOID__BOOL(extSelPrevLine, select_prev_line)
EM_VOID__BOOL(extSelRight, select_right)
EM_VOID__BOOL(extSelScreenDown, select_screen_down)
EM_VOID__BOOL(extSelScreenUp, select_screen_up)
EM_INT_INT__BOOL(extSelToXY, select_to_xy)

EM_VOID__BOOL(toggleBold, toggle_bold)
EM_VOID__BOOL(toggleBottomline, toggle_bottomline)
EM_VOID__BOOL(toggleInsertMode, toggle_insert_mode)
EM_VOID__BOOL(toggleItalic, toggle_italic)
EM_VOID__BOOL(toggleOline, toggle_overline)
EM_VOID__BOOL(togglePlain, toggle_plain)
EM_VOID__BOOL(toggleStrike, toggle_strike)
EM_VOID__BOOL(toggleSub, toggle_sub)
EM_VOID__BOOL(toggleSuper, toggle_super)
EM_VOID__BOOL(toggleTopline, toggle_topline)
EM_VOID__BOOL(toggleUline, toggle_underline)
EM_VOID__BOOL(toggleUnIndent, toggle_unindent)
EM_VOID__BOOL(doBullets, toggle_bullets)
EM_VOID__BOOL(doNumbers, toggle_numbering)

EM_VOID__BOOL(viewPara, view_formatting_marks)
EM_VOID__BOOL(viewPrintLayout, view_print_layout)
EM_VOID__BOOL(viewNormalLayout, view_normal_layout)
EM_VOID__BOOL(viewWebLayout, view_online_layout)

EM_VOID__BOOL(warpInsPtBOB, moveto_bob)
EM_VOID__BOOL(warpInsPtBOD, moveto_bod)
EM_VOID__BOOL(warpInsPtBOL, moveto_bol)
EM_VOID__BOOL(warpInsPtBOP, moveto_bop)
EM_VOID__BOOL(warpInsPtBOW, moveto_bow)
EM_VOID__BOOL(warpInsPtEOB, moveto_eob)
EM_VOID__BOOL(warpInsPtEOD, moveto_eod)
EM_VOID__BOOL(warpInsPtEOL, moveto_eol)
EM_VOID__BOOL(warpInsPtEOP, moveto_eop)
EM_VOID__BOOL(warpInsPtEOW, moveto_eow)
EM_VOID__BOOL(warpInsPtLeft, moveto_left)
EM_VOID__BOOL(warpInsPtNextLine, moveto_next_line)
EM_VOID__BOOL(warpInsPtNextPage, moveto_next_page)
EM_VOID__BOOL(warpInsPtNextScreen, moveto_next_screen)
EM_VOID__BOOL(warpInsPtPrevLine, moveto_prev_line)
EM_VOID__BOOL(warpInsPtPrevPage, moveto_prev_page)
EM_VOID__BOOL(warpInsPtPrevScreen, moveto_prev_screen)
EM_VOID__BOOL(warpInsPtRight, moveto_right)
EM_INT_INT__BOOL(warpInsPtToXY, moveto_to_xy)

EM_VOID__BOOL(zoomWhole, zoom_whole)
EM_VOID__BOOL(zoomWidth, zoom_width)

/**************************************************************************/
/**************************************************************************/

// cleanup - keep EM_NAME around because it's actually useful elsewhere

#undef EM_VOID__BOOL
#undef EM_CHARPTR__BOOL
#undef EM_INT_INT__BOOL
#undef EM_CHARPTR_INT_INT__BOOL

/**************************************************************************/
/**************************************************************************/

static const guint32 ABI_DEFAULT_WIDTH  = 250 ;
static const guint32 ABI_DEFAULT_HEIGHT = 250 ;

/**************************************************************************/
/**************************************************************************/

#define INSTALL_VOID_SIGNAL(signal_offset, signal_name, signal_func) do { \
	abiwidget_signals[signal_offset] = \
		g_signal_new (signal_name, \
					  G_TYPE_FROM_CLASS (klazz), \
					  G_SIGNAL_RUN_LAST, \
					  G_STRUCT_OFFSET (AbiWidgetClass, signal_func), \
									  NULL, NULL, \
									  g_cclosure_marshal_VOID__VOID, \
									  G_TYPE_NONE, 0); } while(0)

#define INSTALL_BOOL_SIGNAL(signal_offset, signal_name, signal_func) do { \
	abiwidget_signals[signal_offset] = \
		g_signal_new (signal_name, \
					  G_TYPE_FROM_CLASS (klazz), \
					  G_SIGNAL_RUN_LAST, \
					  G_STRUCT_OFFSET (AbiWidgetClass, signal_func), \
									  NULL, NULL, \
									  g_cclosure_marshal_VOID__BOOLEAN, \
									  G_TYPE_NONE, 1, G_TYPE_BOOLEAN); } while(0)

#define INSTALL_INT_SIGNAL(signal_offset, signal_name, signal_func) do { \
	abiwidget_signals[signal_offset] = \
		g_signal_new (signal_name, \
					  G_TYPE_FROM_CLASS (klazz), \
					  G_SIGNAL_RUN_LAST, \
					  G_STRUCT_OFFSET (AbiWidgetClass, signal_func), \
									  NULL, NULL, \
									  g_cclosure_marshal_VOID__INT, \
									  G_TYPE_NONE, 1, G_TYPE_INT); } while(0)

#define INSTALL_DOUBLE_SIGNAL(signal_offset, signal_name, signal_func) do { \
	abiwidget_signals[signal_offset] = \
		g_signal_new (signal_name, \
					  G_TYPE_FROM_CLASS (klazz), \
					  G_SIGNAL_RUN_LAST, \
					  G_STRUCT_OFFSET (AbiWidgetClass, signal_func), \
									  NULL, NULL, \
									  g_cclosure_marshal_VOID__DOUBLE, \
									  G_TYPE_NONE, 1, G_TYPE_DOUBLE); } while(0)

#define INSTALL_STRING_SIGNAL(signal_offset, signal_name, signal_func) do { \
	abiwidget_signals[signal_offset] = \
		g_signal_new (signal_name, \
					  G_TYPE_FROM_CLASS (klazz), \
					  G_SIGNAL_RUN_LAST, \
					  G_STRUCT_OFFSET (AbiWidgetClass, signal_func), \
									  NULL, NULL, \
									  g_cclosure_marshal_VOID__STRING, \
									  G_TYPE_NONE, 1, G_TYPE_STRING); } while(0)

#define INSTALL_COLOR_SIGNAL(signal_offset, signal_name, signal_func) do { \
	abiwidget_signals[signal_offset] = \
		g_signal_new (signal_name, \
					  G_TYPE_FROM_CLASS (klazz), \
					  G_SIGNAL_RUN_LAST, \
					  G_STRUCT_OFFSET (AbiWidgetClass, signal_func), \
									  NULL, NULL, \
									  g_cclosure_user_marshal_VOID__INT_INT_INT, \
									  G_TYPE_NONE, 3, G_TYPE_INT, G_TYPE_INT, G_TYPE_INT); } while(0)

enum {
	SIGNAL_BOLD,
	SIGNAL_ITALIC,
	SIGNAL_UNDERLINE,
	SIGNAL_OVERLINE,
	SIGNAL_LINE_THROUGH,
	SIGNAL_TOPLINE,
	SIGNAL_BOTTOMLINE,
	SIGNAL_SUPERSCRIPT,
	SIGNAL_SUBSCRIPT,
	SIGNAL_COLOR,
	SIGNAL_CHANGED,
	SIGNAL_CAN_UNDO,
	SIGNAL_CAN_REDO,
	SIGNAL_FONT_SIZE,
	SIGNAL_FONT_FAMILY,
	SIGNAL_IS_DIRTY,
	SIGNAL_LEFT_ALIGN,
	SIGNAL_RIGHT_ALIGN,
	SIGNAL_CENTER_ALIGN,
	SIGNAL_JUSTIFY_ALIGN,
	SIGNAL_STYLE_NAME,
	SIGNAL_TEXT_SELECTED,
	SIGNAL_IMAGE_SELECTED,
	SIGNAL_SELECTION_CLEARED,
	SIGNAL_ENTER_SELECTION,
	SIGNAL_LEAVE_SELECTION,
	SIGNAL_TABLE_STATE,
	SIGNAL_PAGE_COUNT,
	SIGNAL_CURRENT_PAGE,
	SIGNAL_ZOOM_PERCENTAGE,
	SIGNAL_LAST
};

static guint abiwidget_signals[SIGNAL_LAST] = { 0 };

static void _abi_widget_class_install_signals (AbiWidgetClass * klazz)
{
	INSTALL_BOOL_SIGNAL(SIGNAL_BOLD, "bold", signal_bold);
	INSTALL_BOOL_SIGNAL(SIGNAL_ITALIC, "italic", signal_italic);
	INSTALL_BOOL_SIGNAL(SIGNAL_UNDERLINE, "underline", signal_underline);
	INSTALL_BOOL_SIGNAL(SIGNAL_OVERLINE, "overline", signal_overline);
	INSTALL_BOOL_SIGNAL(SIGNAL_LINE_THROUGH, "line-through", signal_line_through);
	INSTALL_BOOL_SIGNAL(SIGNAL_TOPLINE, "topline", signal_topline);
	INSTALL_BOOL_SIGNAL(SIGNAL_BOTTOMLINE, "bottomline", signal_bottomline);
	INSTALL_BOOL_SIGNAL(SIGNAL_SUPERSCRIPT, "superscript", signal_superscript);
	INSTALL_BOOL_SIGNAL(SIGNAL_SUBSCRIPT, "subscript", signal_subscript);
	INSTALL_COLOR_SIGNAL(SIGNAL_COLOR, "color", signal_color);
	INSTALL_VOID_SIGNAL(SIGNAL_CHANGED, "changed", signal_changed);
	INSTALL_BOOL_SIGNAL(SIGNAL_CAN_UNDO, "can-undo", signal_can_undo);
	INSTALL_BOOL_SIGNAL(SIGNAL_CAN_REDO, "can-redo", signal_can_redo);
	INSTALL_BOOL_SIGNAL(SIGNAL_IS_DIRTY, "is-dirty", signal_is_dirty);
	INSTALL_BOOL_SIGNAL(SIGNAL_LEFT_ALIGN, "left-align", signal_left_align);
	INSTALL_BOOL_SIGNAL(SIGNAL_RIGHT_ALIGN, "right-align", signal_right_align);
	INSTALL_BOOL_SIGNAL(SIGNAL_CENTER_ALIGN, "center-align", signal_center_align);
	INSTALL_BOOL_SIGNAL(SIGNAL_JUSTIFY_ALIGN, "justify-align", signal_justify_align);

	INSTALL_DOUBLE_SIGNAL(SIGNAL_FONT_SIZE, "font-size", signal_font_size);
	INSTALL_STRING_SIGNAL(SIGNAL_FONT_FAMILY, "font-family", signal_font_family);
	INSTALL_STRING_SIGNAL(SIGNAL_STYLE_NAME, "style-name", signal_style_name);

	INSTALL_BOOL_SIGNAL(SIGNAL_TEXT_SELECTED, "text-selected", signal_text_selected);
	INSTALL_BOOL_SIGNAL(SIGNAL_IMAGE_SELECTED, "image-selected", signal_image_selected);
	INSTALL_BOOL_SIGNAL(SIGNAL_SELECTION_CLEARED, "selection-cleared", signal_selection_cleared);
	INSTALL_BOOL_SIGNAL(SIGNAL_ENTER_SELECTION, "enter-selection", signal_enter_selection);
	INSTALL_BOOL_SIGNAL(SIGNAL_LEAVE_SELECTION, "leave-selection", signal_leave_selection);

	INSTALL_BOOL_SIGNAL(SIGNAL_TABLE_STATE, "table-state", signal_table_state);

	INSTALL_INT_SIGNAL(SIGNAL_PAGE_COUNT, "page-count", signal_page_count); // should be UINT32, but we really don't care atm
	INSTALL_INT_SIGNAL(SIGNAL_CURRENT_PAGE, "current-page", signal_current_page); // should be UINT32, but we really don't care atm

	INSTALL_INT_SIGNAL(SIGNAL_ZOOM_PERCENTAGE, "zoom", signal_zoom_percentage);
}

#define FIRE_BOOL(query, var, fire) do { bool val = (query); if (val != var) { var = val; fire(val); } } while(0)
#define FIRE_SINT32(query, var, fire) do { UT_sint32 val = (query); if (val != var) { var = val; fire(val); } } while(0)
#define FIRE_UINT32(query, var, fire) do { UT_uint32 val = (query); if (val != var) { var = val; fire(val); } } while(0)
#define FIRE_UTF8STRING(query, var, fire) do { const UT_UTF8String& val = (query); if (val != var) { var = val; fire(val.utf8_str()); } } while(0)

#define FIRE_BOOL_CHARFMT(prop, prop_val, multiple, var, fire) do {\
std::string sz = PP_getAttribute(prop, props_in);                \
if (!sz.empty())                                                 \
{ bool val; \
if (multiple) \
val = (NULL != strstr(sz.c_str(), prop_val));   \
else \
val = (sz == prop_val);                      \
if (val != var) { \
var = val; \
fire(var); \
} \
} \
} while(0)

#define FIRE_DOUBLE_CHARFMT(prop, var, fire) do { std::string sz = PP_getAttribute(prop, props_in); if (!sz.empty()) { double val = g_ascii_strtod(sz.c_str(), NULL); if (val != var) { var = val; fire(val); } } } while(0)

#define FIRE_STRING_CHARFMT(prop, var, fire) do { std::string sz = PP_getAttribute(prop, props_in); if (!sz.empty()) { if (strcmp(var.utf8_str(), sz.c_str()) != 0) { var = sz; fire(sz.c_str()); } } } while(0)

#define FIRE_COLOR_CHARFMT(prop, var, fire) do { std::string sz = PP_getAttribute(prop, props_in); if (!sz.empty()) { UT_RGBColor val(0,0,0); UT_parseColor(sz.c_str(), val); if (val != var) { var = val; fire(val); } } } while(0)

#define TOOLBAR_DELAY 1000 /* in milliseconds */

class Stateful_ViewListener : public AV_Listener
{
public:
	Stateful_ViewListener(AV_View * pView)
		: m_pView(static_cast<FV_View *>(pView)),
		m_lid((AV_ListenerId)-1)
	{
		init();
		
		AV_ListenerId lid;
		if (pView->addListener(this, &lid))
		{
			m_lid = lid;
		}
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	virtual ~Stateful_ViewListener(void)
	{
		// re-enable this when we are also properly notified if the view is killed under us - MARCM
		// unbind();
	}

	void unbind(void)
	{
		if (m_lid != (AV_ListenerId)-1)
			m_pView->removeListener(m_lid);

		m_lid = (AV_ListenerId)-1;
	}

	virtual bool notify(AV_View * pView, const AV_ChangeMask mask)
	{
		UT_return_val_if_fail(pView == m_pView, false);

		if ((AV_CHG_FMTCHAR | AV_CHG_MOTION) & mask)
		{
			// get current char properties from pView
			PP_PropertyVector props_in;

			if (!m_pView->getCharFormat(props_in))
				return true;

			// NB: maybe *no* properties are consistent across the selection
			if (!props_in.empty())
			{
				FIRE_BOOL_CHARFMT("font-weight", "bold", false, bold_, bold);
				FIRE_BOOL_CHARFMT("font-style", "italic", false, italic_, italic);
				FIRE_BOOL_CHARFMT("text-decoration", "underline", true, underline_, underline);
				FIRE_BOOL_CHARFMT("text-decoration", "overline", true, overline_, overline);
				FIRE_BOOL_CHARFMT("text-decoration", "line-through", true, line_through_, line_through);
				FIRE_BOOL_CHARFMT("text-decoration", "topline", true, topline_, topline);
				FIRE_BOOL_CHARFMT("text-decoration", "bottomline", true, bottomline_, bottomline);
				FIRE_BOOL_CHARFMT("text-position", "superscript", true, superscript_, superscript);
				FIRE_BOOL_CHARFMT("text-position", "subscript", true, subscript_, subscript);
				FIRE_COLOR_CHARFMT("color", color_, color);

				FIRE_DOUBLE_CHARFMT("font-size", font_size_, font_size);
				FIRE_STRING_CHARFMT("font-family", font_family_, font_family);
			}
		}

		if ((AV_CHG_FMTSTYLE | AV_CHG_MOTION) & mask)
		{
			// check the current style
			const gchar * szStyle = NULL;
			m_pView->getStyle(&szStyle);
			if (szStyle == NULL)
				szStyle = "None";
			FIRE_UTF8STRING(szStyle, style_name_, styleName);				
		}

		if ((AV_CHG_MOTION | AV_CHG_PAGECOUNT) & mask)
		{
			UT_uint32 _page_count = m_pView->getLayout()->countPages();
			UT_uint32 _current_page = m_pView->getCurrentPageNumForStatusBar();

			FIRE_UINT32(_page_count, pageCount_, pageCount);
			FIRE_UINT32(_current_page, currentPage_, currentPage);
		}

		if ((AV_CHG_FMTBLOCK | AV_CHG_MOTION) & mask)
		{
			// get current char properties from pView
			PP_PropertyVector props_in;

			if (!m_pView->getBlockFormat(props_in))
				return true;

			// NB: maybe *no* properties are consistent across the selection
			if (!props_in.empty())
			{
				FIRE_BOOL_CHARFMT("text-align", "left", false, leftAlign_, leftAlign);
				FIRE_BOOL_CHARFMT("text-align", "right", false, rightAlign_, rightAlign);
				FIRE_BOOL_CHARFMT("text-align", "center", false, centerAlign_, centerAlign);
				FIRE_BOOL_CHARFMT("text-align", "justify", false, justifyAlign_, justifyAlign);
			}
		}

		if ((AV_CHG_FMTBLOCK | AV_CHG_MOTION) & mask)
		{
			// check if we are in a table or not
			bool b = m_pView->isInTable();
			FIRE_BOOL(b,tableState_,tableState);
		}

		if ((AV_CHG_ALL) & mask)
		{
			// check if we need to fire a 'changed' signal
			if (m_pView && m_pView->getDocument() && 
				m_pView->getDocument()->getPieceTable())
			{
				pt_PieceTable* pPt = m_pView->getDocument()->getPieceTable();
				PX_ChangeRecord* pcr = NULL;
				pPt->getUndo(&pcr);

				// We're trying to be smart here: if the top-most changerecord on the undo stack changed, 
				// then the structure of the document must have changed. Determining if a top-most changerecord
				// has changed can be tricky unfortunately: new changerecords can be coalesced into the top-most
				// one if they share the same type and properties (in the case of insert or delete spans).
				// Therefor, when we are dealing with insert and delete spans, we not only check the actual 
				// changerecord pointer, but also its length, buffer index and block offset. That should cover
				// all cases I think/hope - MARCM
				if (pcr_ != pcr)
				{
					pcr_ = pcr;
					if (pcr_)
					{
						PX_ChangeRecord_Span * pcrs_ = static_cast<PX_ChangeRecord_Span *>(pcr_);
						pcrlen_ = pcrs_->getLength();
						pcrpos_ = pcrs_->getPosition();
						pcrbi_ = pcrs_->getBufIndex();
						pcrbo_ = pcrs_->getBlockOffset();
					}	
					changed();
				}
				else if (pcr_ == pcr && pcr_ && pcr)
				{
					// some additional checks to detect coalesced changerecords
					if (pcr_->getType() == PX_ChangeRecord::PXT_InsertSpan || 
						pcr_->getType() == PX_ChangeRecord::PXT_DeleteSpan )
					{
						// we could have been coalesced! check some additional properties
						// to see if we are
						PX_ChangeRecord_Span * pcrs_ = static_cast<PX_ChangeRecord_Span *>(pcr_);
						if (pcrlen_ !=  pcrs_->getLength() ||
							pcrpos_ != pcrs_->getPosition() ||
							pcrbi_ != pcrs_->getBufIndex() ||
							pcrbo_ != pcrs_->getBlockOffset())
						{
							// yep, we are coalesced, so emit a 'changed' signal after all
							pcrlen_ = pcrs_->getLength();
							pcrpos_ = pcrs_->getPosition();
							pcrbi_ = pcrs_->getBufIndex();
							pcrbo_ = pcrs_->getBlockOffset();
							changed();
						}
					}
				}
			}
		}
		
		if ((AV_CHG_ALL) & mask)
		{
			FIRE_BOOL(m_pView->canDo(true), can_undo_, can_undo);
			FIRE_BOOL(m_pView->canDo(false), can_redo_, can_redo);
			FIRE_BOOL(m_pView->getDocument()->isDirty(), is_dirty_, is_dirty);
			
			XAP_Frame* pFrame = XAP_App::getApp()->getLastFocussedFrame();
			UT_return_val_if_fail(pFrame, false);
			FIRE_SINT32(pFrame->getZoomPercentage(), zoomPercentage_, zoomPercentage); // surely there is a better signal for this than AV_CHG_ALL
		}

		if (mask & AV_CHG_EMPTYSEL)
		{
			// The selection changed; now figure out if we do have a selection or not
			if (m_pView)
			{
				if (m_pView->isSelectionEmpty())
				{
					
					if (textSelected_ || imageSelected_)
					{
						FIRE_BOOL(true, selectionCleared_, selectionCleared);
						textSelected_ = false;
						imageSelected_ = false;
					}
				}
				else 
				{
					if(m_pView->getLastMouseContext() == EV_EMC_IMAGE)
					{
						FIRE_BOOL(true, imageSelected_, imageSelected);
						selectionCleared_ = false;
					}
					else
					{
						FIRE_BOOL(true, textSelected_, textSelected);
						selectionCleared_ = false;
					}
			        PT_DocPosition pos = m_pView->getDocPositionFromLastXY();
					PT_DocPosition left = m_pView->getSelectionLeftAnchor();
					PT_DocPosition right = m_pView->getSelectionRightAnchor();
					bool bBetween = ((pos >= left) && (pos < right));
					if(!enterSelection_ && bBetween)
					{
						FIRE_BOOL(true, enterSelection_, enterSelection);
						leaveSelection_ = false;
					}
					if(!leaveSelection_ && !bBetween)
					{
						FIRE_BOOL(true, leaveSelection_, leaveSelection);
						enterSelection_ = false;
					}

				}
			}
		}
	
		return true;
	}

    virtual AV_ListenerType getType(void) 
	{ 
		// i don't feel like creating a new type if i don't have to. this is semantically
		// similar enough that i don't care
		return AV_LISTENER_PLUGIN;
	}

	virtual void bold(bool /*value*/) {}
	virtual void italic(bool /*value*/) {}
	virtual void underline(bool /*value*/) {}
	virtual void overline(bool /*value*/) {}
	virtual void line_through(bool /*value*/) {}
	virtual void topline(bool /*value*/) {}
	virtual void bottomline(bool /*value*/) {}
	virtual void subscript(bool /*value*/) {}
	virtual void superscript(bool /*value*/) {}
	virtual void color(UT_RGBColor /*value*/) {}
	virtual void font_size(double /*value*/) {}
	virtual void font_family(const char * /*value*/) {}
	virtual void changed(void) {}
	virtual void can_undo(bool /*value*/) {}
	virtual void can_redo(bool /*value*/) {}
	virtual void is_dirty(bool /*value*/) {}
	virtual void leftAlign(bool /*value*/) {}
	virtual void rightAlign(bool /*value*/) {}
	virtual void centerAlign(bool /*value*/) {}
	virtual void justifyAlign(bool /*value*/) {}
	virtual void styleName(const char * /*value*/) {}
	virtual void textSelected(bool /*value*/) {}
	virtual void imageSelected(bool /*value*/) {}
	virtual void selectionCleared(bool /*value*/) {}
	virtual void enterSelection(bool /*value*/) {}
	virtual void leaveSelection(bool /*value*/) {}
	virtual void tableState(bool /*value*/) {}
	virtual void pageCount(guint32 /*value*/) {}
	virtual void currentPage(guint32 /*value*/) {}
	virtual void zoomPercentage(gint32 /*value*/) {}

private:

	void init()
	{
		bold_ = false;
		italic_ = false;
		underline_ = false;
		overline_ = false;
		line_through_ = false;
		topline_ = false;
		bottomline_ = false;
		subscript_ = false;
		superscript_ = false;
		color_ = UT_RGBColor(0,0,0);
		font_size_ = 0.;
		font_family_ = "";
		//
		// set these to true so that after the initial load of a file
		// singals will be emitted to turn the toolbar and menu item 
		// insensitive
		//
		pcr_ = NULL; // the pcr* values are needed to emit a sane 'changed' signal
		pcrlen_ = 0;
		pcrpos_ = 0;
		pcrbi_ = 0;
		pcrbo_ = 0;
		can_undo_ = true;
		can_redo_ = true;
		is_dirty_ = true;
		leftAlign_ = false;
		rightAlign_ = false;
		centerAlign_ = false;
		justifyAlign_ = false;
		style_name_ = "";
		textSelected_ = false;
		imageSelected_ = true;
		selectionCleared_ = false;
		enterSelection_ = false;
		leaveSelection_ = false;
		tableState_ = true;
		pageCount_ = 0;
		currentPage_ = 0;
		zoomPercentage_ = 0;
	}
	bool bold_;
	bool italic_;
	bool underline_;
	bool overline_;
	bool line_through_;
	bool topline_;
	bool bottomline_;
	bool subscript_;
	bool superscript_;
	UT_RGBColor color_;
	double font_size_;
	UT_UTF8String font_family_;
	PX_ChangeRecord* pcr_;
	UT_uint32 pcrlen_;
	PT_DocPosition pcrpos_;
	PT_BufIndex pcrbi_;
	PT_BlockOffset pcrbo_;
	bool can_undo_;
	bool can_redo_;
	bool is_dirty_;
	bool leftAlign_;
	bool rightAlign_;
	bool centerAlign_;
	bool justifyAlign_;
	UT_UTF8String style_name_;
	bool textSelected_;
	bool imageSelected_;
	bool selectionCleared_;
	bool enterSelection_;
	bool leaveSelection_;
	bool tableState_;
	guint32 pageCount_;
	guint32 currentPage_;
	gint32 zoomPercentage_;

	FV_View *			m_pView;
	AV_ListenerId       m_lid;
};



class AbiWidget_ViewListener : public Stateful_ViewListener
{
public:
	AbiWidget_ViewListener(AbiWidget * pWidget,
						   AV_View * pView)
		: Stateful_ViewListener(pView), m_pWidget(pWidget)
	{
	}
	virtual void bold(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_BOLD], 0, (gboolean)value);}
	virtual void italic(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_ITALIC], 0, (gboolean)value);}
	virtual void underline(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_UNDERLINE], 0, (gboolean)value);}
	virtual void overline(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_OVERLINE], 0, (gboolean)value);}
	virtual void line_through(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_LINE_THROUGH], 0, (gboolean)value);}
	virtual void topline(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_TOPLINE], 0, (gboolean)value);}
	virtual void bottomline(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_BOTTOMLINE], 0, (gboolean)value);}
	virtual void subscript(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_SUBSCRIPT], 0, (gboolean)value);}
	virtual void superscript(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_SUPERSCRIPT], 0, (gboolean)value);}
	virtual void color(UT_RGBColor value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_COLOR], 0, (int)value.m_red, (int)value.m_grn, (int)value.m_blu);}
	virtual void font_size(double value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_FONT_SIZE], 0, value);}
	virtual void font_family(const char * value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_FONT_FAMILY], 0, value);}
	virtual void changed(void) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_CHANGED], 0);}
	virtual void can_undo(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_CAN_UNDO], 0, (gboolean)value);}
	virtual void can_redo(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_CAN_REDO], 0, (gboolean)value);}
	virtual void is_dirty(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_IS_DIRTY], 0, (gboolean)value);}
	virtual void leftAlign(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_LEFT_ALIGN], 0, (gboolean)value);}
	virtual void rightAlign(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_RIGHT_ALIGN], 0, (gboolean)value);}
	virtual void centerAlign(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_CENTER_ALIGN], 0, (gboolean)value);}
	virtual void justifyAlign(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_JUSTIFY_ALIGN], 0, (gboolean)value);}
	virtual void styleName(const char * value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_STYLE_NAME], 0, value);}
	virtual void textSelected(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_TEXT_SELECTED], 0, (gboolean)value);}
	virtual void imageSelected(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_IMAGE_SELECTED], 0, (gboolean)value);}
	virtual void selectionCleared(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_SELECTION_CLEARED], 0, (gboolean)value);}
	virtual void enterSelection(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_ENTER_SELECTION], 0, (gboolean)value);}
	virtual void leaveSelection(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_LEAVE_SELECTION], 0, (gboolean)value);}
	virtual void tableState(bool value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_TABLE_STATE], 0, (gboolean)value);}
	virtual void pageCount(guint32 value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_PAGE_COUNT], 0, (guint32)value);}
	virtual void currentPage(guint32 value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_CURRENT_PAGE], 0, (guint32)value);}
	virtual void zoomPercentage(gint32 value) {g_signal_emit (G_OBJECT(m_pWidget), abiwidget_signals[SIGNAL_ZOOM_PERCENTAGE], 0, (gint32)value);}

private:
	AbiWidget *         m_pWidget;
};

static void _abi_widget_unbindListener(AbiWidget *widget)
{
	// Unbind the listener from the view
	AbiPrivData * private_data = (AbiPrivData *)widget->priv;
	AbiWidget_ViewListener * pListener = private_data->m_pViewListener;
	if (!pListener)
		return;

	pListener->unbind();
}

static void _abi_widget_releaseListener(AbiWidget *widget)
{
	// remove a FV_View listener from the widget. see _abi_widget_bindListenerToView() for more details.
	AbiPrivData * private_data = (AbiPrivData *)widget->priv;
	if (!private_data->m_pViewListener)
		return;
	
	DELETEP(private_data->m_pViewListener);
	private_data->m_pViewListener = 0;
}

static bool _abi_widget_bindListenerToView(AbiWidget *widget, AV_View * pView)
{
	UT_return_val_if_fail(pView, false);
	
	// hook up a FV_View listener to the widget. This will let the widget
	// fire GObject signals when things in the view change
	AbiPrivData * private_data = (AbiPrivData *)widget->priv;
	_abi_widget_releaseListener(widget);
	
	private_data->m_pViewListener = new AbiWidget_ViewListener(widget, pView);
	UT_return_val_if_fail(private_data->m_pViewListener, false);

	// notify the listener that a new view has been bound
	private_data->m_pViewListener->notify(pView, AV_CHG_ALL);

	return true;
}

class AbiWidget_FrameListener : public AP_FrameListener
{
public:
	AbiWidget_FrameListener(AbiWidget * pWidget)
		: m_pWidget(pWidget),
		m_iListenerId(-1)
	{
		if (pWidget && pWidget->priv && pWidget->priv->m_pFrame)
		{
			UT_DEBUGMSG(("Registering AbiWidget_FrameListener listener\n"));
			m_iListenerId = pWidget->priv->m_pFrame->registerListener(this);
		}
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}
	
	~AbiWidget_FrameListener()
	{
		
		// TODO: unregister
	}
	
	virtual void signalFrame(AP_FrameSignal signal)
	{
		switch (signal)
		{
			case APF_ReplaceDocument:
				UT_DEBUGMSG(("Replace document signalled\n"));
				//
				// Not sure what got broken for us to need this.
				// Views should be deleted before their documents
				// Nonetheless this fixes bug 11421  crash in sugar
				// when changing document with a shared session.
				//
				//
				if (m_pWidget->priv->m_pFrame->getCurrentView())
				{
					FV_View * pView = static_cast<FV_View *>(m_pWidget->priv->m_pFrame->getCurrentView());
					m_pWidget->priv->m_pDoc = pView->getDocument();
				}
 
				break;
			case APF_ReplaceView:
				UT_DEBUGMSG(("Replace view signalled\n"));
				if (m_pWidget->priv->m_pFrame->getCurrentView() && m_pWidget->priv->m_bMappedToScreen)
					_abi_widget_bindListenerToView(m_pWidget, m_pWidget->priv->m_pFrame->getCurrentView());
				break;
		}
	}
	
private:
	AbiWidget *         m_pWidget;
	UT_sint32			m_iListenerId;
};

/**************************************************************************/
/**************************************************************************/

//
static bool      s_bFirstDrawDone = false;
static bool      s_bFreshDraw = false;
static UT_Timer * s_pToUpdateCursor = NULL;
static XAP_Frame * s_pLoadingFrame = NULL;
static AD_Document * s_pLoadingDoc = NULL;
static UT_sint32 s_iLastYScrollOffset = -1;
static UT_sint32 s_iLastXScrollOffset = -1;

static void s_LoadingCursorCallback(UT_Worker * pTimer )
{
	UT_ASSERT(pTimer);
	xxx_UT_DEBUGMSG(("Update Screen on load Frame %x \n",s_pLoadingFrame));
	XAP_Frame * pFrame = s_pLoadingFrame;
	UT_uint32 iPageCount = 0;
	
	if(pFrame == NULL)
	{
		s_bFirstDrawDone = false;
		return;
	}
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	if(pView)
	{
		GR_Graphics * pG = pView->getGraphics();
		if(pG)
		{
			pG->setCursor(GR_Graphics::GR_CURSOR_WAIT);
		}
		FL_DocLayout * pLayout = pView->getLayout();
		if(pView->getPoint() > 0)
		{
			pLayout->updateLayout();
			iPageCount = pLayout->countPages();

			if(!s_bFirstDrawDone && iPageCount > 1)
			{
				pView->draw();
				s_bFirstDrawDone = true;
			}
			else
			{
				// we only want to draw if we need to:
				//   (1) if the scroller position has changed
				//   (2) if the previous draw was due to a scroll change
				//
				// This way each change of scroller position will
				// result in two draws, the second of which will
				// ensure that anything from the current vieport that
				// was not yet laid out when the first draw was made
				// is drawn
				if(iPageCount > 1)
				{
					if(pView->getYScrollOffset() != s_iLastYScrollOffset ||
					   pView->getXScrollOffset() != s_iLastXScrollOffset)
					{
						pView->updateScreen(true);
						s_iLastYScrollOffset = pView->getYScrollOffset();
						s_iLastXScrollOffset = pView->getXScrollOffset();
						s_bFreshDraw = true;
						xxx_UT_DEBUGMSG(("Incr. loader: primary draw\n"));
					}
					else if(s_bFreshDraw)
					{
						pView->updateScreen(true);
						s_bFreshDraw = false;
						xxx_UT_DEBUGMSG(("Incr. loader: secondary draw\n"));
					}
					else
					{
						xxx_UT_DEBUGMSG(("Incr. loader: draw not needed\n"));
					}
				}
			
			}

			if(iPageCount > 1)
			{
				UT_String msg = pSS->getValue(XAP_STRING_ID_MSG_BuildingDoc);
				pFrame->setStatusMessage ( static_cast<const gchar *>(msg.c_str()) );
			}
			else
			{
				UT_String msg =  pSS->getValue(XAP_STRING_ID_MSG_ImportingDoc);
				pFrame->setStatusMessage ( static_cast<const gchar *>(msg.c_str()) );
			}
		}
		else
		{
			UT_String msg =  pSS->getValue(XAP_STRING_ID_MSG_ImportingDoc);
			pFrame->setStatusMessage ( static_cast<const gchar *>(msg.c_str()) );
		}
	}
	else
	{
		UT_String msg =  pSS->getValue(XAP_STRING_ID_MSG_ImportingDoc);
		pFrame->setStatusMessage ( static_cast<const gchar *>(msg.c_str()) );
		s_bFirstDrawDone = false;
	}
}

/*!
 * Control Method for the updating loader.
\param bool bStartStop true to start the updating loader, flase to stop it
             after the document has loaded.
\param XAP_Frame * pFrame Pointer to the new frame being loaded.
*/
static void s_StartStopLoadingCursor( bool bStartStop, XAP_Frame * pFrame)
{
	// Now construct the timer for auto-updating
	if(bStartStop)
	{
//
// Can't have multiple loading document yet. Need Vectors of loading frames
// and auto-updaters. Do this later.
//
		if(s_pLoadingFrame != NULL)
		{
			return;
		}
		s_pLoadingFrame = pFrame;
		s_pLoadingDoc = pFrame->getCurrentDoc();
		if(s_pToUpdateCursor == NULL)
		{
			s_pToUpdateCursor = UT_Timer::static_constructor(s_LoadingCursorCallback,NULL);
		}
		s_bFirstDrawDone = false;
		s_pToUpdateCursor->set(1000);
		s_pToUpdateCursor->start();
//		s_pLoadingFrame = XAP_App::getApp()->getLastFocussedFrame();
	}
	else
	{
		if(s_pToUpdateCursor != NULL)
		{
			s_pToUpdateCursor->stop();
			DELETEP(s_pToUpdateCursor);
			s_pToUpdateCursor = NULL;
			if(s_pLoadingFrame != NULL)
			{
				s_pLoadingFrame->setCursor(GR_Graphics::GR_CURSOR_DEFAULT);
				FV_View * pView = static_cast<FV_View *>(s_pLoadingFrame->getCurrentView());
				if(pView)
				{
					pView->setCursorToContext();
					pView->focusChange(AV_FOCUS_HERE);
				}
			}
			s_pLoadingFrame = NULL;
		}
		s_pLoadingDoc = NULL;
	}
}

static gboolean
_abi_widget_set_show_margin(AbiWidget * abi, gboolean bShowMargin)
{
	abi->priv->m_bShowMargin = bShowMargin;
	
	if (!abi->priv->m_bMappedToScreen)
		return true;
	
	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	UT_return_val_if_fail(pFrame, false);

	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	UT_return_val_if_fail(pFrame, false);
	
	static_cast<AP_Frame *>(pFrame)->setShowMargin(bShowMargin);
	pView->setViewMode(pView->getViewMode());
	if(pFrame->getZoomType() == XAP_Frame::z_PAGEWIDTH)
	{
		UT_uint32 iZoom =  pView->calculateZoomPercentForPageWidth();
		pFrame->quickZoom(iZoom);
	}
	
	return true;
}

extern "C" gboolean
abi_widget_set_show_margin(AbiWidget * abi, gboolean bShowMargin)
{
	if (abi->priv->m_bShowMargin == bShowMargin)
		return true;
	return _abi_widget_set_show_margin(abi, bShowMargin);
}


extern "C" gboolean
abi_widget_get_show_margin(AbiWidget * abi)
{
	return static_cast<gboolean>(abi->priv->m_bShowMargin);
}


extern "C" gboolean
abi_widget_set_show_authors(AbiWidget * abi, gboolean bShowAuthors)
{
	bool bChanged = (bShowAuthors == abi->priv->m_pDoc->isShowAuthors());
	abi->priv->m_pDoc->setShowAuthors(bShowAuthors);
	return static_cast<gboolean>(bChanged);
}


extern "C" gboolean
abi_widget_get_show_authors(AbiWidget * abi)
{
	return static_cast<gboolean>(abi->priv->m_pDoc->isShowAuthors());
}


extern "C" gboolean
abi_widget_set_word_selections(AbiWidget * abi, gboolean gb)
{
	bool b = static_cast<bool>(gb);
	if(abi->priv->m_bWordSelections == b)
		return gb;
	abi->priv->m_bWordSelections = b;
	if(!abi->priv->m_bMappedToScreen)
	{
		return gb;
	}
	AP_Frame * pFrame = (AP_Frame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
		return gb;
	pFrame->setDoWordSelections(b);
	return gb;
}



extern "C" gboolean
abi_widget_get_word_selections(AbiWidget * abi)
{
	return static_cast<gboolean>(abi->priv->m_bWordSelections);
}


extern "C" gboolean
abi_widget_file_open(AbiWidget * abi)
{
	//
	// Need to release the listner first because it's View pointer
	// will be invalidated once the new document is loaded.
	//
	_abi_widget_unbindListener(abi);
	_abi_widget_releaseListener(abi);
	abi_widget_invoke(abi,"fileOpen");
	
	return TRUE;
}

static IEFileType
s_abi_widget_get_file_type(const char * extension_or_mimetype, const char * contents, UT_uint32 contents_len, bool bImport)
{
	IEFileType ieft = IEFT_Unknown;
	if (extension_or_mimetype && *extension_or_mimetype != '\0')
	{
		ieft = bImport ? IE_Imp::fileTypeForMimetype(extension_or_mimetype)
			: IE_Exp::fileTypeForMimetype(extension_or_mimetype);
			
		if (IEFT_Unknown == ieft)
			ieft = bImport ? IE_Imp::fileTypeForSuffix(extension_or_mimetype)
				: IE_Exp::fileTypeForSuffix(extension_or_mimetype);		
	}
	
	if ((IEFT_Unknown == ieft) && bImport && contents && contents_len)
	{
		ieft = IE_Imp::fileTypeForContents(contents, contents_len);
	}

	if (ieft == IEFT_Unknown && !bImport)
	{
		ieft = IE_Exp::fileTypeForSuffix(".abw");
	}

	return ieft;
}

/**
 * abi_widget_get_content:
 * @w: an #AbiWidget
 * @extension_or_mimetype: content type for the returned selection
 * @exp_props: (allow-none): export properties
 * @iLength: (out) (allow-none): length of returned selection, in bytes
 *
 * Gets all document contents
 *
 * Returns: (transfer full): the selection, the caller must free the memory
 *          through g_free()
 */
extern "C" gchar *
abi_widget_get_content(AbiWidget * w, const char * extension_or_mimetype, const char * exp_props, gint * iLength)
{
	UT_return_val_if_fail(w && w->priv, NULL);
	UT_return_val_if_fail(w->priv->m_pDoc, NULL);

	IEFileType ieft = s_abi_widget_get_file_type(extension_or_mimetype, NULL, 0, false);

	// Don't put this auto-save in the most recent list.
	XAP_App::getApp()->getPrefs()->setIgnoreNextRecent();
	GsfOutputMemory* sink = GSF_OUTPUT_MEMORY(gsf_output_memory_new());
	
	UT_Error result = w->priv->m_pDoc->saveAs(GSF_OUTPUT(sink), ieft, true, (!exp_props || *exp_props == '\0' ? NULL : exp_props));
	if(result != UT_OK)
		return NULL; // leaks sink??
	gsf_output_close(GSF_OUTPUT(sink));
	guint32 size = gsf_output_size (GSF_OUTPUT(sink));
	const guint8* ibytes = gsf_output_memory_get_bytes (sink);
	gchar * szOut = g_new (gchar, size+1);
	memcpy(szOut,ibytes,size);
	szOut[size] = 0;
	g_object_unref(G_OBJECT(sink));
	*iLength = size+1;
	w->priv->m_iContentLength = size+1;
	return szOut;
}

/**
 * abi_widget_get_selection:
 * @w: an #AbiWidget
 * @extension_or_mimetype: content type for the returned selection
 * @iLength: (out) (allow-none): length of returned selection, in bytes
 *
 * Gets the current selection
 *
 * Returns: (transfer full) (array length=iLength): the selection,
 *          the caller must free the memory through g_free()
 */
extern "C" gchar *
abi_widget_get_selection(AbiWidget * w, const gchar * extension_or_mimetype, gint * iLength)
{
	UT_return_val_if_fail(w && w->priv, NULL);
	UT_return_val_if_fail(w->priv->m_pDoc, NULL);	

	XAP_Frame * pFrame = w->priv->m_pFrame;
	UT_return_val_if_fail(w->priv->m_pFrame, NULL); // TODO: remove this restriction

	FV_View * pView = reinterpret_cast<FV_View *>(pFrame->getCurrentView());
	UT_return_val_if_fail(pView, NULL); // TODO: remove this restriction

	if (pView->isSelectionEmpty())
		return NULL;
	
	IEFileType ieft = s_abi_widget_get_file_type(extension_or_mimetype, NULL, 0, false);
	
	// Don't put this auto-save in the most recent list.
	XAP_App::getApp()->getPrefs()->setIgnoreNextRecent();
	GsfOutputMemory* sink = GSF_OUTPUT_MEMORY(gsf_output_memory_new());

	PT_DocPosition low = pView->getSelectionAnchor();
	PT_DocPosition high = pView->getPoint();
	if (high < low)
	{
			PT_DocPosition swap = low;
			low = high;
			high = swap;
	}
	PD_DocumentRange * pDocRange = new PD_DocumentRange(w->priv->m_pDoc, low, high);
	UT_ByteBuf buf;
	IE_Exp * pie = NULL;
	UT_Error errorCode;	
	IEFileType newFileType;
	errorCode = IE_Exp::constructExporter(w->priv->m_pDoc, GSF_OUTPUT(sink), ieft, &pie, &newFileType);
	if (errorCode)
		return NULL;
	pie->copyToBuffer(pDocRange,&buf);
	guint32 size = buf.getLength();
	gchar * szOut = g_new (gchar, size+1);
	memcpy(szOut,buf.getPointer(0),size);
	szOut[size] = 0;
	g_object_unref(G_OBJECT(sink));
	*iLength = size+1;
	w->priv->m_iSelectionLength = size+1;
	return szOut;
}

/**
 * abi_widget_get_mouse_pos:
 * @w: an #AbiWidget
 * @x: (out): return value for the mouse position in the X axis
 * @y: (out): return value for the mouse position in the Y axis
 *
 * Returns the mouse position relative to @w
 *
 * Return value: #TRUE if the mouse position could be retrieved
 */
extern "C" gboolean
abi_widget_get_mouse_pos(AbiWidget * w, gint32 * x, gint32 * y)
{
	AP_UnixFrame * pFrame = (AP_UnixFrame *) w->priv->m_pFrame;
	if(pFrame == NULL)
		return FALSE;
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	if(pView == NULL)
		return FALSE;
	UT_sint32 ix,iy;
	pView->getMousePos(&ix,&iy);
	*x = pView->getGraphics()->tdu(ix);
	*y = pView->getGraphics()->tdu(iy);
	return true;
}

/**
 * abi_widget_render_page_to_image:
 * 
 * Caller owns the returned GdkPixmap and must free it after use.
 * The first page is "1"
 * 
 * Returns: (transfer full): the pixbuf.
 */
extern "C" GdkPixbuf *
abi_widget_render_page_to_image(AbiWidget *abi, int iPage)
{
	//
	// AbiWord counts from 0 but we let the caller count from 1.
	//
	if(iPage <= 0)
	{
		return NULL;
	}
	iPage--;
	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
		return FALSE;
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());

	GR_UnixCairoGraphics  * pVG = static_cast<GR_UnixCairoGraphics *>(pView->getGraphics());
	UT_sint32 iWidth = pVG->tdu(pView->getWindowWidth());
	UT_sint32 iHeight = pVG->tdu(pView->getWindowHeight());
	UT_sint32 iZoom = pVG->getZoomPercentage();
	//
	// Create an offscreen Graphics to draw into
	//
	xxx_UT_DEBUGMSG(("rederpagetoimage Width %d Height %d zoom %d \n",iWidth,iHeight,iZoom));
	cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, iWidth, iHeight);
	cairo_t *cr = cairo_create(surface);

	GR_UnixCairoAllocInfo ai(NULL, false);

	GR_CairoGraphics * pG = static_cast<GR_CairoGraphics*>(GR_UnixCairoGraphics::graphicsAllocator(ai));
	pG->setCairo(cr);
	pG->beginPaint(); // needed to avoid cairo reference loss
	pG->setZoomPercentage(iZoom);
	GR_Painter * pPaint = new GR_Painter(pG);
	pPaint->clearArea(0,0,pView->getWindowWidth(),pView->getWindowHeight());
	dg_DrawArgs da;
	da.pG = pG;
	da.xoff = 0;
	da.yoff = 0;
	if(pView->getViewMode() != VIEW_PRINT)
	{
		FL_DocLayout * pLayout = pView->getLayout();
		fp_Page * pPage = pLayout->getNthPage(iPage);
		if(pPage)
		{
			fl_DocSectionLayout *pDSL = pPage->getOwningSection();
			da.yoff -= pDSL->getTopMargin();
		}
	}
	pView->getLayout()->setQuickPrint(pG);
	pView->draw(iPage, &da);
	pView->getLayout()->setQuickPrint(NULL);
	pView->getLayout()->incrementGraphicTick();
	pG->endPaint();
	cairo_destroy(cr);
	DELETEP(pPaint);
	DELETEP(pG);
	GdkPixbuf *pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, iWidth, iHeight);
	cairo_surface_destroy(surface);
	return pixbuf;
}


extern "C" gboolean
abi_widget_set_font_name(AbiWidget * w, gchar * szName)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	UT_return_val_if_fail ( szName, false );
	
	return abi_widget_invoke_ex (w,"fontFamily",szName,0,0);
}

extern "C" gboolean
abi_widget_set_font_size(AbiWidget * w, gchar * szSize)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	UT_return_val_if_fail ( szSize, false );	
	
	return abi_widget_invoke_ex (w,"fontSize",szSize,0,0);
}

extern "C" gboolean
abi_widget_set_style(AbiWidget * w, gchar * szName)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );	
	
	UT_return_val_if_fail ( szName, false );
	
	AP_UnixFrame * pFrame = (AP_UnixFrame *) w->priv->m_pFrame;
	UT_return_val_if_fail(pFrame, false);

	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	UT_return_val_if_fail(pView, false);
	
	bool res = pView->setStyle(szName, false);
	pView->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR); // I stole this mask from ap_EditMethods; looks weird to me though - MARCM
	
	return res;
}

extern "C" void
abi_widget_toggle_rulers(AbiWidget * abi, gboolean visible)
{
	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame != NULL)
	    pFrame->toggleRuler(visible);
}

extern "C" gboolean
abi_widget_insert_table(AbiWidget * abi, gint32 rows, gint32 cols)
{
	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
		return FALSE;
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	pView->cmdInsertTable(rows, cols, PP_NOPROPS);
	return TRUE;
}

extern "C" gboolean
abi_widget_insert_image(AbiWidget * w, char* szFile, gboolean positioned)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );		
	
	AP_UnixFrame * pFrame = (AP_UnixFrame *) w->priv->m_pFrame;
	UT_return_val_if_fail(pFrame, false);

	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	UT_return_val_if_fail(pView, false);	
	
	UT_return_val_if_fail(szFile, false);

	IEGraphicFileType iegft = IEGFT_Unknown;

	// we own storage for pNewFile and must g_free it.
	UT_DEBUGMSG(("abi_widget_insert_image: loading [%s]\n", szFile));

	FG_Graphic* pFG = NULL;
	UT_Error errorCode;
	errorCode = IE_ImpGraphic::loadGraphic(szFile, iegft, &pFG);
	if (errorCode != UT_OK || !pFG)
		return false;

	errorCode = (positioned ? pView->cmdInsertPositionedGraphic(pFG) : pView->cmdInsertGraphic(pFG));
	if (errorCode != UT_OK)
	{
		DELETEP(pFG);
		return false;
	}
	DELETEP(pFG);
	return true;
}
	
extern "C" gboolean
abi_widget_set_text_color(AbiWidget * w, guint8 red, guint8 green, guint8 blue)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	// get the view
	FV_View * pView = static_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
	UT_return_val_if_fail(pView, FALSE );

	// create the color property
	const PP_PropertyVector properties = {
		"color", UT_std_string_sprintf("%02x%02x%02x", red, green, blue)
	};
	// set the color
	return pView->setCharFormat(properties);
}

/**
 * abi_widget_get_font_names:
 * 
 * Returns: (transfer full): the font names.
 */
extern "C" const gchar**
abi_widget_get_font_names (AbiWidget * /*w*/)
{
	// this is annoying asc getAllFontNames() returns a lot of dupes
	const std::vector<std::string>& vFonts = GR_CairoGraphics::getAllFontNames();

	const gchar** fonts_ar = 
		reinterpret_cast<const gchar**>(g_malloc(sizeof(gchar*) * (vFonts.size() + 1))); // if there are any dupes, this will be too big, but we don't care
	UT_uint32 i;
	UT_uint32 actual_size = 0;
	for	(i = 0; i < vFonts.size(); i++)
	{
		if (!vFonts[i].empty())
		{
			// check for dupes
			UT_uint32 j;
			for (j = 0; j < actual_size; j++)
				if (vFonts[i] ==  fonts_ar[j])
					break;

			if (j == actual_size)
				fonts_ar[actual_size++] = vFonts[i].c_str();
		}
	}
	fonts_ar[actual_size] = NULL;
	return fonts_ar;
}

// TODO: there is quite a bit of overlap between abi_widget_load_file, abi_widget_load_file_from_gsf
// and abi_widget_load_file_from_memory; we need to merge them, where load_file and
// load_file_from_memory call into load_file_from_gsf

extern "C" gboolean
abi_widget_load_file(AbiWidget * w, const gchar * pszFile, const gchar * extension_or_mimetype)
{
	UT_DEBUGMSG(("abi_widget_load_file() - file: %s\n", pszFile));
	
	UT_return_val_if_fail(w && w->priv, false);
	
	IEFileType ieft = s_abi_widget_get_file_type(extension_or_mimetype, NULL, 0, true);
	UT_DEBUGMSG(("Will use ieft %d to load file\n", ieft));

	bool res = false;
	if (w->priv->m_bMappedToScreen)
	{
		UT_return_val_if_fail(w->priv->m_pFrame, FALSE);
		AP_UnixFrame * pFrame = w->priv->m_pFrame;

		s_StartStopLoadingCursor( true, pFrame);
		pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);

		UT_DEBUGMSG(("Attempting to load %s \n", pszFile));
		res = (pFrame->loadDocument(pszFile, ieft, true) == UT_OK);
		
		FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
		w->priv->m_pDoc = pView->getDocument();
		
		s_StartStopLoadingCursor( false, pFrame);
	}
	else
	{
		UT_DEBUGMSG(("Attempting to load %s \n", pszFile));
		// FIXME: DELETEP(abi->priv->m_pDoc);

		w->priv->m_pDoc = new PD_Document();
		w->priv->m_pDoc->readFromFile(pszFile, ieft);		
	}

	if (w->priv->m_bUnlinkFileAfterLoad)
	{
		remove(pszFile);
		w->priv->m_bUnlinkFileAfterLoad = false;
	}

	return res;
}

extern "C" gboolean
abi_widget_load_file_from_gsf(AbiWidget * w, GsfInput * input)
{
	UT_DEBUGMSG(("abi_widget_load_file_from_gsf()\n"));
	
	UT_return_val_if_fail(w && w->priv, false);
	UT_return_val_if_fail(input, false);
	UT_return_val_if_fail(w->priv->m_bMappedToScreen, false);
	
	// you shouldn't load a file unless we are mapped to the screen
	// NOTE: hopefully this restriction will be removed some day - MARCM
	UT_return_val_if_fail(w->priv->m_bMappedToScreen, false);
	UT_return_val_if_fail(w->priv->m_pFrame, FALSE);

	AP_UnixFrame * pFrame = (AP_UnixFrame *) w->priv->m_pFrame;
	
	bool res = false;
	s_StartStopLoadingCursor( true, pFrame);
	pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);
	res = (pFrame->loadDocument(input,IEFT_Unknown) == UT_OK);
	s_StartStopLoadingCursor( false, pFrame);

	return res;
}


extern "C" gboolean 
abi_widget_load_file_from_memory(AbiWidget * w, const gchar * extension_or_mimetype, const gchar * buf, gint length)
{
	UT_DEBUGMSG(("abi_widget_set_content()\n"));
    
	UT_return_val_if_fail(w && w->priv, false);
    UT_return_val_if_fail(buf && length > 0, false);
	
	GsfInputMemory* source = GSF_INPUT_MEMORY(gsf_input_memory_new((guint8 *)buf, length, false));
	UT_return_val_if_fail(source, false);

	IEFileType ieft = s_abi_widget_get_file_type(extension_or_mimetype, buf, length, true);
	UT_DEBUGMSG(("Will use ieft %d to load file\n", ieft));
	
	bool res = false;
	if (w->priv->m_bMappedToScreen)
	{
		UT_return_val_if_fail(w->priv->m_pFrame, FALSE);
		AP_UnixFrame * pFrame = w->priv->m_pFrame;

		s_StartStopLoadingCursor( true, pFrame);
		pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);

		UT_DEBUGMSG(("Attempting to load from stream\n"));		
		res = (pFrame->loadDocument(GSF_INPUT(source), ieft) == UT_OK);
		
		FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
		w->priv->m_pDoc = pView->getDocument();
		
		s_StartStopLoadingCursor(false, pFrame);
	}
	else
	{
		UT_DEBUGMSG(("Attempting to load from stream in unmapped state\n"));
		// FIXME: DELETEP(abi->priv->m_pDoc);

		w->priv->m_pDoc = new PD_Document();
		w->priv->m_pDoc->readFromFile(GSF_INPUT(source), ieft);		
	}

    return res;
}

static gboolean s_abi_widget_map_cb(GObject * /*w*/, gpointer p)
{
	UT_DEBUGMSG(("s_abi_widget_map_cb()\n"));
	
	UT_return_val_if_fail (p != 0, true);
	AbiWidget* abi = reinterpret_cast<AbiWidget*>(p);

	if (abi->priv->m_bMappedToScreen)
		return false;
	
	// now we can set up Abi inside of this GdkWindow
	GtkWidget * widget = GTK_WIDGET(abi);

#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget about to map_to_screen \n");
	fprintf(getlogfile(),"AbiWidget about to map_to_screen ref_count %d \n",G_OBJECT(abi)->ref_count);
#endif

	// we can show stuff on screen now: use the pango graphics class
	XAP_App::getApp()->getGraphicsFactory()->registerAsDefault(GRID_UNIX_PANGO, true);
	
	// construct a new frame and view
	AP_UnixFrame * pFrame  = new AP_UnixFrame();
	UT_return_val_if_fail(pFrame, false);
	static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl())->setTopLevelWindow(widget); 
	pFrame->initialize(XAP_NoMenusWindowLess);
	abi->priv->m_pFrame = pFrame;

	// setup our frame
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail(pFrameData, true);
	pFrameData->m_bIsWidget = true;
	pFrame->setZoomType(XAP_Frame::z_PAGEWIDTH);

	XAP_App::getApp()->rememberFrame ( pFrame ) ;
	XAP_App::getApp()->rememberFocussedFrame ( pFrame ) ;


	if (abi->priv->m_pDoc)
	{
		UT_DEBUGMSG(("Loading document that was opened before we were mapped to screen\n"));
		pFrame->loadDocument(abi->priv->m_pDoc);
	}
	else
	{
		// we currenly don't have any loaded document; just start with a blank document 
		pFrame->loadDocument(NULL, IEFT_Unknown, true);
	}
	
	// setup our view
	FV_View * pView = static_cast<FV_View *>(abi->priv->m_pFrame->getCurrentView());
	UT_return_val_if_fail(pView, true);

	if (!abi->priv->m_pDoc)
		abi->priv->m_pDoc = pView->getDocument();
	
	// start listening for interesting signals
	abi->priv->m_pFrameListener = new AbiWidget_FrameListener(abi);
	_abi_widget_bindListenerToView(abi, pView);
	
	pFrame->toggleRuler(false);
	_abi_widget_set_show_margin(abi, abi->priv->m_bShowMargin); // this will also zoom to our previously set zoom factor; no need to zoom manually
	pFrame->setDoWordSelections(abi->priv->m_bWordSelections);
	pView->setViewMode(VIEW_NORMAL);
	
	// yay, we're done
	abi->priv->m_bMappedToScreen = true;

#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget After Finished s_abi_widget_map_cb ref_count %d \n",G_OBJECT(abi)->ref_count);
#endif
		
	return false;
}

//
// arguments to abiwidget
//
static void abi_widget_get_prop (GObject  *object,
								 guint arg_id,
								 GValue     *arg,
								 GParamSpec * /*pspec*/)
{
    AbiWidget * abi = ABI_WIDGET(object);
	switch(arg_id)
	{
	    case UNLINK_AFTER_LOAD:
		{
			g_value_set_boolean(arg,(gboolean) abi->priv->m_bUnlinkFileAfterLoad);
			break;
		}
	    case CONTENT:
		{
			gint i;
			gchar * bytes = abi_widget_get_content(abi, NULL, NULL, &i);
			g_value_set_string(arg,bytes);
			break;
		}
	    case SELECTION:
		{
			gint i;
			gchar * bytes = abi_widget_get_selection(abi, NULL, &i);
			g_value_set_string(arg,bytes);
			break;
		}
	    case CONTENT_LENGTH:
		{
			g_value_set_int(arg,abi->priv->m_iContentLength);
			break;
		}
	    case SELECTION_LENGTH:
		{
			g_value_set_int(arg,abi->priv->m_iSelectionLength);
			break;
		}
	    case SHADOW_TYPE:
		{
			AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(abi->priv->m_pFrame->getFrameImpl());
			g_value_set_int (arg, pFrameImpl->getShadowType());
			break;
		}
	    default:
			break;
	}
}

extern "C" void
abi_widget_get_property(GObject  *object,
						guint arg_id,
						GValue     *arg,
						GParamSpec *pspec)
{
	abi_widget_get_prop(object,	arg_id,	arg, pspec);
}

//
// props to abiwidget
//
static void abi_widget_set_prop (GObject  *object,
								 guint	arg_id,
								 const GValue *arg,
								 GParamSpec * /*pspec*/)
{
	UT_return_if_fail (object != 0);

	AbiWidget * abi = ABI_WIDGET (object);
	AbiWidgetClass * abi_klazz = ABI_WIDGET_CLASS (G_OBJECT_GET_CLASS(object));

#ifdef LOGFILE
	fprintf(getlogfile(),"setArg %d\n",arg_id);
#endif

	switch(arg_id)
	{
	    case CURSOR_ON:
		{
		     if(g_value_get_boolean(arg) == TRUE)
			      abi_widget_turn_on_cursor(abi);
			 break;
		}
	    case UNLINK_AFTER_LOAD:
		{
		     if(g_value_get_boolean(arg) == TRUE)
			      abi->priv->m_bUnlinkFileAfterLoad = true;
			 else
			      abi->priv->m_bUnlinkFileAfterLoad = false;
			 break;
		}
	    case VIEWPARA:
		{
			abi_klazz->view_formatting_marks (abi);
			break;
		}
	    case VIEWPRINTLAYOUT:
		{
			abi_klazz->view_print_layout (abi);
			break;
		}
	    case VIEWNORMALLAYOUT:
		{
			abi_klazz->view_normal_layout (abi);
			break;
		}
	    case VIEWWEBLAYOUT:
	    {
			abi_klazz->view_online_layout (abi);
			break;
		}
		case SHADOW_TYPE:
		{
			AP_UnixFrameImpl * pFrameImpl = static_cast<AP_UnixFrameImpl *>(abi->priv->m_pFrame->getFrameImpl());
			int shadow = g_value_get_int (arg);
			pFrameImpl->setShadowType((GtkShadowType) shadow);
			break;
		}  
	    default:
			break;
	}
}

extern "C" void 
abi_widget_set_property(GObject  *object,
						guint	arg_id,
						const GValue *arg,
						GParamSpec *pspec)
{
	abi_widget_set_prop(object, arg_id, arg, pspec);
}

static void 
abi_widget_get_preferred_height(GtkWidget *widget,
                                int *minimum_height, int *natural_height)
{
	*minimum_height = *natural_height = ABI_DEFAULT_HEIGHT;
	if (ABI_WIDGET(widget)->child)
		gtk_widget_get_preferred_height(ABI_WIDGET(widget)->child,
		                                minimum_height, natural_height);
}

static void 
abi_widget_get_preferred_width(GtkWidget *widget,
                                int *minimum_width, int *natural_width)
{
	*minimum_width = *natural_width = ABI_DEFAULT_WIDTH;
	if (ABI_WIDGET(widget)->child)
		gtk_widget_get_preferred_height(ABI_WIDGET(widget)->child,
		                                minimum_width, natural_width);
}

//
// Needed for the gtkbin class
//
static void
abiwidget_add(GtkContainer *container,
	      GtkWidget    *widget)
{
	UT_return_if_fail (container != NULL);
	UT_return_if_fail (widget != NULL);

	if (GTK_CONTAINER_CLASS (parent_class)->add)
		GTK_CONTAINER_CLASS (parent_class)->add (container, widget);

	ABI_WIDGET(container)->child = gtk_bin_get_child(GTK_BIN (container));
}

//
// Needed for the gtkbin class
//
static void
abiwidget_remove (GtkContainer *container,
		  GtkWidget    *widget)
{
	UT_return_if_fail (container != NULL);
	UT_return_if_fail (widget != NULL);

	if (GTK_CONTAINER_CLASS (parent_class)->remove)
		GTK_CONTAINER_CLASS (parent_class)->remove (container, widget);

	ABI_WIDGET(container)->child = gtk_bin_get_child(GTK_BIN (container));
}

//
// Needed for the gtkbin class
//
static GType
abiwidget_child_type (GtkContainer *container)
{
	if (!gtk_bin_get_child(GTK_BIN(container)))
		return GTK_TYPE_WIDGET;

	return G_TYPE_NONE;
}

static void
abi_widget_init (AbiWidget * abi)
{
	abi->priv = new AbiPrivData();

	// this isn't really needed, since each widget is
	// guaranteed to be created with g_new0 and we just
	// want everything to be 0/NULL/FALSE anyway right now
	// but i'm keeping it around anyway just in case that changes
	gtk_widget_set_can_focus(GTK_WIDGET(abi), true);
	gtk_widget_set_receives_default(GTK_WIDGET(abi), true);
	gtk_widget_set_can_default(GTK_WIDGET(abi), true);
	gtk_widget_set_has_window(GTK_WIDGET(abi), true);
}

static void
abi_widget_size_allocate (GtkWidget     *widget,
						  GtkAllocation *allocation)
{
	AbiWidget *abi;
	
	UT_return_if_fail (widget != NULL);
	UT_return_if_fail (IS_ABI_WIDGET (widget));
	UT_return_if_fail (allocation != NULL);

	GtkAllocation child_allocation;
	gtk_widget_set_allocation(widget, allocation);

	gint border_width = gtk_container_get_border_width(GTK_CONTAINER (widget));
	GtkStyleContext *ctxt = gtk_widget_get_style_context(widget);
	GtkBorder border;
	gtk_style_context_get_padding(ctxt, gtk_widget_get_state_flags(widget), &border);
 	if (gtk_widget_get_realized(widget))
    {
		// only allocate on realized widgets

		abi = ABI_WIDGET(widget);
		gdk_window_move_resize (gtk_widget_get_window(widget),
					allocation->x+border_width, 
					allocation->y+border_width,
					allocation->width - border_width*2, 
					allocation->height - border_width*2);
		
		if (abi->child)
		{
		     child_allocation.x = border.left;
			 child_allocation.y = border.top;

			 child_allocation.width = MAX (1, 
										   (gint)allocation->width - border.left - border.right - border_width * 2);
			 child_allocation.height = MAX (1, 
											(gint)allocation->height - border.top - border.bottom - border_width * 2);
			 gtk_widget_size_allocate (ABI_WIDGET (widget)->child, &child_allocation);
		}
    }
}

static void
abi_widget_grab_focus (GtkWidget * widget)
{
       UT_return_if_fail(widget != NULL);
       UT_return_if_fail(IS_ABI_WIDGET(widget));

       XAP_Frame *pFrame = ABI_WIDGET(widget)->priv->m_pFrame;
       UT_return_if_fail(pFrame);

       GtkWidget *dArea = static_cast<AP_UnixFrameImpl *>(pFrame->getFrameImpl())->getDrawingArea();
       gtk_widget_grab_focus(dArea);
}

static void
abi_widget_realize (GtkWidget * widget)
{
	AbiWidget * abi;
	GdkWindowAttr attributes;
	gint attributes_mask;
	GtkAllocation alloc;

	// we *must* ensure that we get a GdkWindow to draw into
	// this here is just boilerplate GTK+ code

	UT_return_if_fail (widget != NULL);
	UT_return_if_fail (IS_ABI_WIDGET(widget));

	gtk_widget_set_realized(widget, true);
	abi = ABI_WIDGET(widget);

	gtk_widget_get_allocation(widget, &alloc);
	attributes.x = alloc.x;
	attributes.y = alloc.y;
	attributes.width = ABI_DEFAULT_WIDTH;
	attributes.height = ABI_DEFAULT_HEIGHT;
	attributes.wclass = GDK_INPUT_OUTPUT;
	attributes.window_type = GDK_WINDOW_CHILD;
	attributes.event_mask = gtk_widget_get_events (widget) | 
						GDK_EXPOSURE_MASK | GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK |
						GDK_BUTTON_RELEASE_MASK | GDK_POINTER_MOTION_MASK |
						GDK_POINTER_MOTION_HINT_MASK | GDK_ENTER_NOTIFY_MASK |
						GDK_LEAVE_NOTIFY_MASK |
						GDK_FOCUS_CHANGE_MASK |
						GDK_STRUCTURE_MASK;
	attributes.visual = gtk_widget_get_visual (widget);
	
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;

	gtk_widget_set_window(widget,
	                      gdk_window_new (gtk_widget_get_parent_window (widget),
	                                      &attributes, attributes_mask));
	gdk_window_set_user_data (gtk_widget_get_window(widget), abi);

	//
	// connect a signal handler to load files after abiword is in a stable
	// state.
	//
	g_signal_connect_after(G_OBJECT(widget),"map", 
			       G_CALLBACK (s_abi_widget_map_cb),
			       (gpointer) abi);
}

static void
abi_widget_destroy_gtk (GtkWidget *object)
{
	AbiWidget * abi;
	
	UT_return_if_fail (object != NULL);
	UT_return_if_fail (IS_ABI_WIDGET(object));

	// here we g_free any self-created data
	abi = ABI_WIDGET(object);

	// order of deletion is important here
	XAP_App *pApp = XAP_App::getApp();
	//bool bKillApp = false;

	if (abi->priv) 
	{
		_abi_widget_releaseListener(abi);
		// TODO: release the frame listener
		if(abi->priv->m_pFrame)
		{
#ifdef LOGFILE
			fprintf(getlogfile(),"frame count before forgetting = %d \n",pApp->getFrameCount());
#endif
			//if(pApp->getFrameCount() <= 1)
			//{
			//	bKillApp = true;
			//}
			pApp->forgetFrame(abi->priv->m_pFrame);
			abi->priv->m_pFrame->close();
			delete abi->priv->m_pFrame;
#ifdef LOGFILE
			fprintf(getlogfile(),"frame count = %d \n",pApp->getFrameCount());
#endif
		}
		DELETEP(abi->priv->m_sSearchText);
		DELETEP(abi->priv);
	}

#ifdef LOGFILE
	fprintf(getlogfile(),"abiwidget destroyed in abi_widget_destroy_gtk\n");
#endif
}


static void
abi_widget_class_init (AbiWidgetClass *abi_class)
{

#ifdef LOGFILE
	fprintf(getlogfile(),"Abi_widget class init \n");
#endif

	GObjectClass * gobject_class;
	GtkWidgetClass * widget_class;
	GtkContainerClass *container_class;
	container_class = (GtkContainerClass*) abi_class;

	gobject_class = (GObjectClass *)abi_class;
	widget_class = (GtkWidgetClass *)abi_class;

	// set our parent class
	parent_class = (GtkBinClass *)
		g_type_class_ref (gtk_bin_get_type());
	
	// set our custom destroy method
	widget_class->destroy = abi_widget_destroy_gtk;
	gobject_class->set_property = abi_widget_set_prop;
	gobject_class->get_property = abi_widget_get_prop;

	// set our custom class methods
	widget_class->realize       = abi_widget_realize;
	widget_class->get_preferred_height  = abi_widget_get_preferred_height;
	widget_class->get_preferred_width  = abi_widget_get_preferred_width;
   	widget_class->size_allocate = abi_widget_size_allocate; 
	widget_class->grab_focus    = abi_widget_grab_focus;

	// For the container methods
	container_class->add = abiwidget_add;
	container_class->remove = abiwidget_remove;
	container_class->child_type = abiwidget_child_type;

	// AbiWidget's master "invoke" method
	abi_class->invoke    = abi_widget_invoke;
	abi_class->invoke_ex = abi_widget_invoke_ex;

	// assign handles to Abi's edit methods
	abi_class->align_center  = EM_NAME(alignCenter);
	abi_class->align_justify = EM_NAME(alignJustify);
	abi_class->align_left    = EM_NAME(alignLeft);
	abi_class->align_right   = EM_NAME(alignRight);
	
	abi_class->copy          = EM_NAME(copy);
	abi_class->cut           = EM_NAME(cut);
	abi_class->paste         = EM_NAME(paste);
	abi_class->paste_special = EM_NAME(pasteSpecial);
	abi_class->select_all    = EM_NAME(selectAll);
	abi_class->select_block  = EM_NAME(selectBlock);
	abi_class->select_line   = EM_NAME(selectLine);	
	abi_class->select_word   = EM_NAME(selectWord);	

	abi_class->file_open   = abi_widget_file_open;
	abi_class->file_save   = EM_NAME(fileSave);	
	abi_class->save_immediate   = EM_NAME(saveImmediate);	
	
	abi_class->undo = EM_NAME(undo);
	abi_class->redo = EM_NAME(redo);
	
	abi_class->insert_data  = EM_NAME(insertData);
	abi_class->insert_space = EM_NAME(insertSpace);		
        
	abi_class->delete_bob   = EM_NAME(delBOB);
	abi_class->delete_bod   = EM_NAME(delBOD);
	abi_class->delete_bol   = EM_NAME(delBOL);
	abi_class->delete_bow   = EM_NAME(delBOW);
	abi_class->delete_eob   = EM_NAME(delEOB);
	abi_class->delete_eod   = EM_NAME(delEOD);
	abi_class->delete_eol   = EM_NAME(delEOL);
	abi_class->delete_eow   = EM_NAME(delEOW);
	abi_class->delete_left  = EM_NAME(delLeft);
	abi_class->delete_right = EM_NAME(delRight);
	
	abi_class->edit_header   = EM_NAME(editHeader);
	abi_class->edit_footer   = EM_NAME(editFooter);
	abi_class->remove_header = EM_NAME(removeHeader);
	abi_class->remove_footer = EM_NAME(removeFooter);
	
	abi_class->select_bob         = EM_NAME(extSelBOB);
	abi_class->select_bod         = EM_NAME(extSelBOD);
	abi_class->select_bol         = EM_NAME(extSelBOL);
	abi_class->select_bow         = EM_NAME(extSelBOW);
	abi_class->select_eob         = EM_NAME(extSelEOB);
	abi_class->select_eod         = EM_NAME(extSelEOD);
	abi_class->select_eol         = EM_NAME(extSelEOL);
	abi_class->select_eow         = EM_NAME(extSelEOW);    
	abi_class->select_left        = EM_NAME(extSelLeft);
	abi_class->select_next_line   = EM_NAME(extSelNextLine);
	abi_class->select_page_down   = EM_NAME(extSelPageDown);
	abi_class->select_page_up     = EM_NAME(extSelPageUp);
	abi_class->select_prev_line   = EM_NAME(extSelPrevLine);
	abi_class->select_right       = EM_NAME(extSelRight);
	abi_class->select_screen_down = EM_NAME(extSelScreenDown);
	abi_class->select_screen_up   = EM_NAME(extSelScreenUp);
	abi_class->select_to_xy       = EM_NAME(extSelToXY);
	
	abi_class->toggle_bold        = EM_NAME(toggleBold);
	abi_class->toggle_bottomline  = EM_NAME(toggleBottomline);
	abi_class->toggle_insert_mode = EM_NAME(toggleInsertMode);
	abi_class->toggle_italic      = EM_NAME(toggleItalic);
	abi_class->toggle_overline    = EM_NAME(toggleOline);
	abi_class->toggle_plain       = EM_NAME(togglePlain);
	abi_class->toggle_strike      = EM_NAME(toggleStrike);
	abi_class->toggle_sub         = EM_NAME(toggleSub);
	abi_class->toggle_super       = EM_NAME(toggleSuper);
	abi_class->toggle_topline     = EM_NAME(toggleTopline);
	abi_class->toggle_underline   = EM_NAME(toggleUline);
	abi_class->toggle_unindent    = EM_NAME(toggleUnIndent);
	abi_class->toggle_bullets     = EM_NAME(doBullets);
	abi_class->toggle_numbering   = EM_NAME(doNumbers);
	
	abi_class->view_formatting_marks = EM_NAME(viewPara);
	abi_class->view_print_layout     = EM_NAME(viewPrintLayout);
	abi_class->view_normal_layout    = EM_NAME(viewNormalLayout);
	abi_class->view_online_layout    = EM_NAME(viewWebLayout);
	
	abi_class->moveto_bob         = EM_NAME(warpInsPtBOB);
	abi_class->moveto_bod         = EM_NAME(warpInsPtBOD);
	abi_class->moveto_bol         = EM_NAME(warpInsPtBOL);
	abi_class->moveto_bop         = EM_NAME(warpInsPtBOP);
	abi_class->moveto_bow         = EM_NAME(warpInsPtBOW);
	abi_class->moveto_eob         = EM_NAME(warpInsPtEOB);
	abi_class->moveto_eod         = EM_NAME(warpInsPtEOD);
	abi_class->moveto_eol         = EM_NAME(warpInsPtEOL);
	abi_class->moveto_eop         = EM_NAME(warpInsPtEOP);
	abi_class->moveto_eow         = EM_NAME(warpInsPtEOW);
	abi_class->moveto_left        = EM_NAME(warpInsPtLeft);
	abi_class->moveto_next_line   = EM_NAME(warpInsPtNextLine);
	abi_class->moveto_next_page   = EM_NAME(warpInsPtNextPage);
	abi_class->moveto_next_screen = EM_NAME(warpInsPtNextScreen);
	abi_class->moveto_prev_line   = EM_NAME(warpInsPtPrevLine);
	abi_class->moveto_prev_page   = EM_NAME(warpInsPtPrevPage);
	abi_class->moveto_prev_screen = EM_NAME(warpInsPtPrevScreen);
	abi_class->moveto_right       = EM_NAME(warpInsPtRight);
	abi_class->moveto_to_xy       = EM_NAME(warpInsPtToXY);
	
	abi_class->zoom_whole = EM_NAME(zoomWhole);
	abi_class->zoom_width = EM_NAME(zoomWidth);


// now install GObject properties

	g_object_class_install_property (gobject_class,
									 CURSOR_ON,
									 g_param_spec_boolean("cursor-on",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property (gobject_class,
									 UNLINK_AFTER_LOAD,
									 g_param_spec_boolean("unlink-after-load",
														  NULL,
														  NULL,
														  FALSE,
														  (GParamFlags) G_PARAM_READWRITE));
	g_object_class_install_property(gobject_class,
								  VIEWPARA,
								  g_param_spec_boolean("view-para",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
	g_object_class_install_property(gobject_class,
								  VIEWPRINTLAYOUT,
								  g_param_spec_boolean("view-print-layout",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
	g_object_class_install_property(gobject_class,
								  VIEWNORMALLAYOUT,
								  g_param_spec_boolean("view-normal-layout",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));
	g_object_class_install_property(gobject_class,
								  VIEWWEBLAYOUT,
								  g_param_spec_boolean("view-web-layout",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));

	g_object_class_install_property(gobject_class,
								  CONTENT,
								  g_param_spec_string("content",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READABLE)));

	g_object_class_install_property(gobject_class,
								  SELECTION,
								  g_param_spec_string("selection",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READABLE)));

	g_object_class_install_property(gobject_class,
								  CONTENT_LENGTH,
								  g_param_spec_int("content_length",
													   NULL,
													   NULL,
													   INT_MIN,
													   INT_MAX,
													   0,
													   static_cast<GParamFlags>(G_PARAM_READABLE)));

	g_object_class_install_property(gobject_class,
								  SELECTION_LENGTH,
								  g_param_spec_int("selection_length",
													   NULL,
													   NULL,
													   INT_MIN,
													   INT_MAX,
													   0,
													   static_cast<GParamFlags>(G_PARAM_READABLE)));

	g_object_class_install_property(gobject_class,
								  SHADOW_TYPE,
								  g_param_spec_int("shadow_type", NULL, NULL,
													   (int) GTK_SHADOW_NONE,
													   (int) GTK_SHADOW_ETCHED_OUT,
													   (int) GTK_SHADOW_IN,
													   static_cast<GParamFlags>(G_PARAM_READWRITE)));

	_abi_widget_class_install_signals (abi_class);
}

static void
abi_widget_construct (AbiWidget * /*abi*/, const char * /*file*/)
{
	// this is all that we can do here, because we can't draw until we're
	// realized and have a GdkWindow pointer

	UT_DEBUGMSG(("FIXME: allow loading the file on construction\n"));
		
#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget Constructed %x \n",abi);
#endif
}

/**************************************************************************/
/**************************************************************************/

extern "C" void 
abi_widget_turn_on_cursor(AbiWidget * abi)
{
	if (abi->priv->m_pFrame)
	{
		UT_return_if_fail (abi != 0);
		FV_View * pV = static_cast<FV_View*>(abi->priv->m_pFrame->getCurrentView());
		if (pV)
			pV->focusChange(AV_FOCUS_HERE);
	}
}

extern "C" GType
abi_widget_get_type (void)
{
	static GType abi_type = 0;

	if (!abi_type)
	{
		GTypeInfo info =
		{
			sizeof (AbiWidgetClass),
			NULL,
			NULL,
			(GClassInitFunc)abi_widget_class_init,
			NULL,
			NULL,
			sizeof(AbiWidget),
			0,
			(GInstanceInitFunc)abi_widget_init,
                        NULL
		};

		abi_type = g_type_register_static (gtk_bin_get_type (), "AbiWidget",
								   &info, (GTypeFlags)0);
	}
	
	return abi_type;
}

/**
 * abi_widget_new:
 *
 * Creates a new AbiWord widget using an internal Abiword App
 */
extern "C" GtkWidget *
abi_widget_new (void)
{
	AbiWidget * abi;
	UT_DEBUGMSG(("Constructing AbiWidget \n"));
	abi = static_cast<AbiWidget *>(g_object_new (abi_widget_get_type (), NULL));
	abi_widget_construct (abi, 0);

	return GTK_WIDGET (abi);
}

/**
 * abi_widget_new_with_file:
 *
 * Creates a new AbiWord widget and tries to load the file
 * This uses an internal Abiword App
 *
 * \param file - A file on your HD
 */
extern "C" GtkWidget *
abi_widget_new_with_file (const gchar * file)
{
	AbiWidget * abi;

	UT_return_val_if_fail (file != 0, 0);

	abi = static_cast<AbiWidget *>(g_object_new (abi_widget_get_type (), NULL));
	abi_widget_construct (abi, file);

	return GTK_WIDGET (abi);
}

extern "C" XAP_Frame * 
abi_widget_get_frame ( AbiWidget * w )
{
	UT_return_val_if_fail ( w != NULL, NULL ) ;
	return w->priv->m_pFrame ;
}

/**
 * abi_widget_invoke:
 *
 * Invoke any of abiword's edit methods by name
 *
 * \param w - An AbiWord widget
 * \param mthdName - A null-terminated string representing the method's name
 *
 * \return FALSE if any preconditions fail
 * \return TRUE|FALSE depending on the result of the EditMethod's completion
 *
 * example usage:
 *
 * gboolean b = FALSE; 
 * GtkWidget * w = abi_widget_new ();
 *
 * b = abi_widget_invoke (ABI_WIDGET(w), "alignCenter");
 *
 */
extern "C" gboolean
abi_widget_invoke (AbiWidget * w, const char * mthdName)
{
	return abi_widget_invoke_ex ( w, mthdName, NULL, 0, 0 ) ;
}

/**
 * abi_widget_invoke_ex:
 *
 * Invoke any of abiword's edit methods by name
 *
 * \param w - An AbiWord widget
 * \param mthdName - A null-terminated string representing the method's name
 * \param data - an optional null-terminated string data to pass to the method
 * \param x - an optional x-coordinate to pass to the method (usually 0)
 * \param y - an optional y-coordinate to pass to the method (usuall 0)
 *
 * \return FALSE if any preconditions fail
 * \return TRUE|FALSE depending on the result of the EditMethod's completion
 *
 * example usage:
 *
 * gboolean b = FALSE; 
 * GtkWidget * w = abi_widget_new ();
 *
 * b = abi_widget_invoke_ex (ABI_WIDGET(w), "insertData", "Hello World!", 0, 0);
 * b = abi_widget_invoke_ex (ABI_WIDGET(w), "alignCenter", 0, 0, 0);
 *
 */
extern "C" gboolean
abi_widget_invoke_ex (AbiWidget * w, const char * mthdName, 
		      const char * data, gint32 x, gint32 y)
{
	EV_EditMethodContainer * container;
	EV_EditMethod          * method;
	AV_View                * view;

	UT_DEBUGMSG(("abi_widget_invoke_ex, methodname: %s\n", mthdName));

	// lots of conditional returns - code defensively
	UT_return_val_if_fail (w != 0, FALSE);
	UT_return_val_if_fail (mthdName != 0, FALSE);

	// get the method container
	XAP_App *pApp = XAP_App::getApp();
	container = pApp->getEditMethodContainer();
	UT_return_val_if_fail (container != 0, FALSE);

	// get a handle to the actual EditMethod
	method = container->findEditMethodByName (mthdName);
	UT_return_val_if_fail (method != 0, FALSE);

	// get a valid frame
	UT_return_val_if_fail (w->priv->m_pFrame != 0, FALSE);

	// obtain a valid view
	view = w->priv->m_pFrame->getCurrentView();
	UT_return_val_if_fail (view != 0, FALSE);
	xxx_UT_DEBUGMSG(("Data to invoke %s \n",data));

	// construct the call data
	UT_UCS4String ucs4String = data ? UT_UTF8String(data).ucs4_str() : UT_UCS4String();
	const UT_UCSChar* actualData = data ? ucs4String.ucs4_str() : NULL;
	EV_EditMethodCallData calldata(actualData, (actualData ? ucs4String.size() : 0));
	calldata.m_xPos = x;
	calldata.m_yPos = y;

	// actually invoke
	return (method->Fn(view, &calldata) ? TRUE : FALSE);
}

extern "C" void
abi_widget_draw (AbiWidget * w)
{
	// obtain a valid view
	if (w->priv->m_pFrame)
	{
		// obtain a valid view
		UT_return_if_fail (w != NULL);
		FV_View * view = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
		if (view)
			view->draw();
	}
}

extern "C" gboolean 
abi_widget_save ( AbiWidget * w, const char * fname, const char * extension_or_mimetype, const char * exp_props)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pDoc, FALSE );
	UT_return_val_if_fail ( fname != NULL, FALSE );

	IEFileType ieft = s_abi_widget_get_file_type(extension_or_mimetype, NULL, 0, false);
	return static_cast<AD_Document*>(w->priv->m_pDoc)->saveAs ( fname, ieft, false, (!exp_props || *exp_props == '\0' ? NULL : exp_props) ) == UT_OK ? TRUE : FALSE;
}

extern "C" gboolean 
abi_widget_save_to_gsf ( AbiWidget * w, GsfOutput * output, const char * extension_or_mimetype, const char * exp_props )
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( output != NULL, FALSE );

	IEFileType ieft = s_abi_widget_get_file_type(extension_or_mimetype, NULL, 0, false);
	return w->priv->m_pDoc->saveAs(output, ieft, false, (!exp_props || *exp_props == '\0' ? NULL : exp_props)) == UT_OK ? TRUE : FALSE;
}

extern "C" gboolean 
abi_widget_set_zoom_percentage (AbiWidget * w, guint32 zoom)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	w->priv->m_pFrame->setZoomType(XAP_Frame::z_PERCENT);
	w->priv->m_pFrame->quickZoom(zoom);
	return TRUE;
}

extern "C" guint32 
abi_widget_get_zoom_percentage (AbiWidget * w)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	return w->priv->m_pFrame->getZoomPercentage();
}

static FV_View* 
_get_fv_view(AbiWidget* w)
{
	AV_View* v = w->priv->m_pFrame->getCurrentView();
	UT_return_val_if_fail(v!=0, NULL);
	return static_cast<FV_View*>( v );
}

extern "C" void
abi_widget_set_find_string(AbiWidget * w, gchar * search_str)
{
	*w->priv->m_sSearchText = UT_UTF8String(search_str).ucs4_str();	// ucs4_str returns object instance
	FV_View* v = _get_fv_view(w);
	UT_return_if_fail(v);
	v->findSetFindString( w->priv->m_sSearchText->ucs4_str() );
}

extern "C" gboolean
abi_widget_find_next(AbiWidget * w, gboolean sel_start)
{
	FV_View* v = _get_fv_view(w);
	UT_return_val_if_fail(v, false);
	if (!sel_start || v->isSelectionEmpty())
	{
		v->findSetStartAtInsPoint();
	}
	else
	{
		PT_DocPosition start = std::min(v->getPoint(), v->getSelectionAnchor());
		v->cmdUnselectSelection();
		v->setPoint(start);
		v->findSetStartAt(start);
	}
	bool doneWithEntireDoc = false;
	bool res = v->findNext(doneWithEntireDoc);
	return res;
}

extern "C" gboolean
abi_widget_find_prev(AbiWidget * w)
{
	FV_View* v = _get_fv_view(w);
	UT_return_val_if_fail(v, false);
	bool doneWithEntireDoc = false;
	v->findSetStartAtInsPoint();
	bool res = v->findPrev(doneWithEntireDoc);
	return res;
}

extern "C" guint32
abi_widget_get_page_count(AbiWidget * w)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	FV_View * pView = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
	UT_return_val_if_fail(pView, 0);

	FL_DocLayout* pLayout = pView->getLayout();
	UT_return_val_if_fail(pLayout, 0);

	return pLayout->countPages();
}

extern "C" void
abi_widget_set_current_page(AbiWidget * w, guint32 curpage)
{
	UT_return_if_fail ( w != NULL );
	UT_return_if_fail ( IS_ABI_WIDGET(w) );
	UT_return_if_fail ( w->priv->m_pFrame );
	
	FV_View * pView = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
	UT_return_if_fail( pView );
	
	FL_DocLayout* pLayout = pView->getLayout();
	UT_return_if_fail( pLayout );
	
	UT_return_if_fail( curpage <= (guint32)pLayout->countPages() );	// page are not zero-based, so <= in stead of <

	UT_DEBUGMSG(("Telling the view to jump to page %u!\n", curpage));
	
	UT_UCS4String pageUCS4Str( UT_UTF8String_sprintf( "%u", curpage ).utf8_str(), 0 );
	pView->gotoTarget( AP_JUMPTARGET_PAGE, (UT_UCSChar*) pageUCS4Str.ucs4_str() );
}

extern "C" guint32
abi_widget_get_current_page_num(AbiWidget * w)
{
	UT_return_val_if_fail ( w != NULL, FALSE );
	UT_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	UT_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	FV_View * pView = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
	UT_return_val_if_fail(pView, 0);

	// there's also getCurrentPageNumber() we can use, but that is slower
	return pView->getCurrentPageNumForStatusBar();
}

