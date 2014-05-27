/* AbiWord
 * Copyright (C) 2002 AbiSource, Inc.
 * Copyright (C) 2002 Marc Maurer (uwog@uwog.net)
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

/* See bug 1764
 * "This product is not manufactured, approved, or supported by
 * Corel Corporation or Corel Corporation Limited."
 */

#include "ie_imp_WordPerfect.h"
#include "xap_Module.h"

#define IE_MIMETYPE_WP_51			"application/wordperfect5.1"
#define IE_MIMETYPE_WP_6			"application/wordperfect6"

#define WP_TOP_SOFT_SPACE 128
#define WP_TOP_HARD_HYPHEN 132 // (0x84)
#define WP_TOP_DORMANT_HARD_RETURN 135 // (0x87)
#define WP_TOP_HARD_EOL 204
#define WP_TOP_SOFT_EOL 207
#define WP_TOP_EOL_GROUP 0xD0
#define WP_TOP_PAGE_GROUP 0xD1
#define WP_TOP_COLUMN_GROUP 0xD2
#define WP_TOP_PARAGRAPH_GROUP 0xD3
#define WP_TOP_CHARACTER_GROUP 0xD4
#define WP_TOP_CROSSREFERENCE_GROUP 0xD5
#define WP_TOP_HEADER_FOOTER_GROUP 0xD6
#define WP_TOP_FOOTENDNOTE_GROUP 215 // (0xD7)
#define WP_TOP_SET_NUMBER_GROUP 216  // (0xD8)
#define WP_TOP_NUMBERING_METHOD_GROUP 217 // (0xD9)
#define WP_TOP_DISPLAY_NUMBER_REFERENCE_GROUP 218 // (0xDA)
#define WP_TOP_INCREMENT_NUMBER_GROUP 219 // (0xDB)
#define WP_TOP_DECREMENT_NUMBER_GROUP 220 // (0xDC)
#define WP_TOP_STYLE_GROUP 221 // (0xDD)
#define WP_TOP_MERGE_GROUP 222 // (0xDE)
#define WP_TOP_BOX_GROUP 223 // (0xDF)
#define WP_TOP_TAB_GROUP 224 // (0xE0)
#define WP_TOP_PLATFORM_GROUP 225 // (0xE1)
#define WP_TOP_FORMATTER_GROUP 226 // (0xE2)
#define WP_TOP_EXTENDED_CHARACTER 240// (0xF0)
#define WP_TOP_UNDO_GROUP 241 // (0xF1)
#define WP_TOP_ATTRIBUTE_ON 242 // (0xF2)
#define WP_TOP_ATTRIBUTE_OFF 243 // (0xF3)

#define WP_PARAGRAPH_GROUP_JUSTIFICATION 0x05
#define WP_PARAGRAPH_GROUP_OUTLINE_DEFINE 0x0E
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_LEFT 0x00
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL 0x01
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_CENTER 0x02
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_RIGHT 0x03
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_FULL_ALL_LINES 0x04
#define WP_PARAGRAPH_GROUP_JUSTIFICATION_RESERVED 0x05

#define WP_MULTINATIONAL_CHARACTER_SET 1
#define WP_PHONETIC_SYMBOL_CHARACTER_SET 2
#define WP_TYPOGRAPHIC_SYMBOL_CHARACTER_SET 4
#define WP_ICONIC_SYMBOL_CHARACTER_SET 5
#define WP_MATH_SCIENTIFIC_CHARACTER_SET 6
#define WP_MATH_SCIENTIFIC_EXTENDED_CHARACTER_SET 7
#define WP_GREEK_CHARACTER_SET 8
#define WP_HEBREW_CHARACTER_SET 9
#define WP_CYRILLIC_CHARACTER_SET 10
#define WP_JAPANESE_CHARACTER_SET 11
