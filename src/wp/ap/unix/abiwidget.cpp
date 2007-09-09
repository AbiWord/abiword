/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* The AbiWord Widget 
 *
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001,2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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

#include "gr_UnixPangoGraphics.h"

#include "ev_EditMethod.h"
#include "ut_assert.h"
#include "fv_View.h"
#include "ap_UnixFrame.h"
#include "ap_FrameData.h"
#include "pd_Document.h"
#include "ie_imp.h"
#include "ie_exp.h"
#include "ie_impGraphic.h"
#include "xap_UnixDialogHelper.h"
#include "ap_UnixApp.h"
#include "ut_sleep.h"
#include "fv_View.h"
#include "fl_DocLayout.h"
#include "ut_go_file.h"
#include "ut_timer.h"
#include "ev_Toolbar_Actions.h"
#include "ap_Toolbar_Id.h"

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
 * an instance of this AbiWidget. Following that, Martin undertook the
 * ordeal of exposing this Widget via a Bonobo interface (after first
 * inventing Bonobo, of course) so that other applications such as
 * Ximian Evolution could seamlessly embed instances of AbiWord inside
 * of themselves.
 *
 */

/**************************************************************************/
/**************************************************************************/

class AbiWidget_ViewListener;

// Our widget's private storage data
// UnixApp and UnixFrame already properly inherit from either
// Their GTK+ or GNOME XAP equivalents, so we don't need to worry
// About that here

struct _AbiPrivData {
	AP_UnixFrame         * m_pFrame;
	char           * m_szFilename;
	bool                 m_bMappedToScreen;
	bool                 m_bPendingFile;
	bool                 m_bMappedEventProcessed;
    bool                 m_bUnlinkFileAfterLoad;
	gint                 m_iNumFileLoads;
	AbiWidget_ViewListener * m_pViewListener;
	bool                 m_bShowMargin;
	bool                 m_bWordSelections;
	UT_UTF8String *      m_sMIMETYPE;
	gint                 m_iContentLength;
	gint                 m_iSelectionLength;
#ifdef WITH_BONOBO
	BonoboUIComponent    * m_uic;
#endif
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
  MIMETYPE,
  CONTENT,
  SELECTION,
  CONTENT_LENGTH,
  SELECTION_LENGTH,
  SHADOW_TYPE,
  ARG_LAST
};

// our parent class
static GtkBinClass * parent_class = 0;

static void s_abi_widget_map_cb(GObject * w,  GdkEvent *event,gpointer p);

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
                                           GValue       *return_value,
                                           guint         n_param_values,
                                           const GValue *param_values,
                                           gpointer      invocation_hint,
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

  g_return_if_fail (n_param_values == 4);

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
#define FIRE_UTF8STRING(query, var, fire) do { const UT_UTF8String& val = (query); if (val != var) { var = val; fire(val.utf8_str()); } } while(0)

#define FIRE_BOOL_CHARFMT(prop, prop_val, multiple, var, fire) do {\
const gchar * sz = UT_getAttribute(prop, props_in); \
if (sz) \
{ bool val; \
if (multiple) \
val = (NULL != strstr(sz, prop_val)); \
else \
val = (0 == strcmp(sz, prop_val)); \
if (val != var) { \
var = val; \
fire(var); \
} \
} \
} while(0)

#define FIRE_DOUBLE_CHARFMT(prop, var, fire) do { const gchar * sz = UT_getAttribute(prop, props_in); if (sz) { double val = atof(sz); if (val != var) { var = val; fire(val); } } } while(0) 

#define FIRE_STRING_CHARFMT(prop, var, fire) do { const gchar * sz = UT_getAttribute(prop, props_in); if (sz) { if (strcmp(var.utf8_str(), sz) != 0) { var = sz; fire(sz); } } } while(0) 

#define FIRE_COLOR_CHARFMT(prop, var, fire) do { const gchar * sz = UT_getAttribute(prop, props_in); if (sz) { UT_RGBColor val(0,0,0); UT_parseColor(sz, val); if (val != var) { var = val; fire(val); } } } while(0) 

#define TOOLBAR_DELAY 1000 /* in milliseconds */

class Stateful_ViewListener : public AV_Listener
{
public:
	Stateful_ViewListener(AV_View * pView)
		: m_pView(static_cast<FV_View *>(pView))
	{
		init();
	}

	virtual ~Stateful_ViewListener(void)
	{
		m_pView->removeListener(m_lid);
	}
	
