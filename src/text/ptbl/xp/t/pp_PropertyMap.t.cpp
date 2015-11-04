/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2007,2011 Hubert Figuiere
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


#include <stdio.h>

#include <vector>

#include "tf_test.h"
#include "pp_PropertyMap.h"



TFTEST_MAIN("pp_PropertyMap")
{
	
	TFPASS(strcmp(PP_PropertyMap::abi_property_name(PP_PropertyMap::abi_field_color), "field-color") == 0);

	int num;
	const char** props = PP_PropertyMap::_properties(num);

	// check for the mismatch number of properties.
	// if this test fails, we might have a crash.
	TFPASS(num == PP_PropertyMap::abi__count);

	std::vector<std::string> v1(props, props + num);
	std::vector<std::string> v2 = v1;
	sort(v1.begin(), v1.end());
	// verify they are sorted
	TFPASS(v1 == v2);
 
	// test a few random properties for sanity.
	PP_PropertyMap::AbiPropertyIndex idx;

	TFPASS(PP_PropertyMap::abi_property_lookup("border-merge", idx));
	TFPASS(idx == PP_PropertyMap::abi_border_merge);

	TFPASS(PP_PropertyMap::abi_property_lookup("tight-wrap", idx));
	TFPASS(idx == PP_PropertyMap::abi_tight_wrap);

	TFPASS(PP_PropertyMap::abi_property_lookup("lang", idx));
	TFPASS(idx == PP_PropertyMap::abi_lang);
}

