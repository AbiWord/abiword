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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
#ifndef ABI_WIDGET_H
#define ABI_WIDGET_H

#include <gtk/gtk.h>
#include <gsf/gsf-input.h>
#include <gsf/gsf-output.h>

G_BEGIN_DECLS

/**************************************************************************/
/**************************************************************************/

// needed for our custom gclosure marchallers
#include        <glib-object.h>

#ifdef G_ENABLE_DEBUG
#define g_marshal_value_peek_boolean(v)  g_value_get_boolean (v)
#define g_marshal_value_peek_char(v)     g_value_get_char (v)
#define g_marshal_value_peek_uchar(v)    g_value_get_uchar (v)
#define g_marshal_value_peek_int(v)      g_value_get_int (v)
#define g_marshal_value_peek_uint(v)     g_value_get_uint (v)
#define g_marshal_value_peek_long(v)     g_value_get_long (v)
#define g_marshal_value_peek_ulong(v)    g_value_get_ulong (v)
#define g_marshal_value_peek_int64(v)    g_value_get_int64 (v)
#define g_marshal_value_peek_uint64(v)   g_value_get_uint64 (v)
#define g_marshal_value_peek_enum(v)     g_value_get_enum (v)
#define g_marshal_value_peek_flags(v)    g_value_get_flags (v)
#define g_marshal_value_peek_float(v)    g_value_get_float (v)
#define g_marshal_value_peek_double(v)   g_value_get_double (v)
#define g_marshal_value_peek_string(v)   (char*) g_value_get_string (v)
#define g_marshal_value_peek_param(v)    g_value_get_param (v)
#define g_marshal_value_peek_boxed(v)    g_value_get_boxed (v)
#define g_marshal_value_peek_pointer(v)  g_value_get_pointer (v)
#define g_marshal_value_peek_object(v)   g_value_get_object (v)
#else /* !G_ENABLE_DEBUG */
/* WARNING: This code accesses GValues directly, which is UNSUPPORTED API.
 *          Do not access GValues directly in your code. Instead, use the
 *          g_value_get_*() functions
 */
#define g_marshal_value_peek_boolean(v)  (v)->data[0].v_int
#define g_marshal_value_peek_char(v)     (v)->data[0].v_int
#define g_marshal_value_peek_uchar(v)    (v)->data[0].v_uint
#define g_marshal_value_peek_int(v)      (v)->data[0].v_int
#define g_marshal_value_peek_uint(v)     (v)->data[0].v_uint
#define g_marshal_value_peek_long(v)     (v)->data[0].v_long
#define g_marshal_value_peek_ulong(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_int64(v)    (v)->data[0].v_int64
#define g_marshal_value_peek_uint64(v)   (v)->data[0].v_uint64
#define g_marshal_value_peek_enum(v)     (v)->data[0].v_long
#define g_marshal_value_peek_flags(v)    (v)->data[0].v_ulong
#define g_marshal_value_peek_float(v)    (v)->data[0].v_float
#define g_marshal_value_peek_double(v)   (v)->data[0].v_double
#define g_marshal_value_peek_string(v)   (v)->data[0].v_pointer
#define g_marshal_value_peek_param(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_boxed(v)    (v)->data[0].v_pointer
#define g_marshal_value_peek_pointer(v)  (v)->data[0].v_pointer
#define g_marshal_value_peek_object(v)   (v)->data[0].v_pointer
#endif /* !G_ENABLE_DEBUG */

/**************************************************************************/
/**************************************************************************/

