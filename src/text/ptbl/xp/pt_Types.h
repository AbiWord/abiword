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

#ifndef PT_TYPES_H
#define PT_TYPES_H

#include "ut_types.h"

//! index to actual document data
typedef UT_uint32 PT_BufIndex;
//! index to Attribute/Property Tables
typedef UT_uint32 PT_AttrPropIndex;

//! absolute document position
typedef UT_uint32 PT_DocPosition;
//! block-relative document position
typedef UT_uint32 PT_BlockOffset;

class PD_Document;
/*!
 PD_DocumentRange identifies a piece of the document, using two
 PT_DocPosition absolute document positions as limits.
*/

class ABI_EXPORT PD_DocumentRange
{
public:
	//! Dummy constructure.
	//! Resulting range is invalid until it is initialized with set
	PD_DocumentRange(void)
		{
			m_pDoc = NULL;
			m_pos1 = 0;
			m_pos2 = 0;
		}

	//! Initializing constructor.
	PD_DocumentRange(PD_Document * pDoc, PT_DocPosition k1, PT_DocPosition k2)
		{
			m_pDoc = pDoc;
			m_pos1 = k1;
			m_pos2 = k2;
		}

	//! Set range limits
	void set(PD_Document * pDoc, PT_DocPosition k1, PT_DocPosition k2)
		{
			m_pDoc = pDoc;
			m_pos1 = k1;
			m_pos2 = k2;
		}

	//! Document this range is in
	PD_Document *		m_pDoc;
	//! Lower limit of range
	PT_DocPosition		m_pos1;
	//! Upper limit of range
	PT_DocPosition		m_pos2;
};

//! PTStruxType tells the sub-type of a FragStrux.
typedef enum _PTStruxType
{
	PTX_Section,         // 0
	PTX_Block,           // 1
	PTX_SectionHdrFtr,   // 2
	PTX_SectionEndnote,  // 3
	PTX_SectionTable,    // 4
	PTX_SectionCell,     // 5
	PTX_SectionFootnote, // 6
	PTX_SectionMarginnote, // 7
	PTX_SectionFrame,      // 8
	PTX_EndCell,           // 9
	PTX_EndTable,          // 10
    PTX_EndFootnote,
    PTX_EndMarginnote,
    PTX_EndEndnote,
    PTX_EndFrame
} PTStruxType;

//! PTObjectType tells the sub-type of an FragObject.

typedef enum _PTObjectType { PTO_Image, PTO_Field, PTO_Bookmark, PTO_Hyperlink } PTObjectType;

//! PieceTable states
typedef enum _PTState { PTS_Create=0, PTS_Loading=1, PTS_Editing=2 } PTState;
//! ChangeFormat types
typedef enum _PTChangeFmt { PTC_AddFmt=0, PTC_RemoveFmt=1, PTC_AddStyle=2 } PTChangeFmt;

//! ID of a listener - this is its location in m_vecListeners
typedef UT_uint32 PL_ListenerId;
//! opaque document data
typedef const void * PL_StruxDocHandle;
//! opaque layout data
typedef const void * PL_StruxFmtHandle;

#define PT_PROPS_ATTRIBUTE_NAME			((const XML_Char *)"props")
#define PT_STYLE_ATTRIBUTE_NAME			((const XML_Char *)"style")
#define PT_LEVEL_ATTRIBUTE_NAME			((const XML_Char *)"level")
#define PT_LISTID_ATTRIBUTE_NAME		((const XML_Char *)"listid")
#define PT_PARENTID_ATTRIBUTE_NAME		((const XML_Char *)"parentid")
#define PT_NAME_ATTRIBUTE_NAME			((const XML_Char *)"name")
#define PT_TYPE_ATTRIBUTE_NAME			((const XML_Char *)"type")
#define PT_BASEDON_ATTRIBUTE_NAME		((const XML_Char *)"basedon")
#define PT_FOLLOWEDBY_ATTRIBUTE_NAME	((const XML_Char *)"followedby")
#define PT_ID_ATTRIBUTE_NAME            ((const XML_Char *)"id")
#define PT_HEADER_ATTRIBUTE_NAME	    ((const XML_Char *)"header")
#define PT_HEADEREVEN_ATTRIBUTE_NAME	((const XML_Char *)"header-even")
#define PT_HEADERFIRST_ATTRIBUTE_NAME	((const XML_Char *)"header-first")
#define PT_HEADERLAST_ATTRIBUTE_NAME	((const XML_Char *)"header-last")
#define PT_FOOTER_ATTRIBUTE_NAME	    ((const XML_Char *)"footer")
#define PT_FOOTEREVEN_ATTRIBUTE_NAME	((const XML_Char *)"footer-even")
#define PT_FOOTERFIRST_ATTRIBUTE_NAME	((const XML_Char *)"footer-first")
#define PT_FOOTERLAST_ATTRIBUTE_NAME	((const XML_Char *)"footer-last")
#define PT_REVISION_ATTRIBUTE_NAME      ((const XML_Char *)"revision")
#define PT_ID_ATTRIBUTE_NAME            ((const XML_Char *)"id")


#endif /* PT_TYPES_H */








