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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
#ifndef BOOKMARK_H
#define BOOKMARK_H

#include "ut_types.h"
#include "ut_xml.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Text.h"
#include "pt_Types.h"

class fl_BlockLayout;
class pf_Frag_Object;

/*!
 \note This class will eventually have subclasses to implement the different
 types of fields.
*/

class ABI_EXPORT po_Bookmark
{
 public:
    // TBD: convention for naming
    typedef enum
	{
		POBOOKMARK_START,
		POBOOKMARK_END,
		__last_field_dont_use__
	} BookmarkType;

    po_Bookmark(BookmarkType type, const gchar* name);
    virtual							~po_Bookmark(void);
    void							setBlock(fl_BlockLayout * pBlock);
    fl_BlockLayout *				getBlock( void) const;
	BookmarkType					getBookmarkType(void) const;
	const gchar *				getName(void) const;
	void							setName(const gchar * szValue);
    // probably need different types of update
    // which are overridden in the appropriate subclass
    // eg positionChangeUpdate
    //    referenceChangeUpdate

 private:
    fl_BlockLayout * m_pBlock;
    // will need some more helper functions in here eg. to test
    // whether text has changed to avoid unnecessary updates
    BookmarkType m_iBookmarkType;
    gchar * m_pName;
};

#endif