#define ABI_TYPE_WIDGET        (abi_widget_get_type ())
#define ABI_WIDGET(obj)        (G_TYPE_CHECK_INSTANCE_CAST((obj), ABI_TYPE_WIDGET, AbiWidget))
#define IS_ABI_WIDGET(obj)     (G_TYPE_CHECK_INSTANCE_TYPE((obj), ABI_TYPE_WIDGET))
#define IS_ABI_WIDGET_CLASS(k) (G_TYPE_CHECK_CLASS_TYPE ((k), ABI_TYPE_WIDGET))
#define ABI_WIDGET_CLASS(k)    (G_TYPE_CHECK_CLASS_CAST ((k), ABI_TYPE_WIDGET, AbiWidgetClass))

  /* forward declarations */
  typedef struct _AbiWidget      AbiWidget;
  typedef struct _AbiWidgetClass AbiWidgetClass;
  typedef struct _AbiPrivData    AbiPrivData;

  /*
   * These functions basically just marshall back their arguments into
   * the AbiWord application and return a boolean
   */
  typedef gboolean (*Abi_EditMethod) (AbiWidget *, const char *, gint32, gint32);
  typedef gboolean (*Abi_Void__Bool_EditMethod) (AbiWidget *);
  typedef gboolean (*Abi_Int_Int__Bool_EditMethod) (AbiWidget *, gint32, gint32);
  typedef gboolean (*Abi_CharPtr__Bool_EditMethod) (AbiWidget *, const char *);

  /*
   * Only here for completeness in that that we might want to
   * add signals (such as "saved") later
   */
  typedef gboolean (*AbiSignal) (AbiWidget *, gpointer closure);

  struct _AbiWidget
  {
    GtkBin bin;
    GtkWidget * child;
    /* private instance data */
    AbiPrivData * priv;
  };

  struct  _AbiWidgetClass {
    GtkBinClass parent_class;

    /* invoke any edit method based on its name */
    gboolean (*invoke) (AbiWidget *, const char * mthdName);
    gboolean (*invoke_ex) (AbiWidget *, const char * mthdName,
			   const char * data, gint32 x, gint32 y);

    /* a list of some of our more useful edit methods */
    Abi_Void__Bool_EditMethod align_center;
    Abi_Void__Bool_EditMethod align_justify;
    Abi_Void__Bool_EditMethod align_left;
    Abi_Void__Bool_EditMethod align_right;

    Abi_Void__Bool_EditMethod copy;
    Abi_Void__Bool_EditMethod cut;
    Abi_Void__Bool_EditMethod paste;
    Abi_Void__Bool_EditMethod paste_special;
    Abi_Void__Bool_EditMethod select_all;
    Abi_Void__Bool_EditMethod select_block;
    Abi_Void__Bool_EditMethod select_line;
    Abi_Void__Bool_EditMethod select_word;

    Abi_Void__Bool_EditMethod undo;
    Abi_Void__Bool_EditMethod redo;

    Abi_CharPtr__Bool_EditMethod insert_data;
    Abi_Void__Bool_EditMethod    insert_space;

    Abi_Void__Bool_EditMethod delete_bob;
    Abi_Void__Bool_EditMethod delete_bod;
    Abi_Void__Bool_EditMethod delete_bol;
    Abi_Void__Bool_EditMethod delete_bow;
    Abi_Void__Bool_EditMethod delete_eob;
    Abi_Void__Bool_EditMethod delete_eod;
    Abi_Void__Bool_EditMethod delete_eol;
    Abi_Void__Bool_EditMethod delete_eow;
    Abi_Void__Bool_EditMethod delete_left;
    Abi_Void__Bool_EditMethod delete_right;

    Abi_Void__Bool_EditMethod edit_header;
    Abi_Void__Bool_EditMethod edit_footer;

    Abi_Void__Bool_EditMethod file_open;
    Abi_Void__Bool_EditMethod file_save;

    Abi_Void__Bool_EditMethod remove_header;
    Abi_Void__Bool_EditMethod remove_footer;

    Abi_Void__Bool_EditMethod save_immediate;

    Abi_Void__Bool_EditMethod select_bob;
    Abi_Void__Bool_EditMethod select_bod;
    Abi_Void__Bool_EditMethod select_bol;
    Abi_Void__Bool_EditMethod select_bow;
    Abi_Void__Bool_EditMethod select_eob;
    Abi_Void__Bool_EditMethod select_eod;
    Abi_Void__Bool_EditMethod select_eol;
    Abi_Void__Bool_EditMethod select_eow;
    Abi_Void__Bool_EditMethod select_left;
    Abi_Void__Bool_EditMethod select_next_line;
    Abi_Void__Bool_EditMethod select_page_down;
    Abi_Void__Bool_EditMethod select_page_up;
    Abi_Void__Bool_EditMethod select_prev_line;
    Abi_Void__Bool_EditMethod select_right;
    Abi_Void__Bool_EditMethod select_screen_down;
    Abi_Void__Bool_EditMethod select_screen_up;
    Abi_Int_Int__Bool_EditMethod select_to_xy;

    Abi_Void__Bool_EditMethod toggle_bold;
    Abi_Void__Bool_EditMethod toggle_underline;
    Abi_Void__Bool_EditMethod toggle_bottomline;
    Abi_Void__Bool_EditMethod toggle_insert_mode;
    Abi_Void__Bool_EditMethod toggle_italic;
    Abi_Void__Bool_EditMethod toggle_overline;
    Abi_Void__Bool_EditMethod toggle_plain;
    Abi_Void__Bool_EditMethod toggle_strike;
    Abi_Void__Bool_EditMethod toggle_sub;
    Abi_Void__Bool_EditMethod toggle_super;
    Abi_Void__Bool_EditMethod toggle_topline;
    Abi_Void__Bool_EditMethod toggle_unindent;
    Abi_Void__Bool_EditMethod toggle_bullets;
    Abi_Void__Bool_EditMethod toggle_numbering;

    Abi_Void__Bool_EditMethod view_formatting_marks;
    Abi_Void__Bool_EditMethod view_print_layout;
    Abi_Void__Bool_EditMethod view_normal_layout;
    Abi_Void__Bool_EditMethod view_online_layout;

    Abi_Void__Bool_EditMethod moveto_bob;
    Abi_Void__Bool_EditMethod moveto_bod;
    Abi_Void__Bool_EditMethod moveto_bol;
    Abi_Void__Bool_EditMethod moveto_bop;
    Abi_Void__Bool_EditMethod moveto_bow;
    Abi_Void__Bool_EditMethod moveto_eob;
    Abi_Void__Bool_EditMethod moveto_eod;
    Abi_Void__Bool_EditMethod moveto_eol;
    Abi_Void__Bool_EditMethod moveto_eop;
    Abi_Void__Bool_EditMethod moveto_eow;
    Abi_Void__Bool_EditMethod moveto_left;
    Abi_Void__Bool_EditMethod moveto_next_line;
    Abi_Void__Bool_EditMethod moveto_next_page;
    Abi_Void__Bool_EditMethod moveto_next_screen;
    Abi_Void__Bool_EditMethod moveto_prev_line;
    Abi_Void__Bool_EditMethod moveto_prev_page;
    Abi_Void__Bool_EditMethod moveto_prev_screen;
    Abi_Void__Bool_EditMethod moveto_right;
    Abi_Int_Int__Bool_EditMethod moveto_to_xy;

    Abi_Void__Bool_EditMethod zoom_whole;
    Abi_Void__Bool_EditMethod zoom_width;

    Abi_EditMethod em_pad[20];

    /* signals */
    void (* signal_bold) (AbiWidget * widget, gboolean value);
    void (* signal_italic) (AbiWidget * widget, gboolean value);
    void (* signal_underline) (AbiWidget * widget, gboolean value);
    void (* signal_overline) (AbiWidget * widget, gboolean value);
    void (* signal_line_through) (AbiWidget * widget, gboolean value);
    void (* signal_topline) (AbiWidget * widget, gboolean value);
    void (* signal_bottomline) (AbiWidget * widget, gboolean value);
    void (* signal_superscript) (AbiWidget * widget, gboolean value);
    void (* signal_subscript) (AbiWidget * widget, gboolean value);
    void (* signal_color) (AbiWidget * widget, int r, int g, int b);
    void (* signal_font_size) (AbiWidget * widget, double value);
    void (* signal_font_family) (AbiWidget * widget, const char * value);
    void (* signal_changed) (AbiWidget * widget, gboolean value);
    void (* signal_can_undo) (AbiWidget * widget, gboolean value);
    void (* signal_can_redo) (AbiWidget * widget, gboolean value);
    void (* signal_is_dirty) (AbiWidget * widget, gboolean value);
    void (* signal_left_align) (AbiWidget * widget, gboolean value);
    void (* signal_right_align) (AbiWidget * widget, gboolean value);
    void (* signal_center_align) (AbiWidget * widget, gboolean value);
    void (* signal_justify_align) (AbiWidget * widget, gboolean value);
    void (* signal_style_name) (AbiWidget * widget, const char * value);
    void (* signal_text_selected) (AbiWidget * widget, gboolean value);
    void (* signal_image_selected) (AbiWidget * widget, gboolean value);
    void (* signal_selection_cleared) (AbiWidget * widget, gboolean value);
    void (* signal_enter_selection) (AbiWidget * widget, gboolean value);
    void (* signal_leave_selection) (AbiWidget * widget, gboolean value);
    void (* signal_table_state) (AbiWidget * widget, gboolean value);
    void (* signal_page_count) (AbiWidget * widget, guint32 value);
    void (* signal_current_page) (AbiWidget * widget, guint32 value);
    void (* signal_zoom_percentage) (AbiWidget * widget, gint32 value);

    AbiSignal sig_pad[20];
  };

  /* the public API */

  /* widget creation functions */
  GtkWidget * abi_widget_new (void);
  GtkWidget * abi_widget_new_with_file (const gchar * file);
  GType     abi_widget_get_type	(void);
  void        abi_widget_turn_on_cursor(AbiWidget * widget);
  void        abi_widget_draw(AbiWidget * w);

  void abi_widget_set_property(GObject  *object,
			       guint	arg_id,
			       const GValue *arg,
			       GParamSpec *pspec);
  void abi_widget_get_property(GObject  *object,
			       guint arg_id,
			       GValue     *arg,
			       GParamSpec *pspec);

  /* bindings to our more useful edit methods */

  /* file handing functions */
  gboolean abi_widget_file_open (AbiWidget * w);
  gboolean abi_widget_file_save (AbiWidget * w);
  gboolean abi_widget_load_file(AbiWidget * w, const gchar * pszFile, const gchar * mimetype);
  gboolean abi_widget_load_file_from_gsf(AbiWidget * w, GsfInput * input);
  gboolean abi_widget_load_file_from_memory(AbiWidget * w, const gchar * extension_or_mimetype, const gchar * buf, gint length);
  gboolean abi_widget_save ( AbiWidget * w, const gchar * fname, const gchar * extension_or_mimetype,  const gchar * exp_props );
  gboolean abi_widget_save_immediate (AbiWidget * w);
  gboolean abi_widget_save_to_gsf ( AbiWidget * w, GsfOutput * output, const gchar * extension_or_mimetype, const gchar *exp_props);

  /* paragraph modification functions */
  gboolean abi_widget_align_center (AbiWidget * w);
  gboolean abi_widget_align_justify (AbiWidget * w);
  gboolean abi_widget_align_left (AbiWidget * w);
  gboolean abi_widget_align_right (AbiWidget * w);

  /* copy & paste functions */
  gboolean abi_widget_copy (AbiWidget * w);
  gboolean abi_widget_cut (AbiWidget * w);
  gboolean abi_widget_paste (AbiWidget * w);
  gboolean abi_widget_paste_special (AbiWidget * w);
  gboolean abi_widget_select_all (AbiWidget * w);
  gboolean abi_widget_select_block (AbiWidget * w);
  gboolean abi_widget_select_line (AbiWidget * w);
  gboolean abi_widget_select_word (AbiWidget * w);

  /* undo/redo */
  gboolean abi_widget_undo (AbiWidget * w);
  gboolean abi_widget_redo (AbiWidget * w);

  /* text insertion and removal */
  gboolean abi_widget_insert_data (AbiWidget * w, const char * str);
  gboolean abi_widget_insert_space (AbiWidget * w);

  gboolean abi_widget_delete_bob (AbiWidget * w);
  gboolean abi_widget_delete_bod (AbiWidget * w);
  gboolean abi_widget_delete_bol (AbiWidget * w);
  gboolean abi_widget_delete_bow (AbiWidget * w);
  gboolean abi_widget_delete_eob (AbiWidget * w);
  gboolean abi_widget_delete_eod (AbiWidget * w);
  gboolean abi_widget_delete_eol (AbiWidget * w);
  gboolean abi_widget_delete_eow (AbiWidget * w);
  gboolean abi_widget_delete_left (AbiWidget * w);
  gboolean abi_widget_delete_right (AbiWidget * w);

  gchar * abi_widget_get_content (AbiWidget * w, const gchar * extension_or_mimetype, const gchar * exp_props, gint* iLength);
  gchar * abi_widget_get_selection (AbiWidget * w, const gchar * extension_or_mimetype, gint* iLength);

  /* selection functions */
  gboolean abi_widget_select_bob (AbiWidget * w);
  gboolean abi_widget_select_bod (AbiWidget * w);
  gboolean abi_widget_select_bol (AbiWidget * w);
  gboolean abi_widget_select_bow (AbiWidget * w);
  gboolean abi_widget_select_eob (AbiWidget * w);
  gboolean abi_widget_select_eod (AbiWidget * w);
  gboolean abi_widget_select_eol (AbiWidget * w);
  gboolean abi_widget_select_eow (AbiWidget * w);
  gboolean abi_widget_select_left (AbiWidget * w);
  gboolean abi_widget_select_next_line (AbiWidget * w);
  gboolean abi_widget_select_page_down (AbiWidget * w);
  gboolean abi_widget_select_page_up (AbiWidget * w);
  gboolean abi_widget_select_prev_line (AbiWidget * w);
  gboolean abi_widget_select_right (AbiWidget * w);
  gboolean abi_widget_select_screen_down (AbiWidget * w);
  gboolean abi_widget_select_screen_up (AbiWidget * w);
  gboolean abi_widget_select_to_xy (AbiWidget * w, gint32 x, gint32 y);

  /* text modification functions */
  gboolean abi_widget_toggle_bold (AbiWidget * w);
  gboolean abi_widget_toggle_underline (AbiWidget * w);
  gboolean abi_widget_toggle_bottomline (AbiWidget * w);
  gboolean abi_widget_toggle_insert_mode (AbiWidget * w);
  gboolean abi_widget_toggle_italic (AbiWidget * w);
  gboolean abi_widget_toggle_overline (AbiWidget * w);
  gboolean abi_widget_toggle_plain (AbiWidget * w);
  gboolean abi_widget_toggle_strike (AbiWidget * w);
  gboolean abi_widget_toggle_sub (AbiWidget * w);
  gboolean abi_widget_toggle_super (AbiWidget * w);
  gboolean abi_widget_toggle_topline (AbiWidget * w);
  gboolean abi_widget_toggle_unindent (AbiWidget * w);
  gboolean abi_widget_set_text_color(AbiWidget * w, guint8 red, guint8 green, guint8 blue);
  gboolean abi_widget_toggle_bullets (AbiWidget * w);
  gboolean abi_widget_toggle_numbering (AbiWidget * w);

  /* cursor functions */
  gboolean abi_widget_moveto_bob (AbiWidget * w);
  gboolean abi_widget_moveto_bod (AbiWidget * w);
  gboolean abi_widget_moveto_bol (AbiWidget * w);
  gboolean abi_widget_moveto_bop (AbiWidget * w);
  gboolean abi_widget_moveto_bow (AbiWidget * w);
  gboolean abi_widget_moveto_eob (AbiWidget * w);
  gboolean abi_widget_moveto_eod (AbiWidget * w);
  gboolean abi_widget_moveto_eol (AbiWidget * w);
  gboolean abi_widget_moveto_eop (AbiWidget * w);
  gboolean abi_widget_moveto_eow (AbiWidget * w);
  gboolean abi_widget_moveto_left (AbiWidget * w);
  gboolean abi_widget_moveto_next_line (AbiWidget * w);
  gboolean abi_widget_moveto_next_page (AbiWidget * w);
  gboolean abi_widget_moveto_next_screen (AbiWidget * w);
  gboolean abi_widget_moveto_prev_line (AbiWidget * w);
  gboolean abi_widget_moveto_prev_page (AbiWidget * w);
  gboolean abi_widget_moveto_prev_screen (AbiWidget * w);
  gboolean abi_widget_moveto_right (AbiWidget * w);
  gboolean abi_widget_moveto_to_xy (AbiWidget * w, gint32 x, gint32 y);

  /* search functions */
  void abi_widget_set_find_string (AbiWidget * w, gchar * search_str);
  gboolean abi_widget_find_next (AbiWidget * w, gboolean sel_start);
  gboolean abi_widget_find_prev (AbiWidget * w);

  /* document/page functions */
  guint32 abi_widget_get_page_count(AbiWidget * w);
  guint32 abi_widget_get_current_page_num(AbiWidget * w);
  void abi_widget_set_current_page(AbiWidget * w, guint32 curpage);

  /* view functions */
  gboolean abi_widget_view_formatting_marks (AbiWidget * w);
  gboolean abi_widget_view_print_layout (AbiWidget * w);
  gboolean abi_widget_view_normal_layout (AbiWidget * w);
  gboolean abi_widget_view_online_layout (AbiWidget * w);

  gboolean abi_widget_zoom_whole (AbiWidget * w);
  gboolean abi_widget_zoom_width (AbiWidget * w);
  gboolean abi_widget_set_zoom_percentage (AbiWidget * w, guint32 zoom);
  guint32 abi_widget_get_zoom_percentage (AbiWidget * w);

  gboolean abi_widget_set_word_selections (AbiWidget * w, gboolean b);
  gboolean abi_widget_get_word_selections (AbiWidget * w);

  gboolean abi_widget_set_show_margin (AbiWidget * w, gboolean bShowMargin);
  gboolean abi_widget_get_show_margin (AbiWidget * w);

  gboolean abi_widget_set_show_authors (AbiWidget * w, gboolean bShowAuthors);
  gboolean abi_widget_get_show_authors (AbiWidget * w);

  GdkPixbuf * abi_widget_render_page_to_image(AbiWidget *w, int page_number);

  /* header/footer functions */
  gboolean abi_widget_remove_header (AbiWidget * w);
  gboolean abi_widget_remove_footer (AbiWidget * w);
  gboolean abi_widget_edit_header (AbiWidget * w);
  gboolean abi_widget_edit_footer (AbiWidget * w);

  /* rulers methods */
  void abi_widget_toggle_rulers(AbiWidget * abi, gboolean visible);

  /* table functions */
  gboolean abi_widget_insert_table(AbiWidget * w, gint32 rows, gint32 cols);
  gboolean abi_widget_get_mouse_pos(AbiWidget * w, gint32 * x, gint32 * y);

  /* font/text functions */
  gboolean abi_widget_set_font_name(AbiWidget * w, gchar * szFontName);
  gboolean abi_widget_set_font_size(AbiWidget * w, gchar * szFontsize);
  const gchar** abi_widget_get_font_names (AbiWidget * w);

  /* style functions */
  gboolean abi_widget_set_style(AbiWidget * w, char* szName);

  /* image functions */
  gboolean abi_widget_insert_image(AbiWidget * w, char* szFile, gboolean positioned);

  /* generic editmethod invocation hooks */
  gboolean    abi_widget_invoke(AbiWidget * w, const char * mthdName);
  gboolean    abi_widget_invoke_ex (AbiWidget * w, const char *mthdName, const char * data, gint32 x, gint32 y);

#ifdef ABIWORD_INTERNAL
  /* these functions are used by abiword internally and really aren't exported to the rest of the world */
  XAP_Frame * abi_widget_get_frame ( AbiWidget * w ) ;
#endif

G_END_DECLS

#endif /* ABI_WIDGET_H */