	virtual bool notify(AV_View * pView, const AV_ChangeMask mask)
	{
		UT_return_val_if_fail(pView == m_pView, false);

		if ((AV_CHG_FMTCHAR | AV_CHG_MOTION) & mask)
		{
			// get current char properties from pView
			const gchar ** props_in = NULL;
				
			if (!m_pView->getCharFormat(&props_in))
				return true;
			
			// NB: maybe *no* properties are consistent across the selection
			if (props_in && props_in[0])
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

			FIRE_SINT32(_page_count, pageCount_, pageCount); // should be UINT32 ofcourse
			FIRE_SINT32(_current_page, currentPage_, currentPage); // should be UINT32 ofcourse
		}

		if ((AV_CHG_FMTBLOCK | AV_CHG_MOTION) & mask)
		{
			// get current char properties from pView
			const gchar ** props_in = NULL;
			
			if (!m_pView->getBlockFormat(&props_in))
				return true;
			
			// NB: maybe *no* properties are consistent across the selection
			if (props_in && props_in[0])
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

	void setLID(AV_ListenerId lid)
	{
		m_lid = lid;
	}

	virtual void bold(bool value) {}
	virtual void italic(bool value) {}
	virtual void underline(bool value) {}
	virtual void overline(bool value) {}
	virtual void line_through(bool value) {}
	virtual void topline(bool value) {}
	virtual void bottomline(bool value) {}
	virtual void subscript(bool value) {}
	virtual void superscript(bool value) {}
	virtual void color(UT_RGBColor value) {}
	virtual void font_size(double value) {}
	virtual void font_family(const char * value) {}
	virtual void can_undo(bool value) {}
	virtual void can_redo(bool value) {}
	virtual void is_dirty(bool value) {}
	virtual void leftAlign(bool value) {}
	virtual void rightAlign(bool value) {}
	virtual void centerAlign(bool value) {}
	virtual void justifyAlign(bool value) {}
	virtual void styleName(const char * value) {}
	virtual void textSelected(bool value) {}
	virtual void imageSelected(bool value) {}
	virtual void selectionCleared(bool value) {}
	virtual void enterSelection(bool value) {}
	virtual void leaveSelection(bool value) {}
	virtual void tableState(bool value) {}
	virtual void pageCount(guint32 value) {}
	virtual void currentPage(guint32 value) {}
	virtual void zoomPercentage(gint32 value) {}

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
	// hook up a FV_View listener to the widget. This will let the widget
	// fire GObject signals when things in the view change
	AbiPrivData * private_data = (AbiPrivData *)widget->priv;
	_abi_widget_releaseListener(widget);
	
	private_data->m_pViewListener = new AbiWidget_ViewListener(widget, pView);
	UT_ASSERT(private_data->m_pViewListener);

	AV_ListenerId lid;
	bool bResult = pView->addListener(static_cast<AV_Listener *>(private_data->m_pViewListener), &lid);
	UT_ASSERT(bResult);
	private_data->m_pViewListener->setLID(lid);

	// notify the listener that a new view has been bound
	private_data->m_pViewListener->notify(pView, AV_CHG_ALL);

	return bResult;
}

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


extern "C" gboolean
abi_widget_set_show_margin(AbiWidget * abi, gboolean gb)
{
	bool b = static_cast<bool>(gb);
	if(abi->priv->m_bShowMargin == b)
		return gb;
	abi->priv->m_bShowMargin = b;
	if(!abi->priv->m_bMappedToScreen)
	{
		return gb;
	}
	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
		return gb;
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	if(!pView)
		return gb;
	static_cast<AP_Frame *>(pFrame)->setShowMargin(b);
	pView->setViewMode(pView->getViewMode());
	if(pFrame->getZoomType() == XAP_Frame::z_PAGEWIDTH)
	{
		UT_uint32 iZoom =  pView->calculateZoomPercentForPageWidth();
		pFrame->quickZoom(iZoom);
	}
	return gb;
}


extern "C" gboolean
abi_widget_get_show_margin(AbiWidget * abi)
{
	return static_cast<gboolean>(abi->priv->m_bShowMargin);
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
	_abi_widget_releaseListener(abi);
	abi_widget_invoke(abi,"fileOpen");
	AP_UnixFrame * pFrame = static_cast<AP_UnixFrame *>(abi->priv->m_pFrame);
	_abi_widget_bindListenerToView(abi, pFrame->getCurrentView());
	FV_View * pView = static_cast<FV_View *>(abi->priv->m_pFrame->getCurrentView());
	
	PD_Document *pDoc = pView->getDocument();
	IEFileType ieft  = pDoc->getLastOpenedType();
	if(ieft < 0)
	{
		ieft =  IE_Exp::fileTypeForSuffix(".abw");
	}
	*(abi->priv->m_sMIMETYPE) =  IE_Exp::descriptionForFileType(ieft);
	return TRUE;
}

/*!
 * Extract all the content in the document.
 * Caller must g_free the memory.
 * Number of bytes is returned in iLength
 */
extern "C" gchar *
abi_widget_get_content(AbiWidget * w, char * mimetype, gint * iLength)
{
	// Don't put this auto-save in the most recent list.
	XAP_App::getApp()->getPrefs()->setIgnoreNextRecent();
	GsfOutputMemory* sink = GSF_OUTPUT_MEMORY(gsf_output_memory_new());

	IEFileType ieft = IE_Exp::fileTypeForMimetype(mimetype);
	if(IEFT_Unknown == ieft)
		ieft = IE_Exp::fileTypeForSuffix(mimetype);
	if(IEFT_Unknown == ieft)
		ieft = IE_Exp::fileTypeForSuffix(".abw");
	*(w->priv->m_sMIMETYPE) =  IE_Exp::descriptionForFileType(ieft);


	XAP_Frame * pFrame = w->priv->m_pFrame;
	if(!pFrame)
		return NULL;
	FV_View * view = reinterpret_cast<FV_View *>(pFrame->getCurrentView());
	if(view == NULL)
		return NULL;
	PD_Document * doc = view->getDocument () ;
	UT_Error result = const_cast<PD_Document*>(doc)->saveAs(GSF_OUTPUT(sink),ieft, true);
	if(result != UT_OK)
		return NULL;
	gsf_output_close(GSF_OUTPUT(sink));
	guint32 size = gsf_output_size (GSF_OUTPUT(sink));
	const guint8* ibytes = gsf_output_memory_get_bytes (sink);
	gchar * szOut = new gchar[size+1];
	memcpy(szOut,ibytes,size);
	szOut[size] = 0;
	g_object_unref(G_OBJECT(sink));
	*iLength = size+1;
	w->priv->m_iContentLength = size+1;
	return szOut;
}


/*!
 * Extract all the content in the selection
 * Caller must g_free the memory.
 * Number of bytes is returned in iLength
 */
extern "C" gchar *
abi_widget_get_selection(AbiWidget * w, gchar * mimetype,gint * iLength)
{
	// Don't put this auto-save in the most recent list.
	XAP_App::getApp()->getPrefs()->setIgnoreNextRecent();
	GsfOutputMemory* sink = GSF_OUTPUT_MEMORY(gsf_output_memory_new());

	IEFileType ieft = IE_Exp::fileTypeForMimetype(mimetype);
	if(IEFT_Unknown == ieft)
		ieft = IE_Exp::fileTypeForSuffix(mimetype);
	if(IEFT_Unknown == ieft)
		ieft = IE_Exp::fileTypeForSuffix(".abw");
	*(w->priv->m_sMIMETYPE) =  IE_Exp::descriptionForFileType(ieft);

	XAP_Frame * pFrame = w->priv->m_pFrame;
	if(!pFrame)
		return NULL;
	FV_View * view = reinterpret_cast<FV_View *>(pFrame->getCurrentView());
	if(view == NULL)
		return NULL;
	PD_Document * doc = view->getDocument () ;
	if(view->isSelectionEmpty())
		return NULL;
	PT_DocPosition low = view->getSelectionAnchor();
	PT_DocPosition high = view->getPoint();
	if(high < low)
	{
			PT_DocPosition swap = low;
			low = high;
			high = swap;
	}
	PD_DocumentRange * pDocRange = new 	PD_DocumentRange(doc,low,high);
	UT_ByteBuf buf;
	IE_Exp * pie = NULL;
	UT_Error errorCode;	
	IEFileType newFileType;
	errorCode = IE_Exp::constructExporter(doc,GSF_OUTPUT(sink), ieft, &pie, &newFileType);
	if (errorCode)
		return NULL;
	pie->copyToBuffer(pDocRange,&buf);
	guint32 size = buf.getLength();
	gchar * szOut = new gchar[size+1];
	memcpy(szOut,buf.getPointer(0),size);
	szOut[size] = 0;
	g_object_unref(G_OBJECT(sink));
	*iLength = size+1;
	w->priv->m_iSelectionLength = size+1;
	return szOut;
}


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

extern "C" gboolean
abi_widget_insert_table(AbiWidget * abi, gint32 rows, gint32 cols)
{
	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
		return FALSE;
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	pView->cmdInsertTable(rows,cols,NULL);
	return TRUE;
}


extern "C" gboolean
abi_widget_set_font_name(AbiWidget * w, gchar * szName)
{
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	g_return_val_if_fail ( szName, false );
	
	return abi_widget_invoke_ex (w,"fontFamily",szName,0,0);
}

extern "C" gboolean
abi_widget_set_font_size(AbiWidget * w, gchar * szSize)
{
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	g_return_val_if_fail ( szSize, false );	
	
	return abi_widget_invoke_ex (w,"fontSize",szSize,0,0);
}

extern "C" gboolean
abi_widget_set_style(AbiWidget * w, gchar * szName)
{
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );	
	
	g_return_val_if_fail ( szName, false );
	
	AP_UnixFrame * pFrame = (AP_UnixFrame *) w->priv->m_pFrame;
	g_return_val_if_fail(pFrame, false);

	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	g_return_val_if_fail(pView, false);
	
	bool res = pView->setStyle(szName, false);
	pView->notifyListeners(AV_CHG_MOTION | AV_CHG_HDRFTR); // I stole this mask from ap_EditMethods; looks weird to me though - MARCM
	
	return res;
}

extern "C" gboolean
abi_widget_insert_image(AbiWidget * w, char* szFile, gboolean positioned)
{
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );		
	
