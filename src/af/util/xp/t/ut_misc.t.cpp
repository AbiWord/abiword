/* AbiWord
 * Copyright (C) 2014 Hubert Figuiere
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
#include "ut_misc.h"

TFTEST_MAIN("UT_HeadingDepth")
{
  UT_uint32 depth;
  depth = UT_HeadingDepth("Heading 1");
  TFPASS(depth == 1);

  depth = UT_HeadingDepth("Normal");
  TFPASS(depth == 0);

  depth = UT_HeadingDepth("Heading 10");
  TFPASS(depth == 10);

  depth = UT_HeadingDepth("Numbered Heading 5");
  TFPASS(depth == 5);
}
