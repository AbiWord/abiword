/* AbiWord
 * Copyright (C) 2001 Tomas Frydrych
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

#include "ut_assert.h"
#include "pf_Frag_Object.h"
#include "ut_string.h"
#include "pt_PieceTable.h"
#include "pt_Types.h"
#include "ut_types.h"
#include "po_Bookmark.h"

po_Bookmark::po_Bookmark(pf_Frag_Object& fO, pt_PieceTable * pt,
                   BookmarkType bookmarkType, const gchar* name)
    : m_fragObject(fO),m_pPieceTable(pt),m_iBookmarkType(bookmarkType)
{
	m_pBlock = NULL;
	m_pName = NULL;
	setName(name);
}


po_Bookmark::~po_Bookmark(void)
{
	FREEP(m_pName);
}

void po_Bookmark::setBlock( fl_BlockLayout *pBlock)
{
	m_pBlock = pBlock;
}

fl_BlockLayout* po_Bookmark::getBlock( void) const
{
	return m_pBlock;
}

po_Bookmark::BookmarkType po_Bookmark::getBookmarkType(void) const
{
	return m_iBookmarkType;
}

const gchar* po_Bookmark::getName(void) const
{
	return (const gchar*) m_pName;
}

void po_Bookmark::setName(const gchar* szValue)
{
	FREEP(m_pName);
	m_pName = g_strdup(szValue);
}

