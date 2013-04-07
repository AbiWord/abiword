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

#ifndef AP_Dialog_PageSetup_H
#define AP_Dialog_PageSetup_H

#include "ut_types.h"
#include "ut_units.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "fp_PageSize.h"

class ABI_EXPORT AP_Dialog_PageSetup : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_PageSetup(XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_PageSetup() = 0;

	virtual void runModal(XAP_Frame *pFrame) = 0;

	typedef enum { a_OK, a_CANCEL } tAnswer;
	typedef enum { PORTRAIT, LANDSCAPE } Orientation;

	// declare JavaBean-like accessors for our properties
#define SET_GATHER(a, u)  inline u get##a(void) const {return m_##a;} \
			  inline void set##a(u p##a) {m_##a = p##a;}
	SET_GATHER(PageSize,		fp_PageSize);
	SET_GATHER(PageUnits,		UT_Dimension);
	SET_GATHER(PageOrientation,	Orientation);
	SET_GATHER(PageScale,		int);
	SET_GATHER(MarginUnits,		UT_Dimension);
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
	XAP_Frame * 		m_pFrame;
	AP_Dialog_PageSetup::tAnswer m_answer;

    bool validatePageSettings(void) const;

 private:
	fp_PageSize             m_PageSize;
	UT_Dimension            m_PageUnits;
	Orientation             m_PageOrientation;
	int                     m_PageScale;
	UT_Dimension            m_MarginUnits;
	float                   m_MarginTop;
	float                   m_MarginBottom;
	float                   m_MarginLeft;
	float                   m_MarginRight;
	float                   m_MarginHeader;
	float                   m_MarginFooter;
};

#endif // AP_Dialog_PageSetup_H
