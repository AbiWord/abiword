/* AbiWord
 * Copyright (C) 2000,2001 Mike Nordell, Dom Lachowicz, and others
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

#ifndef FP_PAGESIZE_H
#define FP_PAGESIZE_H

#include "ut_types.h"

class ABI_EXPORT fp_PageSize
{
public:

	enum Unit
	{
		mm,
		cm,
		inch,
		PaperUnit,		//  100 per inch
		LayoutUnit,		// 1440 per inch
		_last_predefined_unit_dont_use_
	};

	enum Predefined
	{
		// If you append a predefined pagesize here, don't forget
		// to update the cpp accordingly.
		A0, A1, A2, A3, A4, A5, A6,
		B0, B1, B2, B3, B4, B5, B5_Japan, B6,
		Legal, Folio, Letter, Half_Letter, 

		Tabloid_Ledger, Monarch, SuperB,
		Envelope_Commercial, Envelope_Monarch,
		Envelope_DL, Envelope_C5, EuroPostcard,

		Custom,
		// append new pagesizes here
		_last_predefined_pagesize_dont_use_
	};

	fp_PageSize(Predefined preDef);
	fp_PageSize(const char *name);
	fp_PageSize(double w, double h, Unit u);

	void Set(Predefined preDef, Unit u = fp_PageSize::_last_predefined_unit_dont_use_);
	void Set(const char *name, Unit u = fp_PageSize::_last_predefined_unit_dont_use_);
	void Set(double w, double h, Unit u);
	void Set(Unit u) {m_unit = u;}
	inline void setScale( double scale) {m_scale = scale;}
	void setPortrait(void);
	void setLandscape(void);
	bool isPortrait(void) { return m_bisPortrait; }
	double Width(Unit u) const;
	double Height(Unit u) const;

	double MarginLeft(Unit u) const;
	double MarginRight(Unit u) const;
	double MarginTop(Unit u) const;
	double MarginBottom(Unit u) const;

	double getScale(void) {return m_scale;}
	Unit getUnit(void) { return m_unit;}
	inline char * getPredefinedName (void) const { return m_predefined; }

	static bool	IsPredefinedName(const char* szPageSizeName);
	static Predefined NameToPredefined(const char *name);
	static const char * PredefinedToName(Predefined preDef);

private:
	char * m_predefined;

	double m_iWidth;
	double m_iHeight;

	double m_iMarginLeft;
	double m_iMarginRight;
	double m_iMarginTop;
	double m_iMarginBottom;

	bool m_bisPortrait;
	double m_scale;
	Unit m_unit;
};

#endif	// FP_PAGESIZE_H
