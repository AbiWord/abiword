/* The AbiWord Widget
 *
 * Copyright (C) 2001 AbiSource, Inc.
 * Copyright (C) 2001 Dom Lachowicz <cinamod@hotmail.com>
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

	/**
	 * These functions basically just marshall back their arguments into
	 * the AbiWord application
	 */
	typedef gboolean (*Abi_EditMethod) (AbiWidget *, const char *, gint32, gint32);
	typedef gboolean (*Abi_Void__Bool_EditMethod) (AbiWidget *);
	typedef gboolean (*Abi_Int_Int__Bool_EditMethod) (AbiWidget *, gint32, gint32);
	typedef gboolean (*Abi_CharPtr__Bool_EditMethod) (AbiWidget *, const char *);
	
	/** 
	 *  only here for completeness in that that we might want to 
	 * add signals (such as "saved") later
	 */
	typedef gboolean (*AbiSignal) (AbiWidget *, gpointer closure);
	
	struct _AbiWidget {
		GtkWidget widget;
		
		/* private instance data */
		AbiPrivData * priv;
	};  
	
	struct  _AbiWidgetClass {
		GtkWidgetClass widget_parent_class;
		
		/* invoke any edit method based on its name */
		gboolean (*invoke) (AbiWidget *, const char * mthdName,
							const char * data, gint32 x, gint32 y);
		
		/* an incomplete list of edit methods - this is purposefully incomplete
		   and only meant for testing */
		Abi_Void__Bool_EditMethod align_center;
		Abi_Void__Bool_EditMethod align_justify;
		Abi_Void__Bool_EditMethod align_left;
		Abi_Void__Bool_EditMethod align_right;
		
		Abi_Void__Bool_EditMethod copy;
		Abi_Void__Bool_EditMethod cut;
		
		Abi_Void__Bool_EditMethod dlg_about;
		Abi_Void__Bool_EditMethod dlg_font;
		
		Abi_Void__Bool_EditMethod dlg_find;
		
		Abi_CharPtr__Bool_EditMethod insert_data;
		Abi_Void__Bool_EditMethod    insert_space;
		
		Abi_Void__Bool_EditMethod print;
		
		Abi_Void__Bool_EditMethod undo;
		Abi_Void__Bool_EditMethod redo;
		
		Abi_Void__Bool_EditMethod select_all;
		
		Abi_Void__Bool_EditMethod toggle_bold;
		Abi_Void__Bool_EditMethod toggle_italic;
		Abi_Void__Bool_EditMethod toggle_underline;
		
		Abi_Void__Bool_EditMethod view_para;
		
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
	
	gboolean    abi_widget_invoke (AbiWidget *, const char *mthdName, 
								   const char * data, gint32 x, gint32 y);
	
#ifdef __cplusplus
}
#endif /* c++ */

#endif /* ABI_WIDGET_H */
