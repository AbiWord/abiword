/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

// Message Boxes used in AP_EditMethods
dcl(MSG_SaveFailed,		"Could not write to the file %s.")
dcl(MSG_RevertBuffer,	"Revert to saved copy of %s?")
dcl(MSG_QueryExit,		"Close all windows and exit?")
dcl(MSG_ConfirmSave,	"Save changes to %s?")
dcl(MSG_ImportError,	"Error importing file %s.")

// Status Bar Messages
dcl(PageInfoField,		"Page: %ld/%d")

/* Find and Replace strings */
dcl(DLG_FR_FindTitle, 		"Find")
dcl(DLG_FR_ReplaceTitle,	"Replace")
dcl(DLG_FR_FindLabel, 		"Fi&nd what:")
dcl(DLG_FR_ReplaceWithLabel,	"Re&place with:")
dcl(DLG_FR_MatchCase,		"&Match case")
dcl(DLG_FR_FindNextButton,	"&Find Next")
dcl(DLG_FR_ReplaceButton,	"&Replace")
dcl(DLG_FR_ReplaceAllButton,	"Replace &All")
dcl(DLG_FR_CancelButton,	"Cancel")
