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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

_FIELDTYPE(DATETIME, AP_STRING_ID_FIELD_Type_Datetime)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_DefaultDateNoTime, date_ntdfl)
_FIELD(DATETIME, AP_STRING_ID_FIELD_Datetime_CurrentTime, time)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_MMDDYY, date_mmddyy)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_DDMMYY, date_ddmmyy)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_MonthDayYear, date_mdy)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_MthDayYear, date_mthdy)
_FIELD(DATETIME, AP_STRING_ID_FIELD_Datetime_CurrentDate, date)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_DefaultDate, date_dfl)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_Wkday, date_wkday)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_DOY, date_doy)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_TimeZone, time_zone)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_Epoch, time_epoch)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_MilTime, time_miltime)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_AMPM, time_ampm)
_FIELD(DATETIME, AP_STRING_ID_FIELD_DateTime_Custom, datetime_custom)

_FIELDTYPE(APPLICATION, AP_STRING_ID_FIELD_Application)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_Filename, file_name)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_ShortFilename, short_file_name)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_MailMerge, mail_merge)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_Version, app_ver)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_CompileDate, app_compiledate)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_CompileTime, app_compiletime)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_BuildId, app_id)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_Target, app_target)
_FIELD(APPLICATION, AP_STRING_ID_FIELD_Application_Options, app_options)

_FIELDTYPE(NUMBERS, AP_STRING_ID_FIELD_Type_Numbers)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_PageNumber, page_number)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_PagesCount, page_count)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_ParaCount, para_count)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_LineCount, line_count)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_WordCount, word_count)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_CharCount, char_count)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_NbspCount, nbsp_count)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_TableSumCols, sum_cols)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_TableSumRows, sum_rows)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_PageReference, page_ref)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_ListLabel,  list_label)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_FootnoteAnchor, footnote_anch)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_FootnoteReference, footnote_ref)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_EndnoteAnchor, endnote_anch)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_EndnoteReference, endnote_ref)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_TOCPageNumber, toc_page_number)
_FIELD(NUMBERS, AP_STRING_ID_FIELD_Numbers_TOCListLabel, toc_list_label)

_FIELDTYPE(DOCUMENT, AP_STRING_ID_FIELD_Type_Document)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Title, meta_title)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Subject, meta_subject)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Creator, meta_creator)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Publisher, meta_publisher)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Contributor, meta_contributor)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Type, meta_type)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Keywords, meta_keywords)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Language, meta_language)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Description, meta_description)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Coverage, meta_coverage)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Rights, meta_rights)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Date, meta_date)
_FIELD(DOCUMENT, AP_STRING_ID_FIELD_Document_Date_Last_Changed, meta_date_last_changed)

