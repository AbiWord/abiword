/* vim: set sw=8: -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * go-locale.c :
 *
 * Copyright (C) 1998 Chris Lahey, Miguel de Icaza
 * Copyright (C) 2003-2005 Jody Goldberg (jody@gnome.org)
 * Copyright (C) 2005-2007 Morten Welinder (terra@gnome.org)
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

#include <goffice/goffice-config.h>
#include "go-locale.h"
#include <glib/gi18n-lib.h>
#ifdef HAVE_LANGINFO_H
#  include <langinfo.h>
#endif
#ifdef G_OS_WIN32
#  include <windows.h>
#endif
#include <string.h>

/*
 * Points to the locale information for number display.  All strings are
 * in UTF-8 encoding.
 */
static gboolean locale_info_cached = FALSE;
static GString *lc_decimal = NULL;
static GString *lc_thousand = NULL;
static gboolean lc_precedes;
static gboolean lc_space_sep;
static GString *lc_currency = NULL;

static gboolean date_format_cached = FALSE;
static GString *lc_date_format = NULL;
static gboolean time_format_cached = FALSE;
static GString *lc_time_format = NULL;

static gboolean date_order_cached = FALSE;

static gboolean locale_is_24h_cached = FALSE;

static gboolean boolean_cached = FALSE;
static char const *lc_TRUE = NULL;
static char const *lc_FALSE = NULL;

char const *
go_setlocale (int category, char const *val)
{
	locale_info_cached = FALSE;
	date_format_cached = FALSE;
	time_format_cached = FALSE;
	date_order_cached = FALSE;
	locale_is_24h_cached = FALSE;
	boolean_cached = FALSE;
	return setlocale (category, val);
}

void
_go_locale_shutdown (void)
{
	if (locale_info_cached) {
		/* Mark everything as uncached.  */
		(void)go_setlocale (LC_ALL, NULL);
	}
#define FREE1(var) if (var) { g_string_free (var, TRUE); var = NULL; }
	FREE1 (lc_decimal);
	FREE1 (lc_thousand);
	FREE1 (lc_currency);
	FREE1 (lc_date_format);
	FREE1 (lc_time_format);
#undef FREE1
}


static void
convert1 (GString *res, char const *lstr, char const *name, char const *def)
{
	char *tmp;

	if (lstr == NULL || lstr[0] == 0) {
		g_string_assign (res, def);
		return;
	}

	tmp = g_locale_to_utf8 (lstr, -1, NULL, NULL, NULL);
	if (tmp) {
		g_string_assign (res, tmp);
		g_free (tmp);
		return;
	}

	g_warning ("Failed to convert locale's %s \"%s\" to UTF-8.", name, lstr);
	g_string_assign (res, def);
}

static void
update_lc (void)
{
	struct lconv *lc = localeconv ();

	if (!lc_decimal)
		lc_decimal = g_string_new (NULL);

	if (!lc_thousand)
		lc_thousand = g_string_new (NULL);

	if (!lc_currency)
		lc_currency = g_string_new (NULL);

	/*
	 * Extract all information here as lc is not guaranteed to stay
	 * valid after next localeconv call which could be anywhere.
	 */

	convert1 (lc_decimal, lc->decimal_point, "decimal separator", ".");
	if (g_utf8_strlen (lc_decimal->str, -1) != 1)
		g_warning ("Decimal separator is not a single character.");

	convert1 (lc_thousand, lc->mon_thousands_sep, "monetary thousands separator",
		  (lc_decimal->str[0] == ',' ? "." : ","));
	if (g_utf8_strlen (lc_thousand->str, -1) != 1)
		g_warning ("Monetary thousands separator is not a single character.");

	if (g_string_equal (lc_thousand, lc_decimal)) {
		g_string_assign (lc_thousand,
				 (lc_decimal->str[0] == ',') ? "." : ",");
		g_warning ("Monetary thousands separator is the same as the decimal separator; converting '%s' to '%s'",
			   lc_decimal->str, lc_thousand->str);
	}

	/* Use != 0 rather than == 1 so that CHAR_MAX (undefined) is true */
	lc_precedes = (lc->p_cs_precedes != 0);

	/* Use == 1 rather than != 0 so that CHAR_MAX (undefined) is false */
	lc_space_sep = (lc->p_sep_by_space == 1);

	convert1 (lc_currency, lc->currency_symbol, "currency symbol",	"$");

	locale_info_cached = TRUE;
}

