/*
 * AiksaurusGTK - A GTK interface to the AikSaurus library
 * Copyright (C) 2001 by Jared Davis
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


#ifndef INCLUDED_JARED_GPL_AIKSAURUSGTK_UTILS_H
#define INCLUDED_JARED_GPL_AIKSAURUSGTK_UTILS_H

//
// AiksaurusGTK_strEquals
// ----------------------
//   Returns true if lhs == rhs, false otherwise.
//   
bool AiksaurusGTK_strEquals(const char* lhs, const char* rhs);


//
// AiksaurusGTK_strCopy
// --------------------
//   Creates a copy of the string str.  This copy must be 
//   delete[]'d by the caller, so use this function carefully.
//   Returns NULL on memory alloc error.
//   
char* AiksaurusGTK_strCopy(const char* str);


//
// AiksaurusGTK_strConcat
// ----------------------
//   Creates a string by merging two strings.  The new string
//   must be delete[]'d by the caller, so use this function
//   carefully.
//   Returns NULL on memory alloc error.
//   
char* AiksaurusGTK_strConcat(const char* a, const char* b);


#endif // INCLUDED_JARED_GPL_AIKSAURUSGTK_UTILS_H
