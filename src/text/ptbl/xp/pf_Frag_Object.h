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


#ifndef PF_FRAG_OBJECT_H
#define PF_FRAG_OBJECT_H

#include "ut_types.h"
#include "pf_Frag.h"

// pf_Frag_Object represents an in-line object (such as
// an image) in the document.

class pf_Frag_Object : public pf_Frag
{
public:
	pf_Frag_Object();
	virtual ~pf_Frag_Object();
	
	virtual UT_Bool			createSpecialChangeRecord(PX_ChangeRecord ** ppcr) const;

#ifdef PT_TEST
	virtual void			__dump(FILE * fp) const;
#endif

protected:
	
};

#endif /* PF_FRAG_OBJECT_H */
