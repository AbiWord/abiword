/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef UT_TEST_H
#define UT_TEST_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

// UT_TEST_H governs multiple inclusion of this header file
//
// UT_TEST is a compile option to select testing of src/util code
// PT_TEST is a compile option to select testing of src/ptbl code
// FMT_TEST is a compile option to select testing of src/fmt code
//
// UT_DEBUG is a compile option to select debugging, we piggy back
//          inclusion of core test routines on this.  (we could
//          also just do a (defined(UT_TEST) || ...)

#include <stdio.h>

#ifdef DEBUG
enum UT_TestStatus {
  UT_Test_SystemError = -1,
  UT_Test_Fail = 0,
  UT_Test_Pass = 1
};

const char * UT_TestStatus_GetMessage(UT_TestStatus status);
#endif /* UT_DEBUG */

#ifdef UT_TEST
void UT_Test(FILE * fp);
#endif

#endif /* UT_TEST_H */
