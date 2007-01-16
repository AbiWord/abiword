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

#include "ut_map.h"


TFTEST_MAIN("UT_Map")
{
	// this class is mostly a wrapper around UT_RBTree
	UT_Map my_map;

	TFPASS(my_map.size() == 0);
	
	TFPASS(my_map.insert("foo", "42"));
	TFPASS(my_map.size() == 1);
	TFPASS(my_map.insert("baz", "41"));
	TFPASS(my_map.size() == 2);

	UT_Map::Iterator iter = my_map.find("baz");
	TFPASS(iter.is_valid());
	TFPASS(strcmp((char*)static_cast<UT_Map::value_t>(iter.value())->first(), "baz") == 0);
	TFPASS(strcmp((char*)static_cast<UT_Map::value_t>(iter.value())->second(), "41") == 0);


	my_map.erase("baz");
	TFFAIL(my_map.find("baz").is_valid());
	TFPASS(my_map.size() == 1);
}
