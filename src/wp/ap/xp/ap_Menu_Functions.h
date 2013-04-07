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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef AP_MENU_FUNCTIONS_H
#define AP_MENU_FUNCTIONS_H

/*****************************************************************
******************************************************************
** This file defines the EV_GetMenuItemState and
** EV_GetMenuItemComputedLabel functions used by
** the set of menu actions.
******************************************************************
*****************************************************************/

#include "ev_Menu_Actions.h"

Defun_EV_GetMenuItemState_Fn(ap_GetState_Changes);
Defun_EV_GetMenuItemState_Fn(ap_GetState_Selection);
Defun_EV_GetMenuItemState_Fn(ap_GetState_Clipboard);
Defun_EV_GetMenuItemState_Fn(ap_GetState_CharFmt);
Defun_EV_GetMenuItemState_Fn(ap_GetState_BlockFmt);
Defun_EV_GetMenuItemState_Fn(ap_GetState_SectFmt);
Defun_EV_GetMenuItemState_Fn(ap_GetState_DocFmt);
Defun_EV_GetMenuItemState_Fn(ap_GetState_Window);
Defun_EV_GetMenuItemState_Fn(ap_GetState_View);
Defun_EV_GetMenuItemState_Fn(ap_GetState_Suggest);
Defun_EV_GetMenuItemState_Fn(ap_GetState_Prefs);
Defun_EV_GetMenuItemState_Fn(ap_GetState_StylesLocked);
Defun_EV_GetMenuItemState_Fn(ap_GetState_ScriptsActive);
Defun_EV_GetMenuItemState_Fn(ap_GetState_SomethingSelected);
Defun_EV_GetMenuItemState_Fn(ap_GetState_HyperlinkOK);
Defun_EV_GetMenuItemState_Fn(ap_GetState_RDF_Query);
Defun_EV_GetMenuItemState_Fn(ap_GetState_RDF_Contact);
Defun_EV_GetMenuItemState_Fn(ap_GetState_haveSemItems);
Defun_EV_GetMenuItemState_Fn(ap_GetState_Spelling);
Defun_EV_GetMenuItemState_Fn(ap_GetState_ColumnsActive);
Defun_EV_GetMenuItemState_Fn(ap_GetState_AutoRevision);
Defun_EV_GetMenuItemState_Fn(ap_GetState_MarkRevisions);
Defun_EV_GetMenuItemState_Fn(ap_GetState_MarkRevisionsCheck);
Defun_EV_GetMenuItemState_Fn(ap_GetState_HasRevisions);
Defun_EV_GetMenuItemState_Fn(ap_GetState_ShowRevisions);
Defun_EV_GetMenuItemState_Fn(ap_GetState_ShowRevisionsAfter);
Defun_EV_GetMenuItemState_Fn(ap_GetState_ShowRevisionsAfterPrev);
Defun_EV_GetMenuItemState_Fn(ap_GetState_ShowRevisionsBefore);
Defun_EV_GetMenuItemState_Fn(ap_GetState_RevisionPresent);
Defun_EV_GetMenuItemState_Fn(ap_GetState_RevisionPresentContext);
Defun_EV_GetMenuItemState_Fn(ap_GetState_RevisionsSelectLevel);
Defun_EV_GetMenuItemState_Fn(ap_GetState_History);
Defun_EV_GetMenuItemState_Fn(ap_GetState_FmtHdrFtr);
Defun_EV_GetMenuItemState_Fn(ap_GetState_BreakOK);
Defun_EV_GetMenuItemState_Fn(ap_GetState_TOCOK);
Defun_EV_GetMenuItemState_Fn(ap_GetState_BookmarkOK);
Defun_EV_GetMenuItemState_Fn(ap_GetState_xmlidOK);
Defun_EV_GetMenuItemState_Fn(ap_GetState_TableOK);
Defun_EV_GetMenuItemState_Fn(ap_GetState_TextToTableOK);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InTOC);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InTable);
Defun_EV_GetMenuItemState_Fn(ap_GetState_PointInTable);
Defun_EV_GetMenuItemState_Fn(ap_GetState_PointOrAnchorInTable);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InTableIsRepeat);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InTableMerged);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InAnnotation);
Defun_EV_GetMenuItemState_Fn(ap_GetState_ToggleAnnotations);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InFootnote);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InImage);
Defun_EV_GetMenuItemState_Fn(ap_GetState_SetPosImage);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InFrame);
Defun_EV_GetMenuItemState_Fn(ap_GetState_AlwaysDisabled); // REMOVE ME
Defun_EV_GetMenuItemState_Fn(ap_GetState_Recent);
Defun_EV_GetMenuItemState_Fn(ap_GetState_Zoom);
Defun_EV_GetMenuItemState_Fn(ap_GetState_Lists);
Defun_EV_GetMenuItemState_Fn(ap_GetState_MailMerge);
Defun_EV_GetMenuItemState_Fn(ap_GetState_InsTextBox);
Defun_EV_GetMenuItemState_Fn(ap_GetState_AnnotationJumpOK);
Defun_EV_GetMenuItemState_Fn(ap_GetState_ToggleRDFAnchorHighlight);

Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Toolbar);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Recent);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Window);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_WindowMore);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_About);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Contents);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Checkver);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Search);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Intro);
Defun_EV_GetMenuItemComputedLabel_Fn(ap_GetLabel_Suggest);

#endif /* AP_MENU_FUNCTIONS_H */
