/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

_FIELDTYPE(DATETIME, AP_STRING_ID_FIELD_Type_Datetime)
_FIELD(DATETIME, AP_STRING_ID_FIELD_Datetime_CurrentTime, time)
_FIELD(DATETIME, AP_STRING_ID_FIELD_Datetime_CurrentDate, date)
_FIELDTYPE(NUMBERS, AP_STRING_ID_FIELD_Type_Numbers)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_PageNumber, page_number)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_PagesCount, page_count)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_ListLabel, list_label)
#ifndef NDEBUG
_FIELDTYPE(PIECETABLE, AP_STRING_ID_FIELD_Type_PieceTable)
_FIELD(PIECETABLE, AP_STRING_ID_FIELD_PieceTable_Test, test)
_FIELD(PIECETABLE, AP_STRING_ID_FIELD_PieceTable_MartinTest, martin_test)
#endif
#ifdef __OLD
_FIELDTYPE(DATETIME, "Date and Time")
_FIELD(DATETIME, "Current time", time)
_FIELDTYPE(NUMBERS, "Numbers")
_FIELD(NUMBERS, "Page number", page_number)
_FIELD(NUMBERS, "Number of pages", page_count)
_FIELD(NUMBERS, "List Label", list_label)
_FIELDTYPE(PIECETABLE, "Piece Table")
_FIELD(PIECETABLE,"Kevins Test",test)
_FIELD(PIECETABLE,"Martins Test",martin_test)
#endif



