/* AbiWord
 * Copyright (C) 2005 Hubert Figuiere
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
#include "tf_test.h"

#include "ut_locale.h"


TFTEST_MAIN("UT_LocaleTransactor")
{
	setlocale(LC_ALL, "fr_FR");
	
	TFPASS(strcmp(setlocale(LC_NUMERIC, NULL), "fr_FR") == 0);

	{
		UT_LocaleTransactor(LC_NUMERIC, "C");
		TFFAIL(strcmp(setlocale(LC_NUMERIC, NULL), "fr_FR") == 0);
		TFPASS(strcmp(setlocale(LC_NUMERIC, NULL), "C") == 0);
	}
	TFPASS(strcmp(setlocale(LC_NUMERIC, NULL), "fr_FR") == 0);
}
