/*
 *  Copyright (C) 2005 Robert Staudinger
 *
 *  This software is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Library General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef ABI_FONT_COMBO_H
#define ABI_FONT_COMBO_H

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define ABI_TYPE_FONT_COMBO                  (abi_font_combo_get_type ())
#define ABI_FONT_COMBO(obj)                  (G_TYPE_CHECK_INSTANCE_CAST ((obj), ABI_TYPE_FONT_COMBO, AbiFontCombo))
#define ABI_FONT_COMBO_CLASS(klass)          (G_TYPE_CHECK_CLASS_CAST ((klass), ABI_TYPE_FONT_COMBO, AbiFontComboClass))
#define ABI_IS_FONT_COMBO(obj)               (G_TYPE_CHECK_INSTANCE_TYPE ((obj), ABI_TYPE_FONT_COMBO))
#define ABI_IS_FONT_COMBO_CLASS(klass)       (G_TYPE_CHECK_CLASS_TYPE ((klass), ABI_TYPE_FONT_COMBO))
#define ABI_FONT_COMBO_GET_CLASS(obj)        (G_TYPE_INSTANCE_GET_CLASS ((obj), ABI_TYPE_FONT_COMBO, AbiFontComboClass))

typedef struct _AbiFontCombo 		AbiFontCombo;
typedef struct _AbiFontComboClass 	AbiFontComboClass;

struct _AbiFontCombo {
	GtkComboBox parent;
};

struct _AbiFontComboClass {
	GtkComboBoxClass parent;

	void (* prelight) (AbiFontCombo	*self, 
			   const gchar	*text);
};

GType abi_font_combo_get_type (void);

GtkWidget * abi_font_combo_new (void);

G_END_DECLS

#endif /* ABI_FONT_COMBO_H */
