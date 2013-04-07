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
/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
*****************************************************************/


/*****************************************************************/
/*****************************************************************/
/** This file defines the set of Id's used for all toolbar      **/
/** things.  Each Id defines a conceptual unit which may be     **/
/** used on one or more toolbars or not at all.                 **/
/*****************************************************************/
/*****************************************************************/

/*****************************************************************/
/*  Each toolbaritem(XXX) will create an ID  AP_TOOLBAR_ID_XXX   */
/*                                                               */
/*  You will have to add TOOLBAR_LABEL_XXX                       */
/*                   and TOOLBAR_TOOLTIP_XXX                     */
/*                   and TOOLBAR_STATUSLINE_XXX                  */
/*  to the ap_Strings_Id.h file                                  */
/*                                                               */
/*  Icons are mapped via the XXX in the ap_Toolbar_Iconmap.h     */
/*  file.  XXX are defaults mapped to icons.  XXX_LANG are the   */
/*  overloaded icons specific to langauges.                      */
/*****************************************************************/

toolbaritem(FILE_NEW)
toolbaritem(FILE_OPEN)
toolbaritem(FILE_SAVE)
toolbaritem(FILE_SAVEAS)
toolbaritem(FILE_PRINT)
toolbaritem(FILE_PRINT_PREVIEW)
toolbaritem(EDIT_UNDO)
toolbaritem(EDIT_REDO)
toolbaritem(EDIT_CUT)
toolbaritem(EDIT_COPY)
toolbaritem(EDIT_PASTE)
toolbaritem(EDIT_HEADER)
toolbaritem(EDIT_FOOTER)
toolbaritem(EDIT_REMOVEHEADER)
toolbaritem(EDIT_REMOVEFOOTER)
toolbaritem(SPELLCHECK)
toolbaritem(IMG)
toolbaritem(FMT_STYLE)
#if XAP_SIMPLE_TOOLBAR
toolbaritem(FMT_CHOOSE)
toolbaritem(VIEW_FULL_SCREEN)
#else
#endif
toolbaritem(FMT_FONT)
toolbaritem(FMT_HYPERLINK)
toolbaritem(FMT_BOOKMARK)
toolbaritem(FMT_SIZE)
toolbaritem(FMT_BOLD)
toolbaritem(FMT_ITALIC)
toolbaritem(FMT_UNDERLINE)
toolbaritem(FMT_OVERLINE)
toolbaritem(FMT_STRIKE)
toolbaritem(FMT_TOPLINE)
toolbaritem(FMT_BOTTOMLINE)
toolbaritem(HELP)
toolbaritem(FMT_SUPERSCRIPT)
toolbaritem(FMT_SUBSCRIPT)
toolbaritem(INSERT_SYMBOL)
toolbaritem(ALIGN_LEFT)
toolbaritem(ALIGN_CENTER)
toolbaritem(ALIGN_RIGHT)
toolbaritem(ALIGN_JUSTIFY)
toolbaritem(PARA_0BEFORE)
toolbaritem(PARA_12BEFORE)
toolbaritem(SINGLE_SPACE)
toolbaritem(MIDDLE_SPACE)
toolbaritem(DOUBLE_SPACE)
toolbaritem(1COLUMN)
toolbaritem(2COLUMN)
toolbaritem(3COLUMN)
toolbaritem(VIEW_SHOWPARA)
toolbaritem(ZOOM)
toolbaritem(LISTS_BULLETS)
toolbaritem(LISTS_NUMBERS)
toolbaritem(COLOR_FORE)
toolbaritem(COLOR_BACK)
toolbaritem(INDENT)
toolbaritem(UNINDENT)
toolbaritem(SCRIPT_PLAY)
toolbaritem(FMTPAINTER)
toolbaritem(FMT_DIR_OVERRIDE_LTR)
toolbaritem(FMT_DIR_OVERRIDE_RTL)
toolbaritem(FMT_DOM_DIRECTION)
toolbaritem(INSERT_TABLE)
toolbaritem(ADD_ROW)
toolbaritem(ADD_COLUMN)
toolbaritem(DELETE_ROW)
toolbaritem(DELETE_COLUMN)
toolbaritem(MERGE_CELLS)
toolbaritem(SPLIT_CELLS)
toolbaritem(MERGELEFT)
toolbaritem(MERGERIGHT)
toolbaritem(MERGEABOVE)
toolbaritem(MERGEBELOW)
#ifdef ENABLE_MENUBUTTON
toolbaritem(MENU)
#endif
toolbaritem(REVISIONS_NEW)
toolbaritem(REVISIONS_SELECT)
toolbaritem(REVISIONS_SHOW_FINAL)
toolbaritem(REVISIONS_FIND_PREV)
toolbaritem(REVISIONS_FIND_NEXT)
toolbaritem(SEMITEM_THIS)
toolbaritem(SEMITEM_NEXT)
toolbaritem(SEMITEM_PREV)
toolbaritem(SEMITEM_EDIT)
toolbaritem(SEMITEM_STYLESHEET_APPLY)

