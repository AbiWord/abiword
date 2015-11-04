/* AbiSource Applications
 * Copyright (C) 2006 Hubert Figuiere
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


#include "tf_test.h"
#include "ut_vector.h"

#define TFSUITE "core.af.util.vector"

TFTEST_MAIN("UT_GenericVector basics")
{
	UT_GenericVector<const char *> v;

	TFPASS(v.getItemCount() == 0);
	v.addItem("foo");
	TFPASS(v.getItemCount() == 1);
	v.addItem("bar");
	TFPASS(v.getItemCount() == 2);
	v.addItem("baz");
	TFPASS(v.getItemCount() == 3);

	TFPASS(strcmp(v[1], "bar") == 0);
	TFPASS(strcmp(v.getNthItem(2), "baz") == 0);

	TFPASS(strcmp(v.getFirstItem(), "foo") == 0);
	TFPASS(strcmp(v.getLastItem(), "baz") == 0);
	TFPASS(strcmp(v.back(), "baz") == 0);

	v.push_back("metropolis");
	TFPASS(v.getItemCount() == 4);
	TFPASS(strcmp(v.back(), "metropolis") == 0);
	
	v.insertItemAt("matrix", 3);
	TFPASS(v.getItemCount() == 5);
	TFPASS(strcmp(v[3], "matrix") == 0);
	TFPASS(strcmp(v[4], "metropolis") == 0);

	v.deleteNthItem(3);
	TFPASS(v.getItemCount() == 4);
	TFPASS(strcmp(v[3], "metropolis") == 0);	

	TFPASS(v.size() == 4);

	TFPASS(v.findItem("metropolis") == 3);
	TFPASS(v.findItem("bar") == 1);

	TFPASS(v.pop_back());
	TFPASS(v.getItemCount() == 3);
	TFPASS(strcmp(v.getLastItem(), "baz") == 0);

	v.clear();
	TFPASS(v.getItemCount() == 0);
}

TFTEST_MAIN("vector sorting")
{
}