GString const *
go_locale_get_decimal (void)
{
	if (!locale_info_cached)
		update_lc ();

	return lc_decimal;
}

GString const *
go_locale_get_thousand (void)
{
	if (!locale_info_cached)
		update_lc ();

	return lc_thousand;
}

/**
 * go_locale_get_currency :
 * @precedes : a pointer to a boolean which is set to TRUE if the currency
 * 		should precede
 * @space_sep: a pointer to a boolean which is set to TRUE if the currency
 * 		should have a space separating it from the the value
 *
 * Play with the default logic so that things come out nicely for the default
 * case.
 *
 * Returns: A string with the default currency
 **/
GString const *
go_locale_get_currency (gboolean *precedes, gboolean *space_sep)
{
	if (!locale_info_cached)
		update_lc ();

	if (precedes)
		*precedes = lc_precedes;

	if (space_sep)
		*space_sep = lc_space_sep;

	return lc_currency;
}

#if defined(G_OS_WIN32)
static void
go_locale_win32_get_user_default (GString *res, unsigned int id)
{
	GError *error = NULL;
	WCHAR fmt_utf16[64];
	int utf16_len = GetLocaleInfoW (LOCALE_USER_DEFAULT,
		id, fmt_utf16, sizeof (fmt_utf16));
	gsize  utf8_len;
	char *fmt_utf8 = g_convert ((gchar *)fmt_utf16, utf16_len*2,
		 "UTF-8", "UTF-16LE", NULL, &utf8_len, &error);
	if (NULL != fmt_utf8)
		g_string_append_len (res, fmt_utf8, utf8_len);
	else if (NULL != error) {
		g_warning ("error: %s", error->message);
		g_error_free (error);
	}
}
#endif

GString const *
go_locale_get_date_format (void)
{
	if (!date_format_cached) {
		if (lc_date_format)
			g_string_truncate (lc_date_format, 0);
		else
			lc_date_format = g_string_new (NULL);

#if defined(G_OS_WIN32)
		go_locale_win32_get_user_default (lc_date_format, LOCALE_SLONGDATE);
#elif defined(HAVE_LANGINFO_H)
		{
			char const *fmt = nl_langinfo (D_FMT);
			/* It appears that sometimes we don't get the %s in the
			 * format as we're supposed to.  */
			const char *first_percent = strchr (fmt, '%');
			if (first_percent)
				fmt = first_percent;

			while (*fmt) {
				if (first_percent) {
					if (*fmt != '%') {
						g_string_append_c (lc_date_format, *fmt);
						fmt++;
						continue;
					}
					fmt++;
				}

				switch (*fmt) {
				case 'a': g_string_append (lc_date_format, "ddd"); break;
				case 'A': g_string_append (lc_date_format, "dddd"); break;
				case 'b': g_string_append (lc_date_format, "mmm"); break;
				case 'B': g_string_append (lc_date_format, "mmmm"); break;
				case 'd': g_string_append (lc_date_format, "dd"); break;
				case 'D': g_string_append (lc_date_format, "mm/dd/yy"); break;
				case 'e': g_string_append (lc_date_format, "d"); break; /* Approx */
				case 'F': g_string_append (lc_date_format, "yyyy-mm-dd"); break;
				case 'h': g_string_append (lc_date_format, "mmm"); break;
				case 'm': g_string_append (lc_date_format, "mm"); break;
				case 't': g_string_append (lc_date_format, "\t"); break;
				case 'y': g_string_append (lc_date_format, "yy"); break;
				case 'Y': g_string_append (lc_date_format, "yyyy"); break;
				case '%':
					if (!first_percent)
						break;
					/* Fall through.  */
				default:
					if (g_ascii_isalpha (*fmt))
						g_warning ("Unhandled locale date code '%c'", *fmt);
					else
						g_string_append_c (lc_date_format, *fmt);
				}
				fmt++;
			}
		}
#endif

		/* Sanity check */
		if (!g_utf8_validate (lc_date_format->str, -1, NULL)) {
			g_warning ("Produced non-UTF-8 date format.  Please report.");
			g_string_truncate (lc_date_format, 0);
		}

		/* Default */
		if (lc_date_format->len == 0) {
			static gboolean warning = TRUE;
			g_string_append (lc_date_format, "yyyy/mm/dd");
			if (warning) {
				g_warning ("Using default system date format: %s",
					   lc_date_format->str);
				warning = FALSE;
			}
		}

		date_format_cached = TRUE;
	}
	return lc_date_format;
}

