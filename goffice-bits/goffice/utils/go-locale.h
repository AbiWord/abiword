/*
 * go-locale.h:
 *
 * Copyright (C) 2007 Morten Welinder (terra@gnome.org)
 * Copyright (C) 2003-2005 Jody Goldberg (jody@gnome.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
#ifndef GO_LOCALE_H
#define GO_LOCALE_H

#include <glib.h>
#include <locale.h>

G_BEGIN_DECLS

char const *   go_setlocale               (int category, char const *val);
void	       go_locale_untranslated_booleans  (void);

char const *   go_locale_boolean_name     (gboolean b);

GString const *go_locale_get_currency     (gboolean *precedes, gboolean *space_sep);
char           go_locale_get_arg_sep      (void);
char           go_locale_get_col_sep      (void);
char           go_locale_get_row_sep      (void);
GString const *go_locale_get_thousand     (void);
GString const *go_locale_get_decimal      (void);
int            go_locale_month_before_day (void);
gboolean       go_locale_24h              (void);

GString const *go_locale_get_date_format  (void);
GString const *go_locale_get_time_format  (void);

void           _go_locale_shutdown        (void);

G_END_DECLS

#endif /* GO_LOCALE_H */
