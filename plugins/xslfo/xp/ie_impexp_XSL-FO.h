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

#ifndef IE_IMPEXP_XSL_FO_H
#define IE_IMPEXP_XSL_FO_H

// we handle a small subset of the XSL-FO spec

#define TT_OTHER		0
#define TT_DOCUMENT		1	// fo:root
#define TT_SECTION		2	// fo:flow
#define TT_BLOCK		3	// fo:block
#define TT_INLINE		4	// fo:inline
#define TT_CHAR			5	// fo:character
#define TT_IMAGE		6	// fo:external-graphic

#define TT_LAYOUT_MASTER_SET	7	// fo:layout-master-set
#define TT_SIMPLE_PAGE_MASTER	8	// fo:simple-page-master
#define TT_REGION_BODY			9	// fo:region-body
#define TT_PAGE_SEQUENCE		10	// fo:page-sequence
#define TT_TABLE				11	// fo:table
#define TT_TABLEBODY			12	// fo:table-body
#define TT_TABLEROW				13	// fo:table-row
#define TT_TABLECOLUMN			14	// fo:table-column
#define TT_TABLECELL			15	// fo:table-cell
#define TT_FOOTNOTE				16	// fo:footnote
#define TT_FOOTNOTEBODY			17	// fo:footnote-body
#define TT_LIST					18	// fo:list
#define TT_LISTITEM				19	// fo:list-item
#define TT_LISTITEMLABEL		20	// fo:list-item-label
#define TT_LISTITEMBODY			21	// fo:list-item-body
#define TT_LISTBLOCK			22	// fo:list-block
#define TT_BASICLINK			23	// fo:basic-link
#define TT_STATIC			24	// fo:static-content

#endif