GString const *
go_locale_get_time_format (void)
{
	if (!time_format_cached) {
		if (lc_time_format)
			g_string_truncate (lc_time_format, 0);
		else
			lc_time_format = g_string_new (NULL);

#if defined(G_OS_WIN32)
		go_locale_win32_get_user_default (lc_time_format, LOCALE_STIME);
#elif defined(HAVE_LANGINFO_H)
		{
			char const *fmt = nl_langinfo (T_FMT);
			/* It appears that sometimes we don't get the %s in the
			 * format as we're supposed to.  */
			const char *first_percent = strchr (fmt, '%');
			if (first_percent)
				fmt = first_percent;

			while (*fmt) {
				if (first_percent) {
					if (*fmt != '%') {
						g_string_append_c (lc_time_format, *fmt);
						fmt++;
						continue;
					}
					fmt++;
				}

				switch (*fmt) {
				case 'H': g_string_append (lc_time_format, "hh"); break;
				case 'I': g_string_append (lc_time_format, "hh"); break;
				case 'k': g_string_append (lc_time_format, "h"); break; /* Approx */
				case 'l': g_string_append (lc_time_format, "h"); break; /* Approx */
				case 'M': g_string_append (lc_time_format, "mm"); break;
				case 'p': g_string_append (lc_time_format, "AM/PM"); break;
				case 'P': g_string_append (lc_time_format, "am/pm"); break;
				case 'r': g_string_append (lc_time_format, "hh:mm:ss AM/PM"); break;
				case 'S': g_string_append (lc_time_format, "ss"); break;
				case 'T': g_string_append (lc_time_format, "hh:mm:ss"); break;
				case 't': g_string_append (lc_time_format, "\t"); break;

				case 'z': case 'Z':
					/* Ignore these time-zone related items.  */
					break;

				case '%':
					if (!first_percent)
						break;
					/* Fall through.  */
				default:
					if (g_ascii_isalpha (*fmt))
						g_warning ("Unhandled locale time code '%c'", *fmt);
					else
						g_string_append_c (lc_time_format, *fmt);
				}
				fmt++;
			}

			/* Since we ignore some stuff, sometimes we get trailing
			   whitespace.  Kill it.  */
			while (lc_time_format->len > 0) {
				const char *s = lc_time_format->str + lc_time_format->len;
				const char *p = g_utf8_prev_char (s);
				if (!g_unichar_isspace (g_utf8_get_char (p)))
					break;
				g_string_truncate (lc_time_format,
						   p - lc_time_format->str);
			}
		}
#endif

		/* Sanity check */
		if (!g_utf8_validate (lc_time_format->str, -1, NULL)) {
			g_warning ("Produced non-UTF-8 time format.  Please report.");
			g_string_truncate (lc_time_format, 0);
		}

		/* Default */
		if (lc_time_format->len == 0) {
			static gboolean warning = TRUE;
			g_string_append (lc_time_format, "dddd, mmmm dd, yyyy");
			if (warning) {
				g_warning ("Using default system time format: %s",
					   lc_time_format->str);
				warning = FALSE;
			}
		}

		time_format_cached = TRUE;
	}
	return lc_time_format;
}

