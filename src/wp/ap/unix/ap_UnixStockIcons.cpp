/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* 
 * Copyright (C) 2006 Robert Staudinger <robert.staudinger@gmail.com>
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

#include "ap_UnixStockIcons.h"
#include "ut_string.h"

typedef struct {
	const gchar *abi_stock_id;
	const gchar *gtk_stock_id;
} AbiStockMapping;

/* 
 * Mapping of abiword icons to their gtk counterparts.
 */
static AbiStockMapping stock_mapping[] = {
  { ABIWORD_FILE_NEW,				GTK_STOCK_NEW },
  { ABIWORD_FILE_OPEN,				GTK_STOCK_OPEN },
  { ABIWORD_FILE_SAVE,				GTK_STOCK_SAVE },
  { ABIWORD_FILE_SAVEAS,			GTK_STOCK_SAVE_AS },
  { ABIWORD_FILE_PRINT,				GTK_STOCK_PRINT },
  { ABIWORD_FILE_PRINT_PREVIEW,		GTK_STOCK_PRINT_PREVIEW },

  { ABIWORD_SPELLCHECK,				GTK_STOCK_SPELL_CHECK },

  { ABIWORD_EDIT_CUT,				GTK_STOCK_CUT },
  { ABIWORD_EDIT_COPY,				GTK_STOCK_COPY },
  { ABIWORD_EDIT_PASTE,				GTK_STOCK_PASTE },
  { ABIWORD_EDIT_UNDO,				GTK_STOCK_UNDO },
  { ABIWORD_EDIT_REDO,				GTK_STOCK_REDO },

  { ABIWORD_HELP,					GTK_STOCK_HELP },
  { ABIWORD_FMT_BOLD,				GTK_STOCK_BOLD },
  { ABIWORD_FMT_ITALIC,				GTK_STOCK_ITALIC },
  { ABIWORD_FMT_UNDERLINE,			GTK_STOCK_UNDERLINE },

  { ABIWORD_ALIGN_LEFT,				GTK_STOCK_JUSTIFY_LEFT },
  { ABIWORD_ALIGN_CENTER,			GTK_STOCK_JUSTIFY_CENTER },
  { ABIWORD_ALIGN_RIGHT,			GTK_STOCK_JUSTIFY_RIGHT },
  { ABIWORD_ALIGN_JUSTIFY,			GTK_STOCK_JUSTIFY_FILL },

  { ABIWORD_UNINDENT,				GTK_STOCK_UNINDENT },
  { ABIWORD_INDENT,					GTK_STOCK_INDENT },

  { ABIWORD_SCRIPT_PLAY,			GTK_STOCK_EXECUTE },
  { NULL, 							NULL }
};

/*
 * Retval must be free'd
 */
gchar *
abiword_get_gtk_stock_id (const gchar * abi_stock_id)
{
	gint i;

	i = 0;
	while (stock_mapping[i].abi_stock_id) {
		if (0 == UT_strcmp (abi_stock_id, stock_mapping[i].abi_stock_id)) {
			return g_strdup (stock_mapping[i].gtk_stock_id);
		}
		i++;
	}

	return NULL;
}