	AP_UnixFrame * pFrame = (AP_UnixFrame *) w->priv->m_pFrame;
	g_return_val_if_fail(pFrame, false);

	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	g_return_val_if_fail(pView, false);	
	
	g_return_val_if_fail(szFile, false);

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
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	// get the view
	FV_View * pView = static_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
	g_return_val_if_fail(pView, FALSE );

	// create the color property
	gchar pszColor[12];
	snprintf(pszColor, 12, "%02x%02x%02x", red, green, blue);
	const gchar * properties[] = { "color", pszColor, 0};

	// set the color
	return pView->setCharFormat(properties);
}

extern "C" const gchar**
abi_widget_get_font_names (AbiWidget * w)
{
	// this is annoying asc getAllFontNames() returns a lot of dupes
	const std::vector<const char *> vFonts = GR_UnixPangoGraphics::getAllFontNames();

	const gchar** fonts_ar = 
		reinterpret_cast<const gchar**>(g_malloc(sizeof(gchar*) * (vFonts.size() + 1))); // if there are any dupes, this will be too big, but we don't care
	UT_uint32 i;
	UT_uint32 actual_size = 0;
	for	(i = 0; i < vFonts.size(); i++)
	{
		if (vFonts[i] != NULL && (*vFonts[i]) != '\0')
		{
			// check for dupes
			UT_uint32 j;
			for (j = 0; j < actual_size; j++)
				if (strcmp(vFonts[i], fonts_ar[j]) == 0)
					break;

			if (j == actual_size)
				fonts_ar[actual_size++] = vFonts[i];
		}
	}
	fonts_ar[actual_size] = NULL;
	return fonts_ar;
}

