/* AbiSource Applications
 * Copyright (C) 2006-2019 Hubert Figuiere
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
#include "ut_stack.h"

#define TFSUITE "core.af.util.stack"

TFTEST_MAIN("UT_Stack basics")
{
	const char FOO[] = "foo";
	const char BAR[] = "bar";
	UT_Stack stack;

	TFPASS(stack.getDepth() == 0);
	void *ptr = nullptr;
	TFPASS(!stack.viewTop(&ptr));
	TFPASS(!stack.pop(&ptr));

	stack.push(const_cast<char*>(FOO));
	TFPASS(stack.getDepth() == 1);
	ptr = nullptr;
	TFPASS(stack.viewTop(&ptr));
	TFPASS(ptr == FOO);

	stack.push(const_cast<char*>(BAR));
	TFPASS(stack.getDepth() == 2);
	ptr = nullptr;
	TFPASS(stack.viewTop(&ptr));
	TFPASS(ptr == BAR);

	ptr = nullptr;
	TFPASS(stack.pop(&ptr));
	TFPASS(ptr == BAR);
	TFPASS(stack.getDepth() == 1);
	ptr = nullptr;
	TFPASS(stack.viewTop(&ptr));
	TFPASS(ptr == FOO);

	stack.clear();
	TFPASS(stack.getDepth() == 0);
	ptr = nullptr;
	TFPASS(!stack.viewTop(&ptr));
	TFPASS(!stack.pop(&ptr));
}

TFTEST_MAIN("UT_NumberStack basics")
{
	UT_NumberStack stack;
	UT_sint32 number = 0;

	TFPASS(stack.getDepth() == 0);
	TFPASS(!stack.viewTop(number));
	TFPASS(!stack.pop(&number));

	stack.push(42);
	TFPASS(stack.getDepth() == 1);
	number = 0;
	TFPASS(stack.viewTop(number));
	TFPASS(number == 42);

	stack.push(69);
	TFPASS(stack.getDepth() == 2);
	number = 0;
	TFPASS(stack.viewTop(number));
	TFPASS(number == 69);

	number = 0;
	TFPASS(stack.pop(&number));
	TFPASS(number == 69);
	TFPASS(stack.getDepth() == 1);
	number = 0;
	TFPASS(stack.viewTop(number));
	TFPASS(number == 42);

	stack.clear();
	number = 0;
	TFPASS(stack.getDepth() == 0);
	TFPASS(!stack.viewTop(number));
	TFPASS(!stack.pop(&number));
}

