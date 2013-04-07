/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (c) 2001,2002 Tomas Frydrych
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


#ifndef PF_FRAG_OBJECT_H
#define PF_FRAG_OBJECT_H

#include "ut_types.h"
#include "pf_Frag.h"
#include "pt_Types.h"
#include "fd_Field.h"
#include "po_Bookmark.h"
/*!
 pf_Frag_Object represents an object (such as
 an image) in the document.
*/

class ABI_EXPORT pf_Frag_Object : public pf_Frag
{
public:
	pf_Frag_Object(pt_PieceTable * pPT,
				   PTObjectType objectType,
				   PT_AttrPropIndex indexAP);
	virtual ~pf_Frag_Object();

	PTObjectType			getObjectType(void) const;
	virtual bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr,
													  PT_DocPosition dpos,
													  PT_BlockOffset blockOffset);

	po_Bookmark *			getBookmark() const;

	virtual bool            usesXID() const {return true;}

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif

protected:
	virtual bool            _isContentEqual(const pf_Frag &f2) const;

	PTObjectType			m_objectType;
	void *					m_pObjectSubclass;
};

#endif /* PF_FRAG_OBJECT_H */
