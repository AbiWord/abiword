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

#ifndef AP_Dialog_PageSetup_H
#define AP_Dialog_PageSetup_H

#include "ut_types.h"
#include "ut_units.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fp_PageSize.h"

class AP_Dialog_PageSetup : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_PageSetup(XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_PageSetup() = 0;

	virtual void runModal(XAP_Frame *pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;
	typedef enum { PORTRAIT, LANDSCAPE } Orientation;

	// declare JavaBean-like accessors for our properties
	// TMN: I'm not sure about the virtuality of these accessors/mutators
#define SET_GATHER(a, u)  virtual inline u get##a(void) const {return m_##a;} \
			  virtual inline void set##a(u p##a) {m_##a = p##a;}
	SET_GATHER(PageSize,		fp_PageSize);
	SET_GATHER(PageUnits,		fp_PageSize::Unit);
	SET_GATHER(PageOrientation,	Orientation);
	SET_GATHER(PageScale,		int);
	SET_GATHER(MarginUnits,		fp_PageSize::Unit);
	SET_GATHER(MarginTop,		float);
	SET_GATHER(MarginBottom,	float);
	SET_GATHER(MarginLeft,		float);
	SET_GATHER(MarginRight,		float);
	SET_GATHER(MarginHeader,	float);
	SET_GATHER(MarginFooter,	float);
#undef SET_GATHER

	virtual inline tAnswer getAnswer (void) const {return m_answer;}

 protected:
	// this should only get used by decendant classes
	inline void setAnswer (tAnswer answer) {m_answer = answer;}

 private:
	AP_Dialog_PageSetup::tAnswer m_answer;

	fp_PageSize             m_PageSize;
	fp_PageSize::Unit       m_PageUnits;
	Orientation             m_PageOrientation;
	int                     m_PageScale;
	fp_PageSize::Unit       m_MarginUnits;
	float                   m_MarginTop;
	float                   m_MarginBottom;
	float                   m_MarginLeft;
	float                   m_MarginRight;
	float                   m_MarginHeader;
	float                   m_MarginFooter;	
};

#endif // AP_Dialog_PageSetup_H
