/* The AbiWord Widget
 *
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001,2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (C) 2002 Martin Sevior <msevior@physics.unimelb.edu.au>
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
#ifndef ABI_WIDGET_H
#define ABI_WIDGET_H

#include <gtk/gtk.h>

#ifdef ABIWORD_INTERNAL
#include "ap_UnixApp.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* c++ */
	
#define ABI_WIDGET_TYPE        (abi_widget_get_type ())
#define ABI_WIDGET(obj)        (GTK_CHECK_CAST((obj), ABI_WIDGET_TYPE, AbiWidget))
#define IS_ABI_WIDGET(obj)     (GTK_CHECK_TYPE((obj), ABI_WIDGET_TYPE))
#define IS_ABI_WIDGET_CLASS(k) (GTK_CHECK_CLASS_TYPE ((k), ABI_WIDGET_TYPE))
#define ABI_WIDGET_CLASS(k)    (GTK_CHECK_CLASS_CAST ((k), ABI_WIDGET_TYPE, AbiWidgetClass))
	
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
    
    /* a list of our more useful edit methods */
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
    Abi_Void__Bool_EditMethod remove_header;
    Abi_Void__Bool_EditMethod remove_footer;
    
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
    Abi_Void__Bool_EditMethod toggle_bottomline;
    Abi_Void__Bool_EditMethod toggle_insert_mode;
    Abi_Void__Bool_EditMethod toggle_italic;
    Abi_Void__Bool_EditMethod toggle_overline;
    Abi_Void__Bool_EditMethod toggle_plain;
    Abi_Void__Bool_EditMethod toggle_strike;
    Abi_Void__Bool_EditMethod toggle_sub;
    Abi_Void__Bool_EditMethod toggle_super;
    Abi_Void__Bool_EditMethod toggle_topline;
    Abi_Void__Bool_EditMethod toggle_underline;
    Abi_Void__Bool_EditMethod toggle_unindent;
    
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

    Abi_Void__Bool_EditMethod zoom_100;
    Abi_Void__Bool_EditMethod zoom_200;
    Abi_Void__Bool_EditMethod zoom_50;
    Abi_Void__Bool_EditMethod zoom_75;
    Abi_Void__Bool_EditMethod zoom_whole;
    Abi_Void__Bool_EditMethod zoom_width;

    Abi_EditMethod em_pad1;
    Abi_EditMethod em_pad2;
    Abi_EditMethod em_pad3;
    
    /* signals */
    
    AbiSignal sig_pad1;
    AbiSignal sig_pad2;
    AbiSignal sig_pad3;
  };
	
  /* the public API */
	GtkWidget * abi_widget_new (void);
	GtkWidget * abi_widget_new_with_file (const gchar * file);
	GtkType     abi_widget_get_type	(void);
	void        abi_widget_map_to_screen(AbiWidget * widget);
	void        abi_widget_turn_on_cursor(AbiWidget * widget);
	void abi_widget_set_property(GObject  *object,
								 guint	arg_id,
								 const GValue *arg,
								 GParamSpec *pspec);
 
	void abi_widget_get_property(GObject  *object,
								 guint arg_id,
								 GValue     *arg,
								 GParamSpec *pspec);

  gboolean    abi_widget_invoke(AbiWidget * w, const char * mthdName);    
  gboolean    abi_widget_invoke_ex (AbiWidget * w, const char *mthdName, 
				    const char * data, gint32 x, gint32 y);
  
  void        abi_widget_draw(AbiWidget * w);

  gboolean abi_widget_save ( AbiWidget * w, const char * fname );
  gboolean abi_widget_save_ext ( AbiWidget * w, const char * fname,
				 const char * extension ) ;
  
#ifdef ABIWORD_INTERNAL
  /* these functions are used by abiword internally and really aren't exported to the rest of the world */
  GtkWidget * abi_widget_new_with_app (AP_UnixApp * pApp);
  GtkWidget * abi_widget_new_with_app_file (AP_UnixApp * pApp,const gchar * file);
  XAP_Frame * abi_widget_get_frame ( AbiWidget * w ) ;
#endif
  
#ifdef __cplusplus
}
#endif /* c++ */

#endif /* ABI_WIDGET_H */









