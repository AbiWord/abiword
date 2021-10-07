/* AbiSource Program Utilities
 * Copyright (C) 2004-2021 Hubert Figuière
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

#pragma once

/* MUST BE PURE C++ !!! */

/// Show the assert. Currently as a dialog box with a "never show me again".
/// Return true if the assert should be ignored next time
bool XAP_CocoaAssertMsg(const char* szMsg, const char* szFile, int iLine);
