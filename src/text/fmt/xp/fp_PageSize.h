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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef FP_PAGESIZE_H
#define FP_PAGESIZE_H

#include "ut_types.h"
#include "ut_units.h"

#if _MSC_VER
#pragma warning(disable: 4522) // multiple assignment operators specified
#endif

class UT_UTF8String;

class ABI_EXPORT fp_PageSize
{
public:

	enum Predefined
	{
		_first_predefined_pagesize_ = 0,

		// If you append a predefined pagesize here, don't forget
		// to update the cpp accordingly.

		psA0 = 0, psA1, psA2, psA3, psA4, psA5, psA6, psA7, psA8, psA9, psA10,
		psB0, psB1, psB2, psB3, psB4, psB5, psB6, psB7, psB8, psB9, psB10,
		psC0, psC1, psC2, psC3, psC4, psC5, psC6, psC7, psC8, psC9, psC10,
		psLegal, psFolio, psLetter,
		ps1_3A4, ps1_4A4, ps1_8A4, ps1_4A3, ps1_3A5,
		psEnvelope_DL, psEnvelope_C6_C5, psEnvelope_no10, psEnvelope_6x9,
		psCustom,

		// append new pagesizes here
		_last_predefined_pagesize_dont_use_
	};

	UT_UTF8String static getDefaultPageMargin(UT_Dimension dim);

	fp_PageSize(Predefined preDef);
	fp_PageSize(const char *name);
	fp_PageSize(double w, double h, UT_Dimension u);
	fp_PageSize&      operator=(fp_PageSize& rhs);
	fp_PageSize&      operator=(const fp_PageSize& rhs);

	bool match(double x, double y);
	void Set(Predefined preDef, UT_Dimension u = DIM_none);
	void Set(const char *name, UT_Dimension u = DIM_none);
	void Set(double w, double h, UT_Dimension u = DIM_none);
	void Set(UT_Dimension u) { m_unit = u; }
	bool Set(const gchar ** attributes);
	inline void setScale( double scale) { m_scale = scale; }
	void setPortrait(void);
	void setLandscape(void);
	bool isPortrait(void) const { return m_bisPortrait; }
	double Width(UT_Dimension u) const;
	double Height(UT_Dimension u) const;

	double getScale(void) const { return m_scale; }
	UT_Dimension getDims(void) const { return m_unit; }
	inline const char * getPredefinedName (void) const { return m_predefined; }

	static bool	IsPredefinedName(const char* szPageSizeName);
	static Predefined NameToPredefined(const char *name);
	static const char * PredefinedToName(Predefined preDef);
	static int PredefinedToLocalName(Predefined preDef);

private:
	const char * m_predefined;

	double m_iWidth;
	double m_iHeight;

	bool m_bisPortrait;
	double m_scale;
	UT_Dimension m_unit;
};

#endif	// FP_PAGESIZE_H
