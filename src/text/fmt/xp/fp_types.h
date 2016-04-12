/* AbiWord
 * Copyright (c) 2003 Tomas Frydrych
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

#ifndef FP_TYPES_H
#define FP_TYPES_H

enum FL_ListType
{
  NUMBERED_LIST = 0,
  LOWERCASE_LIST = 1,
  UPPERCASE_LIST = 2,
  LOWERROMAN_LIST = 3,
  UPPERROMAN_LIST = 4,
  // any new numbered lists should be added below OTHER_NUMBERED_LISTS
  BULLETED_LIST = 5,
  DASHED_LIST = 6,
  SQUARE_LIST = 7,
  TRIANGLE_LIST = 8,
  DIAMOND_LIST = 9,
  STAR_LIST = 10,
  IMPLIES_LIST = 11,
  TICK_LIST = 12,
  BOX_LIST = 13,
  HAND_LIST = 14,
  HEART_LIST = 15,
  ARROWHEAD_LIST = 16,
  // add new bulleted lists here, and increase LAST_BULLETED_LIST accordingly
  // any new numbered lists should be added below OTHER_NUMBERED_LISTS

  // could not just add the extra numbered lists above the bulletted one,
  // since that would break compatibility
  LAST_BULLETED_LIST = 17,
  OTHER_NUMBERED_LISTS = 0x7f,
  ARABICNUMBERED_LIST = 0x80,
  HEBREW_LIST = 0x81,
  NOT_A_LIST = 0xff
};

enum FV_DocPos
{
  FV_DOCPOS_BOB, FV_DOCPOS_EOB,	// block
  FV_DOCPOS_BOD, FV_DOCPOS_EOD,	// document
  FV_DOCPOS_BOP, FV_DOCPOS_EOP,	// page
  FV_DOCPOS_BOL, FV_DOCPOS_EOL,	// line
  FV_DOCPOS_BOS, FV_DOCPOS_EOS,	// sentence
  FV_DOCPOS_BOW, FV_DOCPOS_EOW_MOVE, FV_DOCPOS_EOW_SELECT // word
};

enum ToggleCase
{
  CASE_SENTENCE,
  CASE_LOWER,
  CASE_UPPER,
  CASE_TITLE,
  CASE_TOGGLE,
  CASE_FIRST_CAPITAL,
  CASE_ROTATE
};

enum FormatTable
{
  FORMAT_TABLE_SELECTION,
  FORMAT_TABLE_ROW,
  FORMAT_TABLE_COLUMN,
  FORMAT_TABLE_TABLE
};

enum  BreakSectionType
{
  BreakSectionContinuous,
  BreakSectionNextPage,
  BreakSectionEvenPage,
  BreakSectionOddPage
};

enum ViewMode
{
  VIEW_PRINT,
  VIEW_NORMAL,
  VIEW_WEB,
  VIEW_PREVIEW
};

enum PreViewMode
{
  PREVIEW_NONE,
  PREVIEW_ZOOMED,
  PREVIEW_ADJUSTED_PAGE,
  PREVIEW_CLIPPED,
  PREVIEW_ZOOMED_SCROLL,
  PREVIEW_ADJUSTED_PAGE_SCROLL,
  PREVIEW_CLIPPED_SCROLL
};


#endif /* FP_TYPES_H */