extern "C" gboolean
abi_widget_load_file(AbiWidget * abi, const char * pszFile, const char * mimetype)
{
	UT_DEBUGMSG(("abi_widget_load_file() - file: %s\n", pszFile));

	IEFileType ieft = IEFT_Unknown;
	if (mimetype && *mimetype != '\0')
	{
		ieft = IE_Exp::fileTypeForMimetype(mimetype);
		if(ieft == IEFT_Unknown)
			ieft = IE_Exp::fileTypeForSuffix(mimetype);
	}
	UT_DEBUGMSG(("Will use ieft %d to load file\n", ieft));

	if(abi->priv->m_szFilename)
		g_free(abi->priv->m_szFilename);
	abi->priv->m_szFilename = g_strdup(pszFile);
	if(!abi->priv->m_bMappedToScreen)
	{
	  UT_DEBUGMSG(("Widget not mapped to screen yet, delaying file loading...\n"));
	  abi->priv->m_bPendingFile = true;
	  return FALSE;
	}
	if(abi->priv->m_iNumFileLoads > 0)
	{
		UT_DEBUGMSG(("abi->priv->m_iNumFileLoads > 0, canceling file loading...\n"));
		return FALSE;
	}

	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
	{
		UT_DEBUGMSG(("No frame yet, canceling file loading...\n"));
		return FALSE;
	}
	s_StartStopLoadingCursor( true, pFrame);
//
// First draw blank document
//
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail(pFrameData, false);
	pFrameData->m_pViewMode = VIEW_NORMAL;
	pFrameData->m_bShowRuler = false;
	pFrameData->m_bIsWidget = true;
	if(abi->priv->m_bShowMargin)
		static_cast<AP_Frame *>(pFrame)->setShowMargin(true);
	else
		static_cast<AP_Frame *>(pFrame)->setShowMargin(false);
	if(abi->priv->m_bWordSelections)
		static_cast<AP_Frame *>(pFrame)->setDoWordSelections(true);
	else
		static_cast<AP_Frame *>(pFrame)->setDoWordSelections(false);

	pFrame->loadDocument(NULL,IEFT_Unknown ,true);
	pFrame->toggleRuler(false);
	pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);
//
//Now load the file
//
	UT_DEBUGMSG(("ATTempting to load %s \n",abi->priv->m_szFilename));
	/*bool res=*/ (pFrame->loadDocument(abi->priv->m_szFilename, ieft, true) == UT_OK);
	s_StartStopLoadingCursor( false, pFrame);
	abi->priv->m_bPendingFile = false;
	abi->priv->m_iNumFileLoads += 1;
	if(abi->priv->m_bUnlinkFileAfterLoad)
	{
	  remove(pszFile);
	  abi->priv->m_bUnlinkFileAfterLoad = false;
	}

	// todo: this doesn't belong here. it should be bound as soon as the frame has a view,
	// todo: or whenever the frame changes its view, such as a document load
	UT_DEBUGMSG(("About to bind listener to view \n"));
	_abi_widget_bindListenerToView(abi, pFrame->getCurrentView());
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	if(pFrame->getZoomType() == XAP_Frame::z_PAGEWIDTH)
	{
		UT_uint32 iZoom =  pView->calculateZoomPercentForPageWidth();
		pFrame->quickZoom(iZoom);
	}
	PD_Document *pDoc = pView->getDocument();
	xxx_UT_DEBUGMSG(("Document from view is %x \n",pDoc));
	// check that we could open the document as the type we assumed it was
	// TODO: is this really needed?
	ieft  = pDoc->getLastOpenedType();
	xxx_UT_DEBUGMSG(("Document Type loaded  is %d \n", ieft));
	if(ieft < 0)
	{
		ieft =  IE_Exp::fileTypeForSuffix(".abw");
	}
	xxx_UT_DEBUGMSG(("Setting MIMETYPE %s \n",szMIME));
	xxx_UT_DEBUGMSG(("m_sMIMETYPE pointer %x \n",abi->priv->m_sMIMETYPE));
	*(abi->priv->m_sMIMETYPE) =  IE_Exp::descriptionForFileType(ieft);

	return TRUE;
}

extern "C" gboolean
abi_widget_load_file_from_gsf(AbiWidget * abi, GsfInput * input)
{
	if(abi->priv->m_szFilename)
		g_free(abi->priv->m_szFilename);
	abi->priv->m_szFilename = g_strdup(gsf_input_name (input));
	if(!abi->priv->m_bMappedToScreen)
	{
	  abi->priv->m_bPendingFile = true;
	  return FALSE;
	}
	if(abi->priv->m_iNumFileLoads > 0)
	{
		return FALSE;
	}

	AP_UnixFrame * pFrame = (AP_UnixFrame *) abi->priv->m_pFrame;
	if(pFrame == NULL)
		return FALSE;
	s_StartStopLoadingCursor( true, pFrame);
//
// First draw blank document
//
	AP_FrameData *pFrameData = static_cast<AP_FrameData *>(pFrame->getFrameData());
	UT_return_val_if_fail(pFrameData, false);
	pFrameData->m_pViewMode = VIEW_NORMAL;
	pFrameData->m_bShowRuler = false;
	pFrameData->m_bIsWidget = true;
	if(abi->priv->m_bShowMargin)
		static_cast<AP_Frame *>(pFrame)->setShowMargin(true);
	else
		static_cast<AP_Frame *>(pFrame)->setShowMargin(false);
	if(abi->priv->m_bWordSelections)
		static_cast<AP_Frame *>(pFrame)->setDoWordSelections(true);
	else
		static_cast<AP_Frame *>(pFrame)->setDoWordSelections(false);

	pFrame->loadDocument(NULL,IEFT_Unknown ,true);
	pFrame->toggleRuler(false);
	pFrame->setCursor(GR_Graphics::GR_CURSOR_WAIT);
//
//Now load the file
//
	UT_DEBUGMSG(("ATTempting to load %s \n",abi->priv->m_szFilename));
	/*bool res=*/ ( UT_OK == pFrame->loadDocument(input,IEFT_Unknown));
	s_StartStopLoadingCursor( false, pFrame);
	abi->priv->m_bPendingFile = false;
	abi->priv->m_iNumFileLoads += 1;

	// todo: this doesn't belong here. it should be bound as soon as the frame has a view,
	// todo: or whenever the frame changes its view, such as a document load
	UT_DEBUGMSG(("About to bind listener to view \n"));
	_abi_widget_bindListenerToView(abi, pFrame->getCurrentView());
	FV_View * pView = static_cast<FV_View *>(pFrame->getCurrentView());
	if(pFrame->getZoomType() == XAP_Frame::z_PAGEWIDTH)
	{
		UT_uint32 iZoom =  pView->calculateZoomPercentForPageWidth();
		pFrame->quickZoom(iZoom);
	}
	PD_Document *pDoc = pView->getDocument();
	xxx_UT_DEBUGMSG(("Document from view is %x \n",pDoc));
	IEFileType ieft  = pDoc->getLastOpenedType();
	xxx_UT_DEBUGMSG(("Document Type loaded  is %d \n",ieft));
	if(ieft < 0)
	{
		ieft =  IE_Exp::fileTypeForSuffix(".abw");
	}
	xxx_UT_DEBUGMSG(("Setting MIMETYPE %s \n",szMIME));
	xxx_UT_DEBUGMSG(("m_sMIMETYPE pointer %x \n",abi->priv->m_sMIMETYPE));
	*(abi->priv->m_sMIMETYPE) =  IE_Exp::descriptionForFileType(ieft);

	return TRUE;
}

