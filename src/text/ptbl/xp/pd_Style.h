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


#ifndef PD_STYLE_H
#define PD_STYLE_H

#include "ut_types.h"
#include "pt_Types.h"

class pt_PieceTable;

//////////////////////////////////////////////////////////////////
// PD_Style is the representation for an individual style.

class PD_Style
{
public:
	PD_Style(pt_PieceTable * pPT, PT_AttrPropIndex indexAP);
	~PD_Style();

	inline PT_AttrPropIndex		getIndexAP(void) const	{ return m_indexAP; };
	UT_Bool						setIndexAP(PT_AttrPropIndex indexAP);

	virtual UT_Bool			isUserDefined(void) const { return UT_TRUE; };
	UT_Bool					isUsed(void) const;

protected:
	pt_PieceTable *			m_pPT;
	PT_AttrPropIndex		m_indexAP;

	PD_Style *				m_pBasedOn;
	PD_Style *				m_pFollowedBy;
};


//////////////////////////////////////////////////////////////////
// PD_BuiltinStyle 

class PD_BuiltinStyle : public PD_Style
{
public:
	PD_BuiltinStyle(pt_PieceTable * pPT, PT_AttrPropIndex indexAP);
	~PD_BuiltinStyle();

	virtual UT_Bool			isUserDefined(void) const { return (m_indexAP == m_indexAPOrig); };

protected:
	PT_AttrPropIndex		m_indexAPOrig;	// the builtin one
};

#endif /* PD_STYLE_H */