/*
 * go_locale_month_before_day :
 *
 * A quick utility routine to guess whether the default date format
 * uses day/month or month/day.  Returns a value of the same meaning
 * as go_format_month_before_day, i.e.,
 *
 * 0, if locale uses day before month
 * 1, if locale uses month before day, unless the following applies
 * 2, if locale uses year before month (before day)
 */
int
go_locale_month_before_day (void)
{
	static int month_first = 1;
	if (!date_order_cached) {
		date_order_cached = TRUE;

#if defined(G_OS_WIN32)
		{
			TCHAR str[2];
			GetLocaleInfo (LOCALE_USER_DEFAULT, LOCALE_IDATE, str, 2);
			if (str[0] == L'1')
				month_first = 0;
			else if (str[0] == L'2')
				month_first = 2;
			else
				month_first = 1;
		}

#elif defined(HAVE_LANGINFO_H)
		{
			char const *ptr = nl_langinfo (D_FMT);
			while (ptr && *ptr) {
				char c = *ptr++;
				switch (c) {
				case 'd': case 'D': case 'e':
					month_first = 0;
					ptr = NULL;
					break;
				case 'm': case 'b': case 'B': case 'h':
					month_first = 1;
					ptr = NULL;
					break;
				case 'C': case 'G': case 'g':
				case 'y': case 'Y':
					month_first = 2;
					ptr = NULL;
					break;
				default: ;
				}
			}
		}
#else
		{
			static gboolean warning = TRUE;
			if (warning) {
				g_warning ("Incomplete locale library, dates will be month day year");
				warning = FALSE;
			}
		}
#endif
	}

	return month_first;
}

/**
 * go_locale_24h :
 *
 * Returns: TRUE if the locale uses a 24h clock, FALSE otherwise.
 */
gboolean
go_locale_24h (void)
{
	static gboolean locale_is_24h;

	if (!locale_is_24h_cached) {
		const GString *tf = go_locale_get_time_format ();

		/* Crude.  Figure out how to ask properly.  */
		locale_is_24h = !(strstr (tf->str, "AM/PM") ||
				  strstr (tf->str, "am/pm") ||
				  strstr (tf->str, "A/P") ||
				  strstr (tf->str, "a/p"));
		locale_is_24h_cached = TRUE;
	}

	return locale_is_24h;
}


/* Use comma as the arg separator unless the decimal point is a
 * comma, in which case use a semi-colon
 */
char
go_locale_get_arg_sep (void)
{
	if (go_locale_get_decimal ()->str[0] == ',')
		return ';';
	return ',';
}

char
go_locale_get_col_sep (void)
{
	if (go_locale_get_decimal ()->str[0] == ',')
		return '\\';
	return ',';
}

char
go_locale_get_row_sep (void)
{
	return ';';
}

char const *
go_locale_boolean_name (gboolean b)
{
	if (!boolean_cached) {
		lc_TRUE = _("TRUE");
		lc_FALSE = _("FALSE");
		boolean_cached = TRUE;
	}
	return b ? lc_TRUE : lc_FALSE;
}

/**
 * go_locale_untranslated_booleans :
 *
 * Short circuit the current locale so that we can import files
 * and still produce error messages in the current LC_MESSAGE
 **/
void
go_locale_untranslated_booleans (void)
{
	lc_TRUE = "TRUE";
	lc_FALSE = "FALSE";
	boolean_cached = TRUE;
}
