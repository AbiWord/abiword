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
#include "ut_units.h"

class ABI_EXPORT fp_PageSize
{
public:

	enum Predefined
	{
		_first_predefined_pagesize_ = 0,
		// If you append a predefined pagesize here, don't forget
		// to update the cpp accordingly.

		// Metric sizes, DIN 476
		DIN_4A = 0,
		DIN_2A, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10,
		DIN_4B, DIN_2B, B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10,
		C0, C1, C2, C3, C4, C6, C7, C8, C9, C10,

		// JIS P 0138-61 -- should call this JB5
		B5_Japan,
		
		Legal, Folio, Letter, Half_Letter, Executive,
		Tabloid_Ledger, Monarch, SuperB,
		Envelope_Commercial, Envelope_Monarch,
		Envelope_DL, Envelope_C5, EuroPostcard,

		Custom,
		// append new pagesizes here
		_last_predefined_pagesize_dont_use_
	};

	fp_PageSize(Predefined preDef);
	fp_PageSize(const char *name);
	fp_PageSize(double w, double h, UT_Dimension u);

	void Set(Predefined preDef, UT_Dimension u = DIM_none);
	void Set(const char *name, UT_Dimension u = DIM_none);
	void Set(double w, double h, UT_Dimension u = DIM_none);
	void Set(UT_Dimension u) { m_unit = u; }
	inline void setScale( double scale) { m_scale = scale; }
	void setPortrait(void);
	void setLandscape(void);
	bool isPortrait(void) const { return m_bisPortrait; }
	double Width(UT_Dimension u) const;
	double Height(UT_Dimension u) const;

	/* These accessor methods should be used with the 
	 * predefined page sizes to set proper initial margins. */
	/* I don't think this is done at present. */
	double MarginLeft(UT_Dimension u) const;
	double MarginRight(UT_Dimension u) const;
	double MarginTop(UT_Dimension u) const;
	double MarginBottom(UT_Dimension u) const;

	double getScale(void) { return m_scale; }
	UT_Dimension getDims(void) { return m_unit; }
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
	UT_Dimension m_unit;
};

#endif	// FP_PAGESIZE_H
