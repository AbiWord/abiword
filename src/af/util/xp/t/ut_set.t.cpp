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

#include "ut_set.h"


TFTEST_MAIN("UT_Set")
{
	UT_Set my_set;

	TFPASS(my_set.size() == 0);

	TFPASS(my_set.insert("apple"));
	TFPASS(my_set.size() == 1);
	TFPASS(my_set.insert("orange"));
	TFPASS(my_set.size() == 2);

	UT_Set::Iterator iter = my_set.find("apple");
	TFPASS(iter.is_valid());
	TFPASS(strcmp((char*)iter.value(), "apple") == 0);

	UT_Set::Iterator iterB = my_set.begin();
	TFPASS(iterB.is_valid());
	TFPASS(strcmp((char*)iterB.value(), "apple") == 0);

	UT_Set::Iterator iterE = my_set.end();
	TFFAIL(iterE.is_valid());

	UT_Set::Iterator iterR = my_set.find("orange");
	TFPASS(iterR.is_valid());
	my_set.erase(iterR);
	TFPASS(my_set.size() == 1);
	TFFAIL(my_set.find("orange").is_valid());
}

TFTEST_MAIN("UT_Set with comparator")
{
	UT_Set my_set(ut_lexico_lesser);

	TFPASS(my_set.size() == 0);

	TFPASS(my_set.insert("apple"));
	TFPASS(my_set.size() == 1);
	TFPASS(my_set.insert("orange"));
	TFPASS(my_set.size() == 2);

	UT_Set::Iterator iter = my_set.find_if("apple", ut_lexico_equal);
	TFPASS(iter.is_valid());
	TFPASS(strcmp((char*)iter.value(), "apple") == 0);

	UT_Set::Iterator iterR = my_set.find_if("orange", ut_lexico_equal);
	TFPASS(iterR.is_valid());
	my_set.erase(iterR);
	TFPASS(my_set.size() == 1);
	TFFAIL(my_set.find_if("orange", ut_lexico_equal).is_valid());
}