static gint s_abi_widget_load_file(gpointer p)
{
  AbiWidget * abi = (AbiWidget *) p;

  if(!abi->priv->m_bMappedToScreen)
  {
	abi_widget_map_to_screen(abi);
	return TRUE;
  }
  if(abi->priv->m_bPendingFile)
  {
	char * pszFile = g_strdup(abi->priv->m_szFilename);
	abi_widget_load_file(abi, pszFile, NULL); // Ugly! s_abi_widget_load_file should be removed - MARCM
	g_free(pszFile);
  }
  return FALSE;
}

static void s_abi_widget_map_cb(GObject * w,  GdkEvent *event,gpointer p)
{
  AbiWidget * abi = ABI_WIDGET(p);

  if(!abi->priv->m_bMappedEventProcessed)
  {
	  abi->priv->m_bMappedEventProcessed = true;
//
// Can't load until this event has finished propagating
//
	  g_idle_add(static_cast<GSourceFunc>(s_abi_widget_load_file),static_cast<gpointer>(abi));
  }
}

//
// arguments to abiwidget
//
static void abi_widget_get_prop (GObject  *object,
								 guint arg_id,
								 GValue     *arg,
								 GParamSpec *pspec)
{
    AbiWidget * abi = ABI_WIDGET(object);
	switch(arg_id)
	{
	    case UNLINK_AFTER_LOAD:
		{
			g_value_set_boolean(arg,(gboolean) abi->priv->m_bUnlinkFileAfterLoad);
			break;
		}
	    case MIMETYPE:
		{
			if(abi->priv->m_sMIMETYPE->size() == 0)
			{
				    if(abi->priv->m_pFrame)
					{
						FV_View * pView = static_cast<FV_View *>(abi->priv->m_pFrame->getCurrentView());
						
						PD_Document *pDoc = pView->getDocument();
						IEFileType ieft  = pDoc->getLastOpenedType();
						if(ieft < 0)
						{
								ieft =  IE_Exp::fileTypeForSuffix(".abw");
						}
						*(abi->priv->m_sMIMETYPE) =  IE_Exp::descriptionForFileType(ieft);
					}
			}
			g_value_set_string(arg,(gchar *) abi->priv->m_sMIMETYPE->utf8_str());
			break;
		}
	    case CONTENT:
		{
			gint i;
			gchar * bytes = abi_widget_get_content(abi,(gchar *) abi->priv->m_sMIMETYPE->utf8_str() , &i);
			g_value_set_string(arg,bytes);
			break;
		}
	    case SELECTION:
		{
			gint i;
			gchar * bytes = abi_widget_get_selection(abi,(gchar *) abi->priv->m_sMIMETYPE->utf8_str() , &i);
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
								 GParamSpec *pspec)
{
  g_return_if_fail (object != 0);

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
		case MIMETYPE:
		{
			*(abi->priv->m_sMIMETYPE) = g_value_get_string(arg);
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
abi_widget_size_request (GtkWidget      *widget,
			 GtkRequisition *requisition)
{
	requisition->width = ABI_DEFAULT_WIDTH;
	requisition->height = ABI_DEFAULT_HEIGHT;

	if (ABI_WIDGET(widget)->child)
	  {
		GtkRequisition child_requisition;
      
		gtk_widget_size_request (ABI_WIDGET(widget)->child, &child_requisition);

		requisition->width = child_requisition.width;
		requisition->height = child_requisition.height;
	  }
}

//
// Needed for the gtkbin class
//
static void
abiwidget_add(GtkContainer *container,
	      GtkWidget    *widget)
{
  g_return_if_fail (container != NULL);
  g_return_if_fail (widget != NULL);

  if (GTK_CONTAINER_CLASS (parent_class)->add)
	  GTK_CONTAINER_CLASS (parent_class)->add (container, widget);

  ABI_WIDGET(container)->child = GTK_BIN (container)->child;
}

//
// Needed for the gtkbin class
//
static void
abiwidget_remove (GtkContainer *container,
		  GtkWidget    *widget)
{
  g_return_if_fail (container != NULL);
  g_return_if_fail (widget != NULL);

  if (GTK_CONTAINER_CLASS (parent_class)->remove)
    GTK_CONTAINER_CLASS (parent_class)->remove (container, widget);

  ABI_WIDGET(container)->child = GTK_BIN (container)->child;
}

//
// Needed for the gtkbin class
//
static GtkType
abiwidget_child_type (GtkContainer *container)
{
  if (!GTK_BIN (container)->child)
    return GTK_TYPE_WIDGET;
  else
    return G_TYPE_NONE;
}

static void
abi_widget_init (AbiWidget * abi)
{
	AbiPrivData * priv = g_new0 (AbiPrivData, 1);
	priv->m_pFrame = NULL;
	priv->m_szFilename = NULL;
	priv->m_bMappedToScreen = false;
	priv->m_bPendingFile = false;
	priv->m_bMappedEventProcessed = false;
	priv->m_bUnlinkFileAfterLoad = false;
	priv->m_iNumFileLoads = 0;
	priv->m_bShowMargin = true;
	priv->m_bWordSelections = true;
	priv->m_sMIMETYPE = new UT_UTF8String("");
	priv->m_iContentLength = 0;
	priv->m_iSelectionLength = 0;
	abi->priv = priv;

	// this isn't really needed, since each widget is
	// guaranteed to be created with g_new0 and we just
	// want everything to be 0/NULL/FALSE anyway right now
	// but i'm keeping it around anyway just in case that changes
  GTK_WIDGET_SET_FLAGS (abi, GTK_CAN_FOCUS | GTK_RECEIVES_DEFAULT |GTK_CAN_DEFAULT );
  GTK_WIDGET_UNSET_FLAGS (abi, GTK_NO_WINDOW);
}

static void
abi_widget_size_allocate (GtkWidget     *widget,
						  GtkAllocation *allocation)
{
	AbiWidget *abi;
	
	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_ABI_WIDGET (widget));
	g_return_if_fail (allocation != NULL);

	GtkAllocation child_allocation;
	widget->allocation = *allocation;

	gint border_width = GTK_CONTAINER (widget)->border_width;
	gint xthickness = GTK_WIDGET (widget)->style->xthickness;
	gint ythickness = GTK_WIDGET (widget)->style->ythickness;
 	if (GTK_WIDGET_REALIZED (widget))
    {
		// only allocate on realized widgets

		abi = ABI_WIDGET(widget);
		gdk_window_move_resize (widget->window,
					allocation->x+border_width, 
					allocation->y+border_width,
					allocation->width - border_width*2, 
					allocation->height - border_width*2);
		
		if (abi->child)
		{
		     child_allocation.x = xthickness;
			 child_allocation.y = ythickness;

			 child_allocation.width = MAX (1, 
										   (gint)widget->allocation.width - child_allocation.x * 2 - border_width * 2);
			 child_allocation.height = MAX (1, 
											(gint)widget->allocation.height - child_allocation.y * 2 - border_width * 2);
			 gtk_widget_size_allocate (ABI_WIDGET (widget)->child, &child_allocation);
		}
    }
}

static void
abi_widget_realize (GtkWidget * widget)
{
	AbiWidget * abi;
	GdkWindowAttr attributes;
	gint attributes_mask;

	// we *must* ensure that we get a GdkWindow to draw into
	// this here is just boilerplate GTK+ code

	g_return_if_fail (widget != NULL);
	g_return_if_fail (IS_ABI_WIDGET(widget));

	GTK_WIDGET_SET_FLAGS (widget, GTK_REALIZED);
	abi = ABI_WIDGET(widget);

	attributes.x = widget->allocation.x;
	attributes.y = widget->allocation.y;
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
	attributes.colormap = gtk_widget_get_colormap (widget);
	
	attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;

	widget->window = gdk_window_new (gtk_widget_get_parent_window (widget), &attributes, attributes_mask);
	gdk_window_set_user_data (widget->window, abi);

	widget->style = gtk_style_attach (widget->style, widget->window);
	gtk_style_set_background (widget->style, widget->window, GTK_STATE_NORMAL);

	//
	// connect a signal handler to load files after abiword is in a stable
	// state.
	//
	g_signal_connect_after(G_OBJECT(widget),"map_event", 
			       G_CALLBACK (s_abi_widget_map_cb),
			       (gpointer) abi);
	abi_widget_map_to_screen( abi);
}

static void
abi_widget_destroy_gtk (GtkObject *object)
{
	AbiWidget * abi;
	
	g_return_if_fail (object != NULL);
	g_return_if_fail (IS_ABI_WIDGET(object));

	// here we g_free any self-created data
	abi = ABI_WIDGET(object);

	// order of deletion is important here
	XAP_App *pApp = XAP_App::getApp();
	bool bBonobo = false;
	bool bKillApp = false;

	if(abi->priv) 
		{
			_abi_widget_releaseListener(abi);
			bBonobo = pApp->isBonoboRunning();
			if(abi->priv->m_pFrame)
				{
#ifdef LOGFILE
					fprintf(getlogfile(),"frame count before forgetting = %d \n",pApp->getFrameCount());
#endif
					if(pApp->getFrameCount() <= 1)
						{
							bKillApp = true;
						}
					pApp->forgetFrame(abi->priv->m_pFrame);
					abi->priv->m_pFrame->close();
					delete abi->priv->m_pFrame;
#ifdef LOGFILE
					fprintf(getlogfile(),"frame count = %d \n",pApp->getFrameCount());
#endif
				}
			g_free (abi->priv->m_szFilename);
			delete abi->priv->m_sMIMETYPE;
			g_free (abi->priv);
			abi->priv = NULL;
		}

#ifdef LOGFILE
	fprintf(getlogfile(),"abiwidget destroyed in abi_widget_destroy_gtk\n");
#endif

#ifdef WITH_BONOBO
	if(bBonobo)
	{
		if (GTK_OBJECT_CLASS(parent_class)->destroy)
			GTK_OBJECT_CLASS(parent_class)->destroy (GTK_OBJECT(object));
	// chain up
		BONOBO_CALL_PARENT (BONOBO_OBJECT_CLASS, destroy, BONOBO_OBJECT(object));
		if(bKillApp)
		{
			bonobo_main_quit();
		}
	}
#endif
}


static void
abi_widget_class_init (AbiWidgetClass *abi_class)
{

#ifdef LOGFILE
	fprintf(getlogfile(),"Abi_widget class init \n");
#endif

	GObjectClass * gobject_class;
	GtkObjectClass * object_class;
	GtkWidgetClass * widget_class;
	GtkContainerClass *container_class;
	container_class = (GtkContainerClass*) abi_class;

	gobject_class = (GObjectClass *)abi_class;
	object_class = (GtkObjectClass *)abi_class;
	widget_class = (GtkWidgetClass *)abi_class;

	// set our parent class
	parent_class = (GtkBinClass *)
		gtk_type_class (gtk_bin_get_type());
	
	// set our custom destroy method
	object_class->destroy = abi_widget_destroy_gtk;
	gobject_class->set_property = abi_widget_set_prop;
	gobject_class->get_property = abi_widget_get_prop;

	// set our custom class methods
	widget_class->realize       = abi_widget_realize;
	widget_class->size_request  = abi_widget_size_request;
   	widget_class->size_allocate = abi_widget_size_allocate; 

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
								  MIMETYPE,
								  g_param_spec_string("mimetype",
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
								  g_param_spec_string("content_length",
													   NULL,
													   NULL,
													   FALSE,
													   static_cast<GParamFlags>(G_PARAM_READABLE)));

 g_object_class_install_property(gobject_class,
								  SELECTION_LENGTH,
								  g_param_spec_string("selection_length",
													   NULL,
													   NULL,
													   FALSE,
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
abi_widget_construct (AbiWidget * abi, const char * file)
{
	// this is all that we can do here, because we can't draw until we're
	// realized and have a GdkWindow pointer

	if (file)
		abi->priv->m_szFilename = g_strdup (file);
		
#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget Constructed %x \n",abi);
#endif
}

/**************************************************************************/
/**************************************************************************/

extern "C" void 
abi_widget_map_to_screen(AbiWidget * abi)
{
  g_return_if_fail (abi != 0);
  UT_DEBUGMSG(("Doing map_to_screen \n"));
  GtkWidget * widget = GTK_WIDGET(abi);
  if(abi->priv->m_bMappedToScreen)
  {
	  return;
  }
  // now we can set up Abi inside of this GdkWindow

#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget about to map_to_screen \n");
	fprintf(getlogfile(),"AbiWidget about to map_to_screen ref_count %d \n",G_OBJECT(abi)->ref_count);
#endif

	abi->priv->m_bMappedToScreen = true;
	AP_UnixFrame * pFrame  = new AP_UnixFrame();
	UT_ASSERT(pFrame);
	static_cast<XAP_UnixFrameImpl *>(pFrame->getFrameImpl())->setTopLevelWindow(widget); 
	pFrame->initialize(XAP_NoMenusWindowLess);
	abi->priv->m_pFrame = pFrame;

	XAP_App::getApp()->rememberFrame ( pFrame ) ;
	XAP_App::getApp()->rememberFocussedFrame ( pFrame ) ;

	//_abi_widget_bindListenerToView(abi, pFrame->getCurrentView());

#ifdef LOGFILE
	fprintf(getlogfile(),"AbiWidget After Finished map_to_screen ref_count %d \n",G_OBJECT(abi)->ref_count);
#endif
}

extern "C" void 
abi_widget_turn_on_cursor(AbiWidget * abi)
{
	if(abi->priv->m_pFrame)
	{
		g_return_if_fail (abi != 0);
		FV_View * pV = static_cast<FV_View*>(abi->priv->m_pFrame->getCurrentView());
		if(pV)
			pV->focusChange(AV_FOCUS_HERE);
	}
}

extern "C" GType
abi_widget_get_type (void)
{
	static GType abi_type = 0;

	if (!abi_type){
		GTypeInfo info = {
			sizeof (AbiWidgetClass),
			NULL,
			NULL,
			(GClassInitFunc)abi_widget_class_init,
			NULL,
			NULL,
			sizeof(AbiWidget),
			0,
			(GInstanceInitFunc)abi_widget_init,
		};
		
		abi_type = g_type_register_static (gtk_bin_get_type (), "AbiWidget",
										   &info, (GTypeFlags)0);
	}
	
	return abi_type;
}

/**
 * abi_widget_new
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
 * abi_widget_new_with_file
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

	g_return_val_if_fail (file != 0, 0);

	abi = static_cast<AbiWidget *>(g_object_new (abi_widget_get_type (), NULL));
	abi_widget_construct (abi, file);

	return GTK_WIDGET (abi);
}

extern "C" XAP_Frame * 
abi_widget_get_frame ( AbiWidget * w )
{
  g_return_val_if_fail ( w != NULL, NULL ) ;
  return w->priv->m_pFrame ;
}

#ifdef WITH_BONOBO
extern "C" void
abi_widget_set_Bonobo_uic(AbiWidget * w, BonoboUIComponent * uic)
{
	w->priv->m_uic = uic;
}

extern "C" BonoboUIComponent *
abi_widget_get_Bonobo_uic(AbiWidget * w)
{
	return 	w->priv->m_uic;
}

#endif
/**
 * abi_widget_invoke()
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
 * abi_widget_invoke_ex()
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
	g_return_val_if_fail (w != 0, FALSE);
	g_return_val_if_fail (mthdName != 0, FALSE);

	// get the method container
	XAP_App *pApp = XAP_App::getApp();
	container = pApp->getEditMethodContainer();
	g_return_val_if_fail (container != 0, FALSE);

	// get a handle to the actual EditMethod
	method = container->findEditMethodByName (mthdName);
	g_return_val_if_fail (method != 0, FALSE);

	// get a valid frame
	g_return_val_if_fail (w->priv->m_pFrame != 0, FALSE);

	// obtain a valid view
	view = w->priv->m_pFrame->getCurrentView();
	g_return_val_if_fail (view != 0, FALSE);
	xxx_UT_DEBUGMSG(("Data to invoke %s \n",data));

	// construct the call data
	EV_EditMethodCallData calldata(data, (data ? strlen (data) : 0));
	calldata.m_xPos = x;
	calldata.m_yPos = y;

	// actually invoke
	return (method->Fn(view, &calldata) ? TRUE : FALSE);
}

extern "C" void
abi_widget_draw (AbiWidget * w)
{
	// obtain a valid view
	if(w->priv->m_pFrame)
	{
		// obtain a valid view
		g_return_if_fail (w != NULL);
		FV_View * view = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
		if(view)
			view->draw();
	}
}

extern "C" gboolean
abi_widget_save ( AbiWidget * w, const char * fname )
{
  return abi_widget_save_with_type ( w, fname, ".abw" ) ;
}

static IEFileType getImportFileType(const char * szSuffixOrMime)
{
  IEFileType ieft = IEFT_Unknown;

  if(szSuffixOrMime && *szSuffixOrMime) {
    IE_Imp::fileTypeForMimetype(szSuffixOrMime);
    if(ieft == IEFT_Unknown) {
      UT_String suffix;

      if(*szSuffixOrMime != '.')
		  suffix = ".";
      suffix += szSuffixOrMime;
      ieft = IE_Imp::fileTypeForSuffix(suffix.c_str());
    }
  }

  return ieft;
}

extern "C" gboolean 
abi_widget_save_with_type ( AbiWidget * w, const char * fname,
							const char * extension )
{
  g_return_val_if_fail ( w != NULL, FALSE );
  g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
  g_return_val_if_fail ( fname != NULL, FALSE );

  FV_View * view = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
  if(view == NULL)
	  return false;
  PD_Document * doc = view->getDocument () ;

  IEFileType ieft = getImportFileType (extension);
  if (ieft == IEFT_Unknown)
	  ieft = getImportFileType (".abw");

  return ( static_cast<AD_Document*>(doc)->saveAs ( fname, ieft ) == UT_OK ? TRUE : FALSE ) ;
}

extern "C" gboolean 
abi_widget_save_to_gsf ( AbiWidget * w, GsfOutput * output,
						 const char * extension )
{
  g_return_val_if_fail ( w != NULL, FALSE );
  g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
  g_return_val_if_fail ( output != NULL, FALSE );

  FV_View * view = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
  if(view == NULL)
	  return false;
  PD_Document * doc = view->getDocument () ;

  IEFileType ieft = getImportFileType (extension);
  if (ieft == IEFT_Unknown)
	  ieft = getImportFileType (".abw");

  return ( static_cast<PD_Document*>(doc)->saveAs ( output, ieft ) == UT_OK ? TRUE : FALSE ) ;
}

extern "C" gboolean 
abi_widget_save_ext ( AbiWidget * w, const char * fname,
					  const char * extension )
{
	return abi_widget_save_with_type (w, fname, extension);
}

extern "C" gboolean 
abi_widget_set_zoom_percentage (AbiWidget * w, guint32 zoom)
{
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	w->priv->m_pFrame->setZoomType(XAP_Frame::z_PERCENT);
	w->priv->m_pFrame->quickZoom(zoom);
	return TRUE;
}

extern "C" guint32 
abi_widget_get_zoom_percentage (AbiWidget * w)
{
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	return w->priv->m_pFrame->getZoomPercentage();
}

extern "C" void
abi_widget_set_find_string(AbiWidget * w, gchar * search_str)
{
	// TODO: implement me
}

extern "C" gchar*
abi_widget_find_next(AbiWidget * w)
{
	// TODO: implement me
	return 0;
}

extern "C" guint32
abi_widget_get_page_count(AbiWidget * w)
{
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	FV_View * pView = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
	UT_return_val_if_fail(pView, 0);

	FL_DocLayout* pLayout = pView->getLayout();
	UT_return_val_if_fail(pLayout, 0);

	return pLayout->countPages();
}

extern "C" guint32
abi_widget_get_current_page_num(AbiWidget * w)
{
	g_return_val_if_fail ( w != NULL, FALSE );
	g_return_val_if_fail ( IS_ABI_WIDGET(w), FALSE );
	g_return_val_if_fail ( w->priv->m_pFrame, FALSE );

	FV_View * pView = reinterpret_cast<FV_View *>(w->priv->m_pFrame->getCurrentView());
	UT_return_val_if_fail(pView, 0);

	// there's also getCurrentPageNumber() we can use, but that is slower
	return pView->getCurrentPageNumForStatusBar();
}

